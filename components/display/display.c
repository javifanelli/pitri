#include "display.h"
#include "board_config.h"
#include "font5x7.h"

#include <string.h>
#include "driver/i2c_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_sh1106.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "DISPLAY";

static esp_lcd_panel_handle_t panel = NULL;

// Formato del SH1106: cada byte es una columna de 8 píxeles verticales ("página").
static uint8_t framebuffer[LCD_H_RES * LCD_V_RES / 8];

void display_init(void)
{
    ESP_LOGI(TAG, "Inicializando I2C y SH1106...");

    // El driver escribe 2 líneas por cada cuadro que falla; nosotros
    // hacemos un resumen en display_flush(), así que lo callamos.
    esp_log_level_set("lcd_panel.io.i2c", ESP_LOG_NONE);
    esp_log_level_set("lcd_panel.sh1106", ESP_LOG_NONE);
    i2c_master_bus_config_t i2c_cfg = {
        .clk_source        = I2C_CLK_SRC_DEFAULT,
        .i2c_port          = I2C_HOST,
        .scl_io_num        = I2C_SCL_PIN,
        .sda_io_num        = I2C_SDA_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_cfg, &bus));

    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_io_i2c_config_t io_cfg = ESP_SH1106_DEFAULT_IO_CONFIG;
    io_cfg.dev_addr     = DISPLAY_I2C_ADDR;
    io_cfg.scl_speed_hz = I2C_FRECUENCIA_HZ;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(bus, &io_cfg, &io));

    esp_lcd_panel_sh1106_config_t sh1106_cfg = {};
    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = -1,
        .bits_per_pixel = 1,
        .vendor_config  = &sh1106_cfg,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh1106(io, &panel_cfg, &panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));

    display_clear();
    display_flush();
}

void display_clear(void)
{
    memset(framebuffer, 0, sizeof(framebuffer));
}

// ─── Manejo de errores de comunicación ────────────────────────────────────────
#define FALLOS_SEGUIDOS_PARA_REINICIAR 10     // ~0,3 s de cuadros fallidos
#define RESUMEN_ERRORES_CADA_US        (5 * 1000 * 1000)

static int     fallos_seguidos = 0;
static int     fallos_desde_resumen = 0;
static int     reinicios = 0;
static int64_t ultimo_resumen_us = 0;

// Si el display deja de contestar, lo reinicializamos (por si se le
// desordenó la configuración por un corte o un falso contacto)
static void reiniciar_panel(void)
{
    reinicios++;
    ESP_LOGW(TAG, "El display no responde: reiniciándolo (vez %d)", reinicios);
    if (esp_lcd_panel_reset(panel) == ESP_OK &&
        esp_lcd_panel_init(panel) == ESP_OK &&
        esp_lcd_panel_disp_on_off(panel, true) == ESP_OK) {
        ESP_LOGI(TAG, "Display reiniciado");
    }
}

#if DISPLAY_ROTAR_180
// Da vuelta los 8 bits de un byte (el de arriba pasa a ser el de abajo)
static uint8_t invertir_bits(uint8_t b)
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

// Girar 180° es lo mismo que leer el framebuffer de atrás para adelante
// y dar vuelta cada columna de 8 píxeles
static uint8_t girado[sizeof(framebuffer)];
#endif

void display_flush(void)
{
#if DISPLAY_ROTAR_180
    for (int i = 0; i < sizeof(framebuffer); i++)
        girado[sizeof(framebuffer) - 1 - i] = invertir_bits(framebuffer[i]);
    const uint8_t *enviar = girado;
#else
    const uint8_t *enviar = framebuffer;
#endif

    esp_err_t err = esp_lcd_panel_draw_bitmap(panel, 0, 0, LCD_H_RES, LCD_V_RES, enviar);

    if (err == ESP_OK) {
        fallos_seguidos = 0;
    } else {
        fallos_seguidos++;
        fallos_desde_resumen++;
        if (fallos_seguidos == FALLOS_SEGUIDOS_PARA_REINICIAR) reiniciar_panel();
    }

    // En vez de una línea por cada error, un resumen cada 5 segundos
    int64_t ahora = esp_timer_get_time();
    if (fallos_desde_resumen > 0 && ahora - ultimo_resumen_us >= RESUMEN_ERRORES_CADA_US) {
        ESP_LOGW(TAG, "%d cuadros no se pudieron enviar por I2C en los últimos segundos "
                      "(revisar cables SDA/SCL/GND/3V3)", fallos_desde_resumen);
        fallos_desde_resumen = 0;
        ultimo_resumen_us = ahora;
    }
}

