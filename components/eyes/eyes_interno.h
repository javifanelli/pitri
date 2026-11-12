#ifndef EYES_INTERNO_H
#define EYES_INTERNO_H

// Cosas que comparten eyes.c (ojos, emociones y juegos) y disfraces.c
// (personajes de los modos). NO es parte de lo que usa main.c: eso esta en eyes.h.

#include <stdbool.h>
#include "eyes.h"
#include "board_config.h"

// ─── Forma de los ojos (¡jueguen con estos numeros!) ─────────────────────────
#define OJO_ANCHO        30   // pixeles
#define OJO_ALTO         36
#define OJO_RADIO        9    // que tan redondeadas son las esquinas
#define OJO_SEPARACION   20   // espacio entre los dos ojos
#define OJO_ALTO_CERRADO 3    // grosor de la rayita cuando esta cerrado
#define FLEQUILLO        1    // 1 = Pitri tiene flequillito, 0 = sin flequillo (¡para probar!)

// Hasta donde se pueden mover los ojos sin salirse de la pantalla
#define MAX_X  ((LCD_H_RES - 2 * OJO_ANCHO - OJO_SEPARACION) / 2)   // 24
#define MAX_Y  ((LCD_V_RES - OJO_ALTO) / 2)                          // 14

// Cada cuanto se dibuja un cuadro nuevo de la animacion
#define CUADRO_MS 30

// ─── Como esta la cara en este momento ───────────────────────────────────────
typedef struct {
    float x, y;               // corrimiento desde el centro (adonde mira)
    float ancho, alto;        // tamaño de cada ojo
    float abre_izq, abre_der; // 0 = cerrado, 1 = abierto
    float feliz;              // 0 = normal, 1 = ojitos felices ^ ^
    float enojo;              // 0 = normal, 1 = re enojado (cejas hacia adentro)
    float triste;             // 0 = normal, 1 = re triste (cejas hacia afuera)
    float espiral;            // 0 = ojos normales, 1 = ojos en espiral (mareado)
    float giro;               // cuanto giro la espiral
    float lagrima;            // 0 a 1: por donde va cayendo la lagrima
    float zzz;                // 0 a 1: por donde va subiendo la "Z" cuando duerme
    float corazon;            // 0 = ojos normales, 1 = ojos de corazon (enamorado)
    float corazoncitos;       // 0 a 1: por donde van subiendo los corazoncitos flotantes
    float lentes;             // 0 = sin lentes, 1 = lentes de sol puestos (bajan desde arriba)
    float asombro;            // 0 a 1: boca en forma de "O" (sorpresa)
    float sospecha;           // 0 a 1: boca torcida (sospecha)
    float sueno;              // 0 a 1: boquita de dormido
    float mira;               // hacia donde mira, aparte de donde esta (-1 izq, +1 der)
    float datos;              // 0 a 1: cuanto entraron los datos (texto) desde la derecha
    float fase;               // reloj de los disfraces: cada +1 es una vuelta de su animacion
    float comida_x, comida_y; // donde esta la comida que le estan dando (centro)
    float ondas;              // 0 a 1: ondas de WiFi al costado (mientras se conecta)
} cara_t;

extern const cara_t CARA_NORMAL;
extern cara_t cara;                     // la cara de este momento
extern eyes_disfraz_t disfraz_actual;   // que personaje esta haciendo (o ninguno)
extern eyes_comida_t comida_actual;     // que esta comiendo (o nada)

// ─── Herramientas de eyes.c ──────────────────────────────────────────────────
int  azar(int min, int max);                  // numero al azar entre min y max
void esperar(int ms);                         // espera (se corta si aprietan el boton)
void animar(cara_t destino, int ms);          // pasa suave a la cara "destino"
void mover_a(float x, float y, int ms);       // como animar, pero solo la posicion
void cerrar_y_abrir(bool izq, bool der, int cerrado_ms);
void empezar_accion(void);                    // las funciones publicas la llaman al empezar

// ─── Herramientas de disfraces.c ─────────────────────────────────────────────
bool disfraz_tapa_cara(void);                 // ¿el disfraz reemplaza los ojos y la boca?
void dibujar_disfraz(float cy);               // dibuja lo que agrega el disfraz

// ─── Herramientas de comida.c ────────────────────────────────────────────────
void dibujar_comida_en_cara(void);            // la comida que esta comiendo (si hay)
void comida_reiniciar(void);                  // saca la comida y el texto de la pantalla

#endif // EYES_INTERNO_H
