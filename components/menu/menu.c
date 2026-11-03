#include "menu.h"
#include "display.h"
#include "botones.h"
#include "board_config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "MENU";

/*
 *  Asi se ve el menu (128 x 64 pixeles):
 *
 *   ┌──────────────────────────┐
 *   │          Menu         ▲▼ │  ← titulo (letra chica) y flechitas si hay mas opciones
 *   ├──────────────────────────┤
 *   │█ Halloween ██████████ ●  │  ← opcion marcada: barra blanca con letras negras
 *   │  Navidad                 │
 *   │  Animales                │
 *   └──────────────────────────┘
 */
#define FILAS_VISIBLES  3     // entran 3 opciones con letra grande
#define ALTO_FILA       17
#define Y_PRIMERA_FILA  13
#define REVISAR_CADA_MS 20

static void dibujar(const char *titulo, const char *const opciones[], int cantidad,
                    int elegida, int destacada, int primera_visible)
{
    display_clear();

    // Titulo centrado y una rayita abajo
    display_draw_text((LCD_H_RES - display_text_width(titulo, 1)) / 2, 1, titulo, 1, true);
    display_fill_rect(0, 10, LCD_H_RES, 1, true);

    // Flechitas: hay mas opciones arriba (▲) o abajo (▼)
    if (primera_visible > 0)
        display_fill_triangle(112, 7, 118, 7, 115, 2, true);
    if (primera_visible + FILAS_VISIBLES < cantidad)
        display_fill_triangle(120, 2, 126, 2, 123, 7, true);

    // Las opciones que entran en la pantalla
    for (int fila = 0; fila < FILAS_VISIBLES; fila++) {
        int i = primera_visible + fila;
        if (i >= cantidad) break;

        int y = Y_PRIMERA_FILA + fila * ALTO_FILA;
        bool marcada = (i == elegida);
        if (marcada) display_fill_rect(0, y - 1, LCD_H_RES, ALTO_FILA - 1, true);   // barra blanca

        display_draw_text(6, y, opciones[i], 2, !marcada);          // letras negras sobre la barra
        if (i == destacada) display_fill_circle(121, y + 6, 2, !marcada);
    }

    display_flush();
}

int menu_elegir(const char *titulo, const char *const opciones[], int cantidad,
                int elegida, int destacada, int espera_max_ms)
{
    if (elegida < 0 || elegida >= cantidad) elegida = 0;

    // Apretadas viejas no cuentan
    botones_se_apreto(1);
    botones_se_apreto(2);

    int primera_visible = 0;
    int quieto_ms = 0;
    bool redibujar = true;

    while (quieto_ms < espera_max_ms) {
        if (botones_se_apreto(2)) {                  // bajar
            elegida = (elegida + 1) % cantidad;
            quieto_ms = 0;
            redibujar = true;
        }
        if (botones_se_apreto(1)) {                  // elegir
            ESP_LOGI(TAG, "%s → %s", titulo, opciones[elegida]);
            return elegida;
        }

        if (redibujar) {
            // Correr la "ventana" para que la opcion marcada siempre se vea
            if (elegida < primera_visible) primera_visible = elegida;
            if (elegida >= primera_visible + FILAS_VISIBLES) primera_visible = elegida - FILAS_VISIBLES + 1;
            dibujar(titulo, opciones, cantidad, elegida, destacada, primera_visible);
            redibujar = false;
        }

        vTaskDelay(pdMS_TO_TICKS(REVISAR_CADA_MS));
        quieto_ms += REVISAR_CADA_MS;
    }

    ESP_LOGI(TAG, "%s: nadie eligio nada, se cierra solo", titulo);
    return -1;
}
