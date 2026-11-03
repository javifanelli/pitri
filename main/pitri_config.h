#ifndef PITRI_CONFIG_H
#define PITRI_CONFIG_H

// ─── Comportamiento de Pitri (¡jueguen con estos numeros!) ───────────────────
// Los pines del hardware estan en board_config.h

// Mensaje que muestra al prenderse. '\n' = cambiar de renglon.
// Entran 5 letras por renglon con MENSAJE_ARRANQUE_TAMANO 3, o 10 con tamaño 2.
#define MENSAJE_ARRANQUE        "¡Hola\nSofi!"
#define MENSAJE_ARRANQUE_2      "Soy\nPitri"    // segundo mensaje ("" = no mostrarlo)
#define MENSAJE_ARRANQUE_TAMANO 3          // 1 = chiquito, 3 = grandote
#define MENSAJE_ARRANQUE_MS     3000       // cuanto tiempo se ve

// Mensaje que muestra la primera vez que se conecta al WiFi ("" = no mostrarlo)
#define MENSAJE_WIFI            "¡Tengo\nWiFi!"

// Si pasa este tiempo sin que el sensor vea a nadie, Pitri se va a dormir
#define TIEMPO_SIN_MOVIMIENTO_PARA_DORMIR_MS  (60 * 1000)   // 1 minuto

// ─── Temperatura y humedad (DHT11 / DHT22) ───────────────────────────
#define TIEMPO_ENTRE_LECTURAS_MS       (10 * 1000)  // cada cuanto lee el sensor (minimo 2 s)
#define MOSTRAR_TEMPERATURA_CADA_MS    (5 * 60 * 1000)  // cada cuanto la muestra: 5 minutos (0 = nunca)
#define MOSTRAR_TEMPERATURA_MS         6000             // cuanto tiempo se ve

// ─── Hora (se saca de internet cuando hay WiFi) ─────────────────────────────
// Diferencia con la hora mundial (UTC), en formato TZ: "ART3" = 3 horas menos
#define ZONA_HORARIA  "ART3"
#define MOSTRAR_HORA_MS  5000      // cuanto se ve la hora al elegirla en el menu

// ─── Comida ───────────────────────────────────────────────────────────────
#define COMIDAS_POR_HORA  3        // cuantas veces puede comer en una hora

// ─── Menu (boton 1 lo abre, boton 2 baja, boton 1 elige) ────────────────────
#define MENU_ESPERA_MAX_MS  (20 * 1000)   // si nadie toca nada, se cierra solo

// Descanso entre un juego y otro (se elige un valor al azar en este rango)
#define DESCANSO_MIN_MS  300
#define DESCANSO_MAX_MS  1500

#endif // PITRI_CONFIG_H
