#include "eyes.h"
#include "eyes_interno.h"
#include "display.h"
#include "board_config.h"

#include <math.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_random.h"
#include "esp_log.h"

static const char *TAG = "OJOS";

// La forma de los ojos y la estructura de la cara estan en eyes_interno.h

const cara_t CARA_NORMAL = {
    .x = 0, .y = 0, .ancho = OJO_ANCHO, .alto = OJO_ALTO,
    .abre_izq = 1, .abre_der = 1, .feliz = 0,
};

cara_t cara;

// Texto que se muestra al costado con eyes_mostrar_datos()
#define DATOS_X 78            // donde empieza el texto (deja la izquierda para la carita)
#define DATOS_Y 15            // altura del primer renglon
static char texto_datos[64];

// ─── Ayudantes ───────────────────────────────────────────────────────────────

// Numero al azar entre min y max (incluidos)
int azar(int min, int max)
{
    return min + (int)(esp_random() % (uint32_t)(max - min + 1));
}

// ─── Cortar lo que esta haciendo ─────────────────────────────────────────────
// main.c le puede dar a los ojos una funcion que diga "¡para!" (por ejemplo,
// cuando se aprieta el boton del menu). Cada cuadro de animacion y cada espera
// la consultan: si dice que si, todo lo que sigue del juego se saltea y la
// funcion publica que se estaba ejecutando termina enseguida.

static bool (*hay_que_cortar)(void) = NULL;
static bool cortado = false;

void eyes_set_interrupcion(bool (*funcion)(void))
{
    hay_que_cortar = funcion;
}

void empezar_accion(void)
{
    cortado = false;
}

static bool cortar_ahora(void)
{
    if (!cortado && hay_que_cortar != NULL && hay_que_cortar()) cortado = true;
    return cortado;
}

void esperar(int ms)
{
    // En pedacitos de 50 ms, para poder cortar enseguida si hace falta
    for (int t = 0; t < ms; t += 50) {
        if (cortar_ahora()) return;
        int pedazo = (ms - t < 50) ? ms - t : 50;
        vTaskDelay(pdMS_TO_TICKS(pedazo));
    }
}

// Espiral que gira, para cuando Pitri esta mareado
static void dibujar_espiral(float cx, float cy, float radio, float giro)
{
    const float vueltas = 2.5f;
    const float fin = vueltas * 2 * (float)M_PI;
    for (float a = 0; a < fin; a += 0.05f) {
        float r = radio * a / fin;
        int px = (int)(cx + r * cosf(a + giro));
        int py = (int)(cy + r * sinf(a + giro));
        display_fill_rect(px, py, 2, 2, true);   // trazo de 2 pixeles de grosor
    }
}

// Un corazon con centro en (cx, cy): dos circulos arriba y un triangulo abajo
//
//      ██  ██        r = radio de cada circulo
//     ████████       ancho total = 4 r
//      ██████        alto total  = 3 r
//        ██
static void dibujar_corazon(float cx, float cy, float r)
{
    if (r < 1) r = 1;
    int ri = (int)(r + 0.5f);
    int y0 = (int)(cy - r * 0.5f);     // altura de los centros de los circulos
    display_fill_circle((int)(cx - r), y0, ri, true);
    display_fill_circle((int)(cx + r), y0, ri, true);
    display_fill_triangle((int)(cx - 2 * r), y0, (int)(cx + 2 * r), y0, (int)cx, (int)(y0 + 2 * r), true);
}

// ─── Herramientas para dibujar bocas ─────────────────────────────────────────

// Pedazo de circulo con centro (cx, cy) y radio r, desde el angulo "desde" hasta
// "hasta" (en radianes; 0 = derecha, π/2 = abajo, π = izquierda, 3π/2 = arriba,
// porque en la pantalla y crece para abajo). "grosor" = ancho del trazo.
static void dibujar_arco(float cx, float cy, float r, float desde, float hasta, int grosor)
{
    for (float a = desde; a <= hasta; a += 0.04f) {
        int px = (int)(cx + r * cosf(a));
        int py = (int)(cy + r * sinf(a));
        display_fill_rect(px - grosor / 2, py - grosor / 2, grosor, grosor, true);
    }
}

// Linea recta de (x0, y0) a (x1, y1), con trazo de "grosor" pixeles
static void dibujar_linea(float x0, float y0, float x1, float y1, int grosor)
{
    float dx = x1 - x0, dy = y1 - y0;
    int pasos = (int)fmaxf(fabsf(dx), fabsf(dy));
    if (pasos < 1) pasos = 1;
    for (int i = 0; i <= pasos; i++) {
        float t = (float)i / pasos;
        display_fill_rect((int)(x0 + dx * t), (int)(y0 + dy * t), grosor, grosor, true);
    }
}

