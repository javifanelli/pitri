#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_random.h"

#include "display.h"
#include "eyes.h"

static const char *TAG = "PITRI";

void app_main(void)
{
    ESP_LOGI(TAG, "¡Hola! Soy Pitri :)");

    display_init();
    eyes_init();

    while (1) {
        eyes_jugar();

        // Un descansito entre juego y juego, y a veces un parpadeo
        vTaskDelay(pdMS_TO_TICKS(300 + esp_random() % 1200));
        if (esp_random() % 2) eyes_parpadear();
    }
}
