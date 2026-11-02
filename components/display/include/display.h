#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>

// Pantalla OLED 128x64 monocromática.
// Se dibuja en un buffer en memoria y después display_flush() lo manda al OLED.

void display_init(void);
void display_clear(void);
void display_flush(void);

void display_set_pixel(int x, int y, bool on);
void display_fill_rect(int x, int y, int w, int h, bool on);
// Rectángulo relleno con esquinas redondeadas de radio r
void display_fill_round_rect(int x, int y, int w, int h, int r, bool on);
// Círculo relleno con centro (cx, cy) y radio r
void display_fill_circle(int cx, int cy, int r, bool on);
// Triángulo relleno con esquinas en (x0,y0), (x1,y1), (x2,y2)
void display_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2, bool on);

// ─── Texto ────────────────────────────────────────────────────────────────────
// Letras de 5x7 píxeles. "escala" las agranda: 1 = chiquitas, 2 = el doble...
// Acepta '\n' para cambiar de renglón y los signos ¡ ¿ °.

// Escribe el texto con la esquina de arriba a la izquierda en (x, y)
void display_draw_text(int x, int y, const char *texto, int escala, bool on);
// Escribe el texto centrado en la pantalla (cada renglón centrado)
void display_draw_text_centered(const char *texto, int escala, bool on);
// Ancho en píxeles del renglón más largo
int  display_text_width(const char *texto, int escala);

#endif // DISPLAY_H