// Anillo: circulo blanco con un agujero negro en el medio
static void dibujar_anillo(int cx, int cy, int r_afuera, int r_adentro)
{
    display_fill_circle(cx, cy, r_afuera, true);
    display_fill_circle(cx, cy, r_adentro, false);
}

// Boca sonriente: un pedazo de circulo (la parte de abajo), con trazo de 3 pixeles.
// (cx, cy) es el centro del circulo y r su radio: mas grande = sonrisa mas ancha.
//
//     ·           ·
//      ·         ·
//        · · · ·
static void dibujar_sonrisa(float cx, float cy, float r)
{
    dibujar_arco(cx, cy, r, 0.2f * (float)M_PI, 0.8f * (float)M_PI, 3);   // de 36° a 144°
}

// Rayita negra en diagonal (de abajo a la izquierda hacia arriba a la derecha):
// es el "brillo" del vidrio de los lentes
static void dibujar_reflejo(int x, int y, int largo, int grosor)
{
    for (int k = 0; k < largo; k++)
        display_fill_rect(x + k, y + largo - k, grosor, 1, false);
}

// Lentes de sol. "puestos" va de 0 (todavia arriba, fuera de la pantalla) a 1 (en los ojos)
static void dibujar_lentes(float cx_izq, float cx_der, float cy, float puestos)
{
    const int w = OJO_ANCHO + 10, h = 26;
    int arriba = (int)(cy - 2 - h / 2 - (1 - puestos) * 60);   // bajan desde arriba
    int xi = (int)(cx_izq - w / 2.0f);
    int xd = (int)(cx_der - w / 2.0f);

    display_fill_rect(0, arriba + 3, xi, 2, true);                          // patilla izquierda
    display_fill_rect(xd + w, arriba + 3, LCD_H_RES - (xd + w), 2, true);   // patilla derecha
    display_fill_rect(xi + w, arriba + 2, xd - (xi + w), 3, true);          // puente

    for (int i = 0; i < 2; i++) {
        int x = (i == 0) ? xi : xd;
        display_fill_round_rect(x, arriba, w, h, 8, true);   // vidrio
        dibujar_reflejo(x + 6, arriba + 5, 12, 3);           // brillo grande
        dibujar_reflejo(x + 16, arriba + 11, 7, 1);          // brillo chiquito
    }
}

// Una gotita: circulo abajo y punta arriba
static void dibujar_lagrima(int x, int y)
{
    display_fill_circle(x, y, 3, true);
    display_fill_triangle(x - 3, y, x + 3, y, x, y - 7, true);
}

// lado: -1 = ojo izquierdo, +1 = ojo derecho
static void dibujar_ojo(float cx, float cy, float ancho, float alto, float abre, int lado)
{
    if (cara.espiral > 0.5f) {
        dibujar_espiral(cx, cy, fminf(ancho, alto) / 2 - 1, cara.giro);
        return;
    }

    if (cara.corazon > 0.5f) {
        dibujar_corazon(cx, cy, ancho / 4);    // late cuando cambia el ancho
        return;
    }

    int h = (int)(OJO_ALTO_CERRADO + (alto - OJO_ALTO_CERRADO) * abre);
    int w = (int)ancho;
    int x = (int)(cx - w / 2.0f);
    int y = (int)(cy - h / 2.0f);
    int x_adentro = (lado < 0) ? x + w + 2 : x - 2;   // el borde que da a la nariz
    int x_afuera  = (lado < 0) ? x - 2     : x + w + 2;

    display_fill_round_rect(x, y, w, h, OJO_RADIO, true);

    // Ojitos felices: tapamos la parte de abajo con una "loma" negra
    if (cara.feliz > 0.01f && h > OJO_ALTO_CERRADO) {
        int tapa = (int)(h * 0.6f * cara.feliz);
        display_fill_round_rect(x - 4, y + h - tapa, w + 8, h, (w + 8) / 2, false);
    }

    // Enojado: un triangulo negro corta la esquina de arriba que da a la nariz
    if (cara.enojo > 0.01f) {
        int baja = (int)(h * 0.55f * cara.enojo);
        display_fill_triangle(x_afuera, y - 2, x_adentro, y - 2, x_adentro, y + baja, false);
    }

    // Triste: el triangulo corta la esquina de arriba del lado de afuera
    if (cara.triste > 0.01f) {
        int baja = (int)(h * 0.5f * cara.triste);
        display_fill_triangle(x_adentro, y - 2, x_afuera, y - 2, x_afuera, y + baja, false);
    }

    // Lagrima cayendo desde abajo del ojo hasta salir de la pantalla
    if (cara.lagrima > 0.01f && cara.lagrima < 0.99f) {
        int lx = (int)(cx + lado * w * 0.25f);
        int desde = y + h + 6;
        int hasta = LCD_V_RES + 8;
        dibujar_lagrima(lx, desde + (int)((hasta - desde) * cara.lagrima));
    }
}

