// ─────────────────────────────────────────────────────────────────────────────
//  Disfraces: los personajes de los modos de Pitri
// ─────────────────────────────────────────────────────────────────────────────
//
//  Hay dos clases de disfraces:
//
//   - Los que se ponen ENCIMA de los ojos de Pitri (vampiro, Papa Noel,
//     gallina, cerdo): Pitri sigue teniendo sus ojos, y el disfraz agrega
//     pelo, gorro, pico, orejas... y cambia la boca.
//
//   - Los que son un DIBUJO ENTERO (fantasma, calabaza, arbolito, galleta,
//     tortuga): el disfraz dibuja todo, con sus propios ojos.
//
//  Cada personaje tiene:
//   - una funcion dibujar_xxx(): como se ve en un cuadro
//   - una funcion actuar_xxx(): que hace (se mueve, habla...)
//
//  Los dibujos usan la cara de Pitri (cara.x, cara.y, cara.abre_izq...) para
//  moverse y parpadear, y cara.fase como "reloj" de su animacion propia.
//
#include "eyes_interno.h"
#include "display.h"

#include <math.h>
#include "esp_log.h"

static const char *TAG = "DISFRAZ";

#define PI ((float)M_PI)

eyes_disfraz_t disfraz_actual = DISFRAZ_NINGUNO;

// ─── Herramientas de dibujo (aceptan numeros con coma) ───────────────────────

static void rect(float x, float y, float w, float h, bool on)
{
    display_fill_rect((int)x, (int)y, (int)w, (int)h, on);
}

static void rect_redondo(float x, float y, float w, float h, float r, bool on)
{
    display_fill_round_rect((int)x, (int)y, (int)w, (int)h, (int)r, on);
}

static void circulo(float cx, float cy, float r, bool on)
{
    display_fill_circle((int)cx, (int)cy, (int)r, on);
}

static void triangulo(float x0, float y0, float x1, float y1, float x2, float y2, bool on)
{
    display_fill_triangle((int)x0, (int)y0, (int)x1, (int)y1, (int)x2, (int)y2, on);
}

// Pedazo de circulo (ver dibujar_arco en eyes.c), blanco o negro
static void arco(float cx, float cy, float r, float desde, float hasta, int grosor, bool on)
{
    for (float a = desde; a <= hasta; a += 0.04f)
        rect(cx + r * cosf(a) - grosor / 2, cy + r * sinf(a) - grosor / 2, grosor, grosor, on);
}

// Linea recta, blanca o negra
static void linea(float x0, float y0, float x1, float y1, int grosor, bool on)
{
    float dx = x1 - x0, dy = y1 - y0;
    int pasos = (int)fmaxf(fabsf(dx), fabsf(dy));
    if (pasos < 1) pasos = 1;
    for (int i = 0; i <= pasos; i++) {
        float t = (float)i / pasos;
        rect(x0 + dx * t, y0 + dy * t, grosor, grosor, on);
    }
}

// Estrella de 5 puntas con centro (cx, cy)
static void estrella(float cx, float cy, float r_afuera, float r_adentro)
{
    for (int k = 0; k < 5; k++) {
        float a     = -PI / 2 + k * 2 * PI / 5;     // punta
        float a_izq = a - PI / 5, a_der = a + PI / 5;
        triangulo(cx + r_afuera * cosf(a),      cy + r_afuera * sinf(a),
                  cx + r_adentro * cosf(a_izq), cy + r_adentro * sinf(a_izq),
                  cx + r_adentro * cosf(a_der), cy + r_adentro * sinf(a_der), true);
        triangulo(cx, cy,
                  cx + r_adentro * cosf(a_izq), cy + r_adentro * sinf(a_izq),
                  cx + r_adentro * cosf(a_der), cy + r_adentro * sinf(a_der), true);
    }
}

// Ondita de 0 a 1 que da una vuelta por cada +1 de cara.fase
static float onda(void)
{
    return sinf(cara.fase * 2 * PI);
}

