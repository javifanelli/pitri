# Pitri — Documentación del hardware

- **[CONEXIONES.md](CONEXIONES.md)**: dibujo de cómo se conecta todo, por qué se eligió cada pin y qué pines quedan reservados.
- En el código, los pines están en
  [`components/board_config/include/board_config.h`](../components/board_config/include/board_config.h).

## Datasheets

| Periférico | Archivo | Qué tiene |
|---|---|---|
| ESP32 (chip) | [ESP32_datasheet.pdf](datasheets/ESP32_datasheet.pdf) | Pines, periféricos y características eléctricas del chip |
| ESP32-WROOM-32D (módulo de la placa, según el vendedor) | [ESP32-WROOM-32D_datasheet.pdf](datasheets/ESP32-WROOM-32D_datasheet.pdf) | Pinout del módulo, pines de arranque (strapping) |
| Display OLED 1.3" (controlador SH1106) | [SH1106_OLED_driver.pdf](datasheets/SH1106_OLED_driver.pdf) | Comandos, memoria de pantalla, protocolo I2C |
| Sensor de movimiento HC-SR501 | [HC-SR501_PIR_modulo.pdf](datasheets/HC-SR501_PIR_modulo.pdf) | Alimentación, jumper L/H, potenciómetros de tiempo y sensibilidad |
| Chip del HC-SR501 (BISS0001) | [BISS0001_PIR_controller.pdf](datasheets/BISS0001_PIR_controller.pdf) | El integrado que procesa la señal del PIR |
| Sensores PIR en general | [PIR_guia_Adafruit.pdf](datasheets/PIR_guia_Adafruit.pdf) | Guía de Adafruit: cómo funcionan y cómo ajustarlos |
| Sensor DHT11 (el que usa Pitri) | [DHT11.pdf](datasheets/DHT11.pdf) | Protocolo de 1 cable, tiempos, rangos de medición |
| Sensor DHT22 / AM2302 (alternativa) | [DHT22_AM2302.pdf](datasheets/DHT22_AM2302.pdf) | Igual que el DHT11 pero más preciso; mismo cableado |
| Sensores DHT en general | [DHT_guia_Adafruit.pdf](datasheets/DHT_guia_Adafruit.pdf) | Guía de Adafruit: diferencias DHT11/DHT22 y cómo conectarlos |

Los botones son componentes genéricos y no tienen datasheet.