// Una letra Z de tamaño s, con la esquina de arriba a la izquierda en (x, y)
static void dibujar_z(int x, int y, int s)
{
    display_fill_rect(x, y, s, 2, true);             // raya de arriba
    display_fill_rect(x, y + s - 2, s, 2, true);     // raya de abajo
    for (int k = 0; k < s; k++)                      // diagonal
        display_fill_rect(x + s - 1 - k, y + k, 2, 1, true);
}

// ─── La boca ─────────────────────────────────────────────────────────────────
// Cada emocion tiene su boca. Si hay varias emociones a la vez (por ejemplo,
// mientras pasa de una cara a otra), gana la mas fuerte. Si ninguna llega a
// 0,3, Pitri esta tranquilo y lleva una sonrisita sencilla.

typedef enum {
    BOCA_NINGUNA,
    BOCA_SENCILLA,   // 🙂 tranquilo:  ‿ chiquita
    BOCA_SONRISA,    // 😊 contento:   ‿
    BOCA_TRISTE,     // 😢 triste:     ⁀
    BOCA_ENOJADA,    // 😠 enojado:    /\/\/\  (dientes apretados)
    BOCA_O,          // 😮 sorpresa:   O
    BOCA_MAREADA,    // 😵‍💫 mareado:    ~~~~  (se mueve)
    BOCA_BESO,       // 😍 enamorado:  3
    BOCA_CANCHERA,   // 😎 verano:     __/   (sonrisa de costado)
    BOCA_SOSPECHA,   // 🤨 sospecha:   rayita torcida
    BOCA_DORMIDA,    // 😴 dormido:    o
} boca_t;

