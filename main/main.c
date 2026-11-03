#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"

#include "pitri_config.h"
#include "board_config.h"
#include "display.h"
#include "eyes.h"
#include "motion.h"
#include "dht.h"
#include "wifi.h"
#include "botones.h"

static const char *TAG = "PITRI";

// Muestra un texto grande en el centro de la pantalla durante MENSAJE_ARRANQUE_MS
static void mostrar_mensaje(const char *texto)
{
    if (texto[0] == '\0') return;   // mensaje vacío: no mostrar nada
    display_clear();
    display_draw_text_centered(texto, MENSAJE_ARRANQUE_TAMANO, true);
    display_flush();
    vTaskDelay(pdMS_TO_TICKS(MENSAJE_ARRANQUE_MS));
}

// Espera un rato, pero si alguien llega deja de esperar y saluda
static void descansar(int ms)
{
    for (int t = 0; t < ms; t += 50) {
        if (motion_alguien_llego()) {
            eyes_saludar();
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Pitri se corre a un costado y mira los datos del sensor, que aparecen con letra chica
static void mostrar_temperatura(void)
{
    int16_t t, h;
    if (!dht_ultima_lectura(&t, &h)) return;   // todavía no hay ninguna lectura

    char texto[64];
    snprintf(texto, sizeof(texto), "Temp:\n%s%d.%d°C\nHumedad:\n%d%%",
             t < 0 ? "-" : "", abs(t) / 10, abs(t) % 10, (h + 5) / 10);

    eyes_mostrar_datos(texto, MOSTRAR_TEMPERATURA_MS);
}

// Duerme hasta que el sensor vea a alguien
static void dormir(void)
{
    ESP_LOGI(TAG, "Pasaron %d s sin movimiento", TIEMPO_SIN_MOVIMIENTO_PARA_DORMIR_MS / 1000);
    eyes_dormirse();

    while (!motion_alguien_llego()) {
        eyes_dormido();
    }

    eyes_despertar();
    eyes_saludar();
}

void app_main(void)
{
    ESP_LOGI(TAG, "¡Hola! Soy Pitri :)");

    display_init();
    motion_init();
    dht_iniciar(TIEMPO_ENTRE_LECTURAS_MS);
    wifi_iniciar();          // se conecta "de fondo", no hay que esperarlo
    botones_iniciar();       // por ahora solo avisan por consola cuando se aprietan

    // Mensajes de bienvenida
    mostrar_mensaje(MENSAJE_ARRANQUE);
    mostrar_mensaje(MENSAJE_ARRANQUE_2);

    eyes_init();
    eyes_saludar();

    int64_t ultima_vez_temperatura_us = esp_timer_get_time();
    bool ya_festejo_wifi = false;

    while (1) {
        if (motion_ms_sin_movimiento() >= TIEMPO_SIN_MOVIMIENTO_PARA_DORMIR_MS) {
            dormir();
        }

        // Si alguien llegó mientras jugaba, saluda antes de seguir
        if (motion_alguien_llego()) eyes_saludar();

        // La primera vez que se conecta al WiFi, lo festeja
        if (!ya_festejo_wifi && wifi_conectado()) {
            ya_festejo_wifi = true;
            mostrar_mensaje(MENSAJE_WIFI);
            eyes_init();
            eyes_saludar();
        }

        // Cada tanto muestra la temperatura
        if (MOSTRAR_TEMPERATURA_CADA_MS > 0 &&
            esp_timer_get_time() - ultima_vez_temperatura_us >= (int64_t)MOSTRAR_TEMPERATURA_CADA_MS * 1000) {
            mostrar_temperatura();
            ultima_vez_temperatura_us = esp_timer_get_time();
        }

        eyes_jugar();

        // Un descansito entre juego y juego, y a veces un parpadeo
        descansar(DESCANSO_MIN_MS + esp_random() % (DESCANSO_MAX_MS - DESCANSO_MIN_MS));
        if (esp_random() % 2) eyes_parpadear();
    }
}
