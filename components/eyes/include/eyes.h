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
// "texto" con letra chica (acepta '\n', hasta 4 renglones de ~8 letras).
// Se queda "ms" milisegundos y despues vuelve a la normalidad.
void eyes_mostrar_datos(const char *texto, int ms);

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

// ─── Cortar lo que esta haciendo ─────────────────────────────────────────────
// "funcion" se consulta en cada cuadro de animacion. Si devuelve true, el juego
// o actuacion que se este haciendo termina enseguida (por ejemplo, para abrir
// el menu apenas se aprieta el boton). NULL = no cortar nunca.
void eyes_set_interrupcion(bool (*funcion)(void));

#endif // EYES_H