static void dibujar_boca(float cy, float perspectiva)
{
    // 1. ¿Que boca va? La de la emocion mas fuerte
    float mas_fuerte = 0.3f;
    boca_t boca = BOCA_NINGUNA;
#define CANDIDATA(peso, tipo) if ((peso) > mas_fuerte) { mas_fuerte = (peso); boca = (tipo); }
    CANDIDATA(cara.feliz,    BOCA_SONRISA);
    CANDIDATA(cara.triste,   BOCA_TRISTE);
    CANDIDATA(cara.enojo,    BOCA_ENOJADA);
    CANDIDATA(cara.asombro,  BOCA_O);
    CANDIDATA(cara.espiral,  BOCA_MAREADA);
    CANDIDATA(cara.corazon,  BOCA_BESO);
    CANDIDATA(cara.lentes,   BOCA_CANCHERA);
    CANDIDATA(cara.sospecha, BOCA_SOSPECHA);
    CANDIDATA(cara.sueno,    BOCA_DORMIDA);
#undef CANDIDATA
    if (boca == BOCA_NINGUNA) boca = BOCA_SENCILLA;   // sin emocion fuerte: boca tranquila

    float mx = LCD_H_RES / 2.0f + cara.x;   // la boca sigue a la cara

    // La sonrisa tiene su propia cuenta (crece con la felicidad)
    if (boca == BOCA_SONRISA) {
        // Con menos felicidad los ojos se tapan menos (bajan mas), asi que la
        // sonrisa va un poco mas abajo para no tocarlos
        float boca_y = cy + 6 + (1 - cara.feliz) * 8;
        dibujar_sonrisa(mx, boca_y, 6 + 8 * cara.feliz);
        return;
    }

    // 2. ¿Donde terminan los ojos? La boca va justo abajo
    float mitad_ojo;
    if (cara.espiral > 0.5f) {
        mitad_ojo = fminf(cara.ancho, cara.alto) / 2;
    } else if (cara.corazon > 0.5f) {
        mitad_ojo = 1.5f * cara.ancho / 4;              // la punta del corazon
    } else {
        // La boca sencilla no se mueve cuando parpadea: la ubicamos como si
        // los ojos estuvieran abiertos
        float abre = (boca == BOCA_SENCILLA) ? 1 : fmaxf(cara.abre_izq, cara.abre_der);
        float alto = cara.alto * (1 + fabsf(perspectiva));
        mitad_ojo = (OJO_ALTO_CERRADO + (alto - OJO_ALTO_CERRADO) * abre) / 2;
    }
    if (cara.lentes > 0.5f) mitad_ojo = fmaxf(mitad_ojo, 11);   // abajo de los lentes

    // Alto de cada boca, para que no se salga de la pantalla por abajo
    static const int alto_boca[] = {
        [BOCA_SENCILLA] = 5, [BOCA_TRISTE] = 7, [BOCA_ENOJADA] = 6, [BOCA_O] = 11, [BOCA_MAREADA] = 6,
        [BOCA_BESO] = 16, [BOCA_CANCHERA] = 6, [BOCA_SOSPECHA] = 5, [BOCA_DORMIDA] = 7,
    };
    float y = fminf(cy + mitad_ojo + 4, LCD_V_RES - 1 - alto_boca[boca]);   // arriba de la boca

    // 3. Dibujarla
    switch (boca) {
    case BOCA_SENCILLA:    // sonrisita chiquita y suave, con trazo finito
        dibujar_arco(mx, y - 4, 7, 0.25f * (float)M_PI, 0.75f * (float)M_PI, 2);
        break;

    case BOCA_TRISTE:      // la sonrisa dada vuelta: la parte de arriba del circulo
        dibujar_arco(mx, y + 11, 11, 1.2f * (float)M_PI, 1.8f * (float)M_PI, 3);
        break;

    case BOCA_ENOJADA:     // zigzag de dientes apretados
        for (int i = 0; i < 6; i++) {
            float x0 = mx - 12 + i * 4;
            dibujar_linea(x0, y + (i % 2 ? 4 : 1), x0 + 4, y + (i % 2 ? 1 : 4), 2);
        }
        break;

    case BOCA_O:           // una "O" que crece con el asombro
        dibujar_anillo((int)mx, (int)(y + 5), 3 + (int)(2 * cara.asombro), 1 + (int)(2 * cara.asombro));
        break;

    case BOCA_MAREADA:     // ola que se mueve al mismo tiempo que giran las espirales
        for (int dx = -12; dx <= 12; dx++)
            display_fill_rect((int)mx + dx, (int)(y + 2 + 2 * sinf(dx * 0.5f + cara.giro)), 2, 2, true);
        break;

    case BOCA_BESO:        // un "3": dos pancitas una arriba de la otra
        dibujar_arco(mx - 2, y + 4, 4, -0.5f * (float)M_PI, 0.5f * (float)M_PI, 2);
        dibujar_arco(mx - 2, y + 11, 4, -0.5f * (float)M_PI, 0.5f * (float)M_PI, 2);
        break;

    case BOCA_CANCHERA:    // recta y con una punta que sube: sonrisa de costado
        dibujar_linea(mx - 8, y + 4, mx + 3, y + 4, 2);
        dibujar_linea(mx + 3, y + 4, mx + 9, y, 2);
        break;

    case BOCA_SOSPECHA:    // rayita cortita y torcida
        dibujar_linea(mx - 6, y + 3, mx + 6, y + 1, 2);
        break;

    case BOCA_DORMIDA:     // "o" chiquita, como roncando
        dibujar_anillo((int)mx, (int)(y + 3), 3, 1);
        break;

    default:
        break;
    }
}