// ─── Globo de texto: lo que dice el personaje ────────────────────────────────

static const char *globo_texto = NULL;
static int globo_x, globo_y, globo_escala;

static void decir(const char *texto, int x, int y, int escala)
{
    globo_texto = texto;
    globo_x = x;
    globo_y = y;
    globo_escala = escala;
}

static void callar(void)
{
    globo_texto = NULL;
}

// ═════════════════════════════════════════════════════════════════════════════
//  HALLOWEEN
// ═════════════════════════════════════════════════════════════════════════════

// 🧛 Vampiro: pelo con pico en la frente, cuello de capa y colmillos
static void dibujar_vampiro(float mx, float cy)
{
    float arriba = cy - 32;

    // Pelo peinado para atras, con el "pico" en el medio de la frente
    rect(mx - 46, arriba - 6, 92, 9, true);
    triangulo(mx - 22, arriba + 2, mx + 22, arriba + 2, mx, arriba + 12, true);

    // Cuello alto de la capa, a los costados
    triangulo(mx - 64, 64, mx - 34, 64, mx - 52, 36, true);
    triangulo(mx + 64, 64, mx + 34, 64, mx + 52, 36, true);

    // Boca con dos colmillos
    float y = fminf(cy + 18, 55);
    linea(mx - 12, y, mx + 12, y, 2, true);
    triangulo(mx - 9, y + 1, mx - 4, y + 1, mx - 6.5f, y + 7, true);
    triangulo(mx + 4, y + 1, mx + 9, y + 1, mx + 6.5f, y + 7, true);
}

static void actuar_vampiro(void)
{
    cara_t d = cara;

    mover_a(-16, 0, 700);               // mira a un lado... con cara de malo
    esperar(400);
    mover_a(16, 0, 900);                // ...y al otro
    esperar(400);
    mover_a(0, 0, 500);

    for (int i = 0; i < 3; i++) {       // sube y baja las cejas
        d = cara;
        d.enojo = 0.2f; animar(d, 120);
        d.enojo = 0.9f; animar(d, 120);
    }
    d.enojo = 0.6f;
    animar(d, 150);

    for (int i = 0; i < 4; i++) {       // se rie: muajaja
        mover_a(0, -2, 80);
        mover_a(0, 1, 80);
    }
    mover_a(0, 0, 100);
}

// 👻 Fantasma: sabana con cabeza redonda, borde ondulado y ojos huecos
static void dibujar_fantasma(float mx, float cy)
{
    float arriba = 4 + (cy - 32) + 3 * onda();      // flota para arriba y abajo
    float x0 = mx - 32;

    rect_redondo(x0, arriba, 64, 56, 29, true);     // cabeza redonda
    rect(x0, arriba + 28, 64, 30, true);            // cuerpo recto
    for (int k = 0; k < 5; k++)                     // borde ondulado de la sabana
        circulo(x0 + 6 + 13 * k, arriba + 59, 6, false);

    // Ojos: huecos negros que parpadean
    float h_izq = fmaxf(2, 14 * cara.abre_izq);
    float h_der = fmaxf(2, 14 * cara.abre_der);
    rect_redondo(mx - 17, arriba + 22 - h_izq / 2, 10, h_izq, 5, false);
    rect_redondo(mx + 7,  arriba + 22 - h_der / 2, 10, h_der, 5, false);

    // Boca: un hueco que se agranda para decir "¡Buu!"
    circulo(mx, arriba + 40, 4 + 4 * cara.asombro, false);
}

static void actuar_fantasma(void)
{
    cara_t d = cara;

    for (int i = 0; i < 16; i++) {      // flota de aca para alla
        d.fase += 0.125f;
        d.x = 10 * sinf(i * 0.4f);
        animar(d, 120);
    }

    d.x = -30;                          // se corre para hacer lugar...
    animar(d, 400);
    d.asombro = 1;
    animar(d, 120);
    decir("¡Buu!", 70, 24, 2);          // ...¡y asusta!
    for (int i = 0; i < 10; i++) {
        d.fase += 0.125f;
        animar(d, 120);
    }
    callar();

    d.asombro = 0;
    d.x = 0;
    animar(d, 500);
}

