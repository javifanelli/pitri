#include "menu.h"
#include "display.h"
#include "botones.h"
#include "board_config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "MENU";

/*
 *  Asi se ve el menu (128 x 64 pixeles).
 *
 *  Con titulo (por ejemplo "Modo"): entran 3 opciones
 *   ┌──────────────────────────┐
 *   │          Modo         ▲▼ │  ← titulo (letra chica) y flechitas si hay mas opciones
 *   ├──────────────────────────┤
 *   │█ Halloween ██████████ ●  │  ← opcion marcada: barra blanca con letras negras
 *   │  Navidad                 │
 *   │  Animales                │
 *   └──────────────────────────┘
 *
 *  Sin titulo (el menu principal): entran 4 opciones
 *   ┌──────────────────────────┐
 *   │█ Modo ███████████████████│
 *   │  Comer                   │
 *   │  Hora                    │
 *   │  Salir                   │
 *   └──────────────────────────┘
 */
#define REVISAR_CADA_MS 20

// Como se acomodan las filas, segun haya titulo o no
typedef struct {
    bool con_titulo;
    int  filas;        // cuantas opciones entran
    int  alto_fila;
    int  y_primera;    // altura de la primera fila
} disposicion_t;

static disposicion_t disposicion(const char *titulo)
{
    if (titulo != NULL && titulo[0] != '\0')
        return (disposicion_t){ .con_titulo = true,  .filas = 3, .alto_fila = 17, .y_primera = 13 };
    return     (disposicion_t){ .con_titulo = false, .filas = 4, .alto_fila = 16, .y_primera = 1 };
}

static void dibujar(const char *titulo, const char *const opciones[], int cantidad,
                    int elegida, int destacada, int primera_visible)
{
    disposicion_t d = disposicion(titulo);
    bool hay_arriba = primera_visible > 0;
    bool hay_abajo  = primera_visible + d.filas < cantidad;

    display_clear();

    if (d.con_titulo) {
        // Titulo centrado y una rayita abajo; flechitas al lado si hay mas opciones
        display_draw_text((LCD_H_RES - display_text_width(titulo, 1)) / 2, 1, titulo, 1, true);
        display_fill_rect(0, 10, LCD_H_RES, 1, true);
        if (hay_arriba) display_fill_triangle(112, 7, 118, 7, 115, 2, true);
        if (hay_abajo)  display_fill_triangle(120, 2, 126, 2, 123, 7, true);
    } else {
        // Sin titulo: flechitas en el borde derecho, arriba y abajo
        if (hay_arriba) display_fill_triangle(122, 5, 127, 5, 124, 0, true);
        if (hay_abajo)  display_fill_triangle(122, 58, 127, 58, 124, 63, true);
    }

    // Si hay flechitas en el borde, la barra no llega hasta el final
    int ancho_barra = (!d.con_titulo && (hay_arriba || hay_abajo)) ? LCD_H_RES - 8 : LCD_H_RES;

    // Las opciones que entran en la pantalla
    for (int fila = 0; fila < d.filas; fila++) {
        int i = primera_visible + fila;
        if (i >= cantidad) break;

        int y = d.y_primera + fila * d.alto_fila;
        bool marcada = (i == elegida);
        if (marcada) display_fill_rect(0, y - 1, ancho_barra, d.alto_fila - 1, true);   // barra blanca

        display_draw_text(6, y, opciones[i], 2, !marcada);          // letras negras sobre la barra
        if (i == destacada) display_fill_circle(115, y + 6, 2, !marcada);
    }

    display_flush();
}

int menu_elegir(const char *titulo, const char *const opciones[], int cantidad,
                int elegida, int destacada, int espera_max_ms)
{
    if (elegida < 0 || elegida >= cantidad) elegida = 0;
    disposicion_t d = disposicion(titulo);
    const char *nombre = d.con_titulo ? titulo : "Menu";   // para los mensajes de la consola

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
            ESP_LOGI(TAG, "%s → %s", nombre, opciones[elegida]);
            return elegida;
        }

        if (redibujar) {
            // Correr la "ventana" para que la opcion marcada siempre se vea
            if (elegida < primera_visible) primera_visible = elegida;
            if (elegida >= primera_visible + d.filas) primera_visible = elegida - d.filas + 1;
            dibujar(titulo, opciones, cantidad, elegida, destacada, primera_visible);
            redibujar = false;
        }

        vTaskDelay(pdMS_TO_TICKS(REVISAR_CADA_MS));
        quieto_ms += REVISAR_CADA_MS;
    }

    ESP_LOGI(TAG, "%s: nadie eligio nada, se cierra solo", nombre);
    return -1;
}

// ─── Menu con dibujos ────────────────────────────────────────────────────────

static void dibujar_con_dibujos(const char *titulo, int cantidad, void (*dibujar_opcion)(int), int elegida)
{
    display_clear();

    display_draw_text((LCD_H_RES - display_text_width(titulo, 1)) / 2, 1, titulo, 1, true);
    display_fill_rect(0, 10, LCD_H_RES, 1, true);

    // Puntitos: uno por opcion, el de la opcion actual relleno
    for (int i = 0; i < cantidad; i++) {
        int x = LCD_H_RES - 4 - (cantidad - 1 - i) * 6;
        display_fill_circle(x, 5, 2, true);
        if (i != elegida) display_fill_circle(x, 5, 1, false);
    }

    dibujar_opcion(elegida);
    display_flush();
}

int menu_elegir_dibujos(const char *titulo, int cantidad, void (*dibujar_opcion)(int opcion),
                        int elegida, int espera_max_ms)
{
    if (elegida < 0 || elegida >= cantidad) elegida = 0;

    botones_se_apreto(1);
    botones_se_apreto(2);

    int quieto_ms = 0;
    bool redibujar = true;

    while (quieto_ms < espera_max_ms) {
        if (botones_se_apreto(2)) {                  // siguiente
            elegida = (elegida + 1) % cantidad;
            quieto_ms = 0;
            redibujar = true;
        }
        if (botones_se_apreto(1)) {                  // elegir
            ESP_LOGI(TAG, "%s → opcion %d", titulo, elegida);
            return elegida;
        }
        if (redibujar) {
            dibujar_con_dibujos(titulo, cantidad, dibujar_opcion, elegida);
            redibujar = false;
        }
        vTaskDelay(pdMS_TO_TICKS(REVISAR_CADA_MS));
        quieto_ms += REVISAR_CADA_MS;
    }

    ESP_LOGI(TAG, "%s: nadie eligio nada, se cierra solo", titulo);
    return -1;
}