void display_set_pixel(int x, int y, bool on)
{
    if (x < 0 || x >= LCD_H_RES || y < 0 || y >= LCD_V_RES) return;
    uint8_t *b = &framebuffer[x + (y / 8) * LCD_H_RES];
    if (on) *b |=  (1 << (y & 7));
    else    *b &= ~(1 << (y & 7));
}

void display_fill_rect(int x, int y, int w, int h, bool on)
{
    for (int j = y; j < y + h; j++)
        for (int i = x; i < x + w; i++)
            display_set_pixel(i, j, on);
}

void display_fill_round_rect(int x, int y, int w, int h, int r, bool on)
{
    if (w <= 0 || h <= 0) return;
    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;

    for (int j = 0; j < h; j++) {
        // Distancia vertical al centro de la esquina más cercana
        int dy = 0;
        if (j < r)              dy = r - j;
        else if (j >= h - r)    dy = j - (h - r - 1);

        // Cuánto "se mete" la fila por la curva de la esquina
        int inset = 0;
        if (dy > 0) {
            int dx = 0;
            while ((dx + 1) * (dx + 1) + dy * dy <= r * r) dx++;
            inset = r - dx;
        }
        for (int i = x + inset; i < x + w - inset; i++)
            display_set_pixel(i, y + j, on);
    }
}

void display_fill_circle(int cx, int cy, int r, bool on)
{
    for (int j = -r; j <= r; j++)
        for (int i = -r; i <= r; i++)
            if (i * i + j * j <= r * r)
                display_set_pixel(cx + i, cy + j, on);
}

// Dice de qué lado de la línea (a→b) queda el punto p
static int lado_de_linea(int ax, int ay, int bx, int by, int px, int py)
{
    return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
}

void display_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2, bool on)
{
    int min_x = x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2);
    int max_x = x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2);
    int min_y = y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2);
    int max_y = y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2);

    // Un punto está adentro si queda del mismo lado de las tres líneas
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            int a = lado_de_linea(x0, y0, x1, y1, x, y);
            int b = lado_de_linea(x1, y1, x2, y2, x, y);
            int c = lado_de_linea(x2, y2, x0, y0, x, y);
            if ((a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0))
                display_set_pixel(x, y, on);
        }
    }
}

// ─── Texto ────────────────────────────────────────────────────────────────────

#define SEPARACION_LETRAS 1   // columnas vacías entre letra y letra
#define SEPARACION_RENGLON 2  // filas vacías entre renglón y renglón

// Caracteres especiales (fuera del ASCII)
#define GLIFO_ABRE_EXCLAMACION   1   // ¡
#define GLIFO_ABRE_PREGUNTA      2   // ¿
#define GLIFO_GRADO              3   // °

static const uint8_t columnas_grado[FONT_ANCHO] = {0x06, 0x09, 0x09, 0x06, 0x00};

