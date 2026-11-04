// ─────────────────────────────────────────────────────────────────────────────
//  Comida: los dibujos de la comida y la animacion de Pitri comiendo
// ─────────────────────────────────────────────────────────────────────────────
//
//  Cada comida se dibuja con las formas de siempre (rectangulos, circulos,
//  triangulos). Todas las medidas se multiplican por "s" (el tamano), asi el
//  mismo dibujo sirve grande (en el menu) y chiquito (cuando Pitri come).
//
#include "eyes_interno.h"
#include "display.h"

#include <math.h>
#include "esp_log.h"

static const char *TAG = "COMIDA";

#define PI ((float)M_PI)

eyes_comida_t comida_actual = COMIDA_NINGUNA;

static int mordidas_actuales = 0;          // cuantos mordiscos lleva la comida
static const char *globo_texto = NULL;     // lo que dice mientras come

// ─── Herramientas de dibujo (aceptan numeros con coma) ───────────────────────

static void rect(float x, float y, float w, float h, bool on)
{
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    display_fill_rect((int)x, (int)y, (int)w, (int)h, on);
}

static void rect_redondo(float x, float y, float w, float h, float r, bool on)
{
    display_fill_round_rect((int)x, (int)y, (int)w, (int)h, (int)r, on);
}

static void circulo(float cx, float cy, float r, bool on)
{
    display_fill_circle((int)cx, (int)cy, (int)(r + 0.5f), on);
}

static void triangulo(float x0, float y0, float x1, float y1, float x2, float y2, bool on)
{
    display_fill_triangle((int)x0, (int)y0, (int)x1, (int)y1, (int)x2, (int)y2, on);
}

static void arco(float cx, float cy, float r, float desde, float hasta, bool on)
{
    for (float a = desde; a <= hasta; a += 0.06f)
        rect(cx + r * cosf(a), cy + r * sinf(a), 1, 1, on);
}

// ─── Los dibujos ─────────────────────────────────────────────────────────────
// (cx, cy) es el centro del dibujo. Las cuentas estan hechas para s = 1.

// 🎂 Torta: plato, dos pisos, crema que chorrea, granitos y una velita
static void dibujar_torta(float cx, float cy, float s)
{
    cy += 6 * s;                                            // para que quede centrada

    rect(cx - 17 * s, cy + 10 * s, 34 * s, 2 * s, true);    // plato
    rect_redondo(cx - 13 * s, cy - 4 * s, 26 * s, 14 * s, 2 * s, true);   // bizcochuelo
    rect(cx - 13 * s, cy + 3 * s, 26 * s, 1, false);        // relleno (una rayita)
    rect_redondo(cx - 14 * s, cy - 9 * s, 28 * s, 7 * s, 3 * s, true);    // crema de arriba

    // La crema chorrea: una linea negra ondulada separa la crema del bizcochuelo
    for (float dx = -13 * s; dx <= 13 * s; dx += 1)
        rect(cx + dx, cy - 3 * s + 1.5f * s * sinf(dx / s * 0.6f), 1, 1, false);

    // Granitos de colores
    rect(cx - 9 * s, cy - 7 * s, s, s, false);
    rect(cx - 3 * s, cy - 6 * s, s, s, false);
    rect(cx + 4 * s, cy - 7 * s, s, s, false);
    rect(cx + 10 * s, cy - 6 * s, s, s, false);

    // Velita con su llama
    rect(cx - 1 * s, cy - 16 * s, 3 * s, 7 * s, true);
    circulo(cx + 0.5f * s, cy - 19 * s, 2 * s, true);
    triangulo(cx - 1.5f * s, cy - 19 * s, cx + 2.5f * s, cy - 19 * s, cx + 0.5f * s, cy - 24 * s, true);
}

// 🍟 Papas fritas: cajita con una sonrisa y palitos que asoman
static void dibujar_papas(float cx, float cy, float s)
{
    cy += 2.5f * s;

    // Las papas (cada una de distinto alto), con un espacio negro entre ellas
    static const int8_t alto_papa[7] = { 13, 16, 12, 17, 14, 15, 12 };
    for (int k = 0; k < 7; k++)
        rect(cx + (-9 + 3 * k) * s, cy - alto_papa[k] * s, 2 * s, (alto_papa[k] - 1) * s, true);

    // La cajita: mas ancha arriba que abajo
    rect(cx - 9 * s, cy - 2 * s, 18 * s, 14 * s, true);
    triangulo(cx - 12 * s, cy - 2 * s, cx - 9 * s, cy - 2 * s, cx - 9 * s, cy + 12 * s, true);
    triangulo(cx + 12 * s, cy - 2 * s, cx + 9 * s, cy - 2 * s, cx + 9 * s, cy + 12 * s, true);
    arco(cx, cy + 1 * s, 5 * s, 0.2f * PI, 0.8f * PI, false);   // el dibujito de la caja
}