static void dibujar(void)
{
    // Perspectiva: el ojo del lado hacia donde mira se ve un poquito mas grande.
    // Normalmente mira hacia donde estan los ojos (x); "mira" la puede cambiar.
    float p = (cara.x / MAX_X + cara.mira) * 0.12f;
    if (p >  0.15f) p =  0.15f;
    if (p < -0.15f) p = -0.15f;

    float cy     = LCD_V_RES / 2.0f + cara.y;
    float cx_izq = LCD_H_RES / 2.0f - OJO_SEPARACION / 2.0f - cara.ancho / 2.0f + cara.x;
    float cx_der = LCD_H_RES / 2.0f + OJO_SEPARACION / 2.0f + cara.ancho / 2.0f + cara.x;

    display_clear();

    // Disfraces que son un dibujo entero (fantasma, calabaza, arbolito...):
    // el disfraz dibuja todo, con sus propios ojos y boca
    if (disfraz_tapa_cara()) {
        dibujar_disfraz(cy);
        display_flush();
        return;
    }

    dibujar_ojo(cx_izq, cy, cara.ancho, cara.alto * (1 - p), cara.abre_izq, -1);
    dibujar_ojo(cx_der, cy, cara.ancho, cara.alto * (1 + p), cara.abre_der, +1);

    // La boca: la del disfraz (colmillos, pico, hocico...) o la de la emocion
    if (disfraz_actual != DISFRAZ_NINGUNO) {
        dibujar_disfraz(cy);
    } else {
        dibujar_boca(cy, p);
    }

    // La comida que le estan dando (si esta comiendo)
    dibujar_comida_en_cara();

    // Datos (texto chico) que entran deslizandose desde la derecha
    if (cara.datos > 0.01f) {
        int x = (int)(LCD_H_RES - (LCD_H_RES - DATOS_X) * cara.datos);
        display_draw_text(x, DATOS_Y, texto_datos, 1, true);
    }

    // Corazoncitos que suben flotando: uno a cada costado y uno en el medio
    if (cara.corazoncitos > 0.01f && cara.corazoncitos < 0.99f) {
        float t1 = cara.corazoncitos;
        float t2 = fmodf(cara.corazoncitos + 0.5f, 1.0f);   // el del medio va desfasado
        dibujar_corazon(8,                 LCD_V_RES + 6 - 80 * t1, 3);
        dibujar_corazon(LCD_H_RES - 9,     LCD_V_RES + 6 - 80 * t1, 3);
        dibujar_corazon(LCD_H_RES / 2.0f,  LCD_V_RES + 6 - 80 * t2, 3);
    }

    // Lentes de sol (van arriba de los ojos)
    if (cara.lentes > 0.01f) {
        dibujar_lentes(cx_izq, cx_der, cy, cara.lentes);
    }

    // "Z" que sube y crece desde arriba del ojo derecho
    if (cara.zzz > 0.01f && cara.zzz < 0.99f) {
        int s  = 4 + (int)(6 * cara.zzz);
        int zx = (int)(cx_der + cara.ancho / 2 + 2 + 8 * cara.zzz);
        int zy = (int)(cy - 6 - 22 * cara.zzz);
        dibujar_z(zx, zy, s);
    }

    display_flush();
}

static float mezclar(float a, float b, float t) { return a + (b - a) * t; }

// Pasa suavemente de la cara actual a "destino" en "ms" milisegundos
void animar(cara_t destino, int ms)
{
    cara_t inicio = cara;
    int pasos = ms / CUADRO_MS;
    if (pasos < 1) pasos = 1;

    TickType_t ultimo = xTaskGetTickCount();
    for (int i = 1; i <= pasos; i++) {
        if (cortar_ahora()) return;     // ¡apretaron el boton! dejamos todo como esta

        float t = (float)i / pasos;
        t = t * t * (3 - 2 * t);   // arranca y frena despacito

        cara.x        = mezclar(inicio.x,        destino.x,        t);
        cara.y        = mezclar(inicio.y,        destino.y,        t);
        cara.ancho    = mezclar(inicio.ancho,    destino.ancho,    t);
        cara.alto     = mezclar(inicio.alto,     destino.alto,     t);
        cara.abre_izq = mezclar(inicio.abre_izq, destino.abre_izq, t);
        cara.abre_der = mezclar(inicio.abre_der, destino.abre_der, t);
        cara.feliz    = mezclar(inicio.feliz,    destino.feliz,    t);
        cara.enojo    = mezclar(inicio.enojo,    destino.enojo,    t);
        cara.triste   = mezclar(inicio.triste,   destino.triste,   t);
        cara.espiral  = mezclar(inicio.espiral,  destino.espiral,  t);
        cara.giro     = mezclar(inicio.giro,     destino.giro,     t);
        cara.lagrima  = mezclar(inicio.lagrima,  destino.lagrima,  t);
        cara.zzz      = mezclar(inicio.zzz,      destino.zzz,      t);
        cara.corazon  = mezclar(inicio.corazon,  destino.corazon,  t);
        cara.corazoncitos = mezclar(inicio.corazoncitos, destino.corazoncitos, t);
        cara.lentes   = mezclar(inicio.lentes,   destino.lentes,   t);
        cara.asombro  = mezclar(inicio.asombro,  destino.asombro,  t);
        cara.sospecha = mezclar(inicio.sospecha, destino.sospecha, t);
        cara.sueno    = mezclar(inicio.sueno,    destino.sueno,    t);
        cara.mira     = mezclar(inicio.mira,     destino.mira,     t);
        cara.datos    = mezclar(inicio.datos,    destino.datos,    t);
        cara.fase     = mezclar(inicio.fase,     destino.fase,     t);
        cara.comida_x = mezclar(inicio.comida_x, destino.comida_x, t);
        cara.comida_y = mezclar(inicio.comida_y, destino.comida_y, t);

        dibujar();
        xTaskDelayUntil(&ultimo, pdMS_TO_TICKS(CUADRO_MS));
    }
}

