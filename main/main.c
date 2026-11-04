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
//   Menu:  Modo / Comer / Tomar / Hora / Sensor / Salir
//   Comer: Torta / Papas fritas / Hamburguesa / Atras (con dibujos)
//   Tomar: Agua / Cafe / Mate / Atras (con dibujos)
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

static void abrir_menu(void)
{
    static const char *const opciones_menu[] = { "Modo", "Comer", "Tomar", "Hora", "Sensor", "Salir" };
    static const char *const opciones_modo[] = { "Normal", "Halloween", "Navidad", "Animales", "Volver" };
    const int opcion_modo = 0, opcion_comer = 1, opcion_tomar = 2, opcion_hora = 3, opcion_sensor = 4;
    const int volver = 4;

    ESP_LOGI(TAG, "Abriendo el menu");
    while (1) {
        int elegida = menu_elegir("", opciones_menu, 6, 0, -1, MENU_ESPERA_MAX_MS);

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

// ─── Pantallas automaticas ───────────────────────────────────────────────────

// Pitri se corre a un costado y mira los datos del sensor, que aparecen con letra chica
static void mostrar_temperatura(void)
{
    int16_t t, h;
    if (!dht_ultima_lectura(&t, &h)) return;   // todavia no hay ninguna lectura

    char texto[64];
    snprintf(texto, sizeof(texto), "Temp:\n%s%d.%d°C\nHumedad:\n%d%%",
             t < 0 ? "-" : "", abs(t) / 10, abs(t) % 10, (h + 5) / 10);

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
    while (!wifi_conectado() && !botones_pendiente(1) &&
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
    reloj_iniciar(ZONA_HORARIA);   // cuando haya WiFi, pone la hora
    botones_iniciar();       // boton 1 = menu, boton 2 = moverse en el menu

    // Mensajes de bienvenida
    mostrar_mensaje(MENSAJE_ARRANQUE);
    mostrar_mensaje(MENSAJE_ARRANQUE_2);

    eyes_set_interrupcion(boton_menu_apretado);
    eyes_init();
    esperar_wifi_al_arrancar();   // carita con ondas hasta que se conecta
    eyes_saludar();

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

        // Si el WiFi llego tarde (despues del arranque), lo festeja ahora
        if (!ya_festejo_wifi && wifi_conectado()) festejar_wifi();

        // En punto: la hora cada 5 minutos y el sensor cada 15
        revisar_horarios();

        // Juega (modo normal) o hace un personaje (los otros modos)
        if (modo == MODO_NORMAL) eyes_jugar();
        else                     eyes_actuar(elegir_personaje());

        // Un descansito entre juego y juego, y a veces un parpadeo
        descansar(DESCANSO_MIN_MS + esp_random() % (DESCANSO_MAX_MS - DESCANSO_MIN_MS));
        if (esp_random() % 2) eyes_parpadear();
    }
}