// 🍔 Hamburguesa: pan con semillas, lechuga, carne y pan de abajo
static void dibujar_hamburguesa(float cx, float cy, float s)
{
    // Pan de arriba: medio circulo
    circulo(cx, cy - 1 * s, 13 * s, true);
    rect(cx - 14 * s, cy - 1 * s + 1, 28 * s, 15 * s, false);

    // Semillas de sesamo
    rect(cx - 6 * s, cy - 9 * s, s, 2 * s, false);
    rect(cx,         cy - 11 * s, s, 2 * s, false);
    rect(cx + 6 * s, cy - 8 * s, s, 2 * s, false);
    rect(cx - 2 * s, cy - 5 * s, s, 2 * s, false);
    rect(cx + 4 * s, cy - 4 * s, s, 2 * s, false);

    // Lechuga: una linea ondulada
    for (float dx = -14 * s; dx <= 14 * s; dx += 1)
        rect(cx + dx, cy + 1 * s + 1.0f * s * sinf(dx / s), 1, 1.5f * s, true);

    rect_redondo(cx - 13 * s, cy + 4 * s, 26 * s, 4 * s, 2 * s, true);   // carne
    rect_redondo(cx - 13 * s, cy + 9 * s, 26 * s, 5 * s, 2 * s, true);   // pan de abajo
}

// ─── Las bebidas ─────────────────────────────────────────────────────────────
// Los recipientes se dibujan con borde blanco y adentro negro; el liquido es la
// parte blanca de adentro, y su altura depende de cuantos sorbos tomo (0 a 3).

static float cuanto_queda(int sorbos)
{
    if (sorbos < 0) sorbos = 0;
    if (sorbos > 3) sorbos = 3;
    return (3 - sorbos) / 3.0f;
}

// 🍼 Botella: tapita, pico, cuerpo y el liquido adentro
static void dibujar_botella(float cx, float cy, float s, int sorbos)
{
    cy += 1 * s;

    rect(cx - 4 * s, cy - 19 * s, 8 * s, 3 * s, true);                     // tapita
    rect(cx - 3 * s, cy - 15 * s, 6 * s, 8 * s, true);                     // pico
    rect(cx - 1.5f * s, cy - 15 * s, 3 * s, 7 * s, false);
    rect_redondo(cx - 8 * s, cy - 8 * s, 16 * s, 24 * s, 4 * s, true);     // cuerpo
    rect_redondo(cx - 6 * s, cy - 6 * s, 12 * s, 20 * s, 3 * s, false);

    float alto = 18 * s * cuanto_queda(sorbos);                            // liquido
    if (alto >= 1) rect(cx - 5 * s, cy + 13 * s - alto, 10 * s, alto, true);
}

// ☕ Taza: con asa, platito, el liquido adentro y vapor si esta llena
static void dibujar_taza(float cx, float cy, float s, int sorbos)
{
    cy += 2 * s;

    circulo(cx + 10 * s, cy + 1 * s, 6 * s, true);                         // asa
    circulo(cx + 10 * s, cy + 1 * s, 3.5f * s, false);
    rect_redondo(cx - 12 * s, cy - 8 * s, 20 * s, 19 * s, 3 * s, true);    // taza
    rect(cx - 10 * s, cy - 8 * s, 16 * s, 17 * s, false);                  // adentro (abierta arriba)
    rect(cx - 16 * s, cy + 12 * s, 30 * s, 2 * s, true);                   // platito

    float alto = 14 * s * cuanto_queda(sorbos);                            // liquido
    if (alto >= 1) rect(cx - 9 * s, cy + 8 * s - alto, 14 * s, alto, true);

    // Vapor: dos lineas onduladas arriba (solo si esta llena y se ve grande)
    if (sorbos == 0 && s >= 1) {
        for (float dy = 0; dy < 8 * s; dy += 1) {
            rect(cx - 5 * s + 1.5f * s * sinf(dy / s * 0.8f), cy - 11 * s - dy, 1, 1, true);
            rect(cx + 2 * s + 1.5f * s * sinf(dy / s * 0.8f + 1.5f), cy - 11 * s - dy, 1, 1, true);
        }
    }
}

