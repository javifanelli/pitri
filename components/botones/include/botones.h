#ifndef BOTONES_H
#define BOTONES_H

#include <stdbool.h>

// Los 2 botones de Pitri (pines en board_config.h).
// Una tarea los vigila todo el tiempo y avisa por consola cuando se aprietan.

#define CANT_BOTONES 2

void botones_iniciar(void);

// true si el botón "numero" (1 o 2) se apretó desde la última vez que se
// preguntó. Avisa una sola vez por cada apretada.
bool botones_se_apreto(int numero);

#endif // BOTONES_H
