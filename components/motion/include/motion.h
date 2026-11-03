#ifndef MOTION_H
#define MOTION_H

#include <stdbool.h>
#include <stdint.h>

// Sensor de movimiento PIR HC-SR501.
// Una tarea lo vigila todo el tiempo y avisa por consola cada vez que
// empieza o termina un movimiento.

void motion_init(void);

// true si alguien llego desde la ultima vez que se pregunto.
// Avisa una sola vez por cada llegada (no se repite mientras la persona sigue ahi).
bool motion_alguien_llego(void);

// true mientras el sensor sigue viendo movimiento
bool motion_hay_alguien(void);

// Milisegundos que pasaron desde el ultimo movimiento (0 si hay movimiento ahora)
uint32_t motion_ms_sin_movimiento(void);

#endif // MOTION_H