// 🧉 Mate: calabacita redonda, virola, yerba asomando y la bombilla.
// (En el mate no se ve el liquido, asi que los sorbos no cambian el dibujo)
static void dibujar_mate(float cx, float cy, float s)
{
    cy += 2.5f * s;

    circulo(cx, cy + 4 * s, 9.5f * s, true);                              // panza
    rect_redondo(cx - 7 * s, cy - 8 * s, 14 * s, 10 * s, 3 * s, true);    // cuello
    rect(cx - 7 * s, cy - 4 * s, 14 * s, 1, false);                       // virola (una rayita)
    rect(cx - 6 * s, cy + 14 * s, 12 * s, 2 * s, true);                   // base

    // Boca del mate con la yerba asomando
    rect_redondo(cx - 5.5f * s, cy - 8 * s, 11 * s, 3.5f * s, 1.5f * s, false);
    rect(cx - 3 * s, cy - 7 * s, s, s, true);
    rect(cx - 0.5f * s, cy - 6.5f * s, s, s, true);

    // Bombilla: sale de la yerba en diagonal y termina en la boquilla
    for (float t = 0; t <= 1; t += 0.05f)
        rect(cx + 2 * s + 6 * s * t, cy - 7 * s - 11 * s * t, 1.5f * s, 1.5f * s, true);
    rect(cx + 6 * s, cy - 19 * s, 5 * s, 1.5f * s, true);
}

static bool es_bebida(eyes_comida_t c)
{
    return c == COMIDA_BOTELLA || c == COMIDA_TAZA || c == COMIDA_MATE;
}

// Medio ancho de cada comida: ahi van las marcas de los mordiscos (lado izquierdo)
static float medio_ancho(eyes_comida_t c)
{
    switch (c) {
    case COMIDA_TORTA:       return 13;
    case COMIDA_PAPAS:       return 11;
    case COMIDA_HAMBURGUESA: return 13;
    default:                 return 0;
    }
}

void eyes_dibujar_comida(eyes_comida_t comida, float cx, float cy, float s, int mordidas)
{
    switch (comida) {
    case COMIDA_TORTA:       dibujar_torta(cx, cy, s);       break;
    case COMIDA_PAPAS:       dibujar_papas(cx, cy, s);       break;
    case COMIDA_HAMBURGUESA: dibujar_hamburguesa(cx, cy, s); break;
    case COMIDA_BOTELLA:     dibujar_botella(cx, cy, s, mordidas); return;   // en las bebidas, "mordidas"
    case COMIDA_TAZA:        dibujar_taza(cx, cy, s, mordidas);    return;   // son los sorbos
    case COMIDA_MATE:        dibujar_mate(cx, cy, s);              return;
    default:                 return;
    }

    // Mordiscos: "bocados" negros del lado que da a la boca de Pitri
    float borde = cx - medio_ancho(comida) * s;
    if (mordidas >= 1) circulo(borde, cy - 3 * s, 6 * s, false);
    if (mordidas >= 2) circulo(borde + 2 * s, cy + 6 * s, 7 * s, false);
}

// ─── Lo que usa eyes.c ───────────────────────────────────────────────────────

#define TAMANO_AL_COMER 0.7f

void dibujar_comida_en_cara(void)
{
    if (comida_actual != COMIDA_NINGUNA)
        eyes_dibujar_comida(comida_actual, cara.comida_x, cara.comida_y, TAMANO_AL_COMER, mordidas_actuales);
    if (globo_texto != NULL)
        display_draw_text(90, 4, globo_texto, 1, true);
}

void comida_reiniciar(void)
{
    comida_actual = COMIDA_NINGUNA;
    mordidas_actuales = 0;
    globo_texto = NULL;
}

// ─── La animacion de comer ───────────────────────────────────────────────────

