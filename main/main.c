#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_random.h"

#include "pitri_config.h"
#include "display.h"
#include "eyes.h"
#include "motion.h"

static const char *TAG = "PITRI";

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

    // Mensaje de bienvenida
    display_clear();
    display_draw_text_centered(MENSAJE_ARRANQUE, MENSAJE_ARRANQUE_TAMANO, true);
    display_flush();
    vTaskDelay(pdMS_TO_TICKS(MENSAJE_ARRANQUE_MS));

    eyes_init();
    eyes_saludar();

    while (1) {
        if (motion_ms_sin_movimiento() >= TIEMPO_SIN_MOVIMIENTO_PARA_DORMIR_MS) {
            dormir();
        }

        // Si alguien llegó mientras jugaba, saluda antes de seguir
        if (motion_alguien_llego()) eyes_saludar();

        eyes_jugar();

        // Un descansito entre juego y juego, y a veces un parpadeo
        descansar(DESCANSO_MIN_MS + esp_random() % (DESCANSO_MAX_MS - DESCANSO_MIN_MS));
        if (esp_random() % 2) eyes_parpadear();
    }
}
