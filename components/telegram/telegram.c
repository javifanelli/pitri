#include "telegram.h"
#include "wifi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"

// Los datos del bot estan en telegram_secretos.h, que NO se sube al repo (es
// publico). Si el archivo existe, CMakeLists.txt define PITRI_HAY_SECRETOS_TELEGRAM.
#ifdef PITRI_HAY_SECRETOS_TELEGRAM
#include "telegram_secretos.h"
#endif

static const char *TAG = "TELEGRAM";

/*
 * Como funciona:
 *   - Telegram guarda los mensajes que le mandan al bot.
 *   - Pitri le pregunta "¿hay mensajes nuevos?" (getUpdates). Si no hay,
 *     Telegram se queda esperando hasta ESPERA_S segundos antes de contestar
 *     (asi, apenas llega un mensaje, Pitri se entera).
 *   - Cada mensaje tiene un numero (update_id). En la proxima pregunta, Pitri
 *     dice "ya tengo hasta el numero tal", y Telegram los borra.
 *   - La conexion es segura (HTTPS): nadie en el medio puede leer los mensajes.
 */
#define ESPERA_S          25            // cuanto espera Telegram antes de contestar "nada nuevo"
#define REINTENTO_MS      (15 * 1000)   // si algo falla, cuanto espera para volver a probar
#define MAX_RESPUESTA     (16 * 1024)   // tamaño maximo de lo que contesta Telegram

// ─── Los mensajes guardados ──────────────────────────────────────────────────

static char pendientes[TELEGRAM_MAX_SIN_LEER][TELEGRAM_MAX_LETRAS + 1];
static int  cant_pendientes = 0;
static unsigned total_recibidos = 0;
static SemaphoreHandle_t candado;   // para que la tarea y main.c no los toquen a la vez

#ifdef PITRI_HAY_SECRETOS_TELEGRAM
// Guarda un mensaje nuevo. Devuelve false si ya no hay lugar (no se pisa ninguno
// sin leer, porque Sofi puede estar leyendo justo el mas viejo).
static bool guardar(const char *texto)
{
    xSemaphoreTake(candado, portMAX_DELAY);
    if (cant_pendientes == TELEGRAM_MAX_SIN_LEER) {
        xSemaphoreGive(candado);
        return false;
    }
    snprintf(pendientes[cant_pendientes], sizeof(pendientes[0]), "%s", texto);
    cant_pendientes++;
    total_recibidos++;
    xSemaphoreGive(candado);
    return true;
}
#endif

unsigned telegram_recibidos(void)
{
    if (candado == NULL) return 0;
    xSemaphoreTake(candado, portMAX_DELAY);
    unsigned n = total_recibidos;
    xSemaphoreGive(candado);
    return n;
}

int telegram_sin_leer(void)
{
    if (candado == NULL) return 0;
    xSemaphoreTake(candado, portMAX_DELAY);
    int n = cant_pendientes;
    xSemaphoreGive(candado);
    return n;
}

bool telegram_ver(char *texto, size_t tamano)
{
    if (candado == NULL) return false;
    xSemaphoreTake(candado, portMAX_DELAY);
    bool hay = cant_pendientes > 0;
    if (hay) snprintf(texto, tamano, "%s", pendientes[0]);
    xSemaphoreGive(candado);
    return hay;
}

void telegram_borrar(void)
{
    if (candado == NULL) return;
    xSemaphoreTake(candado, portMAX_DELAY);
    if (cant_pendientes > 0) {
        memmove(pendientes[0], pendientes[1], sizeof(pendientes[0]) * (TELEGRAM_MAX_SIN_LEER - 1));
        cant_pendientes--;
    }
    xSemaphoreGive(candado);
}

#ifdef PITRI_HAY_SECRETOS_TELEGRAM

// ─── Preparar el texto para la pantalla de Pitri ─────────────────────────────
// Las letras de Pitri no tienen tildes ni emojis: las tildes se cambian por la
// letra sola, algunos emojis por caritas de texto, y el resto se saltea.

// Lee el proximo caracter UTF-8 y avanza el puntero (devuelve su numero Unicode)
static unsigned proximo(const unsigned char **p)
{
    const unsigned char *s = *p;
    unsigned cp;
    int largo;
    if (s[0] < 0x80)                { cp = s[0];        largo = 1; }
    else if ((s[0] & 0xE0) == 0xC0) { cp = s[0] & 0x1F; largo = 2; }
    else if ((s[0] & 0xF0) == 0xE0) { cp = s[0] & 0x0F; largo = 3; }
    else if ((s[0] & 0xF8) == 0xF0) { cp = s[0] & 0x07; largo = 4; }
    else                            { *p = s + 1; return '?'; }
    for (int i = 1; i < largo; i++) {
        if ((s[i] & 0xC0) != 0x80) { *p = s + i; return '?'; }   // cortado: lo salteamos
        cp = (cp << 6) | (s[i] & 0x3F);
    }
    *p = s + largo;
    return cp;
}

