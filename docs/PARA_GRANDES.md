# 🧑‍🔧 Pitri — Para grandes

Guía técnica: cómo está organizado el código, cómo compilarlo y cómo agregarle cosas.

---

## Plataforma

| | |
|---|---|
| Placa | ESP32 DevKit de 30 pines (eMakers, CH9102X, módulo ESP32-WROOM-32, 4 MB) |
| Framework | **ESP-IDF v6.0** (instalado en `C:\esp\v6.0\esp-idf`) |
| Lenguaje | C (gnu23) |
| Dependencias externas | `tny-robotics/sh1106-esp-idf` (driver del display, vía Component Manager) |

---

## Compilar y cargar

Desde la terminal de ESP-IDF (o la extensión de VS Code), en la carpeta del proyecto:

```
idf.py build                  # compilar
idf.py -p COM11 flash monitor # cargar en la placa y ver la consola (Ctrl+] para salir)
```

- La primera vez, o después de **agregar, renombrar o borrar un componente**, conviene
  `idf.py reconfigure` (o `idf.py fullclean` si hay errores raros de componentes).
- El puerto COM puede cambiar; se ve en el Administrador de dispositivos (CH9102X).

---

## Organización del código

```
pitri/
├── main/
│   ├── main.c             ← el "cerebro": arranque y bucle principal
│   └── pitri_config.h     ← números de comportamiento (tiempos, mensaje, etc.)
├── components/            ← una carpeta por "módulo" reutilizable
│   ├── board_config/      ← pines y datos del hardware (solo un .h)
│   ├── display/           ← pantalla: framebuffer, formas, texto, rotación
│   ├── eyes/              ← los ojos: animación, expresiones y juegos
│   ├── motion/            ← sensor de movimiento PIR
│   └── dht/               ← sensor de temperatura y humedad DHT11/DHT22
├── docs/                  ← documentación, diagrama y datasheets
└── managed_components/    ← librerías descargadas (no se tocan a mano)
```

### Reglas que seguimos

- **`main/` solo tiene `main.c` y la configuración.** Todo lo demás va en un componente.
- **Los pines viven solo en [board_config.h](../components/board_config/include/board_config.h).**
  Ningún otro archivo escribe un número de GPIO. Si se cambia un cable, se cambia ahí (y en
  [CONEXIONES.md](CONEXIONES.md) y el script del diagrama).
- **Los números de comportamiento viven en [pitri_config.h](../main/pitri_config.h)**, los
  de forma de los ojos arriba de [eyes.c](../components/eyes/eyes.c).
- **Nombres en castellano** en lo nuestro (funciones internas, variables, comentarios),
  para que Sofía pueda leerlo. Las funciones públicas de cada componente llevan el prefijo
  del componente (`display_…`, `eyes_…`, `motion_…`, `dht_…`).
- **Comentarios que expliquen el porqué**, pensando en alguien que está aprendiendo.
- **Cada componente declara exactamente lo que usa** en su `CMakeLists.txt` (`REQUIRES`).
  En ESP-IDF v6 el componente `driver` ya no trae todo: GPIO es `esp_driver_gpio`, I2C es
  `esp_driver_i2c`, etc.

---

## Componentes

### `board_config`
Solo un header con `#define`: pines, dirección I2C del display, resolución, modelo de
sensor DHT, rotación de pantalla, pines reservados para audio. Ver
[CONEXIONES.md](CONEXIONES.md) para el porqué de cada pin.

### `display` — [display.h](../components/display/include/display.h)
- Framebuffer de 1 bpp en formato de páginas del SH1106 (cada byte = columna de 8 píxeles).
- `display_flush()` lo manda por I2C. Si `DISPLAY_ROTAR_180` está en 1, lo gira por software
  antes de mandarlo (así sobrevive a reinicios del panel).
- Manejo de errores: silencia los logs del driver, resume los fallos cada 5 s y reinicia el
  panel tras 10 cuadros fallidos seguidos.
- Primitivas: píxel, rectángulo, rectángulo redondeado, círculo, triángulo.
- Texto con fuente 5×7 escalable, `\n`, y UTF-8 para `¡ ¿ °`.

### `eyes` — [eyes.h](../components/eyes/include/eyes.h)
- Estado de la cara en `cara_t` (posición, tamaño, apertura por ojo, `feliz`, `enojo`,
  `triste`, `espiral`, `giro`, `lagrima`, `zzz`).
