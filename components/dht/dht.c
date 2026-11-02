#include "dht.h"
#include "board_config.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

#if SENSOR_TH_MODELO == DHT_MODELO_DHT11
#define PULSO_DESPERTAR_US 20000   // el DHT11 necesita al menos 18 ms
#elif SENSOR_TH_MODELO == DHT_MODELO_DHT22
#define PULSO_DESPERTAR_US 1200    // el DHT22 necesita al menos 1 ms
#else
#error "SENSOR_TH_MODELO tiene que ser DHT_MODELO_DHT11 o DHT_MODELO_DHT22 (ver board_config.h)"
#endif

// Si una lectura falla, se reintenta hasta esta cantidad de veces (esperando entre medio)
#define REINTENTOS          2
#define ESPERA_REINTENTO_MS 2000

/*
 * Cómo hablan el DHT11 y el DHT22 (un solo cable, ver docs/datasheets/):
 *
 *   1. El ESP32 tira la línea a 0 ("¡despertate!"): 18 ms el DHT11, 1 ms el DHT22
 *   2. La suelta y el sensor contesta: 80 µs en 0 y 80 µs en 1
 *   3. Manda 40 bits. Cada bit empieza con 50 µs en 0 y después:
 *        - 1 durante ~27 µs  → es un 0
 *        - 1 durante ~70 µs  → es un 1
 *   4. Los 40 bits son 5 bytes: humedad (2), temperatura (2), verificación (1)
 *
 *   Lo único que cambia entre los dos es cómo vienen los números:
 *     DHT11: [hum entera] [hum decimal] [temp entera] [temp decimal]
 *     DHT22: [humedad en décimas, 16 bits] [temperatura en décimas, 16 bits]
 */

static portMUX_TYPE candado = portMUX_INITIALIZER_UNLOCKED;

static volatile bool    hay_lectura = false;
static volatile int16_t ultima_temp = 0;
static volatile int16_t ultima_hum  = 0;

// Espera a que la línea tenga "nivel". Devuelve cuántos µs tardó, o -1 si se pasó del tiempo.
static int esperar_nivel(int nivel, int max_us)
{
    int64_t t0 = esp_timer_get_time();
    while (gpio_get_level(PIN_DHT) != nivel) {
        if (esp_timer_get_time() - t0 > max_us) return -1;
    }
    return (int)(esp_timer_get_time() - t0);
}

// Convierte los 5 bytes en temperatura y humedad (en décimas)
static void convertir(const uint8_t d[5], int16_t *temp, int16_t *hum)
{
#if SENSOR_TH_MODELO == DHT_MODELO_DHT11
    *hum  = d[0] * 10 + d[1] % 10;
    *temp = d[2] * 10 + (d[3] & 0x7F) % 10;
    if (d[3] & 0x80) *temp = -*temp;   // los DHT11 nuevos marcan bajo cero con este bit
#else
    *hum  = (d[0] << 8) | d[1];
    *temp = ((d[2] & 0x7F) << 8) | d[3];
    if (d[2] & 0x80) *temp = -*temp;   // el bit de arriba indica temperatura bajo cero
#endif
}

esp_err_t dht_leer(int16_t *temperatura_decimas, int16_t *humedad_decimas)
{
    uint8_t d[5] = {0};
    bool ok = true;

    // 1. "¡Despertate!"
    gpio_set_level(PIN_DHT, 0);
    esp_rom_delay_us(PULSO_DESPERTAR_US);

    // Lo que sigue es muy rápido: que nada nos interrumpa mientras escuchamos
    portENTER_CRITICAL(&candado);
    gpio_set_level(PIN_DHT, 1);

    // 2. Respuesta del sensor. Primero esperamos que la línea suba (la pull-up
    //    tarda unos µs). Si no la vemos subir, puede ser que el sensor ya la haya
    //    tirado a 0 para contestar: seguimos igual.
    //    Después: 0 de ~80 µs, 1 de ~80 µs y arranca el primer bit.
    esperar_nivel(1, 40);
    if (esperar_nivel(0, 100) < 0 || esperar_nivel(1, 120) < 0 || esperar_nivel(0, 120) < 0) {
        ok = false;
    }

    // 3. Los 40 bits: medimos la parte en 0 y la parte en 1.
    //    Si la parte en 1 es más larga que la parte en 0, es un 1.
    for (int i = 0; i < 40 && ok; i++) {
        int bajo = esperar_nivel(1, 100);
        int alto = (bajo < 0) ? -1 : esperar_nivel(0, 120);
        if (alto < 0) { ok = false; break; }
        if (alto > bajo) d[i / 8] |= 1 << (7 - i % 8);
    }
    portEXIT_CRITICAL(&candado);

    if (!ok) return ESP_ERR_TIMEOUT;

    // 4. Verificación: el quinto byte es la suma de los otros cuatro
    if (((d[0] + d[1] + d[2] + d[3]) & 0xFF) != d[4]) return ESP_ERR_INVALID_CRC;

    int16_t temp, hum;
    convertir(d, &temp, &hum);
    if (temperatura_decimas) *temperatura_decimas = temp;
    if (humedad_decimas)     *humedad_decimas = hum;
    return ESP_OK;
}

static void tarea_lecturas(void *arg)
{
    uint32_t intervalo_ms = (uint32_t)(uintptr_t)arg;

    vTaskDelay(pdMS_TO_TICKS(DHT_INTERVALO_MIN_MS));   // el sensor necesita un ratito al prenderse

    while (1) {
        int16_t t, h;
        esp_err_t err = dht_leer(&t, &h);
        for (int r = 0; r < REINTENTOS && err != ESP_OK; r++) {
            vTaskDelay(pdMS_TO_TICKS(ESPERA_REINTENTO_MS));
            err = dht_leer(&t, &h);
        }
        if (err == ESP_OK) {
            ultima_temp = t;
            ultima_hum  = h;
            hay_lectura = true;
        }
        vTaskDelay(pdMS_TO_TICKS(intervalo_ms));
    }
}

void dht_iniciar(uint32_t intervalo_ms)
{
    if (intervalo_ms < DHT_INTERVALO_MIN_MS) intervalo_ms = DHT_INTERVALO_MIN_MS;

    // Pin "open drain": el ESP32 solo tira a 0 o suelta; la resistencia pull-up lo lleva a 1
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << PIN_DHT,
        .mode         = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));
    gpio_set_level(PIN_DHT, 1);

    // Mientras lee, bloquea las interrupciones ~5 ms en su núcleo. Lo ponemos
    // en el último núcleo (el 1 en el ESP32) para no molestar al display, que
    // usa interrupciones de I2C en el núcleo 0.
    xTaskCreatePinnedToCore(tarea_lecturas, "dht", 3072, (void *)(uintptr_t)intervalo_ms, 4, NULL,
                            portNUM_PROCESSORS - 1);
}

bool dht_ultima_lectura(int16_t *temperatura_decimas, int16_t *humedad_decimas)
{
    if (!hay_lectura) return false;
    if (temperatura_decimas) *temperatura_decimas = ultima_temp;
    if (humedad_decimas)     *humedad_decimas = ultima_hum;
    return true;
}
