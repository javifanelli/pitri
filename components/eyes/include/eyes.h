#ifndef EYES_H
#define EYES_H

#include <stdbool.h>

// Dibuja los ojos de Pitri en el centro de la pantalla (y le saca el disfraz)
void eyes_init(void);

// Elige un juego al azar y lo hace (la funcion vuelve cuando termina)
void eyes_jugar(void);

// Un parpadeo rapido
void eyes_parpadear(void);

// Reaccion cuando alguien llega: se sorprende y se pone contento
void eyes_saludar(void);

// Cierra los ojos de a poco hasta quedar dormido
void eyes_dormirse(void);

// Un "respiro" dormido (~1,2 s): llamarla una y otra vez mientras duerme
void eyes_dormido(void);

// Abre los ojos de golpe y parpadea
void eyes_despertar(void);

// La carita se corre a la izquierda y mira hacia la derecha, donde aparece
// "texto" (acepta '\n'). Se queda "ms" milisegundos y despues vuelve a la normalidad.
//   escala 1 = letra chica: hasta 4 renglones de ~8 letras
//   escala 2 = letra grande: 1 o 2 renglones de 5 letras (por ejemplo "21:15")
void eyes_mostrar_datos(const char *texto, int escala, int ms);

// Un "paso" (~1,2 s) de la carita de costado mirando unas ondas de WiFi que se
// van encendiendo. Llamarla una y otra vez mientras se conecta.
void eyes_onditas(void);

// ─── Disfraces (personajes de los modos) ─────────────────────────────────────
typedef enum {
    DISFRAZ_NINGUNO,
    // Halloween
    DISFRAZ_VAMPIRO,
    DISFRAZ_FANTASMA,
    DISFRAZ_CALABAZA,
    // Navidad
    DISFRAZ_PAPA_NOEL,
    DISFRAZ_ARBOLITO,
    DISFRAZ_GALLETA,
    // Animales
    DISFRAZ_GALLINA,
    DISFRAZ_CERDO,
    DISFRAZ_TORTUGA,
} eyes_disfraz_t;

// Se pone el disfraz (si no lo tenia puesto) y hace la actuacion de ese
// personaje. El disfraz queda puesto cuando termina.
void eyes_actuar(eyes_disfraz_t personaje);

// Se saca el disfraz y vuelve a ser Pitri
void eyes_sacar_disfraz(void);

// ─── Comida ──────────────────────────────────────────────────────────────────
typedef enum {
    COMIDA_NINGUNA,
    COMIDA_TORTA,
    COMIDA_PAPAS,
    COMIDA_HAMBURGUESA,
} eyes_comida_t;

// Dibuja una comida con su centro en (cx, cy). "tamano" agranda o achica el
// dibujo (1 = unos 30 pixeles de alto). "mordidas" (0 a 2) le dibuja marcas de
// mordiscos. Solo dibuja: no llama a display_flush().
void eyes_dibujar_comida(eyes_comida_t comida, float cx, float cy, float tamano, int mordidas);

// Animacion de Pitri comiendo: la comida entra, Pitri la muerde 3 veces,
// mastica y dice "¡Rico!"
void eyes_comer(eyes_comida_t comida);

// Pitri esta lleno: mueve la cabeza diciendo que no
void eyes_decir_que_no(void);

// ─── Cortar lo que esta haciendo ─────────────────────────────────────────────
// "funcion" se consulta en cada cuadro de animacion. Si devuelve true, el juego
// o actuacion que se este haciendo termina enseguida (por ejemplo, para abrir
// el menu apenas se aprieta el boton). NULL = no cortar nunca.
void eyes_set_interrupcion(bool (*funcion)(void));

#endif // EYES_H
