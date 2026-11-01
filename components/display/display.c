#include "display.h"
#include "board_config.h"

#include <string.h>
#include "driver/i2c_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_sh1106.h"
#include "esp_log.h"

static const char *TAG = "DISPLAY";

static esp_lcd_panel_handle_t panel = NULL;

// Formato del SH1106: cada byte es una columna de 8 píxeles verticales ("página").
static uint8_t framebuffer[LCD_H_RES * LCD_V_RES / 8];

void display_init(void)
{
    ESP_LOGI(TAG, "Inicializando I2C y SH1106...");
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

void display_flush(void)
{
    esp_lcd_panel_draw_bitmap(panel, 0, 0, LCD_H_RES, LCD_V_RES, framebuffer);
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