// Lee el próximo caracter del texto (entiende UTF-8 para ¡ y ¿) y avanza el puntero
static int proximo_caracter(const char **p)
{
    const unsigned char *s = (const unsigned char *)*p;
    if (*s == 0) return 0;

    if (*s < 0x80) {             // ASCII común
        (*p)++;
        return *s;
    }
    if (s[0] == 0xC2 && s[1] == 0xA1) { *p += 2; return GLIFO_ABRE_EXCLAMACION; }
    if (s[0] == 0xC2 && s[1] == 0xBF) { *p += 2; return GLIFO_ABRE_PREGUNTA; }
    if (s[0] == 0xC2 && s[1] == 0xB0) { *p += 2; return GLIFO_GRADO; }

    // Otro caracter UTF-8 que no tenemos: lo salteamos entero y dibujamos '?'
    (*p)++;
    while ((**p & 0xC0) == 0x80) (*p)++;
    return '?';
}

// Devuelve las 5 columnas de un caracter
static void columnas_de(int c, uint8_t col[FONT_ANCHO])
{
    if (c == GLIFO_GRADO) {
        memcpy(col, columnas_grado, FONT_ANCHO);
        return;
    }

    bool dar_vuelta = false;
    if (c == GLIFO_ABRE_EXCLAMACION) { c = '!'; dar_vuelta = true; }
    if (c == GLIFO_ABRE_PREGUNTA)    { c = '?'; dar_vuelta = true; }
    if (c < FONT_PRIMERO || c > FONT_ULTIMO) c = '?';

    for (int i = 0; i < FONT_ANCHO; i++) {
        uint8_t b = font5x7[c - FONT_PRIMERO][i];
        if (dar_vuelta) {        // ¡ y ¿ son ! y ? patas para arriba
            uint8_t r = 0;
            for (int k = 0; k < FONT_ALTO; k++)
                if (b & (1 << k)) r |= 1 << (FONT_ALTO - 1 - k);
            b = r;
        }
        col[i] = b;
    }
}

static void dibujar_caracter(int x, int y, int c, int escala, bool on)
{
    uint8_t col[FONT_ANCHO];
    columnas_de(c, col);
    for (int i = 0; i < FONT_ANCHO; i++)
        for (int k = 0; k < FONT_ALTO; k++)
            if (col[i] & (1 << k))
                display_fill_rect(x + i * escala, y + k * escala, escala, escala, on);
}

// Ancho del renglón que empieza en p (hasta '\n' o fin)
static int ancho_renglon(const char *p, int escala)
{
    int letras = 0;
    int c;
    while ((c = proximo_caracter(&p)) != 0 && c != '\n') letras++;
    if (letras == 0) return 0;
    return (letras * (FONT_ANCHO + SEPARACION_LETRAS) - SEPARACION_LETRAS) * escala;
}

int display_text_width(const char *texto, int escala)
{
    int max = 0;
    const char *p = texto;
    while (1) {
        int w = ancho_renglon(p, escala);
        if (w > max) max = w;
        const char *salto = strchr(p, '\n');
        if (!salto) break;
        p = salto + 1;
    }
    return max;
}

static void dibujar_texto(int x, int y, const char *texto, int escala, bool on, bool centrado)
{
    const char *p = texto;
    int alto_renglon = (FONT_ALTO + SEPARACION_RENGLON) * escala;
    int cx = centrado ? (LCD_H_RES - ancho_renglon(p, escala)) / 2 : x;
    int c;

    while ((c = proximo_caracter(&p)) != 0) {
        if (c == '\n') {
            y += alto_renglon;
            cx = centrado ? (LCD_H_RES - ancho_renglon(p, escala)) / 2 : x;
            continue;
        }
        dibujar_caracter(cx, y, c, escala, on);
        cx += (FONT_ANCHO + SEPARACION_LETRAS) * escala;
    }
}

void display_draw_text(int x, int y, const char *texto, int escala, bool on)
{
    dibujar_texto(x, y, texto, escala, on, false);
}

void display_draw_text_centered(const char *texto, int escala, bool on)
{
    int renglones = 1;
    for (const char *p = texto; *p; p++)
        if (*p == '\n') renglones++;

    int alto = (renglones * (FONT_ALTO + SEPARACION_RENGLON) - SEPARACION_RENGLON) * escala;
    dibujar_texto(0, (LCD_V_RES - alto) / 2, texto, escala, on, true);
}