// 🎃 Calabaza: ojos y nariz triangulares, boca con dientes y un cabito
static void dibujar_calabaza(float mx, float cy)
{
    rect_redondo(mx - 46, cy - 24, 92, 52, 24, true);              // la calabaza
    arco(mx - 88, cy + 2, 52, -0.42f, 0.42f, 1, false);            // gajos
    arco(mx + 88, cy + 2, 52, PI - 0.42f, PI + 0.42f, 1, false);
    rect(mx - 3, cy - 31, 7, 9, true);                             // cabito
    linea(mx + 4, cy - 29, mx + 12, cy - 33, 2, true);             // hojita

    // Ojos triangulares (parpadean, como la luz de una vela)
    float h_izq = fmaxf(2, 14 * cara.abre_izq);
    float h_der = fmaxf(2, 14 * cara.abre_der);
    triangulo(mx - 30, cy - 2, mx - 12, cy - 2, mx - 21, cy - 2 - h_izq, false);
    triangulo(mx + 12, cy - 2, mx + 30, cy - 2, mx + 21, cy - 2 - h_der, false);

    triangulo(mx - 4, cy + 7, mx + 4, cy + 7, mx, cy + 1, false);   // nariz

    // Boca con las puntas para arriba y dos dientes
    rect(mx - 26, cy + 12, 52, 9, false);
    triangulo(mx - 26, cy + 12, mx - 26, cy + 21, mx - 33, cy + 8, false);
    triangulo(mx + 26, cy + 12, mx + 26, cy + 21, mx + 33, cy + 8, false);
    rect(mx - 14, cy + 12, 6, 4, true);     // diente de arriba
    rect(mx + 8,  cy + 17, 6, 4, true);     // diente de abajo
}

static void actuar_calabaza(void)
{
    cara_t d = cara;

    for (int i = 0; i < 4; i++) {       // se hamaca
        d.x = (i % 2) ? 6 : -6;
        animar(d, 400);
    }
    d.x = 0;
    animar(d, 300);

    for (int i = 0; i < 10; i++) {      // la vela de adentro titila
        d.abre_izq = d.abre_der = azar(0, 1) ? 1 : 0.55f;
        animar(d, 90);
    }
    d.abre_izq = d.abre_der = 1;
    animar(d, 100);
    esperar(400);
}

// ═════════════════════════════════════════════════════════════════════════════
//  NAVIDAD
// ═════════════════════════════════════════════════════════════════════════════

// 🎅 Papa Noel: gorro con pompon, barba, bigote y boca sonriente
static void dibujar_papa_noel(float mx, float cy)
{
    // Gorro: un cono que cae hacia la izquierda, con pompon en la punta
    triangulo(mx - 34, cy - 26, mx + 34, cy - 26, mx - 6, cy - 36, true);
    linea(mx - 6, cy - 35, mx - 44, cy - 24, 5, true);
    circulo(mx - 48, cy - 22, 5, true);
    rect_redondo(mx - 40, cy - 27, 80, 8, 3, true);    // borde de piel
    linea(mx - 38, cy - 28, mx + 38, cy - 28, 1, false);

    // Barba grandota
    rect_redondo(mx - 34, cy + 14, 68, 34, 14, true);

    // Bigote: dos lomitas con contorno negro, y la boca abajo
    arco(mx - 8, cy + 20, 8, PI, 2 * PI, 1, false);
    arco(mx + 8, cy + 20, 8, PI, 2 * PI, 1, false);
    arco(mx, cy + 22, 5, 0.2f * PI, 0.8f * PI, 2, false);
}

