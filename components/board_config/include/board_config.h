#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/*
 * ═════════════════════════════════════════════════════════════════════════════
 *  PITRI — Conexiones de todos los perifericos
 * ═════════════════════════════════════════════════════════════════════════════
 *
 *  Placa: ESP32 DevKit de 30 pines (eMakers, estilo DOIT DevKit V1, USB CH9102X,
 *         modulo ESP32-WROOM-32, 4 MB). Nombres de pines como en la serigrafia.
 *  Dibujo y explicacion:  docs/CONEXIONES.md
 *  Datasheets:            docs/datasheets/
 *
 *  ┌──────────────────────┬────────────┬────────────┬─────────────────────────┐
 *  │ Periferico           │ Pata       │ ESP32      │ Notas                   │
 *  ├──────────────────────┼────────────┼────────────┼─────────────────────────┤
 *  │ Display OLED SH1106  │ VCC        │ 3V3        │ 128x64, I2C, dir. 0x3C  │
 *  │ (1.3")               │ GND        │ GND        │                         │
 *  │                      │ SCL        │ D22        │ I2C estandar del ESP32  │
 *  │                      │ SDA        │ D21        │                         │
 *  ├──────────────────────┼────────────┼────────────┼─────────────────────────┤
 *  │ Sensor PIR HC-SR501  │ VCC        │ VIN (5V)   │ ¡Necesita 5V!           │
 *  │                      │ GND        │ GND        │                         │
 *  │                      │ OUT        │ D27        │ Salida 3.3V, activa 1   │
 *  ├──────────────────────┼────────────┼────────────┼─────────────────────────┤
 *  │ Sensor DHT11         │ VCC (+)    │ 3V3        │ Pull-up 10k DATA→3V3    │
 *  │                      │ DATA       │ D4         │ (los modulos ya la      │
 *  │                      │ GND (-)    │ GND        │  traen)                 │
 *  ├──────────────────────┼────────────┼────────────┼─────────────────────────┤
 *  │ Boton 1              │ pata 1     │ RX2 (16)   │ La otra pata a GND.     │
 *  │ Boton 2              │ pata 1     │ TX2 (17)   │ Pull-up interno.        │
 *  ├──────────────────────┼────────────┼────────────┼─────────────────────────┤
 *  │ Boton BOOT (en placa)│ —          │ GPIO 0     │ No sale al header       │
 *  └──────────────────────┴────────────┴────────────┴─────────────────────────┘
 *
 *  Reservados para el futuro (NO conectar nada aca todavia):
 *    - GPIO 25, 26:            DAC → salida de audio
 *    - GPIO 32-36, 39 (ADC1):  entrada de audio. El ADC2 no funciona con WiFi
 *                              prendido, por eso el audio va por el ADC1.
 *    - GPIO 18, 19, 23, 5:     SPI (tarjeta SD, o I2S para audio)
 *  Libres para lo que venga:  GPIO 13 y 14
 *  No usar:                    TX0/RX0 (USB), D2, D12, D15 (arranque), EN (reset)
 */

#include "driver/gpio.h"

// ─── Display OLED SH1106 (I2C) ────────────────────────────────────────────────
#define I2C_HOST          I2C_NUM_0
#define I2C_SDA_PIN       GPIO_NUM_21
#define I2C_SCL_PIN       GPIO_NUM_22
#define I2C_FRECUENCIA_HZ (400 * 1000)
#define DISPLAY_I2C_ADDR  0x3C
#define LCD_H_RES         128
#define LCD_V_RES         64
#define DISPLAY_ROTAR_180 1          // 1 = imagen dada vuelta (segun como esta montado)

// ─── Sensor de movimiento PIR HC-SR501 ────────────────────────────────────────
#define PIN_PIR           GPIO_NUM_27
#define PIR_NIVEL_DETECTA 1          // OUT = 1 cuando ve movimiento

// ─── Sensor de temperatura y humedad DHT11 / DHT22 ────────────────────────────
#define DHT_MODELO_DHT11  11
#define DHT_MODELO_DHT22  22
#define SENSOR_TH_MODELO  DHT_MODELO_DHT11   // ← cual esta conectado
#define PIN_DHT           GPIO_NUM_4
#define DHT_INTERVALO_MIN_MS 2000    // no leerlo mas seguido que cada 2 s

// ─── Botones ──────────────────────────────────────────────────────────────────
#define PIN_BTN_1         GPIO_NUM_16
#define PIN_BTN_2         GPIO_NUM_17
#define PIN_BTN_BOOT      GPIO_NUM_0
#define BTN_NIVEL_APRETADO 0         // con pull-up: apretado = 0, suelto = 1

// ─── Reservados para audio (todavia sin usar) ─────────────────────────────────
#define PIN_AUDIO_DAC_1   GPIO_NUM_25   // DAC canal 1
#define PIN_AUDIO_DAC_2   GPIO_NUM_26   // DAC canal 2
#define PIN_AUDIO_ADC     GPIO_NUM_36   // ADC1 canal 0 (VP), solo entrada

#endif // BOARD_CONFIG_H
