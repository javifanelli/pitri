#ifndef MOTION_H
#define MOTION_H

#include <stdbool.h>
#include <stdint.h>

// Sensor de movimiento PIR HC-SR501.
// Una tarea lo vigila todo el tiempo y avisa por consola cada vez que
// empieza o termina un movimiento.

void motion_init(void);

// true si alguien llegó desde la última vez que se preguntó.
// Avisa una sola vez por cada llegada (no se repite mientras la persona sigue ahí).
bool motion_alguien_llego(void);

// true mientras el sensor sigue viendo movimiento
bool motion_hay_alguien(void);

// Milisegundos que pasaron desde el último movimiento (0 si hay movimiento ahora)
uint32_t motion_ms_sin_movimiento(void);

#endif // MOTION_H