// Igual que animar, pero cambiando solo la posicion
void mover_a(float x, float y, int ms)
{
    cara_t d = cara;
    d.x = x;
    d.y = y;
    animar(d, ms);
}

// ─── Parpadeos ───────────────────────────────────────────────────────────────

void cerrar_y_abrir(bool izq, bool der, int cerrado_ms)
{
    cara_t abierta = cara;
    cara_t cerrada = cara;
    if (izq) cerrada.abre_izq = 0;
    if (der) cerrada.abre_der = 0;

    animar(cerrada, 60);
    esperar(cerrado_ms);
    animar(abierta, 90);
}

void eyes_parpadear(void)
{
    empezar_accion();
    cerrar_y_abrir(true, true, 30);
}

// ─── Juegos ──────────────────────────────────────────────────────────────────

// Mira a un lugar al azar y se queda un ratito
static void juego_mirar(void)
{
    mover_a(azar(-MAX_X, MAX_X), azar(-MAX_Y, MAX_Y), azar(150, 400));
    esperar(azar(400, 1500));
}

// Vuelve a mirar al frente
static void juego_centro(void)
{
    animar(CARA_NORMAL, 300);
    esperar(azar(300, 1000));
}

static void juego_doble_parpadeo(void)
{
    eyes_parpadear();
    esperar(80);
    eyes_parpadear();
}

// Guiña un ojo (a veces el izquierdo, a veces el derecho) con carita feliz
static void juego_guino(void)
{
    cara_t d = cara;
    d.feliz = 0.5f;
    animar(d, 150);

    bool izquierdo = azar(0, 1);
    cerrar_y_abrir(izquierdo, !izquierdo, 350);

    d = cara;
    d.feliz = 0;
    animar(d, 150);
}

// Abre los ojos re grandes
static void juego_sorpresa(void)
{
    cara_t d = cara;
    d.ancho = OJO_ANCHO + 6;
    d.alto  = OJO_ALTO + 10;
    d.y     = -8;            // sube un poco la cara para que entre la boca en "O"
    d.asombro = 1;
    animar(d, 120);
    esperar(azar(500, 900));
    d.ancho = OJO_ANCHO;
    d.alto  = OJO_ALTO;
    d.y     = 0;
    d.asombro = 0;
    animar(d, 250);
}

// Ojitos felices ^ ^ dando saltitos
static void juego_contento(void)
{
    cara_t d = CARA_NORMAL;
    d.feliz = 1;
    animar(d, 200);
    for (int i = 0; i < 3; i++) {
        mover_a(0, -5, 120);
        mover_a(0, 0, 120);
    }
    esperar(400);
    animar(CARA_NORMAL, 200);
}

// Da una vuelta con los ojos
static void juego_girar(void)
{
    float r_x = MAX_X * 0.7f, r_y = MAX_Y * 0.8f;
    int sentido = azar(0, 1) ? 1 : -1;
    mover_a(r_x, 0, 150);
    for (int i = 1; i <= 16; i++) {
        float ang = sentido * i * 2 * (float)M_PI / 16;
        mover_a(r_x * cosf(ang), r_y * sinf(ang), 60);
    }
    animar(CARA_NORMAL, 200);
}

// Entrecierra los ojos y mira de un lado al otro, como desconfiando
static void juego_sospecha(void)
{
    cara_t d = CARA_NORMAL;
    d.abre_izq = d.abre_der = 0.35f;
    d.sospecha = 1;
    animar(d, 200);
    mover_a(-MAX_X, 0, 500);
    esperar(300);
    mover_a(MAX_X, 0, 700);
    esperar(300);
    animar(CARA_NORMAL, 250);
}

// Le da sueño: se le cierran los ojos de a poco y despues se despierta
static void juego_sueno(void)
{
    cara_t d = CARA_NORMAL;
    d.y = 4;
    d.abre_izq = d.abre_der = 0.3f;
    d.sueno = 1;
    animar(d, 1200);
    d.abre_izq = d.abre_der = 0.0f;
    animar(d, 800);
    esperar(azar(800, 2000));
    animar(CARA_NORMAL, 100);   // ¡se desperto!
    juego_doble_parpadeo();
}

// Recorre la pantalla de punta a punta
static void juego_pasear(void)
{
    int lado = azar(0, 1) ? 1 : -1;
    mover_a(lado * MAX_X, azar(-MAX_Y, MAX_Y), 400);
    esperar(300);
    mover_a(-lado * MAX_X, azar(-MAX_Y, MAX_Y), 700);
    esperar(300);
    mover_a(0, 0, 400);
}