static void actuar_papa_noel(void)
{
    cara_t d = cara;

    d.x = -26;                          // se corre para hacer lugar al texto
    animar(d, 400);
    d.feliz = 1;
    animar(d, 150);

    static const char *const jo[] = { "Jo", "Jo\njo", "Jo\njo\njo!" };
    for (int i = 0; i < 3; i++) {       // un "jo" por cada saltito
        decir(jo[i], 84, 8, 2);
        d.y = -4; animar(d, 130);
        d.y = 0;  animar(d, 170);
    }
    esperar(900);
    callar();

    d.feliz = 0;
    d.x = 0;
    animar(d, 400);
}

// 🎄 Arbolito: tres pisos, estrella, luces que titilan y nieve que cae
static const int8_t luces_arbolito[][2] = {
    { -6, 20 }, { 6, 22 }, { -12, 32 }, { 4, 31 }, { 14, 34 },
    { -20, 48 }, { -6, 45 }, { 10, 48 }, { 24, 50 }, { 0, 40 },
};

static void dibujar_arbolito(float mx, float cy)
{
    float arriba = cy - 32;

    // Nieve que cae (atras del arbol)
    for (int i = 0; i < 14; i++) {
        float x = fmodf(i * 37 + 11, 128);
        float y = fmodf(i * 23 + cara.fase * 40, 70) - 3;
        rect(x, y, 2, 2, true);
    }

    // Tres pisos de hojas y el tronco
    triangulo(mx, arriba + 9,  mx - 16, arriba + 24, mx + 16, arriba + 24, true);
    triangulo(mx, arriba + 16, mx - 24, arriba + 38, mx + 24, arriba + 38, true);
    triangulo(mx, arriba + 27, mx - 34, arriba + 54, mx + 34, arriba + 54, true);
    rect(mx - 5, arriba + 54, 10, 9, true);

    // Luces: la mitad se prende y la otra mitad se apaga, y van turnandose
    int turno = ((int)(cara.fase * 4)) % 2;
    for (int k = 0; k < (int)(sizeof(luces_arbolito) / sizeof(luces_arbolito[0])); k++) {
        if (k % 2 == turno)
            circulo(mx + luces_arbolito[k][0], arriba + luces_arbolito[k][1], 2, false);
    }

    estrella(mx, arriba + 6, 6 + onda(), 2.5f);    // la estrella late un poquito
}

static void actuar_arbolito(void)
{
    cara_t d = cara;

    for (int i = 0; i < 50; i++) {      // nieva y titilan las luces
        d.fase += 0.1f;
        animar(d, 90);
    }

    d.x = -30;                          // se corre para el saludo
    animar(d, 500);
    decir("¡Feliz\nNavidad!", 74, 22, 1);
    for (int i = 0; i < 25; i++) {
        d.fase += 0.1f;
        animar(d, 90);
    }
    callar();

    d.x = 0;
    animar(d, 500);
}

// 🍪 Galleta de jengibre: cabeza, cuerpo, brazos y piernas, con botones y glaseado
static void dibujar_galleta(float mx, float cy)
{
    float arriba = cy - 32;
    float paso = 3 * onda();            // brazos y piernas se mueven al reves

    rect_redondo(mx - 12, arriba + 42 + paso, 10, 24, 4, true);   // piernas
    rect_redondo(mx + 2,  arriba + 42 - paso, 10, 24, 4, true);
    rect_redondo(mx - 29, arriba + 22 - paso, 18, 9, 4, true);    // brazos
    rect_redondo(mx + 11, arriba + 22 + paso, 18, 9, 4, true);
    rect_redondo(mx - 13, arriba + 19, 26, 28, 8, true);          // cuerpo
    circulo(mx, arriba + 11, 10, true);                           // cabeza

    // Glaseado en las puntas de los brazos y las piernas
    linea(mx - 24, arriba + 23 - paso, mx - 24, arriba + 29 - paso, 1, false);
    linea(mx + 23, arriba + 23 + paso, mx + 23, arriba + 29 + paso, 1, false);
    linea(mx - 11, arriba + 57 + paso, mx - 4,  arriba + 57 + paso, 1, false);
    linea(mx + 3,  arriba + 57 - paso, mx + 10, arriba + 57 - paso, 1, false);

    for (int k = 0; k < 3; k++)                                   // botones
        circulo(mx, arriba + 26 + 7 * k, 2, false);

    // Cara: ojitos (o rayitas si parpadea) y sonrisa
    if (cara.abre_izq > 0.4f) circulo(mx - 4, arriba + 9, 2, false);
    else                      linea(mx - 6, arriba + 9, mx - 2, arriba + 9, 1, false);
    if (cara.abre_der > 0.4f) circulo(mx + 4, arriba + 9, 2, false);
    else                      linea(mx + 2, arriba + 9, mx + 6, arriba + 9, 1, false);
    arco(mx, arriba + 11, 5, 0.2f * PI, 0.8f * PI, 1, false);
}

