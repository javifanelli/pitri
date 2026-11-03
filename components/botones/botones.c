#include "botones.h"
#include "board_config.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "BOTONES";

/*
 * Cómo se lee un botón:
 *   - Cada botón está entre su pin y GND. Con el pull-up interno, el pin lee 1
 *     cuando el botón está suelto y 0 cuando está apretado (BTN_NIVEL_APRETADO).
 *   - Al apretar, el metal "rebota" y durante unos milisegundos el pin salta
 *     entre 0 y 1. Para no contar varias apretadas, solo creemos un cambio
 *     cuando se mantiene igual durante CAMBIO_FIRME lecturas seguidas.
 */
#define REVISAR_CADA_MS 10
#define CAMBIO_FIRME    3      // 3 lecturas iguales = 30 ms sin rebotar

static const gpio_num_t pines[CANT_BOTONES] = { PIN_BTN_1, PIN_BTN_2 };

static volatile bool se_apreto[CANT_BOTONES];

static void tarea_botones(void *arg)
{
    bool apretado[CANT_BOTONES] = { false };    // lo que creemos que está pasando
    int  lecturas_distintas[CANT_BOTONES] = { 0 };

    while (1) {
        for (int i = 0; i < CANT_BOTONES; i++) {
            bool ahora = gpio_get_level(pines[i]) == BTN_NIVEL_APRETADO;

            if (ahora == apretado[i]) {
                lecturas_distintas[i] = 0;          // nada cambió
            } else if (++lecturas_distintas[i] >= CAMBIO_FIRME) {
                apretado[i] = ahora;                // cambió de verdad (no es rebote)
                lecturas_distintas[i] = 0;
                if (ahora) {
                    ESP_LOGI(TAG, "¡Botón %d apretado!", i + 1);
                    se_apreto[i] = true;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(REVISAR_CADA_MS));
    }
}

void botones_iniciar(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << PIN_BTN_1) | (1ULL << PIN_BTN_2),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,     // suelto = 1, apretado = 0
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));

    xTaskCreate(tarea_botones, "botones", 2560, NULL, 5, NULL);
    ESP_LOGI(TAG, "Botones listos (GPIO %d y %d)", PIN_BTN_1, PIN_BTN_2);
}

bool botones_se_apreto(int numero)
{
    if (numero < 1 || numero > CANT_BOTONES) return false;
    if (!se_apreto[numero - 1]) return false;
    se_apreto[numero - 1] = false;
    return true;
}
