#ifndef MENU_H
#define MENU_H

// Menu en pantalla que se maneja con los dos botones:
//   boton 2 = bajar a la opcion siguiente (al llegar al final vuelve arriba)
//   boton 1 = elegir la opcion marcada

// Muestra "titulo" y la lista de "opciones" y espera a que se elija una.
//   elegida:     que opcion aparece marcada al abrir (0 = la primera)
//   destacada:   opcion que lleva un puntito al costado (por ejemplo, el modo
//                actual), o -1 para ninguna
//   espera_max_ms: si nadie toca nada durante este tiempo, el menu se cierra
// Devuelve el numero de la opcion elegida (0, 1, 2...) o -1 si se cerro solo.
int menu_elegir(const char *titulo, const char *const opciones[], int cantidad,
                int elegida, int destacada, int espera_max_ms);

#endif // MENU_H