static void actuar_galleta(void)
{
    cara_t d = cara;

    for (int i = 0; i < 8; i++) {       // baila
        d.fase += 0.25f;
        d.y = (i % 2) ? -3 : 0;
        animar(d, 200);
    }
    d.y = 0;

    d.x = -34;                          // se corre y desafia
    animar(d, 400);
    decir("¡Atrapame!", 64, 28, 1);
    esperar(1300);
    callar();

    for (int i = 0; i < 12; i++) {      // ¡sale corriendo para la derecha!
        d.fase += 0.25f;
        d.x += 11;
        animar(d, 70);
    }
    cara.x = d.x = -100;                // y vuelve a aparecer por la izquierda
    for (int i = 0; i < 10; i++) {
        d.fase += 0.25f;
        d.x += 10;
        animar(d, 70);
    }
    d.x = 0;
    animar(d, 100);
}

// ═════════════════════════════════════════════════════════════════════════════
//  ANIMALES
// ═════════════════════════════════════════════════════════════════════════════

// 🐔 Gallina: cresta, pico (que se abre para cacarear) y moquito
static void dibujar_gallina(float mx, float cy)
{
    circulo(mx - 9, cy - 25, 6, true);                  // cresta
    circulo(mx,     cy - 28, 7, true);
    circulo(mx + 9, cy - 25, 6, true);

    float abre = 4 * cara.asombro;                      // pico
    triangulo(mx - 9, cy + 14, mx + 9, cy + 14, mx, cy + 7, true);
    triangulo(mx - 8, cy + 15 + abre, mx + 8, cy + 15 + abre, mx, cy + 23 + abre, true);

    circulo(mx + 3, cy + 29 + abre, 3, true);           // moquito
}

static void actuar_gallina(void)
{
    cara_t d = cara;
    float y_normal = d.y;

    for (int i = 0; i < 3; i++) {       // picotea el piso
        d.y = y_normal + 12;
        d.abre_izq = d.abre_der = 0.3f;
        animar(d, 120);
        d.y = y_normal;
        d.abre_izq = d.abre_der = 1;
        animar(d, 160);
        esperar(150);
    }

    for (int i = 0; i < 3; i++) {       // cacarea mirando a un lado y al otro
        decir("¡Coc!", (i % 2) ? 96 : 2, 2, 1);
        d.asombro = 1;
        d.x = (i % 2) ? 4 : -4;
        animar(d, 120);
        d.asombro = 0;
        animar(d, 220);
    }
    callar();
    d.x = 0;
    animar(d, 200);
}

// 🐷 Cerdo: orejas que aletean y hocico con dos agujeritos
static void dibujar_cerdo(float mx, float cy)
{
    float aleteo = 3 * onda();
    triangulo(mx - 34, cy - 18, mx - 48, cy - 6, mx - 56, cy - 26 + aleteo, true);   // orejas
    triangulo(mx + 34, cy - 18, mx + 48, cy - 6, mx + 56, cy - 26 - aleteo, true);

    rect_redondo(mx - 16, cy + 15, 32, 19, 9, true);    // hocico
    rect_redondo(mx - 9, cy + 20, 5, 9, 2, false);      // agujeritos de la nariz
    rect_redondo(mx + 4, cy + 20, 5, 9, 2, false);
}