// Se marea: los ojos se vuelven espirales que giran y la cabeza se bambolea
static void juego_mareado(void)
{
    cara_t d = CARA_NORMAL;
    d.espiral = 1;
    animar(d, 60);

    for (int i = 0; i < 30; i++) {
        d.giro += 0.9f;                       // la espiral sigue girando
        d.x = 8 * cosf(i * 0.5f);             // y la cara da vueltitas
        d.y = 5 * sinf(i * 0.5f);
        animar(d, 60);
    }

    // Vuelve en si: cierra los ojos, sacude la cabeza y los abre normales
    cara = CARA_NORMAL;
    cara.abre_izq = cara.abre_der = 0;
    dibujar();
    for (int i = 0; i < 2; i++) {
        mover_a(-6, 0, 90);
        mover_a(6, 0, 90);
    }
    animar(CARA_NORMAL, 200);
    juego_doble_parpadeo();
}

// Se enoja: cejas para adentro y tiembla de bronca
static void juego_enojado(void)
{
    cara_t d = CARA_NORMAL;
    d.enojo = 1;
    d.abre_izq = d.abre_der = 0.8f;
    d.y = 2;
    animar(d, 250);

    for (int i = 0; i < 6; i++) {
        mover_a(-3, 2, 50);
        mover_a(3, 2, 50);
    }
    mover_a(0, 2, 50);
    esperar(azar(800, 1500));
    animar(CARA_NORMAL, 300);
}

// Se pone triste y le caen lagrimas
static void juego_llorar(void)
{
    cara_t d = CARA_NORMAL;
    d.triste = 1;
    d.abre_izq = d.abre_der = 0.75f;
    d.y = 3;
    animar(d, 400);

    int lagrimas = azar(2, 4);
    for (int i = 0; i < lagrimas; i++) {
        cara.lagrima = 0;
        d.lagrima = 1;
        animar(d, 700);
        d.lagrima = 0;
        mover_a(0, 1, 100);                   // un sollozo
        mover_a(0, 3, 100);
    }

    cara.lagrima = 0;
    esperar(500);
    animar(CARA_NORMAL, 400);
}

// Se enamora: los ojos se vuelven corazones que laten y suben corazoncitos
static void juego_enamorado(void)
{
    cara_t d = CARA_NORMAL;
    d.corazon = 1;
    animar(d, 100);

    // 12 latidos: grande, chico, grande, chico... balanceandose de lado a lado
    for (int i = 0; i < 12; i++) {
        bool grande = (i % 2 == 0);
        d.ancho = grande ? OJO_ANCHO + 6 : OJO_ANCHO - 2;
        d.x     = grande ? -4 : 4;

        // los corazoncitos dan una vuelta completa cada 4 latidos
        if (i % 4 == 0) cara.corazoncitos = 0;
        d.corazoncitos = (i % 4 + 1) / 4.0f;

        animar(d, grande ? 120 : 200);   // el latido es rapido y el "descanso" mas lento
    }

    cara.corazoncitos = 0;
    animar(CARA_NORMAL, 150);           // vuelven los ojos normales
    eyes_parpadear();
}

// Verano: bajan unos lentes de sol, se mueve canchero y despues se los saca
static void juego_verano(void)
{
    cara_t d = CARA_NORMAL;
    animar(d, 150);                     // mira al frente

    d.lentes = 1;
    d.abre_izq = d.abre_der = 0.55f;    // entrecierra los ojos, relajado (asi no asoman por abajo)
    animar(d, 500);                     // ¡bajan los lentes!
    esperar(300);

    for (int i = 0; i < 4; i++) {       // se mueve al ritmo de la musica
        mover_a(-6, 2, 300);
        mover_a(6, -2, 300);
    }
    mover_a(0, 0, 200);
    esperar(500);

    d = cara;
    d.lentes = 0;
    d.abre_izq = d.abre_der = 1;
    animar(d, 400);                     // se los sube y abre bien los ojos

    d.feliz = 1;                        // y guiña un ojo contento
    animar(d, 150);
    cerrar_y_abrir(false, true, 300);
    animar(CARA_NORMAL, 200);
}

typedef struct {
    const char *nombre;
    void (*hacer)(void);
    int peso;   // mas peso = sale mas seguido
} juego_t;

static const juego_t juegos[] = {
    { "mirar",          juego_mirar,          10 },
    { "centro",         juego_centro,          4 },
    { "doble parpadeo", juego_doble_parpadeo,  3 },
    { "guiño",          juego_guino,           2 },
    { "sorpresa",       juego_sorpresa,        2 },
    { "contento",       juego_contento,        2 },
    { "girar",          juego_girar,           1 },
    { "sospecha",       juego_sospecha,        1 },
    { "sueño",          juego_sueno,           1 },
    { "pasear",         juego_pasear,          2 },
    { "mareado",        juego_mareado,         1 },
    { "enojado",        juego_enojado,         1 },
    { "llorar",         juego_llorar,          1 },
    { "enamorado",      juego_enamorado,       2 },
    { "verano",         juego_verano,          2 },
};
#define CANT_JUEGOS ((int)(sizeof(juegos) / sizeof(juegos[0])))

