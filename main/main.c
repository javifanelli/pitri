#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
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

// Los 3 personajes de cada modo
static const eyes_disfraz_t personajes[CANT_MODOS][3] = {
    [MODO_HALLOWEEN] = { DISFRAZ_VAMPIRO,   DISFRAZ_FANTASMA, DISFRAZ_CALABAZA },
    [MODO_NAVIDAD]   = { DISFRAZ_PAPA_NOEL, DISFRAZ_ARBOLITO, DISFRAZ_GALLETA  },
    [MODO_ANIMALES]  = { DISFRAZ_GALLINA,   DISFRAZ_CERDO,    DISFRAZ_TORTUGA  },
};

static modo_t modo = MODO_NORMAL;
static eyes_disfraz_t ultimo_personaje = DISFRAZ_NINGUNO;

// Elige un personaje del modo actual, distinto del ultimo que hizo
static eyes_disfraz_t elegir_personaje(void)
{
    if (modo == MODO_NORMAL) return DISFRAZ_NINGUNO;   // el modo normal no tiene personajes

    eyes_disfraz_t p;
    do {
        p = personajes[modo][esp_random() % 3];
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
//   Menu:  Modo / Comer / Hora / Salir
//   Comer: Torta / Papas fritas / Hamburguesa / Atras (con dibujos)
//   Modo:  Normal / Halloween / Navidad / Animales / Volver

// Para que los ojos corten lo que estan haciendo apenas se aprieta el boton 1
static bool boton_menu_apretado(void)
{
    return botones_pendiente(1);
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

// ─── Comer ───────────────────────────────────────────────────────────────────
// Pitri puede comer hasta COMIDAS_POR_HORA veces en una hora. Si ya comio todo
// eso, dice que no. No le pasa nada malo si no come: Pitri nunca muere.

static const eyes_comida_t comidas[] = { COMIDA_TORTA, COMIDA_PAPAS, COMIDA_HAMBURGUESA };
static const char *const nombres_comidas[] = { "Torta", "Papas fritas", "Hamburguesa" };
#define CANT_COMIDAS 3

// Cuando comio las ultimas veces (en microsegundos desde que se prendio)
static int64_t cuando_comio[COMIDAS_POR_HORA];

static int comidas_en_la_ultima_hora(void)
{
    const int64_t una_hora_us = 60LL * 60 * 1000 * 1000;
    int64_t ahora = esp_timer_get_time();
    int cuantas = 0;
    for (int i = 0; i < COMIDAS_POR_HORA; i++)
        if (cuando_comio[i] != 0 && ahora - cuando_comio[i] < una_hora_us) cuantas++;
    return cuantas;
}

static void dar_de_comer(int cual)
{
    if (comidas_en_la_ultima_hora() >= COMIDAS_POR_HORA) {
        ESP_LOGI(TAG, "Pitri esta lleno (ya comio %d veces en la ultima hora)", COMIDAS_POR_HORA);
        eyes_decir_que_no();
        return;
    }

    // Anotamos la comida en el lugar de la mas vieja
    int mas_vieja = 0;
    for (int i = 1; i < COMIDAS_POR_HORA; i++)
        if (cuando_comio[i] < cuando_comio[mas_vieja]) mas_vieja = i;
    cuando_comio[mas_vieja] = esp_timer_get_time();

    ESP_LOGI(TAG, "Pitri come %s (%d de %d en la ultima hora)",
             nombres_comidas[cual], comidas_en_la_ultima_hora(), COMIDAS_POR_HORA);
    eyes_comer(comidas[cual]);
}

// Dibuja cada opcion del menu "Comer": la comida grande con su nombre, o "Atras"
static void dibujar_opcion_comida(int opcion)
{
    if (opcion < CANT_COMIDAS) {
        eyes_dibujar_comida(comidas[opcion], LCD_H_RES / 2.0f, 32, 1.1f, 0);
        const char *nombre = nombres_comidas[opcion];
        display_draw_text((LCD_H_RES - display_text_width(nombre, 1)) / 2, 55, nombre, 1, true);
    } else {
        display_draw_text((LCD_H_RES - display_text_width("Atras", 3)) / 2, 27, "Atras", 3, true);
    }
}

static void abrir_menu(void)
{
    static const char *const opciones_menu[] = { "Modo", "Comer", "Hora", "Salir" };
    static const char *const opciones_modo[] = { "Normal", "Halloween", "Navidad", "Animales", "Volver" };
    const int opcion_modo = 0, opcion_comer = 1, opcion_hora = 2;
    const int volver = 4;
    const int atras = CANT_COMIDAS;

    ESP_LOGI(TAG, "Abriendo el menu");
    while (1) {
        int elegida = menu_elegir("", opciones_menu, 4, 0, -1, MENU_ESPERA_MAX_MS);

        if (elegida == opcion_comer) {
            int c = menu_elegir_dibujos("Comer", CANT_COMIDAS + 1, dibujar_opcion_comida, 0, MENU_ESPERA_MAX_MS);
            if (c == atras) continue;               // vuelve al menu anterior
            if (c >= 0) dar_de_comer(c);
            break;                                  // despues vuelve a la pantalla principal
        }
        if (elegida == opcion_hora) {
            mostrar_hora();
            break;                                  // despues vuelve a la pantalla principal
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
    if (modo == MODO_NORMAL) eyes_saludar();
    else                     eyes_actuar(elegir_personaje());
}

// Espera un rato, pero corta si alguien llega o si aprietan el boton del menu
static void descansar(int ms)
{
    for (int t = 0; t < ms; t += 50) {
        if (botones_pendiente(1)) return;
        if (motion_alguien_llego()) {
            recibir_visita();
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Pitri se corre a un costado y mira los datos del sensor, que aparecen con letra chica
static void mostrar_temperatura(void)
{
    int16_t t, h;
    if (!dht_ultima_lectura(&t, &h)) return;   // todavia no hay ninguna lectura

    char texto[64];
    snprintf(texto, sizeof(texto), "Temp:\n%s%d.%d°C\nHumedad:\n%d%%",
             t < 0 ? "-" : "", abs(t) / 10, abs(t) % 10, (h + 5) / 10);

    eyes_sacar_disfraz();
    eyes_mostrar_datos(texto, MOSTRAR_TEMPERATURA_MS);
}

// Duerme hasta que el sensor vea a alguien (o hasta que aprieten el boton del menu)
static void dormir(void)
{
    ESP_LOGI(TAG, "Pasaron %d s sin movimiento", TIEMPO_SIN_MOVIMIENTO_PARA_DORMIR_MS / 1000);
    eyes_sacar_disfraz();
    eyes_dormirse();

    bool llego_alguien = false;
    while (!botones_pendiente(1)) {
        if (motion_alguien_llego()) {
            llego_alguien = true;
            break;
        }
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
    reloj_iniciar(ZONA_HORARIA);   // cuando haya WiFi, pone la hora (todavia no la mostramos)
    botones_iniciar();       // boton 1 = menu, boton 2 = moverse en el menu

    // Mensajes de bienvenida
    mostrar_mensaje(MENSAJE_ARRANQUE);
    mostrar_mensaje(MENSAJE_ARRANQUE_2);

    eyes_set_interrupcion(boton_menu_apretado);
    eyes_init();
    eyes_saludar();

    int64_t ultima_vez_temperatura_us = esp_timer_get_time();
    bool ya_festejo_wifi = false;

    while (1) {
        // Boton 1: abre el menu (el boton 2 solo sirve adentro del menu)
        botones_se_apreto(2);
        if (botones_se_apreto(1)) {
            abrir_menu();
            continue;
        }

        if (motion_ms_sin_movimiento() >= TIEMPO_SIN_MOVIMIENTO_PARA_DORMIR_MS) {
            dormir();
            continue;
        }

        // Si alguien llego mientras jugaba, lo recibe antes de seguir
        if (motion_alguien_llego()) recibir_visita();

        // La primera vez que se conecta al WiFi, lo festeja
        if (!ya_festejo_wifi && wifi_conectado()) {
            ya_festejo_wifi = true;
            mostrar_mensaje(MENSAJE_WIFI);
            eyes_init();
            eyes_saludar();
        }

        // Cada tanto muestra la temperatura
        if (MOSTRAR_TEMPERATURA_CADA_MS > 0 &&
            esp_timer_get_time() - ultima_vez_temperatura_us >= (int64_t)MOSTRAR_TEMPERATURA_CADA_MS * 1000) {
            mostrar_temperatura();
            ultima_vez_temperatura_us = esp_timer_get_time();
        }

        // Juega (modo normal) o hace un personaje (los otros modos)
        if (modo == MODO_NORMAL) eyes_jugar();
        else                     eyes_actuar(elegir_personaje());

        // Un descansito entre juego y juego, y a veces un parpadeo
        descansar(DESCANSO_MIN_MS + esp_random() % (DESCANSO_MAX_MS - DESCANSO_MIN_MS));
        if (esp_random() % 2) eyes_parpadear();
    }
}
