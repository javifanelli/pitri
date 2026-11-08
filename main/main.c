#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"

#include "pitri_config.h"
#include "board_config.h"
#include "display.h"
#include "eyes.h"
#include "motion.h"
#include "dht.h"
#include "wifi.h"
#include "botones.h"
#include "menu.h"
#include "reloj.h"
#include "telegram.h"
#include "familia.h"

static const char *TAG = "PITRI";

// ─── Modos ───────────────────────────────────────────────────────────────────
// En el modo normal Pitri hace sus juegos. En los otros, hace de personajes.

typedef enum {
    MODO_NORMAL,
    MODO_HALLOWEEN,
    MODO_NAVIDAD,
    MODO_ANIMALES,
    CANT_MODOS
} modo_t;

static const char *const nombres_modos[CANT_MODOS] = {
    "Normal", "Halloween", "Navidad", "Animales",
};

// Los personajes de cada modo
#define PERSONAJES_POR_MODO 5
static const eyes_disfraz_t personajes[CANT_MODOS][PERSONAJES_POR_MODO] = {
    [MODO_HALLOWEEN] = { DISFRAZ_VAMPIRO,   DISFRAZ_FANTASMA, DISFRAZ_CALABAZA,     DISFRAZ_MURCIELAGO, DISFRAZ_BRUJITA },
    [MODO_NAVIDAD]   = { DISFRAZ_PAPA_NOEL, DISFRAZ_ARBOLITO, DISFRAZ_RENO,         DISFRAZ_MUNECO_NIEVE, DISFRAZ_DUENDE },
    [MODO_ANIMALES]  = { DISFRAZ_GALLINA,   DISFRAZ_CERDO,    DISFRAZ_VACA,         DISFRAZ_GATO,       DISFRAZ_PERRO   },
};

static modo_t modo = MODO_NORMAL;
static eyes_disfraz_t ultimo_personaje = DISFRAZ_NINGUNO;

// Elige un personaje del modo actual, distinto del ultimo que hizo
static eyes_disfraz_t elegir_personaje(void)
{
    if (modo == MODO_NORMAL) return DISFRAZ_NINGUNO;   // el modo normal no tiene personajes

    eyes_disfraz_t p;
    do {
        p = personajes[modo][esp_random() % PERSONAJES_POR_MODO];
    } while (p == ultimo_personaje);
    ultimo_personaje = p;
    return p;
}

// ─── Pantallas de texto ──────────────────────────────────────────────────────

