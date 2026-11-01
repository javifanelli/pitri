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

#endif // DISPLAY_H