// Que poner en lugar de cada caracter especial (NULL = saltearlo)
static const char *reemplazo(unsigned cp)
{
    static const char *const latinos = "AAAAAAACEEEEIIIIDNOOOOOxOUUUUYTsaaaaaaaceeeeiiiidnooooo/ouuuuyty";
    static char una[2];
    if (cp >= 0xC0 && cp <= 0xFF) {                     // letras con tilde, ñ, ç...
        una[0] = latinos[cp - 0xC0];
        una[1] = '\0';
        return una;
    }
    switch (cp) {
    case 0xA1: return "\xC2\xA1";                       // ¡ ¿ ° (Pitri si los tiene)
    case 0xBF: return "\xC2\xBF";
    case 0xB0: return "\xC2\xB0";
    case 0x2018: case 0x2019: return "'";
    case 0x201C: case 0x201D: return "\"";
    case 0x2013: case 0x2014: return "-";
    case 0x2026: return "...";
    case 0x2764: case 0x1F495: case 0x1F496: case 0x1F497: case 0x1F498:
    case 0x1F499: case 0x1F49A: case 0x1F49B: case 0x1F49C: case 0x1F49D:
    case 0x1F970: case 0x1F60D: case 0x1F9E1: case 0x1F90D: case 0x1F49E:
        return "<3";                                    // corazones
    case 0x263A: case 0x1F642: case 0x1F600: case 0x1F601: case 0x1F603:
    case 0x1F604: case 0x1F60A: case 0x1F607:
        return ":)";                                    // caritas felices
    case 0x1F602: case 0x1F923: case 0x1F606: case 0x1F605:
        return ":D";                                    // risas
    case 0x1F618: case 0x1F617: case 0x1F619: case 0x1F61A:
        return ":*";                                    // besitos
    case 0x1F641: case 0x1F622: case 0x1F62D: case 0x2639:
        return ":(";                                    // tristes
    case 0x1F609: return ";)";
    case 0x1F61B: case 0x1F61C: return ":P";
    case 0x1F44D: return "OK";
    default: return NULL;
    }
}

static void preparar_texto(const char *original, char *salida, size_t tamano)
{
    const unsigned char *p = (const unsigned char *)original;
    size_t n = 0;
    while (*p && n + 4 < tamano) {
        unsigned cp = proximo(&p);
        if (cp == '\r') continue;
        if (cp == '\t') cp = ' ';
        if (cp == '\n' || (cp >= 0x20 && cp < 0x7F)) {
            salida[n++] = (char)cp;
            continue;
        }
        const char *r = reemplazo(cp);
        if (r == NULL) continue;                        // emoji que no tenemos
        size_t largo = strlen(r);
        if (n + largo + 1 > tamano) break;
        memcpy(salida + n, r, largo);
        n += largo;
    }
    while (n > 0 && (salida[n - 1] == ' ' || salida[n - 1] == '\n')) n--;   // espacios del final
    salida[n] = '\0';
}

// ─── Hablar con Telegram ─────────────────────────────────────────────────────

static const long long permitidos[] = TELEGRAM_IDS_PERMITIDOS;
#define CANT_PERMITIDOS ((int)(sizeof(permitidos) / sizeof(permitidos[0])))

static bool es_permitido(long long id)
{
    for (int i = 0; i < CANT_PERMITIDOS; i++)
        if (permitidos[i] != 0 && permitidos[i] == id) return true;
    return false;
}

static bool nadie_permitido(void)
{
    for (int i = 0; i < CANT_PERMITIDOS; i++)
        if (permitidos[i] != 0) return false;
    return true;
}

static char  *respuesta;            // lo que va contestando Telegram
static size_t largo_respuesta;
static bool   desbordo;

static esp_err_t al_recibir(esp_http_client_event_t *evento)
{
    if (evento->event_id == HTTP_EVENT_ON_DATA) {
        if (largo_respuesta + evento->data_len >= MAX_RESPUESTA) {
            desbordo = true;
        } else {
            memcpy(respuesta + largo_respuesta, evento->data, evento->data_len);
            largo_respuesta += evento->data_len;
        }
    }
    return ESP_OK;
}

static long long proximo_update = 0;    // "ya tengo hasta el numero tal"

static long long numero(const cJSON *objeto, const char *nombre)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(objeto, nombre);
    return cJSON_IsNumber(item) ? (long long)item->valuedouble : 0;
}