- `animar(destino, ms)` interpola todos los campos con smoothstep a ~33 fps (`CUADRO_MS 30`).
- Juegos en una tabla `juegos[]` con pesos; `eyes_jugar()` elige uno al azar.
- API pública: `eyes_init`, `eyes_jugar`, `eyes_parpadear`, `eyes_saludar`,
  `eyes_dormirse`, `eyes_dormido`, `eyes_despertar`.
- Todas las funciones **bloquean** mientras dura la animación.

### `motion` — [motion.h](../components/motion/include/motion.h)
- Tarea propia que lee el PIR cada 50 ms y loguea inicio y fin de movimiento.
- Ignora los primeros 30 s (calentamiento del sensor).
- `motion_alguien_llego()` (flanco, se consume al leerlo), `motion_hay_alguien()`,
  `motion_ms_sin_movimiento()`.

### `dht` — [dht.h](../components/dht/include/dht.h)
- Driver propio (bit-banging) para DHT11 y DHT22, elegido en tiempo de compilación con
  `SENSOR_TH_MODELO`. No usa `esp-idf-lib/dht` para evitar problemas con ESP-IDF v6.
- Tarea propia fijada al núcleo 1 (bloquea interrupciones ~5 ms por lectura; el I2C del
  display vive en el núcleo 0). Reintenta 2 veces si falla. No escribe en consola.
- Valores en **décimas** (`int16_t`): evita depender de `printf` con floats.
- `dht_ultima_lectura()` devuelve la última lectura buena.

---

## Flujo de `main.c`

```
app_main
├── display_init, motion_init, dht_iniciar
├── mensaje de bienvenida (MENSAJE_ARRANQUE)
├── eyes_init + eyes_saludar
└── while (1)
    ├── ¿sin movimiento ≥ TIEMPO_SIN_MOVIMIENTO_PARA_DORMIR_MS?  → dormir()
    ├── ¿motion_alguien_llego()?                                  → eyes_saludar()
    ├── ¿pasó MOSTRAR_TEMPERATURA_CADA_MS?                        → mostrar_temperatura()
    ├── eyes_jugar()
    └── descansar(azar)   ← revisa el sensor cada 50 ms
```

Limitación conocida: como las animaciones bloquean, si alguien llega en medio de un juego
largo, Pitri lo saluda recién cuando termina (hasta ~5 s de demora).

---

## Cómo agregar…

### …un juego
Ver [NUEVO_JUEGO.md](NUEVO_JUEGO.md).

### …una expresión nueva (algo que hoy la cara no sabe dibujar)
1. Agregar un campo `float` a `cara_t` en `eyes.c` (0 = sin efecto).
2. Agregarlo a la lista de `mezclar` dentro de `animar()` (si no, no se anima).
3. Dibujarlo en `dibujar_ojo()` o `dibujar()` cuando sea > 0.

### …un periférico nuevo
1. Elegir pin en [CONEXIONES.md](CONEXIONES.md) (hay libres: D13, D14; los reservados para
   audio y SPI están listados ahí).
2. Agregar las macros en `board_config.h`.
3. Crear `components/<nombre>/` con `CMakeLists.txt`, `<nombre>.c` e `include/<nombre>.h`.
4. Agregarlo al `REQUIRES` de `main/CMakeLists.txt`.
5. Actualizar `docs/CONEXIONES.md`, el diagrama (`python docs/img/gen_conexiones.py
   docs/img/conexiones.svg`), el datasheet en `docs/datasheets/` y el
   [Diario](DIARIO.md).

---

## Problemas conocidos y soluciones

| Síntoma | Causa / solución |
|---|---|
| `driver/gpio.h: No such file or directory` | ESP-IDF v6: agregar `esp_driver_gpio` al `REQUIRES` del componente |
| `i2c transaction failed` en ráfagas | Cableado del display (soldaduras, largo de cables, pull-ups). El display se reinicia solo |
| DHT: el sensor no contesta | Revisar el **orden de las patas** del módulo (leer la serigrafía: S/DATA, +, −). Nos pasó |
| DHT: temperatura absurda con datos "válidos" | Sensor dañado; el driver lee bien los bits |
| PIR detecta cosas que no existen | Primeros 30 s de calentamiento, o está muy cerca de la antena WiFi |
