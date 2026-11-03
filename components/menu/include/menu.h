#ifndef MENU_H
#define MENU_H

// Menu en pantalla que se maneja con los dos botones:
//   boton 2 = bajar a la opcion siguiente (al llegar al final vuelve arriba)
//   boton 1 = elegir la opcion marcada

// Muestra "titulo" y la lista de "opciones" y espera a que se elija una.
//   titulo: "" = sin titulo (asi entran 4 opciones en vez de 3)
//   elegida:     que opcion aparece marcada al abrir (0 = la primera)
//   destacada:   opcion que lleva un puntito al costado (por ejemplo, el modo
//                actual), o -1 para ninguna
//   espera_max_ms: si nadie toca nada durante este tiempo, el menu se cierra
// Devuelve el numero de la opcion elegida (0, 1, 2...) o -1 si se cerro solo.
int menu_elegir(const char *titulo, const char *const opciones[], int cantidad,
                int elegida, int destacada, int espera_max_ms);

// Igual, pero mostrando las opciones de a una, cada una con un dibujo.
// "dibujar_opcion" dibuja la opcion numero "opcion" en la zona de abajo del
// titulo (y = 12 a 63). Arriba a la derecha aparecen puntitos que muestran en
// que opcion estas. Devuelve la opcion elegida o -1 si se cerro solo.
int menu_elegir_dibujos(const char *titulo, int cantidad, void (*dibujar_opcion)(int opcion),
                        int elegida, int espera_max_ms);

#endif // MENU_H
