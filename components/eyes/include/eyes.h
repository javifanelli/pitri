#ifndef EYES_H
#define EYES_H

// Dibuja los ojos de Pitri en el centro de la pantalla
void eyes_init(void);

// Elige un juego al azar y lo hace (la función vuelve cuando termina)
void eyes_jugar(void);

// Un parpadeo rápido
void eyes_parpadear(void);

// Reacción cuando alguien llega: se sorprende y se pone contento
void eyes_saludar(void);

// Cierra los ojos de a poco hasta quedar dormido
void eyes_dormirse(void);

// Un "respiro" dormido (~1,2 s): llamarla una y otra vez mientras duerme
void eyes_dormido(void);

// Abre los ojos de golpe y parpadea
void eyes_despertar(void);

#endif // EYES_H
