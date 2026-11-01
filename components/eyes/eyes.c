#include "eyes.h"
#include "display.h"
#include "board_config.h"

#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_random.h"
#include "esp_log.h"

static const char *TAG = "OJOS";

// ─── Forma de los ojos (¡jueguen con estos números!) ─────────────────────────
#define OJO_ANCHO        30   // píxeles
#define OJO_ALTO         36
#define OJO_RADIO        9    // qué tan redondeadas son las esquinas
#define OJO_SEPARACION   20   // espacio entre los dos ojos
#define OJO_ALTO_CERRADO 3    // grosor de la rayita cuando está cerrado

// Hasta dónde se pueden mover los ojos sin salirse de la pantalla
#define MAX_X  ((LCD_H_RES - 2 * OJO_ANCHO - OJO_SEPARACION) / 2)   // 24
#define MAX_Y  ((LCD_V_RES - OJO_ALTO) / 2)                          // 14

// Cada cuánto se dibuja un cuadro nuevo de la animación
#define CUADRO_MS 30

// ─── Cómo está la cara en este momento ───────────────────────────────────────
typedef struct {
    float x, y;               // corrimiento desde el centro (adónde mira)
    float ancho, alto;        // tamaño de cada ojo
    float abre_izq, abre_der; // 0 = cerrado, 1 = abierto
    float feliz;              // 0 = normal, 1 = ojitos felices ^ ^
    float enojo;              // 0 = normal, 1 = re enojado (cejas hacia adentro)
    float triste;             // 0 = normal, 1 = re triste (cejas hacia afuera)
    float espiral;            // 0 = ojos normales, 1 = ojos en espiral (mareado)
    float giro;               // cuánto giró la espiral
    float lagrima;            // 0 a 1: por dónde va cayendo la lágrima
    float zzz;                // 0 a 1: por dónde va subiendo la "Z" cuando duerme
} cara_t;

static const cara_t CARA_NORMAL = {
    .x = 0, .y = 0, .ancho = OJO_ANCHO, .alto = OJO_ALTO,
    .abre_izq = 1, .abre_der = 1, .feliz = 0,
};

static cara_t cara;

// ─── Ayudantes ───────────────────────────────────────────────────────────────

// Número al azar entre min y max (incluidos)
static int azar(int min, int max)
{
    return min + (int)(esp_random() % (uint32_t)(max - min + 1));
}

static void esperar(int ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

// Espiral que gira, para cuando Pitri está mareado
static void dibujar_espiral(float cx, float cy, float radio, float giro)
{
    const float vueltas = 2.5f;
    const float fin = vueltas * 2 * (float)M_PI;
    for (float a = 0; a < fin; a += 0.05f) {
        float r = radio * a / fin;
        int px = (int)(cx + r * cosf(a + giro));
        int py = (int)(cy + r * sinf(a + giro));
        display_fill_rect(px, py, 2, 2, true);   // trazo de 2 píxeles de grosor
    }
}

// Una gotita: círculo abajo y punta arriba
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

    // Enojado: un triángulo negro corta la esquina de arriba que da a la nariz
    if (cara.enojo > 0.01f) {
        int baja = (int)(h * 0.55f * cara.enojo);
        display_fill_triangle(x_afuera, y - 2, x_adentro, y - 2, x_adentro, y + baja, false);
    }

    // Triste: el triángulo corta la esquina de arriba del lado de afuera
    if (cara.triste > 0.01f) {
        int baja = (int)(h * 0.5f * cara.triste);
        display_fill_triangle(x_adentro, y - 2, x_afuera, y - 2, x_afuera, y + baja, false);
    }

    // Lágrima cayendo desde abajo del ojo hasta salir de la pantalla
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

static void dibujar(void)
{
    // Perspectiva: el ojo del lado hacia donde mira se ve un poquito más grande
    float p = (cara.x / MAX_X) * 0.12f;

    float cy     = LCD_V_RES / 2.0f + cara.y;
    float cx_izq = LCD_H_RES / 2.0f - OJO_SEPARACION / 2.0f - cara.ancho / 2.0f + cara.x;
    float cx_der = LCD_H_RES / 2.0f + OJO_SEPARACION / 2.0f + cara.ancho / 2.0f + cara.x;

    display_clear();
    dibujar_ojo(cx_izq, cy, cara.ancho, cara.alto * (1 - p), cara.abre_izq, -1);
    dibujar_ojo(cx_der, cy, cara.ancho, cara.alto * (1 + p), cara.abre_der, +1);

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
static void animar(cara_t destino, int ms)
{
    cara_t inicio = cara;
    int pasos = ms / CUADRO_MS;
    if (pasos < 1) pasos = 1;

    TickType_t ultimo = xTaskGetTickCount();
    for (int i = 1; i <= pasos; i++) {
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

        dibujar();
        xTaskDelayUntil(&ultimo, pdMS_TO_TICKS(CUADRO_MS));
    }
}

// Igual que animar, pero cambiando solo la posición
static void mover_a(float x, float y, int ms)
{
    cara_t d = cara;
    d.x = x;
    d.y = y;
    animar(d, ms);
}

// ─── Parpadeos ───────────────────────────────────────────────────────────────

static void cerrar_y_abrir(bool izq, bool der, int cerrado_ms)
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
    animar(d, 120);
    esperar(azar(500, 900));
    d.ancho = OJO_ANCHO;
    d.alto  = OJO_ALTO;
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
    animar(d, 200);
    mover_a(-MAX_X, 0, 500);
    esperar(300);
    mover_a(MAX_X, 0, 700);
    esperar(300);
    animar(CARA_NORMAL, 250);
}

// Le da sueño: se le cierran los ojos de a poco y después se despierta
static void juego_sueno(void)
{
    cara_t d = CARA_NORMAL;
    d.y = 4;
    d.abre_izq = d.abre_der = 0.3f;
    animar(d, 1200);
    d.abre_izq = d.abre_der = 0.0f;
    animar(d, 800);
    esperar(azar(800, 2000));
    animar(CARA_NORMAL, 100);   // ¡se despertó!
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

    // Vuelve en sí: cierra los ojos, sacude la cabeza y los abre normales
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

// Se pone triste y le caen lágrimas
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

typedef struct {
    const char *nombre;
    void (*hacer)(void);
    int peso;   // más peso = sale más seguido
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
};
#define CANT_JUEGOS ((int)(sizeof(juegos) / sizeof(juegos[0])))

// ─── Funciones públicas ──────────────────────────────────────────────────────

void eyes_init(void)
{
    cara = CARA_NORMAL;
    dibujar();
}

// ─── Dormir ──────────────────────────────────────────────────────────────────

void eyes_dormirse(void)
{
    ESP_LOGI(TAG, "Me voy a dormir... zzz");
    cara_t d = CARA_NORMAL;
    d.y = 4;
    d.abre_izq = d.abre_der = 0.3f;
    animar(d, 1200);                 // se le van cerrando los ojos
    d.abre_izq = d.abre_der = 0;
    animar(d, 800);
}

void eyes_dormido(void)
{
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
    ESP_LOGI(TAG, "¡Me desperté!");
    cara.zzz = 0;
    animar(CARA_NORMAL, 120);
    juego_doble_parpadeo();
}

void eyes_saludar(void)
{
    ESP_LOGI(TAG, "¡Hola! Saludando");
    animar(CARA_NORMAL, 150);   // deja lo que estaba haciendo y mira al frente
    juego_sorpresa();
    juego_contento();
}

void eyes_jugar(void)
{
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