// Muestra un texto centrado en la pantalla durante "ms" milisegundos
static void mostrar_texto(const char *texto, int tamano, int ms)
{
    if (texto[0] == '\0') return;   // texto vacio: no mostrar nada
    display_clear();
    display_draw_text_centered(texto, tamano, true);
    display_flush();
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void mostrar_mensaje(const char *texto)
{
    mostrar_texto(texto, MENSAJE_ARRANQUE_TAMANO, MENSAJE_ARRANQUE_MS);
}

// ─── El menu ─────────────────────────────────────────────────────────────────
//   Menu:  Modo / Comer / Tomar / Hora / Sensor / Mensaje / Familia / Salir
//   Comer: Torta / Papas fritas / Hamburguesa / Atras (con dibujos)
//   Tomar: Agua / Cafe / Mate / Atras (con dibujos)
//   Modo:  Normal / Halloween / Navidad / Animales / Volver

static bool atendiendo_familia = false;   // true mientras reacciona a lo que le dijo la familia

// ¿Hay que cortar lo que esta haciendo? Si aprietan el boton 1 (menu), el 2 con
// un mensaje sin leer, o si le hablo alguien de la familia (salvo que ya este
// contestandole a alguien). Asi los ojos cortan lo que estan haciendo enseguida.
static bool hay_boton_para_atender(void)
{
    return botones_pendiente(1) || (botones_pendiente(2) && telegram_sin_leer() > 0) ||
           (!atendiendo_familia && familia_hay_evento());
}

static void cambiar_modo(modo_t nuevo)
{
    modo = nuevo;
    ultimo_personaje = DISFRAZ_NINGUNO;
    ESP_LOGI(TAG, "Modo %s", nombres_modos[modo]);

    char texto[32];
    snprintf(texto, sizeof(texto), "Modo\n%s", nombres_modos[modo]);
    mostrar_texto(texto, 2, 1500);
}

// Muestra la hora grande, el dia y la fecha durante MOSTRAR_HORA_MS.
// Si se aprieta cualquier boton, sale antes.
static void mostrar_hora(void)
{
    static const char *const dias[7] = {
        "Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado",
    };

    struct tm t;
    if (!reloj_hora(&t)) {
        mostrar_texto(wifi_conectado() ? "Buscando\nla hora..." : "Sin WiFi\nno tengo\nhora", 2, 2500);
        return;
    }

    ESP_LOGI(TAG, "Mostrando la hora");
    botones_se_apreto(1);
    botones_se_apreto(2);

    for (int ms = 0; ms < MOSTRAR_HORA_MS; ms += 250) {
        if (botones_se_apreto(1) || botones_se_apreto(2)) break;
        reloj_hora(&t);

        // Los dos puntos titilan: aparecen un segundo si y otro no
        char hora[32], fecha[40];
        snprintf(hora, sizeof(hora), "%02d%c%02d", t.tm_hour, (t.tm_sec % 2) ? ' ' : ':', t.tm_min);
        snprintf(fecha, sizeof(fecha), "%02d/%02d/%04d", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);

        display_clear();
        display_draw_text((LCD_H_RES - display_text_width(hora, 3)) / 2, 4, hora, 3, true);
        display_draw_text((LCD_H_RES - display_text_width(dias[t.tm_wday], 2)) / 2, 32, dias[t.tm_wday], 2, true);
        display_draw_text((LCD_H_RES - display_text_width(fecha, 1)) / 2, 53, fecha, 1, true);
        display_flush();

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

// Muestra la temperatura grande y la humedad, con el mismo estilo que la hora,
// durante MOSTRAR_HORA_MS. Si se aprieta cualquier boton, sale antes.
static void mostrar_sensor(void)
{
    int16_t t, h;
    if (!dht_ultima_lectura(&t, &h)) {
        mostrar_texto("Todavia no\nhay datos", 2, 2500);
        return;
    }

    ESP_LOGI(TAG, "Mostrando el sensor");
    botones_se_apreto(1);
    botones_se_apreto(2);

    for (int ms = 0; ms < MOSTRAR_HORA_MS; ms += 250) {
        if (botones_se_apreto(1) || botones_se_apreto(2)) break;
        dht_ultima_lectura(&t, &h);

        char temp[32], hum[32];
        snprintf(temp, sizeof(temp), "%s%d.%d°C", t < 0 ? "-" : "", abs(t) / 10, abs(t) % 10);
        snprintf(hum, sizeof(hum), "Hum %d%%", (h + 5) / 10);
        const char *abajo = "Temperatura y humedad";

        display_clear();
        display_draw_text((LCD_H_RES - display_text_width(temp, 3)) / 2, 4, temp, 3, true);
        display_draw_text((LCD_H_RES - display_text_width(hum, 2)) / 2, 32, hum, 2, true);
        display_draw_text((LCD_H_RES - display_text_width(abajo, 1)) / 2, 53, abajo, 1, true);
        display_flush();

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

// ─── Mensajes de Telegram ────────────────────────────────────────────────────
// Cuando llega un mensaje, Pitri avisa y aparece un sobrecito arriba a la
// derecha. Sofi lo lee con el boton 2 (o desde el menu, en "Mensaje"). Cuando
// lo termina de leer y elige "Salir", el mensaje se borra.

#define LETRAS_POR_RENGLON   21     // con letra chica (6 pixeles cada letra)
#define RENGLONES_POR_PAGINA 6
#define MAX_RENGLONES        40

static unsigned recibidos_avisados = 0;   // hasta que mensaje ya aviso

static bool hay_mensaje_sin_leer(void)
{
    return telegram_sin_leer() > 0;
}

// Parte el texto en renglones de hasta LETRAS_POR_RENGLON letras, cortando en
// los espacios para no partir las palabras. Devuelve cuantos renglones quedaron.
static int partir_en_renglones(const char *texto, char renglones[][LETRAS_POR_RENGLON * 2 + 1], int max)
{
    int n = 0;
    const char *p = texto;
    while (*p && n < max) {
        // Hasta donde entra en este renglon (¡ ¿ ° ocupan 2 bytes, pero son una letra)
        const char *fin = p, *ultimo_espacio = NULL;
        int cuenta = 0;
        while (*fin && *fin != '\n') {
            if (((unsigned char)*fin & 0xC0) != 0x80) {
                if (cuenta == LETRAS_POR_RENGLON) break;
                cuenta++;
            }
            if (*fin == ' ') ultimo_espacio = fin;
            fin++;
        }
        const char *corte = fin;
        if (*fin && *fin != '\n' && ultimo_espacio != NULL) corte = ultimo_espacio;
        int bytes = (int)(corte - p);
        memcpy(renglones[n], p, bytes);
        renglones[n][bytes] = '\0';
        n++;
        p = corte;
        if (*p == ' ' || *p == '\n') p++;     // el espacio o el enter no pasan al renglon siguiente
    }
    return n;
}

// Como termino la lectura de un mensaje
typedef enum {
    LECTURA_TERMINADA,   // llego al final y eligio "Salir": se borra
    LECTURA_CORTADA,     // apreto el boton 1 antes del final: queda sin leer
    LECTURA_SE_CERRO,    // nadie toco nada por LEER_MENSAJE_MAX_MS: se borra y vuelve a la cara
} lectura_t;

// Muestra un mensaje: el titulo arriba y el texto abajo, de a 6 renglones.
// Despues del ultimo renglon aparece la opcion "Salir".
//   Boton 2 = bajar. Cuando ya se ve "Salir", el boton 2 (o el 1) la elige.
//   Boton 1 antes de llegar al final = cerrar sin terminar (el mensaje queda sin leer).
//   Si pasa LEER_MENSAJE_MAX_MS sin que se apriete ningun boton, se cierra solo.
//   Cada vez que se aprieta un boton, la cuenta vuelve a empezar.
static lectura_t leer_mensaje(const char *texto)
{
    static char renglones[MAX_RENGLONES][LETRAS_POR_RENGLON * 2 + 1];
    int n = partir_en_renglones(texto, renglones, MAX_RENGLONES);
    int total = n + 1;                                  // los renglones y "Salir"
    int arriba_max = (total > RENGLONES_POR_PAGINA) ? total - RENGLONES_POR_PAGINA : 0;

    ESP_LOGI(TAG, "Sofi lee un mensaje");
    botones_se_apreto(1);
    botones_se_apreto(2);

    int arriba = 0, quieto_ms = 0;                      // "arriba" = primer renglon que se ve
    bool redibujar = true;
    while (quieto_ms < LEER_MENSAJE_MAX_MS) {
        bool se_ve_salir = (arriba == arriba_max);
        if (redibujar) {
            display_clear();
            display_draw_text(0, 0, MENSAJE_TITULO, 1, true);
            if (!se_ve_salir)                           // flechita: hay mas abajo
                display_fill_triangle(LCD_H_RES - 8, 2, LCD_H_RES - 2, 2, LCD_H_RES - 5, 6, true);
            display_fill_rect(0, 9, LCD_H_RES, 1, true);
            for (int r = 0; r < RENGLONES_POR_PAGINA; r++) {
                int i = arriba + r, y = 12 + r * 9;
                if (i < n) {
                    display_draw_text(1, y, renglones[i], 1, true);
                } else if (i == n) {                    // "Salir", en una barra blanca
                    display_fill_rect(0, y - 1, LCD_H_RES, 9, true);
                    display_draw_text((LCD_H_RES - display_text_width("Salir", 1)) / 2, y, "Salir", 1, false);
                }
            }
            display_flush();
            redibujar = false;
        }
        if (botones_se_apreto(2)) {
            if (se_ve_salir) return LECTURA_TERMINADA;  // eligio "Salir": lo leyo entero
            arriba += RENGLONES_POR_PAGINA;             // baja una pagina
            if (arriba > arriba_max) arriba = arriba_max;
            redibujar = true;
            quieto_ms = 0;
        }
        if (botones_se_apreto(1))                       // cerrar (si ya se veia "Salir", cuenta como leido)
            return se_ve_salir ? LECTURA_TERMINADA : LECTURA_CORTADA;
        vTaskDelay(pdMS_TO_TICKS(50));
        quieto_ms += 50;
    }
    ESP_LOGI(TAG, "Nadie toco nada: el mensaje se cierra solo");
    return LECTURA_SE_CERRO;
}

// Lee los mensajes sin leer, uno atras del otro. Cada uno que se lee entero se
// borra (y se va el sobrecito cuando no queda ninguno). Si Sofi cierra uno antes
// de terminarlo, ese y los que siguen quedan para despues. Si se cierra solo
// (nadie toco nada), se borra y vuelve a la cara; los que siguen quedan.
static void leer_mensajes_nuevos(void)
{
    char texto[TELEGRAM_MAX_LETRAS + 1];
    while (telegram_ver(texto, sizeof(texto))) {
        lectura_t como = leer_mensaje(texto);
        if (como == LECTURA_CORTADA) break;
        telegram_borrar();
        ESP_LOGI(TAG, "Mensaje borrado");
        if (como == LECTURA_SE_CERRO) break;
    }
    recibidos_avisados = telegram_recibidos();
    botones_se_apreto(1);
    botones_se_apreto(2);
}

// Desde el menu: los mensajes sin leer (o un aviso si no hay)
static void mensajes_desde_menu(void)
{
    if (telegram_sin_leer() > 0) leer_mensajes_nuevos();
    else mostrar_texto(telegram_configurado() ? "No hay\nmensajes" : "Sin\nTelegram", 2, 2000);
}

// Pitri se sorprende y avisa que hay un mensaje
static void avisar_mensaje(void)
{
    recibidos_avisados = telegram_recibidos();
    ESP_LOGI(TAG, "Aviso que hay un mensaje");
    familia_mandar("*", FAMILIA_MENSAJE, NULL);    // ¡que la familia tambien le avise a Sofi!
    eyes_sacar_disfraz();
    eyes_saludar();
    eyes_mostrar_datos("¡Mensaje\nnuevo!\nApreta\nboton 2", 1, MOSTRAR_AVISO_MENSAJE_MS);
}

// ─── Comer y tomar ───────────────────────────────────────────────────────────
// Pitri puede comer hasta COMIDAS_POR_HORA veces y tomar hasta BEBIDAS_POR_HORA
// veces en una hora. Si ya llego a eso, esta lleno y dice que no. No le pasa
// nada malo si no come ni toma: Pitri nunca muere.

static const eyes_comida_t comidas[] = { COMIDA_TORTA, COMIDA_PAPAS, COMIDA_HAMBURGUESA };
static const char *const nombres_comidas[] = { "Torta", "Papas fritas", "Hamburguesa" };
#define CANT_COMIDAS 3

static const eyes_comida_t bebidas[] = { COMIDA_BOTELLA, COMIDA_TAZA, COMIDA_MATE };
static const char *const nombres_bebidas[] = { "Agua", "Cafe", "Mate" };
#define CANT_BEBIDAS 3

// Cuando comio y cuando tomo las ultimas veces (microsegundos desde que se prendio)
static int64_t cuando_comio[COMIDAS_POR_HORA];
static int64_t cuando_tomo[BEBIDAS_POR_HORA];

// Cuantas de las veces anotadas fueron en los ultimos 60 minutos
static int veces_en_la_ultima_hora(const int64_t *veces, int cantidad)
{
    const int64_t una_hora_us = 60LL * 60 * 1000 * 1000;
    int64_t ahora = esp_timer_get_time();
    int cuantas = 0;
    for (int i = 0; i < cantidad; i++)
        if (veces[i] != 0 && ahora - veces[i] < una_hora_us) cuantas++;
    return cuantas;
}

// Anota "ahora" en el lugar de la vez mas vieja
static void anotar(int64_t *veces, int cantidad)
{
    int mas_vieja = 0;
    for (int i = 1; i < cantidad; i++)
        if (veces[i] < veces[mas_vieja]) mas_vieja = i;
    veces[mas_vieja] = esp_timer_get_time();
}

static void dar_de_comer(int cual)
{
    if (veces_en_la_ultima_hora(cuando_comio, COMIDAS_POR_HORA) >= COMIDAS_POR_HORA) {
        ESP_LOGI(TAG, "Pitri esta lleno (ya comio %d veces en la ultima hora)", COMIDAS_POR_HORA);
        eyes_decir_que_no();
        return;
    }
    anotar(cuando_comio, COMIDAS_POR_HORA);
    ESP_LOGI(TAG, "Pitri come %s (%d de %d en la ultima hora)", nombres_comidas[cual],
             veces_en_la_ultima_hora(cuando_comio, COMIDAS_POR_HORA), COMIDAS_POR_HORA);
    eyes_comer(comidas[cual]);
}

static void dar_de_tomar(int cual)
{
    if (veces_en_la_ultima_hora(cuando_tomo, BEBIDAS_POR_HORA) >= BEBIDAS_POR_HORA) {
        ESP_LOGI(TAG, "Pitri esta lleno (ya tomo %d veces en la ultima hora)", BEBIDAS_POR_HORA);
        eyes_decir_que_no();
        return;
    }
    anotar(cuando_tomo, BEBIDAS_POR_HORA);
    ESP_LOGI(TAG, "Pitri toma %s (%d de %d en la ultima hora)", nombres_bebidas[cual],
             veces_en_la_ultima_hora(cuando_tomo, BEBIDAS_POR_HORA), BEBIDAS_POR_HORA);
    eyes_tomar(bebidas[cual]);
}

// Dibuja una opcion de los menus "Comer" y "Tomar": el dibujo grande con su
// nombre abajo, o la palabra "Atras" si ya se pasaron todas
static void dibujar_opcion(const eyes_comida_t *lista, const char *const *nombres, int cantidad, int opcion)
{
    if (opcion < cantidad) {
        eyes_dibujar_comida(lista[opcion], LCD_H_RES / 2.0f, 32, 1.1f, 0);
        display_draw_text((LCD_H_RES - display_text_width(nombres[opcion], 1)) / 2, 55, nombres[opcion], 1, true);
    } else {
        display_draw_text((LCD_H_RES - display_text_width("Atras", 3)) / 2, 27, "Atras", 3, true);
    }
}

static void dibujar_opcion_comida(int opcion) { dibujar_opcion(comidas, nombres_comidas, CANT_COMIDAS, opcion); }
static void dibujar_opcion_bebida(int opcion) { dibujar_opcion(bebidas, nombres_bebidas, CANT_BEBIDAS, opcion); }

// ─── La familia (Pitri, Flufi, ...) ──────────────────────────────────────────
// Los personajes de la casa se hablan por la red WiFi (ver docs/FAMILIA.md).

static int64_t proxima_iniciativa_us = 0;   // cuando le toca empezar una charla o un juego

// Dice algo de la familia al costado de la cara
static void decir_familia(const char *texto)
{
    eyes_sacar_disfraz();
    eyes_mostrar_datos(texto, 1, MOSTRAR_FAMILIA_MS);
}

// La cara que acompaña a un renglon de una charla
static void emocionarse(familia_emocion_t emocion)
{
    switch (emocion) {
    case FAMILIA_EMOCION_CONTENTA:
    case FAMILIA_EMOCION_RISA: eyes_jugar_juego("contento"); break;
    case FAMILIA_EMOCION_AMOR: eyes_enamorarse();             break;
    default:                                                  break;
    }
}

// Dice el renglon "renglon" de la charla y le pasa el turno a "con"
static void decir_renglon(int charla, int renglon, const char *con)
{
    familia_emocion_t emocion;
    const char *texto = familia_charla(charla, renglon, &emocion);
    if (texto == NULL) return;
    decir_familia(texto);
    emocionarse(emocion);
    if (familia_charla(charla, renglon + 1, NULL) != NULL) {      // si sigue, le toca al otro
        char dato[16];
        snprintf(dato, sizeof(dato), "%d %d", charla, renglon + 1);
        familia_mandar(con, FAMILIA_CHARLA, dato);
    }
}

static void empezar_charla(const char *con)
{
    ESP_LOGI(TAG, "Empiezo una charla con %s", con);
    bool antes = atendiendo_familia;
    atendiendo_familia = true;          // que no se corte lo que dice
    decir_renglon((int)(esp_random() % familia_cant_charlas()), 0, con);
    atendiendo_familia = antes;
}

// Le propone un juego a "con" y lo hacen los dos a la vez
static void jugar_con(const char *con)
{
    const char *juego = familia_juego_junto((int)(esp_random() % familia_cant_juegos_juntos()));
    ESP_LOGI(TAG, "Juego a \"%s\" con %s", juego, con);
    familia_mandar(con, FAMILIA_JUGAR, juego);
    bool antes = atendiendo_familia;
    atendiendo_familia = true;          // que no se corte el juego
    eyes_jugar_juego(juego);
    atendiendo_familia = antes;
}

// Cada tanto, si hay alguien conectado, empieza una charla o un juego
static void iniciativa_familia(void)
{
    int64_t ahora = esp_timer_get_time();
    int64_t espera = (FAMILIA_INICIATIVA_MIN_S +
                      esp_random() % (FAMILIA_INICIATIVA_MAX_S - FAMILIA_INICIATIVA_MIN_S)) * 1000000LL;
    if (proxima_iniciativa_us == 0) proxima_iniciativa_us = ahora + espera;
    if (ahora < proxima_iniciativa_us) return;
    proxima_iniciativa_us = ahora + espera;

    char con[FAMILIA_MAX_NOMBRE];
    if (!familia_alguien_al_azar(con)) return;
    if (esp_random() % 3 == 0) jugar_con(con);       // a veces juegan, y mas seguido charlan
    else                       empezar_charla(con);
}

// Reacciona a lo que le dijeron los demas
static void atender_familia(void)
{
    familia_evento_t e;
    char texto[64];
    atendiendo_familia = true;          // mientras reacciona, lo que llega espera su turno
    while (familia_proximo_evento(&e)) {
        switch (e.tipo) {
        case FAMILIA_HOLA:
        case FAMILIA_SALUDO:
            eyes_saludar();
            snprintf(texto, sizeof(texto), "¡Hola\n%s!", e.de);
            decir_familia(texto);
            break;
        case FAMILIA_BESO:
            eyes_enamorarse();
            snprintf(texto, sizeof(texto), "¡Un beso\nde\n%s!", e.de);
            decir_familia(texto);
            break;
        case FAMILIA_CHARLA: {
            int charla, renglon;
            if (sscanf(e.dato, "%d %d", &charla, &renglon) == 2) decir_renglon(charla, renglon, e.de);
            break;
        }
        case FAMILIA_VISITA:
            eyes_saludar();
            decir_familia("¡Llego\nalguien!");
            break;
        case FAMILIA_COMIDA:
            snprintf(texto, sizeof(texto), "¡Gracias\n%s!", e.de);
            decir_familia(texto);
            eyes_comer(comidas[esp_random() % CANT_COMIDAS]);
            break;
        case FAMILIA_COSQUILLAS:
            eyes_jugar_juego("contento");
            snprintf(texto, sizeof(texto), "¡Jaja!\n¡Basta,\n%s!", e.de);
            decir_familia(texto);
            break;
        case FAMILIA_JUGAR:
            eyes_jugar_juego(e.dato);       // el mismo juego que el otro, a la vez
            break;
        case FAMILIA_TRISTE:                // ¡a consolarlo!
            snprintf(texto, sizeof(texto), "¡No\nllores,\n%s!", e.de);
            decir_familia(texto);
            familia_mandar(e.de, FAMILIA_BESO, NULL);
            break;
        case FAMILIA_CLIMA: {               // el tiempo de afuera, que le cuenta otro
            int t;
            char cielo[20];
            if (sscanf(e.dato, "%d %19s", &t, cielo) == 2) {
                int grados = (t + (t >= 0 ? 5 : -5)) / 10;
                snprintf(texto, sizeof(texto), "Afuera:\n%s\n%d°C", cielo, grados);
                decir_familia(texto);
            }
            break;
        }
        case FAMILIA_SENSOR: {              // lo que mide el sensor de otro
            int t, h;
            if (sscanf(e.dato, "%d %d", &t, &h) == 2) {
                snprintf(texto, sizeof(texto), "%s:\nHumedad\n%d%%", e.de, h);
                decir_familia(texto);
            }
            break;
        }
        case FAMILIA_MENSAJE:
            snprintf(texto, sizeof(texto), "¡%s\ntiene un\nmensaje!", e.de);
            decir_familia(texto);
            break;
        case FAMILIA_DORMIR:
            eyes_jugar_juego("sueno");      // le da sueño a el tambien
            snprintf(texto, sizeof(texto), "%s se\ndurmio", e.de);
            decir_familia(texto);
            break;
        default:
            break;
        }
    }
    atendiendo_familia = false;
}

// Desde el menu: elegir a alguien de la familia y que hacerle
static void menu_familia(void)
{
    if (!familia_configurada()) {
        mostrar_texto("Sin\nfamilia", 2, 2000);
        return;
    }
    static char nombres[FAMILIA_MAX_MIEMBROS][FAMILIA_MAX_NOMBRE];
    int n = familia_conectados(nombres, FAMILIA_MAX_MIEMBROS);
    if (n == 0) {
        mostrar_texto("No hay\nnadie\nconectado", 2, 2000);
        return;
    }
    const char *opciones[FAMILIA_MAX_MIEMBROS + 1];
    for (int i = 0; i < n; i++) opciones[i] = nombres[i];
    opciones[n] = "Atras";
    int quien = menu_elegir("Familia", opciones, n + 1, 0, -1, MENU_ESPERA_MAX_MS);
    if (quien < 0 || quien >= n) return;                 // "Atras" o se cerro solo
    const char *con = nombres[quien];

    static const char *const acciones[] = { "Beso", "Cosquis", "Convidar", "Charlar", "Jugar", "Atras" };
    switch (menu_elegir(con, acciones, 6, 0, -1, MENU_ESPERA_MAX_MS)) {
    case 0: familia_mandar(con, FAMILIA_BESO, NULL);       mostrar_texto("¡Beso\nenviado!", 2, 1500);  break;
    case 1: familia_mandar(con, FAMILIA_COSQUILLAS, NULL); mostrar_texto("¡Cosqui\ncosqui!", 2, 1500); break;
    case 2: familia_mandar(con, FAMILIA_COMIDA, NULL);     mostrar_texto("¡Le\nconvide!", 2, 1500);    break;
    case 3: eyes_init(); empezar_charla(con); break;
    case 4: eyes_init(); jugar_con(con);      break;
    default: break;
    }
}

static void abrir_menu(void)
{
    static const char *const opciones_menu[] = { "Modo", "Comer", "Tomar", "Hora", "Sensor", "Mensaje", "Familia", "Salir" };
    static const char *const opciones_modo[] = { "Normal", "Halloween", "Navidad", "Animales", "Volver" };
    const int opcion_modo = 0, opcion_comer = 1, opcion_tomar = 2, opcion_hora = 3, opcion_sensor = 4;
    const int opcion_mensaje = 5, opcion_familia = 6;
    const int volver = 4;

    ESP_LOGI(TAG, "Abriendo el menu");
    while (1) {
        int elegida = menu_elegir("", opciones_menu, 8, 0, -1, MENU_ESPERA_MAX_MS);

        if (elegida == opcion_comer) {
            int c = menu_elegir_dibujos("Comer", CANT_COMIDAS + 1, dibujar_opcion_comida, 0, MENU_ESPERA_MAX_MS);
            if (c == CANT_COMIDAS) continue;        // "Atras": vuelve al menu anterior
            if (c >= 0) dar_de_comer(c);
            break;                                  // despues vuelve a la pantalla principal
        }
        if (elegida == opcion_tomar) {
            int b = menu_elegir_dibujos("Tomar", CANT_BEBIDAS + 1, dibujar_opcion_bebida, 0, MENU_ESPERA_MAX_MS);
            if (b == CANT_BEBIDAS) continue;        // "Atras": vuelve al menu anterior
            if (b >= 0) dar_de_tomar(b);
            break;
        }
        if (elegida == opcion_hora) {
            mostrar_hora();
            break;
        }
        if (elegida == opcion_sensor) {
            mostrar_sensor();
            break;
        }
        if (elegida == opcion_mensaje) {
            mensajes_desde_menu();
            break;
        }
        if (elegida == opcion_familia) {
            menu_familia();
            break;
        }
        if (elegida != opcion_modo) break;          // "Salir" o se cerro solo

        int m = menu_elegir("Modo", opciones_modo, 5, modo, modo, MENU_ESPERA_MAX_MS);
        if (m == volver) continue;                  // vuelve al menu anterior
        if (m >= 0) cambiar_modo((modo_t)m);
        break;
    }

    botones_se_apreto(1);   // apretadas sueltas no cuentan
    botones_se_apreto(2);
    eyes_init();            // vuelve la cara de Pitri (en el modo que estaba)
}

// ─── Comportamiento ──────────────────────────────────────────────────────────

// Cuando llega alguien: en modo normal saluda, en los otros hace un personaje
static void recibir_visita(void)
{
    // Le avisa a la familia que llego alguien (como mucho cada 2 minutos)
    static int64_t ultimo_aviso_us = 0;
    if (ultimo_aviso_us == 0 || esp_timer_get_time() - ultimo_aviso_us > 120LL * 1000000) {
        familia_mandar("*", FAMILIA_VISITA, NULL);
        ultimo_aviso_us = esp_timer_get_time();
    }
    if (telegram_sin_leer() > 0) {      // ¡primero, el mensaje!
        avisar_mensaje();
        return;
    }
    if (modo == MODO_NORMAL) eyes_saludar();
    else                     eyes_actuar(elegir_personaje());
}

// Espera un rato, pero corta si alguien llega o si aprietan el boton del menu
static void descansar(int ms)
{
    for (int t = 0; t < ms; t += 50) {
        if (hay_boton_para_atender() || familia_hay_evento()) return;
        if (motion_alguien_llego()) {
            recibir_visita();
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// ─── Pantallas automaticas ───────────────────────────────────────────────────

// Pitri se corre a un costado y mira los datos del sensor, que aparecen con letra chica
static void mostrar_temperatura(void)
{
    int16_t t, h;
    if (!dht_ultima_lectura(&t, &h)) return;   // todavia no hay ninguna lectura

    char texto[64];
    snprintf(texto, sizeof(texto), "Temp:\n%s%d.%d°C\nHumedad:\n%d%%",
             t < 0 ? "-" : "", abs(t) / 10, abs(t) % 10, (h + 5) / 10);

    char dato[24];                                 // se lo cuenta a la familia
    snprintf(dato, sizeof(dato), "%d %d", t, (h + 5) / 10);
    familia_mandar("*", FAMILIA_SENSOR, dato);

    eyes_sacar_disfraz();
    eyes_mostrar_datos(texto, 1, MOSTRAR_SENSOR_MS);
}

// Pitri se corre a un costado y mira la hora, que aparece con letra grande
static void mostrar_hora_de_costado(const struct tm *t)
{
    char texto[32];
    snprintf(texto, sizeof(texto), "%02d:%02d", t->tm_hour, t->tm_min);
    ESP_LOGI(TAG, "Son las %s", texto);

    eyes_sacar_disfraz();
    eyes_mostrar_datos(texto, 2, MOSTRAR_HORA_COSTADO_MS);
}

// Muestra la hora en x:00, x:05, x:10... y el sensor en x:00, x:15, x:30, x:45.
// Si todavia no sabe la hora, muestra el sensor cada SENSOR_CADA_MINUTOS
// contando desde que se prendio.
static void revisar_horarios(void)
{
    static int ultima_hora_mostrada = -1;      // "minuto del año" de la ultima vez
    static int ultimo_sensor_mostrado = -1;
    static int64_t ultimo_sensor_sin_hora_us = 0;

    struct tm t;
    if (reloj_hora(&t)) {
        int este_minuto = t.tm_yday * 24 * 60 + t.tm_hour * 60 + t.tm_min;

        if (t.tm_min % HORA_CADA_MINUTOS == 0 && este_minuto != ultima_hora_mostrada) {
            ultima_hora_mostrada = este_minuto;
            mostrar_hora_de_costado(&t);
        }
        if (t.tm_min % SENSOR_CADA_MINUTOS == 0 && este_minuto != ultimo_sensor_mostrado) {
            ultimo_sensor_mostrado = este_minuto;
            mostrar_temperatura();
        }
    } else {
        int64_t ahora = esp_timer_get_time();
        if (ahora - ultimo_sensor_sin_hora_us >= (int64_t)SENSOR_CADA_MINUTOS * 60 * 1000 * 1000) {
            ultimo_sensor_sin_hora_us = ahora;
            mostrar_temperatura();
        }
    }
}

// ─── WiFi ────────────────────────────────────────────────────────────────────

static bool ya_festejo_wifi = false;

// "Tengo wifi!" con la carita de costado (solo la primera vez que se conecta)
static void festejar_wifi(void)
{
    ya_festejo_wifi = true;
    ESP_LOGI(TAG, "Tengo wifi!");
    if (MENSAJE_WIFI[0] == '\0') return;       // mensaje vacio: no mostrar nada
    eyes_sacar_disfraz();
    eyes_mostrar_datos(MENSAJE_WIFI, 1, MOSTRAR_WIFI_MS);
}

// Al arrancar: carita con ondas mientras se conecta (hasta WIFI_ESPERA_ARRANQUE_MS)
static void esperar_wifi_al_arrancar(void)
{
    if (!wifi_configurado()) return;           // sin datos de red no hay nada que esperar

    int64_t inicio = esp_timer_get_time();
    while (!wifi_conectado() && !hay_boton_para_atender() &&
           esp_timer_get_time() - inicio < (int64_t)WIFI_ESPERA_ARRANQUE_MS * 1000) {
        eyes_onditas();
    }

    if (wifi_conectado()) festejar_wifi();
    else ESP_LOGI(TAG, "Todavia no hay wifi: sigo y lo festejo cuando llegue");
}

// Duerme hasta que el sensor vea a alguien (o hasta que aprieten el boton del menu)
static void dormir(void)
{
    ESP_LOGI(TAG, "Pasaron %d s sin movimiento", TIEMPO_SIN_MOVIMIENTO_PARA_DORMIR_MS / 1000);
    familia_mandar("*", FAMILIA_DORMIR, NULL);   // le cuenta a la familia que se va a dormir
    eyes_sacar_disfraz();
    eyes_dormirse();

    bool llego_alguien = false;
    while (!hay_boton_para_atender()) {
        if (motion_alguien_llego()) {
            llego_alguien = true;
            break;
        }
        if (telegram_recibidos() != recibidos_avisados) break;   // ¡llego un mensaje!
        if (familia_hay_evento()) break;                          // ¡le hablo alguien de la familia!
        eyes_dormido();
    }

    eyes_despertar();
    if (llego_alguien) recibir_visita();
}

void app_main(void)
{
    ESP_LOGI(TAG, "¡Hola! Soy Pitri :)");

    display_init();
    motion_init();
    dht_iniciar(TIEMPO_ENTRE_LECTURAS_MS);
    wifi_iniciar();          // se conecta "de fondo", no hay que esperarlo
    reloj_iniciar(ZONA_HORARIA);   // cuando haya WiFi, pone la hora
    telegram_iniciar();      // cuando haya WiFi, busca mensajes de Telegram
    familia_iniciar(MI_NOMBRE);   // cuando haya WiFi, saluda a la familia y la escucha
    botones_iniciar();       // boton 1 = menu, boton 2 = leer mensajes y moverse en el menu

    // Mensajes de bienvenida
    mostrar_mensaje(MENSAJE_ARRANQUE);
    mostrar_mensaje(MENSAJE_ARRANQUE_2);

    eyes_set_interrupcion(hay_boton_para_atender);
    eyes_set_sobre(hay_mensaje_sin_leer);     // sobrecito si hay un mensaje sin leer
    eyes_init();
    esperar_wifi_al_arrancar();   // carita con ondas hasta que se conecta
    eyes_saludar();

    while (1) {
        // Boton 1: abre el menu
        if (botones_se_apreto(1)) {
            abrir_menu();
            continue;
        }
        // Boton 2: lee los mensajes (si no hay ninguno, no hace nada)
        if (botones_se_apreto(2) && telegram_sin_leer() > 0) {
            leer_mensajes_nuevos();
            eyes_init();
            continue;
        }

        // Lo que le dijeron los demas de la familia (saludos, besos, charlas...)
        atender_familia();
        // Y cada tanto, Pitri empieza una charla o un juego
        iniciativa_familia();

        // Si llego un mensaje nuevo, avisa
        if (telegram_recibidos() != recibidos_avisados) {
            avisar_mensaje();
            continue;
        }

        if (motion_ms_sin_movimiento() >= TIEMPO_SIN_MOVIMIENTO_PARA_DORMIR_MS) {
            dormir();
            continue;
        }

        // Si alguien llego mientras jugaba, lo recibe antes de seguir
        if (motion_alguien_llego()) recibir_visita();

        // Si el WiFi llego tarde (despues del arranque), lo festeja ahora
        if (!ya_festejo_wifi && wifi_conectado()) festejar_wifi();

        // En punto: la hora cada 5 minutos y el sensor cada 15
        revisar_horarios();

        // Juega (modo normal) o hace un personaje (los otros modos)
        if (modo == MODO_NORMAL) {
            if (strcmp(eyes_jugar(), "llorar") == 0) familia_mandar("*", FAMILIA_TRISTE, NULL);   // ¡que lo consuelen!
        }
        else                     eyes_actuar(elegir_personaje());

        // Un descansito entre juego y juego, y a veces un parpadeo
        descansar(DESCANSO_MIN_MS + esp_random() % (DESCANSO_MAX_MS - DESCANSO_MIN_MS));
        if (esp_random() % 2) eyes_parpadear();
    }
}
