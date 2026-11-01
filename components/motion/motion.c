#include "motion.h"
#include "board_config.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "MOVIMIENTO";

// El HC-SR501 tarda casi un minuto en "calentarse" después de encenderse
// y en ese tiempo puede avisar movimientos que no existen. Los ignoramos.
#define CALENTAMIENTO_MS 30000

// Cada cuánto se mira el sensor
#define REVISAR_CADA_MS  50

static volatile bool    llego_alguien = false;
static volatile int64_t ultimo_movimiento_us = 0;

static int64_t ahora_us(void) { return esp_timer_get_time(); }

static bool calentando(void)
{
    return ahora_us() < (int64_t)CALENTAMIENTO_MS * 1000;
}

static bool sensor_activo(void)
{
    return gpio_get_level(PIN_PIR) == PIR_NIVEL_DETECTA;
}

// Tarea que vigila el sensor todo el tiempo y avisa por consola
static void tarea_vigilar(void *arg)
{
    bool antes = false;
    bool aviso_listo = false;

    while (1) {
        if (!aviso_listo && !calentando()) {
            ESP_LOGI(TAG, "Sensor listo, ya detecta movimiento");
            aviso_listo = true;
        }

        bool ahora = sensor_activo();

        if (ahora && !antes) {
            if (calentando()) {
                ESP_LOGI(TAG, "Movimiento detectado (ignorado: el sensor se está calentando)");
            } else {
                ESP_LOGI(TAG, "¡Movimiento detectado!");
                llego_alguien = true;
            }
        } else if (!ahora && antes && !calentando()) {
            ESP_LOGI(TAG, "Ya no hay movimiento");
        }

        if (ahora && !calentando()) {
            ultimo_movimiento_us = ahora_us();
        }

        antes = ahora;
        vTaskDelay(pdMS_TO_TICKS(REVISAR_CADA_MS));
    }
}

void motion_init(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << PIN_PIR,
        .mode         = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,   // si el sensor está desconectado, lee 0
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));

    ultimo_movimiento_us = ahora_us();
    xTaskCreate(tarea_vigilar, "movimiento", 3072, NULL, 5, NULL);

    ESP_LOGI(TAG, "Sensor en GPIO %d (calentando %d s)", PIN_PIR, CALENTAMIENTO_MS / 1000);
}

bool motion_alguien_llego(void)
{
    if (!llego_alguien) return false;
    llego_alguien = false;
    return true;
}

bool motion_hay_alguien(void)
{
    return !calentando() && sensor_activo();
}

uint32_t motion_ms_sin_movimiento(void)
{
    if (motion_hay_alguien()) return 0;
    return (uint32_t)((ahora_us() - ultimo_movimiento_us) / 1000);
}
