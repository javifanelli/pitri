#ifndef PITRI_TELEGRAM_H
#define PITRI_TELEGRAM_H

#include <stdbool.h>
#include <stddef.h>

// Mensajes de Telegram para que Sofi los lea en Pitri.
//
// Pitri tiene un bot de Telegram. Cuando hay WiFi, cada tanto le pregunta a
// Telegram si llego algun mensaje nuevo. Solo acepta los mensajes de las
// personas que estan en telegram_secretos.h (que NO se sube al repo); los de
// cualquier otra persona se ignoran. Si ese archivo no existe, Pitri funciona
// igual, sin Telegram.

// Tamaño maximo de un mensaje (los mas largos se cortan)
#define TELEGRAM_MAX_LETRAS 400

// Cuantos mensajes sin leer se guardan (si llega otro, no entra)
#define TELEGRAM_MAX_SIN_LEER 3

// Arranca la tarea que busca mensajes "de fondo" (espera a que haya WiFi)
void telegram_iniciar(void);

// true si hay datos del bot (telegram_secretos.h)
bool telegram_configurado(void);

// Cuantos mensajes llegaron y Sofi todavia no leyo
int telegram_sin_leer(void);

// Cuantos mensajes llegaron en total desde que se prendio (sirve para saber si
// llego uno nuevo: si el numero cambio, llego otro)
unsigned telegram_recibidos(void);

// Copia el mensaje sin leer mas viejo en "texto" (ya preparado para la pantalla:
// sin tildes ni emojis), SIN borrarlo. Devuelve false si no hay ninguno.
bool telegram_ver(char *texto, size_t tamano);

// Borra el mensaje sin leer mas viejo (el que muestra telegram_ver), cuando Sofi
// ya lo leyo entero
void telegram_borrar(void);

#endif // PITRI_TELEGRAM_H
