#ifndef DHT_H
#define DHT_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

// Sensor de temperatura y humedad DHT11 o DHT22 (se elige en board_config.h
// con SENSOR_TH_MODELO).
// Los valores estan en DECIMAS: 234 = 23,4 °C   ·   451 = 45,1 %

// Prepara el pin y arranca una tarea que lee el sensor cada "intervalo_ms"
// (minimo DHT_INTERVALO_MIN_MS). Si una lectura falla, la reintenta sola.
void dht_iniciar(uint32_t intervalo_ms);

// Devuelve la ultima lectura buena. false si todavia no hubo ninguna.
bool dht_ultima_lectura(int16_t *temperatura_decimas, int16_t *humedad_decimas);

// Lee el sensor en este momento (bloquea ~25 ms con el DHT11, ~6 ms con el DHT22).
// Normalmente no hace falta: la tarea ya lo lee sola.
esp_err_t dht_leer(int16_t *temperatura_decimas, int16_t *humedad_decimas);

#endif // DHT_H