static void actuar_cerdo(void)
{
    cara_t d = cara;
    float y_normal = d.y;

    for (int i = 0; i < 8; i++) {       // mueve el hocico y las orejas
        d.fase += 0.25f;
        d.x = (i % 2) ? 3 : -3;
        animar(d, 110);
    }
    d.x = 0;

    for (int i = 0; i < 2; i++) {       // ¡oink!
        decir("¡Oink!", (int)(LCD_H_RES / 2) - 17, 1, 1);
        d.y = y_normal - 2; animar(d, 120);
        d.y = y_normal;     animar(d, 200);
        esperar(300);
        callar();
        animar(d, 30);
        esperar(300);
    }
}

// 🐢 Tortuga (de costado): caparazon con dibujitos, patas, cola y cabeza que se esconde
static void dibujar_tortuga(float mx, float cy)
{
    float base = cy + 14;                       // linea de abajo del caparazon
    float paso = 2 * onda();                    // las patas caminan
    float hx   = mx + 34 - 16 * cara.asombro;   // cabeza: asombro 1 = escondida

    // Cuello y cabeza (se dibujan antes, asi el caparazon los tapa al esconderse)
    if (hx > mx + 20) rect_redondo(mx + 18, base - 13, hx - mx - 18, 8, 3, true);
    circulo(hx, base - 10, 8, true);
    if (cara.abre_der > 0.4f) circulo(hx + 2, base - 13, 1, false);
    else                      linea(hx, base - 13, hx + 4, base - 13, 1, false);
    arco(hx + 1, base - 9, 4, 0.1f * PI, 0.6f * PI, 1, false);

    // Caparazon: medio circulo, con un borde y dibujitos
    circulo(mx - 4, base, 26, true);
    rect(mx - 31, base + 1, 56, 27, false);     // borra la mitad de abajo
    rect_redondo(mx - 33, base - 3, 60, 6, 3, true);
    arco(mx - 4, base, 15, PI, 2 * PI, 1, false);
    for (int k = 1; k <= 3; k++) {
        float a = PI + k * PI / 4;
        linea(mx - 4 + 15 * cosf(a), base + 15 * sinf(a),
              mx - 4 + 25 * cosf(a), base + 25 * sinf(a), 1, false);
    }

    // Patas y cola
    rect_redondo(mx - 24, base + 1, 9, 12 + paso, 3, true);
    rect_redondo(mx + 8,  base + 1, 9, 12 - paso, 3, true);
    triangulo(mx - 30, base - 1, mx - 39, base + 3, mx - 30, base + 3, true);
}

static void actuar_tortuga(void)
{
    cara_t d = cara;

    cara.x = d.x = -100;                // entra caminando despacito desde la izquierda
    for (int i = 0; i < 24; i++) {
        d.fase += 0.2f;
        d.x += 100.0f / 24;
        animar(d, 150);
    }
    d.x = 0;
    animar(d, 100);
    esperar(400);
    cerrar_y_abrir(true, true, 100);    // parpadea

    d = cara;
    d.asombro = 1;                      // ¡se esconde en el caparazon!
    animar(d, 300);
    esperar(1200);
    d.asombro = 0;                      // y vuelve a asomarse
    animar(d, 500);
    esperar(300);
    cerrar_y_abrir(true, true, 80);
}

// ═════════════════════════════════════════════════════════════════════════════
//  La lista de personajes
// ═════════════════════════════════════════════════════════════════════════════

typedef struct {
    const char *nombre;
    bool tapa_cara;                         // ¿es un dibujo entero (sin los ojos de Pitri)?
    float ancho, alto, y;                   // tamaño y altura de los ojos de Pitri (si los usa)
    float enojo, abre;                      // cara de base
    void (*dibujar)(float mx, float cy);
    void (*actuar)(void);
} personaje_t;

