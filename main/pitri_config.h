#ifndef PITRI_CONFIG_H
#define PITRI_CONFIG_H

// ─── Comportamiento de Pitri (¡jueguen con estos números!) ───────────────────
// Los pines del hardware están en board_config.h

// Mensaje que muestra al prenderse. '\n' = cambiar de renglón.
// Entran 5 letras por renglón con MENSAJE_ARRANQUE_TAMANO 3, o 10 con tamaño 2.
#define MENSAJE_ARRANQUE        "¡Hola\nSofi!"
#define MENSAJE_ARRANQUE_2      "Soy\nPitri"    // segundo mensaje ("" = no mostrarlo)
#define MENSAJE_ARRANQUE_TAMANO 3          // 1 = chiquito, 3 = grandote
#define MENSAJE_ARRANQUE_MS     3000       // cuánto tiempo se ve

// Mensaje que muestra la primera vez que se conecta al WiFi ("" = no mostrarlo)
#define MENSAJE_WIFI            "¡Tengo\nWiFi!"

// Si pasa este tiempo sin que el sensor vea a nadie, Pitri se va a dormir
#define TIEMPO_SIN_MOVIMIENTO_PARA_DORMIR_MS  (60 * 1000)   // 1 minuto

// ─── Temperatura y humedad (DHT11 / DHT22) ───────────────────────────
#define TIEMPO_ENTRE_LECTURAS_MS       (10 * 1000)  // cada cuánto lee el sensor (mínimo 2 s)
#define MOSTRAR_TEMPERATURA_CADA_MS    (5 * 60 * 1000)  // cada cuánto la muestra: 5 minutos (0 = nunca)
#define MOSTRAR_TEMPERATURA_MS         6000             // cuánto tiempo se ve

// Descanso entre un juego y otro (se elige un valor al azar en este rango)
#define DESCANSO_MIN_MS  300
#define DESCANSO_MAX_MS  1500

#endif // PITRI_CONFIG_H