void eyes_comer(eyes_comida_t comida)
{
    if (comida == COMIDA_NINGUNA || es_bebida(comida)) return;   // las bebidas usan eyes_tomar()
    empezar_accion();
    disfraz_actual = DISFRAZ_NINGUNO;      // para comer se saca el disfraz
    ESP_LOGI(TAG, "¡A comer!");

    // Pitri se corre un poco a la izquierda y mira hacia la derecha,
    // por donde va a aparecer la comida
    cara_t d = CARA_NORMAL;
    d.x = -18;
    d.y = -6;
    d.mira = 1.5f;
    d.comida_x = 150;                      // la comida empieza afuera de la pantalla
    d.comida_y = 30;
    cara.comida_x = d.comida_x;
    cara.comida_y = d.comida_y;
    mordidas_actuales = 0;
    globo_texto = NULL;
    comida_actual = comida;
    animar(d, 300);

    // 1. Entra la comida y Pitri dice "¡Oh!" con la boca
    d.comida_x = 106;
    d.asombro = 0.6f;
    animar(d, 500);
    globo_texto = "Mmm!";
    esperar(500);

    // 2. Se la acerca a la boca, que se abre bien grande
    globo_texto = NULL;
    float boca_x = LCD_H_RES / 2.0f + d.x;
    d.comida_x = boca_x + 24;
    d.comida_y = 54;
    d.asombro = 1;
    animar(d, 400);

    // 3. Tres mordiscos: cierra la boca, aparece la marca, la vuelve a abrir
    for (int m = 1; m <= 3; m++) {
        d.asombro = 0;
        animar(d, 120);
        if (m < 3) mordidas_actuales = m;
        else       comida_actual = COMIDA_NINGUNA;   // ¡el ultimo mordisco se la termina!
        d.asombro = (m < 3) ? 1 : 0;
        animar(d, 160);
        esperar(150);
    }

    // 4. Mastica
    for (int i = 0; i < 4; i++) {
        d.asombro = 0.45f;
        d.y = -7;
        animar(d, 110);
        d.asombro = 0;
        d.y = -6;
        animar(d, 110);
    }

    // 5. ¡Rico!
    d.feliz = 1;
    globo_texto = "¡Rico!";
    animar(d, 250);
    esperar(1200);
    comida_reiniciar();
    animar(CARA_NORMAL, 400);
}

// ─── La animacion de tomar ───────────────────────────────────────────────────

void eyes_tomar(eyes_comida_t bebida)
{
    if (!es_bebida(bebida)) return;
    empezar_accion();
    disfraz_actual = DISFRAZ_NINGUNO;      // para tomar se saca el disfraz
    ESP_LOGI(TAG, "¡A tomar!");

    // Igual que al comer: Pitri se corre a la izquierda y mira a la derecha
    cara_t d = CARA_NORMAL;
    d.x = -18;
    d.y = -6;
    d.mira = 1.5f;
    d.comida_x = 150;
    d.comida_y = 32;
    cara.comida_x = d.comida_x;
    cara.comida_y = d.comida_y;
    mordidas_actuales = 0;                 // aca cuenta los sorbos
    globo_texto = NULL;
    comida_actual = bebida;
    animar(d, 300);

    // 1. Llega la bebida
    d.comida_x = 106;
    d.asombro = 0.5f;
    animar(d, 500);
    globo_texto = "Sed!";
    esperar(500);

    // 2. Se la acerca a la boca, con la boquita chiquita para tomar
    globo_texto = NULL;
    float boca_x = LCD_H_RES / 2.0f + d.x;
    d.comida_x = boca_x + 20;
    d.comida_y = 53;
    d.asombro = 0.35f;
    animar(d, 400);

    // 3. Tres sorbos: la levanta un poquito, "glu", y el liquido baja
    for (int sorbo = 1; sorbo <= 3; sorbo++) {
        // El ultimo sorbo del mate hace el ruidito de cuando se termina
        globo_texto = (bebida == COMIDA_MATE && sorbo == 3) ? "Slurp" : "Glu";
        d.comida_y = 49;
        animar(d, 200);
        mordidas_actuales = sorbo;
        d.comida_y = 53;
        animar(d, 200);
        globo_texto = NULL;
        esperar(150);
    }

    // 4. Se llevan el recipiente vacio
    d.comida_x = 150;
    d.asombro = 0;
    animar(d, 400);
    comida_actual = COMIDA_NINGUNA;

    // 5. ¡Ahh!
    d.feliz = 1;
    globo_texto = "Ahh!";
    animar(d, 250);
    esperar(1200);
    comida_reiniciar();
    animar(CARA_NORMAL, 400);
}