// Revisa lo que contesto Telegram y guarda los mensajes que valen
static void procesar(const char *json)
{
    cJSON *raiz = cJSON_Parse(json);
    if (raiz == NULL) {
        ESP_LOGW(TAG, "No entendi lo que contesto Telegram");
        return;
    }

    const cJSON *update;
    cJSON_ArrayForEach(update, cJSON_GetObjectItemCaseSensitive(raiz, "result")) {
        long long id_update = numero(update, "update_id");
        if (id_update >= proximo_update) proximo_update = id_update + 1;

        const cJSON *mensaje = cJSON_GetObjectItemCaseSensitive(update, "message");
        if (mensaje == NULL) continue;
        long long de   = numero(cJSON_GetObjectItemCaseSensitive(mensaje, "from"), "id");
        long long chat = numero(cJSON_GetObjectItemCaseSensitive(mensaje, "chat"), "id");

        // Solo mensajes directos (no de grupos) de las personas permitidas
        if (de == 0 || de != chat || !es_permitido(de)) {
            if (nadie_permitido() && de != 0 && de == chat)
                ESP_LOGW(TAG, "Me escribio el usuario %lld. Si sos vos, pone ese numero en "
                              "TELEGRAM_IDS_PERMITIDOS (telegram_secretos.h)", de);
            else
                ESP_LOGW(TAG, "Llego un mensaje de alguien que no conozco: lo ignoro");
            continue;
        }

        const cJSON *texto = cJSON_GetObjectItemCaseSensitive(mensaje, "text");
        if (!cJSON_IsString(texto)) texto = cJSON_GetObjectItemCaseSensitive(mensaje, "caption");
        if (!cJSON_IsString(texto)) {
            ESP_LOGI(TAG, "Llego un mensaje sin texto (foto, sticker...): lo salteo");
            continue;
        }

        char limpio[TELEGRAM_MAX_LETRAS + 1];
        preparar_texto(texto->valuestring, limpio, sizeof(limpio));
        if (limpio[0] == '\0') continue;
        if (guardar(limpio))
            ESP_LOGI(TAG, "¡Llego un mensaje nuevo! (%d letras)", (int)strlen(limpio));
        else
            ESP_LOGW(TAG, "Ya hay %d mensajes sin leer: este no entra", TELEGRAM_MAX_SIN_LEER);
    }
    cJSON_Delete(raiz);
}

// Le pregunta a Telegram si hay mensajes nuevos. Devuelve el codigo HTTP (200 = bien)
static int preguntar(esp_http_client_handle_t cliente)
{
    char url[256];
    snprintf(url, sizeof(url),
             "https://api.telegram.org/bot%s/getUpdates?timeout=%d&limit=5&offset=%lld"
             "&allowed_updates=%%5B%%22message%%22%%5D",
             TELEGRAM_TOKEN, ESPERA_S, proximo_update);
    esp_http_client_set_url(cliente, url);

    largo_respuesta = 0;
    desbordo = false;
    esp_err_t err = esp_http_client_perform(cliente);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "No pude hablar con Telegram (%s)", esp_err_to_name(err));
        return -1;
    }
    int codigo = esp_http_client_get_status_code(cliente);
    if (codigo == 200 && !desbordo) {
        respuesta[largo_respuesta] = '\0';
        procesar(respuesta);
    }
    return codigo;
}

static void tarea_telegram(void *arg)
{
    respuesta = malloc(MAX_RESPUESTA);
    if (respuesta == NULL) {
        ESP_LOGE(TAG, "No hay memoria para Telegram");
        vTaskDelete(NULL);
        return;
    }

    esp_http_client_config_t cfg = {
        .url               = "https://api.telegram.org",
        .crt_bundle_attach = esp_crt_bundle_attach,   // para comprobar que es Telegram de verdad
        .timeout_ms        = (ESPERA_S + 10) * 1000,
        .event_handler     = al_recibir,
        .keep_alive_enable = true,
    };
    esp_http_client_handle_t cliente = NULL;
    bool avise_conexion = false;

    while (1) {
        if (!wifi_conectado()) {                    // sin WiFi no hay Telegram
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }
        if (cliente == NULL) cliente = esp_http_client_init(&cfg);

        int codigo = preguntar(cliente);
        if (codigo == 200) {
            if (!avise_conexion) {
                ESP_LOGI(TAG, "¡Conectado con Telegram! Esperando mensajes...");
                avise_conexion = true;
            }
            continue;                               // enseguida vuelve a preguntar
        }

        if (codigo == 401 || codigo == 404)
            ESP_LOGW(TAG, "Telegram no reconoce el token (revisar TELEGRAM_TOKEN en telegram_secretos.h)");
        else if (codigo > 0)
            ESP_LOGW(TAG, "Telegram contesto con el codigo %d", codigo);

        // Algo fallo: cerramos la conexion y probamos de nuevo en un rato
        esp_http_client_cleanup(cliente);
        cliente = NULL;
        avise_conexion = false;
        vTaskDelay(pdMS_TO_TICKS(codigo == 401 || codigo == 404 ? 5 * REINTENTO_MS : REINTENTO_MS));
    }
}

#endif // PITRI_HAY_SECRETOS_TELEGRAM

// ─── Arranque ────────────────────────────────────────────────────────────────

bool telegram_configurado(void)
{
#ifdef PITRI_HAY_SECRETOS_TELEGRAM
    return true;
#else
    return false;
#endif
}

void telegram_iniciar(void)
{
    candado = xSemaphoreCreateMutex();
#ifdef PITRI_HAY_SECRETOS_TELEGRAM
    if (nadie_permitido())
        ESP_LOGW(TAG, "Todavia no hay nadie permitido: escribile algo al bot para saber tu numero");
    xTaskCreate(tarea_telegram, "telegram", 8192, NULL, 4, NULL);
#else
    ESP_LOGW(TAG, "No hay datos del bot: sigo sin Telegram. Para usarlo, copia "
                  "telegram_secretos.ejemplo.h como telegram_secretos.h y completalo");
#endif
}