static const personaje_t personajes[] = {
    [DISFRAZ_VAMPIRO]   = { "vampiro",    false, OJO_ANCHO, OJO_ALTO, 0, 0.6f, 0.8f, dibujar_vampiro,   actuar_vampiro   },
    [DISFRAZ_FANTASMA]  = { "fantasma",   true,  OJO_ANCHO, OJO_ALTO, 0, 0,    1,    dibujar_fantasma,  actuar_fantasma  },
    [DISFRAZ_CALABAZA]  = { "calabaza",   true,  OJO_ANCHO, OJO_ALTO, 0, 0,    1,    dibujar_calabaza,  actuar_calabaza  },
    [DISFRAZ_PAPA_NOEL] = { "Papa Noel",  false, 24,        24,       0, 0,    1,    dibujar_papa_noel, actuar_papa_noel },
    [DISFRAZ_ARBOLITO]  = { "arbolito",   true,  OJO_ANCHO, OJO_ALTO, 0, 0,    1,    dibujar_arbolito,  actuar_arbolito  },
    [DISFRAZ_GALLETA]   = { "galleta",    true,  OJO_ANCHO, OJO_ALTO, 0, 0,    1,    dibujar_galleta,   actuar_galleta   },
    [DISFRAZ_GALLINA]   = { "gallina",    false, 22,        22,      -3, 0,    1,    dibujar_gallina,   actuar_gallina   },
    [DISFRAZ_CERDO]     = { "cerdo",      false, 22,        24,      -6, 0,    1,    dibujar_cerdo,     actuar_cerdo     },
    [DISFRAZ_TORTUGA]   = { "tortuga",    true,  OJO_ANCHO, OJO_ALTO, 0, 0,    1,    dibujar_tortuga,   actuar_tortuga   },
};
#define CANT_PERSONAJES ((int)(sizeof(personajes) / sizeof(personajes[0])))

static bool es_valido(eyes_disfraz_t d)
{
    return d > DISFRAZ_NINGUNO && d < CANT_PERSONAJES;
}

// La cara de base de un personaje (posicion, tamaño de ojos...)
static cara_t cara_de(eyes_disfraz_t d)
{
    cara_t c = CARA_NORMAL;
    if (!es_valido(d)) return c;
    c.ancho = personajes[d].ancho;
    c.alto  = personajes[d].alto;
    c.y     = personajes[d].y;
    c.enojo = personajes[d].enojo;
    c.abre_izq = c.abre_der = personajes[d].abre;
    return c;
}

// ─── Lo que usa eyes.c ───────────────────────────────────────────────────────

bool disfraz_tapa_cara(void)
{
    return es_valido(disfraz_actual) && personajes[disfraz_actual].tapa_cara;
}

void dibujar_disfraz(float cy)
{
    if (!es_valido(disfraz_actual)) return;
    personajes[disfraz_actual].dibujar(LCD_H_RES / 2.0f + cara.x, cy);

    if (globo_texto != NULL)
        display_draw_text(globo_x, globo_y, globo_texto, globo_escala, true);
}

// ─── Lo que usa main.c ───────────────────────────────────────────────────────

// Cierra los ojos, cambia de disfraz y los vuelve a abrir
static void cambiar_a(eyes_disfraz_t nuevo)
{
    cara_t d = cara;
    d.abre_izq = d.abre_der = 0;
    animar(d, 150);

    disfraz_actual = nuevo;
    callar();
    cara = cara_de(nuevo);
    cara.abre_izq = cara.abre_der = 0;
    animar(cara_de(nuevo), 200);
}

void eyes_actuar(eyes_disfraz_t personaje)
{
    if (!es_valido(personaje)) return;
    empezar_accion();

    if (personaje != disfraz_actual) cambiar_a(personaje);

    ESP_LOGI(TAG, "Actuando de %s", personajes[personaje].nombre);
    personajes[personaje].actuar();
    callar();
}

void eyes_sacar_disfraz(void)
{
    if (disfraz_actual == DISFRAZ_NINGUNO) return;
    empezar_accion();
    cambiar_a(DISFRAZ_NINGUNO);
}
