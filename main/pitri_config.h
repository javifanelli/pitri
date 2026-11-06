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

// ─── WiFi ────────────────────────────────────────────────────────────────────
// Al arrancar, mientras se conecta, muestra la carita con ondas de WiFi
#define WIFI_ESPERA_ARRANQUE_MS (20 * 1000)  // si no conecta en este tiempo, sigue igual
// Mensaje (letra chica, al costado de la carita) cuando se conecta ("" = no mostrarlo)
#define MENSAJE_WIFI            "Tengo\nwifi!"
#define MOSTRAR_WIFI_MS         3000

// Si pasa este tiempo sin que el sensor vea a nadie, Pitri se va a dormir
#define TIEMPO_SIN_MOVIMIENTO_PARA_DORMIR_MS  (60 * 1000)   // 1 minuto

// ─── Temperatura y humedad (DHT11 / DHT22) ───────────────────────────
#define TIEMPO_ENTRE_LECTURAS_MS       (10 * 1000)  // cada cuanto lee el sensor (minimo 2 s)

// ─── Hora (se saca de internet cuando hay WiFi) ─────────────────────────────
// Diferencia con la hora mundial (UTC), en formato TZ: "ART3" = 3 horas menos
#define ZONA_HORARIA  "ART3"

// ─── Pantallas automaticas (en punto) ───────────────────────────────────────
#define HORA_CADA_MINUTOS        5      // la hora en x:00, x:05, x:10...
#define SENSOR_CADA_MINUTOS      15     // temperatura y humedad en x:00, x:15, x:30, x:45
#define MOSTRAR_HORA_COSTADO_MS  4000   // cuanto se ve la hora (carita de costado)
#define MOSTRAR_SENSOR_MS        6000   // cuanto se ven los datos del sensor

// Cuanto se ve la hora o el sensor al elegirlos en el menu
#define MOSTRAR_HORA_MS  5000

// ─── Comida y bebida ───────────────────────────────────────────────────────────────
#define COMIDAS_POR_HORA  3        // cuantas veces puede comer en una hora
#define BEBIDAS_POR_HORA  3        // cuantas veces puede tomar en una hora

// ─── Mensajes de Telegram ────────────────────────────────────────────────────
// Los datos del bot van en components/telegram/telegram_secretos.h (no se sube al repo)
#define MENSAJE_TITULO            "Mensaje de papa"   // arriba del mensaje (hasta 15 letras)
#define MOSTRAR_AVISO_MENSAJE_MS  4000                // cuanto se ve el aviso "¡Mensaje nuevo!"
#define LEER_MENSAJE_MAX_MS       (30 * 1000)         // si nadie toca ningun boton, el mensaje se cierra solo y se borra

// ─── Menu (boton 1 lo abre, boton 2 baja, boton 1 elige) ────────────────────
#define MENU_ESPERA_MAX_MS  (20 * 1000)   // si nadie toca nada, se cierra solo

// Descanso entre un juego y otro (se elige un valor al azar en este rango)
#define DESCANSO_MIN_MS  300
#define DESCANSO_MAX_MS  1500

#endif // PITRI_CONFIG_H
