// ─────────────────────────────────────────────────────────────────────────────
//  Disfraces: los personajes de los modos de Pitri
// ─────────────────────────────────────────────────────────────────────────────
//
//  Hay dos clases de disfraces:
//
//   - Los que se ponen ENCIMA de los ojos de Pitri (la mayoria): Pitri sigue
//     teniendo sus ojos, y el disfraz agrega pelo, gorro, pico, orejas... y
//     cambia la boca. Son los que mejor quedan: se nota que es Pitri disfrazado.
//
//   - Los que son un DIBUJO ENTERO (fantasma, calabaza, arbolito): el disfraz
//     dibuja todo. La calabaza igual tiene huecos con la forma de los ojos de Pitri.
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

// Destello: una crucecita, como una estrellita que brilla
static void destello(float x, float y, float r)
{
    rect(x - r, y, 2 * r + 1, 1, true);
    rect(x, y - r, 1, 2 * r + 1, true);
}

// Copitos de nieve que caen por toda la pantalla (se mueven con cara.fase)
static void nieve(void)
{
    for (int i = 0; i < 14; i++) {
        float x = fmodf(i * 37 + 11, 128);
        float y = fmodf(i * 23 + cara.fase * 40, 70) - 3;
        rect(x, y, 2, 2, true);
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

// 🎃 Calabaza: con huecos con la forma de los ojos de Pitri, nariz, boca con dientes y un cabito
static void dibujar_calabaza(float mx, float cy)
{
    rect_redondo(mx - 46, cy - 24, 92, 52, 24, true);              // la calabaza
    arco(mx - 88, cy + 2, 52, -0.42f, 0.42f, 1, false);            // gajos
    arco(mx + 88, cy + 2, 52, PI - 0.42f, PI + 0.42f, 1, false);
    rect(mx - 3, cy - 31, 7, 9, true);                             // cabito
    linea(mx + 4, cy - 29, mx + 12, cy - 33, 2, true);             // hojita

    // Ojos: huecos con la forma de los ojos de Pitri (parpadean, como la luz de una vela)
    float h_izq = fmaxf(2, 20 * cara.abre_izq);
    float h_der = fmaxf(2, 20 * cara.abre_der);
    rect_redondo(mx - 28, cy - 5 - h_izq / 2, 18, h_izq, 6, false);
    rect_redondo(mx + 10, cy - 5 - h_der / 2, 18, h_der, 6, false);

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

// 🦇 Murcielago: alas con borde ondulado que aletean, orejitas y colmillitos
static void dibujar_ala(float mx, float cy, int lado, float aleteo)
{
    float ax = mx + lado * 34, ay = cy - 6;                      // hombro
    float bx = mx + lado * 62, by = cy - 18 + 6 * aleteo;        // punta del ala
    float px = mx + lado * 60, py = cy + 8 + 4 * aleteo;
    float dx = mx + lado * 34, dy = cy + 12;
    triangulo(ax, ay, bx, by, dx, dy, true);
    triangulo(bx, by, px, py, dx, dy, true);
    circulo(mx + lado * 52, cy + 12 + 2 * aleteo, 5, false);    // borde ondulado:
    circulo(mx + lado * 42, cy + 14 + aleteo, 5, false);        // dos "mordidas"
}

static void dibujar_murcielago(float mx, float cy)
{
    float aleteo = onda();
    dibujar_ala(mx, cy, -1, aleteo);
    dibujar_ala(mx, cy, +1, aleteo);

    triangulo(mx - 22, cy - 12, mx - 12, cy - 12, mx - 18, cy - 22, true);   // orejitas
    triangulo(mx + 22, cy - 12, mx + 12, cy - 12, mx + 18, cy - 22, true);

    arco(mx, cy + 12, 6, 0.25f * PI, 0.75f * PI, 1, true);                   // sonrisa
    triangulo(mx - 4, cy + 16, mx - 2, cy + 16, mx - 3, cy + 20, true);      // colmillitos
    triangulo(mx + 2, cy + 16, mx + 4, cy + 16, mx + 3, cy + 20, true);
}

static void actuar_murcielago(void)
{
    cara_t d = cara;
    float y_normal = d.y;

    for (int i = 0; i < 16; i++) {      // vuela: aletea, sube y baja, y va de un lado al otro
        d.fase += 0.25f;
        d.y = y_normal + 3 * sinf(i * 0.8f);
        d.x = 10 * sinf(i * 0.3f);
        animar(d, 100);
    }
    d.x = 0;
    d.y = y_normal;
    animar(d, 200);

    decir("¡Iiih!", (int)(LCD_H_RES / 2) - 17, 1, 1);
    for (int i = 0; i < 6; i++) {       // chilla aleteando rapido
        d.fase += 0.5f;
        animar(d, 80);
    }
    esperar(500);
    callar();
}

// 🧙 Brujita: sombrero en punta con hebilla y estrellitas que titilan alrededor
static void dibujar_brujita(float mx, float cy)
{
    // Sombrero: el cono (con la punta doblada) y el ala ancha
    triangulo(mx - 24, cy - 17, mx + 24, cy - 17, mx + 10, cy - 36, true);
    linea(mx + 10, cy - 35, mx + 20, cy - 31, 2, true);
    rect(mx - 22, cy - 22, 44, 2, false);                       // cinta
    rect(mx - 4, cy - 24, 8, 6, true);                          // hebilla
    rect(mx - 2, cy - 23, 4, 4, false);
    rect_redondo(mx - 44, cy - 17, 88, 4, 2, true);             // ala

    // Estrellitas: la mitad brilla y la otra mitad no, y van turnandose
    int turno = ((int)(cara.fase * 4)) % 2;
    if (turno == 0) { destello(mx - 52, cy - 2, 3);  destello(mx + 46, cy - 26, 2); }
    else            { destello(mx + 52, cy + 6, 3);  destello(mx - 46, cy + 16, 2); }

    arco(mx, cy + 10, 5, 0.25f * PI, 0.75f * PI, 1, true);      // sonrisa
}

static void actuar_brujita(void)
{
    cara_t d = cara;

    for (int i = 0; i < 8; i++) {       // mira las estrellitas que titilan
        d.fase += 0.25f;
        d.x = (i < 4) ? -8 : 8;
        animar(d, 200);
    }
    d.x = 0;
    animar(d, 200);

    float y_normal = d.y;
    decir("¡Abracadabra!", (int)(LCD_H_RES / 2) - 38, 56, 1);   // ¡hace magia!
    d.feliz = 1;
    for (int i = 0; i < 8; i++) {
        d.fase += 0.25f;
        d.y = y_normal + ((i % 2) ? -2 : 2);
        animar(d, 120);
    }
    esperar(600);
    callar();
    d.feliz = 0;
    d.y = y_normal;
    animar(d, 200);
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

    nieve();    // atras del arbol

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

// 🦌 Reno: cuernos ramificados, orejitas y una nariz que se prende y se apaga
static void dibujar_cuerno(float mx, float cy, int lado)
{
    linea(mx + lado * 14, cy - 12, mx + lado * 24, cy - 30, 2, true);   // rama principal
    linea(mx + lado * 19, cy - 21, mx + lado * 30, cy - 24, 2, true);   // ramitas
    linea(mx + lado * 22, cy - 26, mx + lado * 18, cy - 33, 2, true);
}

static void dibujar_reno(float mx, float cy)
{
    dibujar_cuerno(mx, cy, -1);
    dibujar_cuerno(mx, cy, +1);

    float aleteo = 2 * onda();                                          // orejitas
    rect_redondo(mx - 48, cy - 12 + aleteo, 14, 6, 3, true);
    rect_redondo(mx + 34, cy - 12 - aleteo, 14, 6, 3, true);

    // Nariz redonda: prendida (llena) o apagada (solo el borde)
    circulo(mx, cy + 15, 5, true);
    if (((int)(cara.fase * 4)) % 2) circulo(mx, cy + 15, 3, false);

    arco(mx, cy + 18, 6, 0.25f * PI, 0.75f * PI, 1, true);              // sonrisa
}

static void actuar_reno(void)
{
    cara_t d = cara;
    float y_normal = d.y;

    for (int i = 0; i < 8; i++) {       // la nariz titila y mueve las orejitas
        d.fase += 0.25f;
        animar(d, 200);
    }
    for (int i = 0; i < 3; i++) {       // salta, como tirando del trineo
        d.fase += 0.25f;
        d.y = y_normal - 5; animar(d, 150);
        d.fase += 0.25f;
        d.y = y_normal;     animar(d, 150);
    }
    for (int i = 0; i < 4; i++) {
        d.fase += 0.25f;
        animar(d, 200);
    }
}

// ⛄ Muneco de nieve: galera, nariz de zanahoria, boca de carbon, bufanda y nieve
static void dibujar_muneco_nieve(float mx, float cy)
{
    nieve();

    rect(mx - 17, cy - 33, 34, 18, true);                       // galera
    rect(mx - 17, cy - 19, 34, 2, false);                       // cinta
    rect(mx - 26, cy - 15, 52, 3, true);                        // ala

    triangulo(mx - 3, cy + 12, mx - 3, cy + 17, mx + 16, cy + 15, true);   // zanahoria

    for (int k = 0; k < 5; k++) {                               // boca de carbon
        float a = (0.25f + k * 0.125f) * PI;
        rect(mx + 8 * cosf(a) - 1, cy + 14 + 8 * sinf(a) - 1, 2, 2, true);
    }

    rect(mx - 30, cy + 24, 60, 5, true);                        // bufanda
    rect(mx + 14, cy + 24, 7, 10, true);
}

static void actuar_muneco_nieve(void)
{
    cara_t d = cara;
    float x_normal = d.x;

    for (int i = 0; i < 30; i++) {      // nieva
        d.fase += 0.1f;
        animar(d, 90);
    }

    decir("¡Brrr!", 92, 2, 1);          // ¡que frio! tiembla
    for (int i = 0; i < 12; i++) {
        d.fase += 0.1f;
        d.x = x_normal + ((i % 2) ? 1.5f : -1.5f);
        animar(d, 60);
    }
    d.x = x_normal;
    animar(d, 60);
    callar();
    for (int i = 0; i < 10; i++) {
        d.fase += 0.1f;
        animar(d, 90);
    }
}

// 🧝 Duende: gorro en punta con cascabel y orejas puntiagudas
static void dibujar_duende(float mx, float cy)
{
    float tilin = 2 * onda();           // el cascabel se mueve

    triangulo(mx - 26, cy - 16, mx + 26, cy - 16, mx + 6, cy - 38, true);   // gorro
    linea(mx + 6, cy - 36, mx + 30, cy - 28 + tilin, 4, true);              // punta caida
    circulo(mx + 33, cy - 26 + tilin, 3, true);                             // cascabel
    rect(mx + 33, cy - 26 + tilin, 1, 1, false);
    rect_redondo(mx - 28, cy - 18, 56, 5, 2, true);                         // borde

    triangulo(mx - 34, cy - 6, mx - 34, cy + 6, mx - 52, cy - 14, true);    // orejas
    triangulo(mx + 34, cy - 6, mx + 34, cy + 6, mx + 52, cy - 14, true);

    arco(mx, cy + 11, 7, 0.2f * PI, 0.8f * PI, 2, true);                    // sonrisa grande
}

static void actuar_duende(void)
{
    cara_t d = cara;
    float y_normal = d.y;

    for (int i = 0; i < 6; i++) {       // mira para todos lados, curioso
        d.fase += 0.25f;
        d.x = (i % 3 == 0) ? -10 : (i % 3 == 1) ? 10 : 0;
        animar(d, 250);
    }

    decir("¡Ji ji!", 2, 2, 1);          // se rie
    d.feliz = 1;
    for (int i = 0; i < 4; i++) {
        d.fase += 0.25f;
        d.y = y_normal - 3; animar(d, 120);
        d.fase += 0.25f;
        d.y = y_normal;     animar(d, 120);
    }
    esperar(400);
    callar();
    d.feliz = 0;
    animar(d, 200);
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

// 🐮 Vaca: cuernitos, orejas a los costados y un hocico grande con agujeritos
static void dibujar_vaca(float mx, float cy)
{
    float aleteo = 2 * onda();

    // Cuernitos arriba, apuntando para afuera
    triangulo(mx - 20, cy - 15, mx - 12, cy - 15, mx - 30, cy - 27, true);
    triangulo(mx + 20, cy - 15, mx + 12, cy - 15, mx + 30, cy - 27, true);

    // Orejas a los costados (se mueven un poquito), con una rayita adentro
    rect_redondo(mx - 58, cy - 12 + aleteo, 20, 8, 4, true);
    rect_redondo(mx + 38, cy - 12 - aleteo, 20, 8, 4, true);
    rect(mx - 54, cy - 9 + aleteo, 11, 2, false);
    rect(mx + 43, cy - 9 - aleteo, 11, 2, false);

    // Hocico grande con dos agujeritos y la boca (que se abre para el "¡Muu!")
    rect_redondo(mx - 20, cy + 14, 40, 18, 9, true);
    rect_redondo(mx - 12, cy + 18, 6, 5, 2, false);
    rect_redondo(mx + 6,  cy + 18, 6, 5, 2, false);
    if (cara.asombro > 0.5f) circulo(mx, cy + 27, 3, false);
    else                     arco(mx, cy + 22, 6, 0.25f * PI, 0.75f * PI, 1, false);
}

static void actuar_vaca(void)
{
    cara_t d = cara;
    float y_normal = d.y;

    for (int i = 0; i < 8; i++) {       // rumia: mueve el hocico de un lado al otro
        d.fase += 0.25f;
        d.x = (i % 2) ? 2 : -2;
        animar(d, 180);
    }
    d.x = 0;
    animar(d, 150);

    cerrar_y_abrir(true, true, 250);    // parpadea despacito, muy tranquila
    esperar(300);

    decir("¡Muu!", (int)(LCD_H_RES / 2) - 14, 1, 1);
    d = cara;
    d.asombro = 1;                      // abre la boca
    d.y = y_normal - 2;
    animar(d, 200);
    esperar(1000);
    callar();
    d.asombro = 0;
    d.y = y_normal;
    animar(d, 250);
}

// 🐱 Gato: orejas en punta (una se mueve), naricita, bigotes y boquita en "w"
static void dibujar_gato(float mx, float cy)
{
    float tic = 2 * onda();             // la oreja derecha se mueve

    triangulo(mx - 34, cy - 14, mx - 14, cy - 14, mx - 28, cy - 30, true);              // orejas
    triangulo(mx - 30, cy - 15, mx - 18, cy - 15, mx - 27, cy - 25, false);
    triangulo(mx + 34, cy - 14, mx + 14, cy - 14, mx + 28 + tic, cy - 30 + tic, true);
    triangulo(mx + 30, cy - 15, mx + 18, cy - 15, mx + 27 + tic, cy - 25 + tic, false);

    triangulo(mx - 3, cy + 14, mx + 3, cy + 14, mx, cy + 18, true);     // naricita

    if (cara.asombro > 0.5f) {                                          // boca: "o" o "w"
        circulo(mx, cy + 21, 2, true);
    } else {
        arco(mx - 3, cy + 18, 3, 0.1f * PI, 0.9f * PI, 1, true);
        arco(mx + 3, cy + 18, 3, 0.1f * PI, 0.9f * PI, 1, true);
    }

    for (int lado = -1; lado <= 1; lado += 2) {                         // bigotes
        linea(mx + lado * 12, cy + 16, mx + lado * 30, cy + 14, 1, true);
        linea(mx + lado * 12, cy + 19, mx + lado * 31, cy + 19, 1, true);
        linea(mx + lado * 12, cy + 22, mx + lado * 30, cy + 24, 1, true);
    }
}

static void actuar_gato(void)
{
    cara_t d = cara;
    float x_normal = d.x;

    for (int i = 0; i < 6; i++) {       // mueve la oreja
        d.fase += 0.25f;
        animar(d, 150);
    }

    decir("rrr", 96, 2, 1);             // ronronea temblando un poquito
    d.feliz = 0.6f;
    for (int i = 0; i < 14; i++) {
        d.x = x_normal + ((i % 2) ? 1 : -1);
        animar(d, 60);
    }
    d.x = x_normal;
    callar();
    animar(d, 100);

    decir("¡Miau!", (int)(LCD_H_RES / 2) - 17, 1, 1);
    d.feliz = 0;
    d.asombro = 1;
    animar(d, 200);
    esperar(900);
    callar();
    d.asombro = 0;
    animar(d, 200);
}

// 🐶 Perro: orejas largas caidas, nariz con brillito y la lengua afuera
static void dibujar_perro(float mx, float cy)
{
    float sacudida = 2 * onda();

    rect_redondo(mx - 46, cy - 14 + sacudida, 12, 30, 6, true);         // orejas
    rect_redondo(mx + 34, cy - 14 - sacudida, 12, 30, 6, true);

    rect_redondo(mx - 6, cy + 15, 12, 8, 4, true);                      // nariz
    circulo(mx - 2, cy + 17, 1, false);                                 // brillito

    linea(mx, cy + 23, mx, cy + 25, 1, true);                           // boca
    arco(mx - 3, cy + 23, 3, 0.1f * PI, 0.9f * PI, 1, true);
    arco(mx + 3, cy + 23, 3, 0.1f * PI, 0.9f * PI, 1, true);

    if (cara.asombro > 0.3f) {                                          // lengua afuera
        float largo = 7 + 2 * fabsf(onda());
        rect_redondo(mx - 4, cy + 26, 8, largo, 3, true);
        rect(mx, cy + 27, 1, largo - 3, false);
    }
}

static void actuar_perro(void)
{
    cara_t d = cara;
    float y_normal = d.y;

    d.asombro = 0.6f;                   // jadea con la lengua afuera
    for (int i = 0; i < 8; i++) {
        d.fase += 0.25f;
        animar(d, 150);
    }

    for (int i = 0; i < 4; i++) {       // sacude la cabeza
        d.fase += 0.25f;
        d.x = (i % 2) ? 4 : -4;
        animar(d, 100);
    }
    d.x = 0;
    d.asombro = 0;
    animar(d, 150);

    for (int i = 0; i < 2; i++) {       // ¡guau!
        decir("¡Guau!", (int)(LCD_H_RES / 2) - 17, 1, 1);
        d.y = y_normal - 3; animar(d, 120);
        d.y = y_normal;     animar(d, 150);
        esperar(250);
        callar();
        animar(d, 30);
        esperar(200);
    }
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
    //                        nombre           tapa   ancho      alto      y   enojo abre
    [DISFRAZ_VAMPIRO]      = { "vampiro",      false, OJO_ANCHO, OJO_ALTO, 0,  0.6f, 0.8f, dibujar_vampiro,      actuar_vampiro      },
    [DISFRAZ_FANTASMA]     = { "fantasma",     true,  OJO_ANCHO, OJO_ALTO, 0,  0,    1,    dibujar_fantasma,     actuar_fantasma     },
    [DISFRAZ_CALABAZA]     = { "calabaza",     true,  OJO_ANCHO, OJO_ALTO, 0,  0,    1,    dibujar_calabaza,     actuar_calabaza     },
    [DISFRAZ_MURCIELAGO]   = { "murcielago",   false, 22,        22,       0,  0,    1,    dibujar_murcielago,   actuar_murcielago   },
    [DISFRAZ_BRUJITA]      = { "brujita",      false, 24,        26,       4,  0,    1,    dibujar_brujita,      actuar_brujita      },
    [DISFRAZ_PAPA_NOEL]    = { "Papa Noel",    false, 24,        24,       0,  0,    1,    dibujar_papa_noel,    actuar_papa_noel    },
    [DISFRAZ_ARBOLITO]     = { "arbolito",     true,  OJO_ANCHO, OJO_ALTO, 0,  0,    1,    dibujar_arbolito,     actuar_arbolito     },
    [DISFRAZ_RENO]         = { "reno",         false, 22,        22,       2,  0,    1,    dibujar_reno,         actuar_reno         },
    [DISFRAZ_MUNECO_NIEVE] = { "muneco",       false, 22,        22,       2,  0,    1,    dibujar_muneco_nieve, actuar_muneco_nieve },
    [DISFRAZ_DUENDE]       = { "duende",       false, 22,        24,       4,  0,    1,    dibujar_duende,       actuar_duende       },
    [DISFRAZ_GALLINA]      = { "gallina",      false, 22,        22,      -3,  0,    1,    dibujar_gallina,      actuar_gallina      },
    [DISFRAZ_CERDO]        = { "cerdo",        false, 22,        24,      -6,  0,    1,    dibujar_cerdo,        actuar_cerdo        },
    [DISFRAZ_VACA]         = { "vaca",         false, 22,        24,      -6,  0,    1,    dibujar_vaca,         actuar_vaca         },
    [DISFRAZ_GATO]         = { "gato",         false, 22,        24,      -2,  0,    1,    dibujar_gato,         actuar_gato         },
    [DISFRAZ_PERRO]        = { "perro",        false, 22,        24,      -4,  0,    1,    dibujar_perro,        actuar_perro        },
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