// ─── Funciones publicas ──────────────────────────────────────────────────────

void eyes_init(void)
{
    empezar_accion();
    disfraz_actual = DISFRAZ_NINGUNO;   // vuelve a ser Pitri, sin disfraz
    comida_reiniciar();
    cara = CARA_NORMAL;
    dibujar();
}

// ─── Estoy lleno ─────────────────────────────────────────────────────────────

void eyes_decir_que_no(void)
{
    empezar_accion();
    disfraz_actual = DISFRAZ_NINGUNO;
    comida_reiniciar();

    cara_t d = CARA_NORMAL;
    d.sospecha = 1;                     // boca torcida: "mmm, no"
    animar(d, 200);

    for (int i = 0; i < 3; i++) {       // mueve la cabeza: no, no, no
        mover_a(-10, 0, 110);
        mover_a(10, 0, 110);
    }
    mover_a(0, 0, 110);

    // Se corre a un costado y lo dice con letras (como cuando muestra los datos)
    snprintf(texto_datos, sizeof(texto_datos), "No,\ngracias!\nEstoy\nlleno");
    d.ancho = 20;
    d.alto  = 26;
    d.x     = -28;
    d.mira  = 2;
    d.datos = 1;
    animar(d, 400);
    esperar(2000);
    animar(CARA_NORMAL, 400);
}

// ─── Mostrar datos ───────────────────────────────────────────────────────────

void eyes_mostrar_datos(const char *texto, int ms)
{
    empezar_accion();
    snprintf(texto_datos, sizeof(texto_datos), "%s", texto);

    // La carita se achica y se corre a la izquierda, mirando hacia la derecha,
    // mientras los datos entran deslizandose
    cara_t d = CARA_NORMAL;
    d.ancho = 20;
    d.alto  = 26;
    d.x     = -28;
    d.mira  = 2;           // aunque esta a la izquierda, mira a la derecha
    d.datos = 1;
    animar(d, 500);

    esperar(ms / 2);
    eyes_parpadear();      // un parpadeo en el medio, para que se vea vivo
    esperar(ms / 2);

    animar(CARA_NORMAL, 400);   // los datos se van y la carita vuelve al centro
}

// ─── Dormir ──────────────────────────────────────────────────────────────────

void eyes_dormirse(void)
{
    empezar_accion();
    ESP_LOGI(TAG, "Me voy a dormir... zzz");
    cara_t d = CARA_NORMAL;
    d.y = 4;
    d.abre_izq = d.abre_der = 0.3f;
    d.sueno = 1;
    animar(d, 1200);                 // se le van cerrando los ojos
    d.abre_izq = d.abre_der = 0;
    animar(d, 800);
}

void eyes_dormido(void)
{
    empezar_accion();
    // Respira: los ojos cerrados suben y bajan mientras sube una "Z"
    static bool inspira = true;
    cara_t d = cara;
    d.abre_izq = d.abre_der = 0;
    d.y = inspira ? 2 : 5;

    if (inspira) {
        cara.zzz = 0;
        d.zzz = 1;
    } else {
        d.zzz = 0;
    }
    animar(d, 1200);
    cara.zzz = 0;
    inspira = !inspira;
}

void eyes_despertar(void)
{
    empezar_accion();
    ESP_LOGI(TAG, "¡Me desperte!");
    cara.zzz = 0;
    animar(CARA_NORMAL, 120);
    juego_doble_parpadeo();
}

void eyes_saludar(void)
{
    empezar_accion();
    ESP_LOGI(TAG, "¡Hola! Saludando");
    animar(CARA_NORMAL, 150);   // deja lo que estaba haciendo y mira al frente
    juego_sorpresa();
    juego_contento();
}

void eyes_jugar(void)
{
    empezar_accion();
    int total = 0;
    for (int i = 0; i < CANT_JUEGOS; i++) total += juegos[i].peso;

    int r = azar(0, total - 1);
    for (int i = 0; i < CANT_JUEGOS; i++) {
        if (r < juegos[i].peso) {
            ESP_LOGI(TAG, "Juego: %s", juegos[i].nombre);
            juegos[i].hacer();
            return;
        }
        r -= juegos[i].peso;
    }
}
