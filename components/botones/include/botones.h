#ifndef BOTONES_H
#define BOTONES_H

#include <stdbool.h>

// Los 2 botones de Pitri (pines en board_config.h).
// Una tarea los vigila todo el tiempo y avisa por consola cuando se aprietan.

#define CANT_BOTONES 2

void botones_iniciar(void);

// true si el boton "numero" (1 o 2) se apreto desde la ultima vez que se
// pregunto. Avisa una sola vez por cada apretada.
bool botones_se_apreto(int numero);

// true si hay una apretada del boton "numero" esperando, pero SIN consumirla
// (la proxima llamada a botones_se_apreto() la va a seguir viendo)
bool botones_pendiente(int numero);

#endif // BOTONES_H
