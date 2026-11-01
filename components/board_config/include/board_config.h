#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// Mismo hardware que el proyecto dht22

#include "driver/gpio.h"

// ─── Botones y LEDs ───────────────────────────────────────────────────────────
#define PIN_BOOT_BTN  GPIO_NUM_0
#define PIN_BTN       GPIO_NUM_4
#define PIN_LED_R     GPIO_NUM_15
#define PIN_LED_G     GPIO_NUM_4
#define PIN_LED_B     GPIO_NUM_2

// ─── I2C (Display OLED SH1106) ────────────────────────────────────────────────
#define I2C_SDA_PIN   GPIO_NUM_19
#define I2C_SCL_PIN   GPIO_NUM_18
#define I2C_HOST      I2C_NUM_0

// ─── Sensor DHT22 (todavía sin usar) ──────────────────────────────────────────
#define PIN_DHT       GPIO_NUM_23

// ─── Display ──────────────────────────────────────────────────────────────────
#define LCD_H_RES     128
#define LCD_V_RES     64

#endif // BOARD_CONFIG_H
