# 🧑‍🔧 Pitri — Para grandes

Guia tecnica: como esta organizado el codigo, como compilarlo y como agregarle cosas.

---

## Plataforma

| | |
|---|---|
| Placa | ESP32 DevKit de 30 pines (eMakers, CH9102X, modulo ESP32-WROOM-32, 4 MB) |
| Framework | **ESP-IDF v6.0** (instalado en `C:\esp\v6.0\esp-idf`) |
| Lenguaje | C (gnu23) |
| Dependencias externas | `tny-robotics/sh1106-esp-idf` (driver del display, via Component Manager) |

---

## Compilar y cargar

Desde la terminal de ESP-IDF (o la extension de VS Code), en la carpeta del proyecto:

```
idf.py build                  # compilar
idf.py -p COM11 flash monitor # cargar en la placa y ver la consola (Ctrl+] para salir)
```

- La primera vez, o despues de **agregar, renombrar o borrar un componente**, conviene
  `idf.py reconfigure` (o `idf.py fullclean` si hay errores raros de componentes).
- El puerto COM puede cambiar; se ve en el Administrador de dispositivos (CH9102X).

---

## Organizacion del codigo

```
pitri/
├── main/
│   ├── main.c             ← el "cerebro": arranque y bucle principal
│   └── pitri_config.h     ← numeros de comportamiento (tiempos, mensaje, etc.)
├── components/            ← una carpeta por "modulo" reutilizable
│   ├── board_config/      ← pines y datos del hardware (solo un .h)
│   ├── display/           ← pantalla: framebuffer, formas, texto, rotacion
│   ├── eyes/              ← los ojos: animacion, expresiones, juegos y disfraces
│   ├── motion/            ← sensor de movimiento PIR
│   ├── dht/               ← sensor de temperatura y humedad DHT11/DHT22
│   ├── wifi/              ← conexion WiFi (datos privados en wifi_secretos.h)
│   ├── botones/           ← los 2 botones (con anti-rebote)
│   ├── menu/              ← menu en pantalla manejado con los botones
│   └── reloj/             ← hora por internet (SNTP)
├── docs/                  ← documentacion, diagrama y datasheets
└── managed_components/    ← librerias descargadas (no se tocan a mano)
```

### Reglas que seguimos

- **`main/` solo tiene `main.c` y la configuracion.** Todo lo demas va en un componente.
- **Los pines viven solo en [board_config.h](../components/board_config/include/board_config.h).**
  Ningun otro archivo escribe un numero de GPIO. Si se cambia un cable, se cambia ahi (y en
  [CONEXIONES.md](CONEXIONES.md) y el script del diagrama).
- **Los numeros de comportamiento viven en [pitri_config.h](../main/pitri_config.h)**, los
  de forma de los ojos arriba de [eyes_interno.h](../components/eyes/eyes_interno.h).
- **Nombres en castellano** en lo nuestro (funciones internas, variables, comentarios),
  para que Sofia pueda leerlo. Las funciones publicas de cada componente llevan el prefijo
  del componente (`display_…`, `eyes_…`, `motion_…`, `dht_…`).
- **Comentarios que expliquen el porque**, pensando en alguien que esta aprendiendo.
- **Cada componente declara exactamente lo que usa** en su `CMakeLists.txt` (`REQUIRES`).
  En ESP-IDF v6 el componente `driver` ya no trae todo: GPIO es `esp_driver_gpio`, I2C es
  `esp_driver_i2c`, etc.

---

## Componentes

### `board_config`
Solo un header con `#define`: pines, direccion I2C del display, resolucion, modelo de
sensor DHT, rotacion de pantalla, pines reservados para audio. Ver
[CONEXIONES.md](CONEXIONES.md) para el porque de cada pin.

### `display` — [display.h](../components/display/include/display.h)
- Framebuffer de 1 bpp en formato de paginas del SH1106 (cada byte = columna de 8 pixeles).
- `display_flush()` lo manda por I2C. Si `DISPLAY_ROTAR_180` esta en 1, lo gira por software
  antes de mandarlo (asi sobrevive a reinicios del panel).
- Manejo de errores: silencia los logs del driver, resume los fallos cada 5 s y reinicia el
  panel tras 10 cuadros fallidos seguidos.
- Primitivas: pixel, rectangulo, rectangulo redondeado, circulo, triangulo.
- Texto con fuente 5×7 escalable, `\n`, y UTF-8 para `¡ ¿ °`.

### `eyes` — [eyes.h](../components/eyes/include/eyes.h)
- Estado de la cara en `cara_t` (posicion, tamaño, apertura por ojo, `feliz`, `enojo`,
  `triste`, `espiral`, `giro`, `lagrima`, `zzz`, `corazon`, `corazoncitos`, `lentes`,
  `asombro`, `sospecha`, `sueno`, `mira`, `datos`).
- Perspectiva: `p = (x / MAX_X + mira) · 0.12`, limitada a ±0,15. `mira` permite que la cara
  este en un lado y mire hacia el otro.
- `eyes_mostrar_datos(texto, escala, ms)`: achica la cara, la corre a la izquierda
  mirando a la derecha y hace entrar `texto` deslizandose (escala 1 desde x = 78, escala 2
  desde x = 68 con la cara mas corrida), centrado verticalmente. Lo arma `de_costado()`.
- `eyes_onditas()`: un paso (~1,2 s) de la cara de costado con el simbolo de WiFi
  (`cara.ondas`), cuyos arcos se encienden segun `cara.fase`.
- Boca: siempre hay una. `dibujar_boca()` elige el tipo segun el peso de emocion mas alto
  (umbral 0,3; si ninguno llega, `BOCA_SENCILLA`, que se ubica como si los ojos estuvieran
  abiertos para no saltar al parpadear) y la
  ubica debajo de los ojos (calcula donde terminan segun apertura, perspectiva, espiral,
  corazon o lentes) sin salirse de la pantalla. Para agregar una boca: un valor en
  `boca_t`, una `CANDIDATA`, su alto en `alto_boca[]` y su `case`.
- `animar(destino, ms)` interpola todos los campos con smoothstep a ~33 fps (`CUADRO_MS 30`).
- Juegos en una tabla `juegos[]` con pesos; `eyes_jugar()` elige uno al azar.
- API publica: `eyes_init`, `eyes_jugar`, `eyes_parpadear`, `eyes_saludar`,
  `eyes_dormirse`, `eyes_dormido`, `eyes_despertar`, `eyes_mostrar_datos`, `eyes_actuar`,
  `eyes_sacar_disfraz`, `eyes_set_interrupcion`, `eyes_dibujar_comida`, `eyes_comer`,
  `eyes_tomar`, `eyes_decir_que_no`.
- Bebidas: son valores de `eyes_comida_t` (`COMIDA_BOTELLA` = agua, `COMIDA_TAZA` = cafe,
  `COMIDA_MATE`). En `eyes_dibujar_comida` el parametro `mordidas` es la cantidad de sorbos
  (0 a 3) y define la altura del liquido (el mate no lo usa).
- Comida: `comida_actual` + `cara.comida_x/y` (interpolados por `animar`) dibujan la comida
  durante la animacion; `comida_reiniciar()` la saca (lo llama `eyes_init`).
  `eyes_dibujar_comida(c, cx, cy, tamano, mordidas)` es solo dibujo (lo usa el menu).
- Archivos: `eyes.c` (cara, animacion, emociones, juegos), `disfraces.c` (personajes),
  `comida.c` (dibujos de comida y bebida, animaciones de comer y tomar) y
  `eyes_interno.h` (lo que comparten: `cara_t`, `cara`, `animar`, `mover_a`, `esperar`...).
- Disfraces: `disfraz_actual` decide que se dibuja. Los que `tapa_cara` reemplazan todo el
  dibujo; los otros dibujan los ojos de Pitri y agregan sus partes en lugar de la boca.
  `eyes_actuar(p)` cambia de disfraz (cierra y abre los ojos) y ejecuta su actuacion.
  `cara.fase` es el reloj de las animaciones propias de cada disfraz.
- Interrupcion: `eyes_set_interrupcion(fn)`. `animar()` y `esperar()` consultan `fn` en
  cada cuadro / cada 50 ms; si devuelve true, marcan `cortado` y todo lo que sigue retorna
  enseguida. Cada funcion publica llama a `empezar_accion()` al entrar (resetea la marca).
- Todas las funciones **bloquean** mientras dura la animacion.

### `motion` — [motion.h](../components/motion/include/motion.h)
- Tarea propia que lee el PIR cada 50 ms y loguea inicio y fin de movimiento.
- Ignora los primeros 30 s (calentamiento del sensor).
- `motion_alguien_llego()` (flanco, se consume al leerlo), `motion_hay_alguien()`,
  `motion_ms_sin_movimiento()`.

### `dht` — [dht.h](../components/dht/include/dht.h)
- Driver propio (bit-banging) para DHT11 y DHT22, elegido en tiempo de compilacion con
  `SENSOR_TH_MODELO`. No usa `esp-idf-lib/dht` para evitar problemas con ESP-IDF v6.
- Tarea propia fijada al nucleo 1 (bloquea interrupciones ~5 ms por lectura; el I2C del
  display vive en el nucleo 0). Reintenta 2 veces si falla. No escribe en consola.
- Valores en **decimas** (`int16_t`): evita depender de `printf` con floats.
- `dht_ultima_lectura()` devuelve la ultima lectura buena.

### `wifi` — [wifi.h](../components/wifi/include/wifi.h)
- Modo estacion, conexion "de fondo": `wifi_iniciar()` no bloquea.
- Reintentos con backoff exponencial (1 s → 60 s) y motivo de desconexion en castellano.
- Credenciales solo en RAM (`WIFI_STORAGE_RAM`). Nunca se loguean ni el SSID ni la contraseña.
- Si no existe `wifi_secretos.h`, compila igual y Pitri sigue sin WiFi (avisa por consola).
- `wifi_conectado()` dice si tiene IP; `wifi_configurado()`, si hay datos de red.

### `botones` — [botones.h](../components/botones/include/botones.h)
- Tarea propia que lee `PIN_BTN_1` y `PIN_BTN_2` cada 10 ms, con pull-up interno.
- Anti-rebote: un cambio vale si se repite 3 lecturas seguidas (30 ms).
- Loguea `BOTONES: ¡Boton N apretado!` y deja una marca para `botones_se_apreto(n)`.

### `menu` — [menu.h](../components/menu/include/menu.h)
- `menu_elegir(titulo, opciones, cantidad, elegida, destacada, espera_max_ms)`: bloquea
  hasta que se elige (boton 1) y devuelve el indice, o -1 si pasa `espera_max_ms` sin tocar.
- Boton 2 avanza (circular). Con titulo: 3 filas visibles; con titulo `""` (menu
  principal): sin titulo y 4 filas. Letra escala 2, ventana que se corre y flechitas ▲▼ si
  hay mas. `destacada` muestra un puntito (se usa para el modo actual).
- `menu_elegir_dibujos(titulo, cantidad, dibujar_opcion, elegida, espera_max_ms)`: una
  opcion por pantalla, dibujada por el callback, con puntitos de posicion arriba a la derecha.

### `reloj` — [reloj.h](../components/reloj/include/reloj.h)
- `reloj_iniciar(zona)`: configura `TZ` y lanza una tarea que espera WiFi e inicia
  `esp_netif_sntp` (`pool.ntp.org`). La tarea se borra sola; SNTP sigue corrigiendo la hora
  periodicamente.
- `reloj_sincronizado()` y `reloj_hora(&tm)` (hora local; false si todavia no hay hora).
- Loguea `RELOJ: ¡Ya se la hora! dd/mm/aaaa hh:mm:ss` la primera vez.
- `main.c` la muestra desde el menu (`mostrar_hora()`, durante `MOSTRAR_HORA_MS`).

---

## WiFi

⚠️ **El repo es publico. Nunca se suben datos de la casa ni de las personas** (contraseñas,
nombre de la red, direcciones, nombres completos, etc.).

Para configurar la red:

1. Copiar `components/wifi/wifi_secretos.ejemplo.h` como `components/wifi/wifi_secretos.h`
   (en la misma carpeta).
2. Completar `WIFI_NOMBRE_RED` y `WIFI_CONTRASENA` en la copia.
3. Compilar y cargar. En la consola deberia aparecer `WIFI: ¡Conectado a la red WiFi!`.

`wifi_secretos.h` (y cualquier `*secretos*.h`) esta en el `.gitignore`. Antes de un commit
conviene mirar `git status` y confirmar que no aparece. Si alguna vez se sube por error,
**hay que cambiar la contraseña del WiFi**: borrarlo del repo no alcanza, porque queda en el
historial de git.

Otros datos que tampoco se suben: `sdkconfig` y `build/` (ya ignorados), porque el binario
compilado contiene la contraseña.

---

## Flujo de `main.c`

```
app_main
├── display_init, motion_init, dht_iniciar, wifi_iniciar, reloj_iniciar, botones_iniciar
├── mensajes de bienvenida (MENSAJE_ARRANQUE y MENSAJE_ARRANQUE_2)
├── eyes_init
├── esperar_wifi_al_arrancar()   ← eyes_onditas() hasta conectar (max WIFI_ESPERA_ARRANQUE_MS)
│                                  y "Tengo wifi!" de costado
├── eyes_saludar
└── while (1)
    ├── ¿boton 1?                                                 → abrir_menu()
    ├── ¿sin movimiento ≥ TIEMPO_SIN_MOVIMIENTO_PARA_DORMIR_MS?  → dormir()
    ├── ¿motion_alguien_llego()?                                  → recibir_visita()
    ├── ¿WiFi llego tarde?                                        → festejar_wifi()
    ├── revisar_horarios(): x:00/x:05... hora de costado; x:00/x:15... sensor de costado
    │   (sin hora de internet: sensor cada SENSOR_CADA_MINUTOS desde el arranque)
    ├── modo normal: eyes_jugar()  /  otros modos: eyes_actuar(personaje al azar)
    └── descansar(azar)   ← revisa el sensor y el boton cada 50 ms
```

El boton 1 corta cualquier animacion al instante (ver interrupcion en `eyes`). La llegada
de alguien, en cambio, todavia espera a que termine el juego actual (hasta ~5 s).

Menu principal: Modo / Comer / Tomar / Hora / Sensor / Salir. Comer: Torta / Papas fritas /
Hamburguesa / Atras. Tomar: Agua / Cafe / Mate / Atras. Los limites (`COMIDAS_POR_HORA`,
`BEBIDAS_POR_HORA`) son ventanas moviles de 60 minutos con `esp_timer` (no necesitan la hora
de internet), separadas, y se reinician al apagar. Modos: `modo_t` en `main.c`, con una tabla de
`PERSONAJES_POR_MODO` (5) personajes por modo. El modo no se guarda:
al reiniciar, Pitri arranca en Normal.

---

## Como agregar…

### …un juego o un personaje
Ver [NUEVO_JUEGO.md](NUEVO_JUEGO.md).

### …un modo
1. Agregar los personajes nuevos en `disfraces.c` / `eyes.h` (ver NUEVO_JUEGO.md).
2. En `main.c`: un valor en `modo_t` (antes de `CANT_MODOS`), su nombre en
   `nombres_modos`, su fila en `personajes` y la opcion en `opciones_modo` (antes de
   "Volver", actualizando `volver` y la cantidad).

### …una expresion nueva (algo que hoy la cara no sabe dibujar)
1. Agregar un campo `float` a `cara_t` en `eyes_interno.h` (0 = sin efecto).
2. Agregarlo a la lista de `mezclar` dentro de `animar()` (si no, no se anima).
3. Dibujarlo en `dibujar_ojo()` o `dibujar()` cuando sea > 0.

### …un periferico nuevo
1. Elegir pin en [CONEXIONES.md](CONEXIONES.md) (hay libres: D13, D14; los reservados para
   audio y SPI estan listados ahi).
2. Agregar las macros en `board_config.h`.
3. Crear `components/<nombre>/` con `CMakeLists.txt`, `<nombre>.c` e `include/<nombre>.h`.
4. Agregarlo al `REQUIRES` de `main/CMakeLists.txt`.
5. Actualizar `docs/CONEXIONES.md`, el diagrama (`python docs/img/gen_conexiones.py
   docs/img/conexiones.svg`), el datasheet en `docs/datasheets/` y el
   [Diario](DIARIO.md).

---

## Problemas conocidos y soluciones

| Sintoma | Causa / solucion |
|---|---|
| `driver/gpio.h: No such file or directory` | ESP-IDF v6: agregar `esp_driver_gpio` al `REQUIRES` del componente |
| `i2c transaction failed` en rafagas | Cableado del display (soldaduras, largo de cables, pull-ups). El display se reinicia solo |
| DHT: el sensor no contesta | Revisar el **orden de las patas** del modulo (leer la serigrafia: S/DATA, +, −). Nos paso |
| DHT: temperatura absurda con datos "validos" | Sensor dañado; el driver lee bien los bits |
| PIR detecta cosas que no existen | Primeros 30 s de calentamiento, o esta muy cerca de la antena WiFi |
| `WIFI: No hay datos de WiFi` | Falta `components/wifi/wifi_secretos.h` (ver [WiFi](#wifi)) |
| `WIFI: Sin conexion: la red no me deja entrar` | Contraseña incorrecta en `wifi_secretos.h` |
| `WIFI: Sin conexion: no encuentro la red` | Nombre de red mal escrito, router apagado o red de 5 GHz (el ESP32 solo usa 2,4 GHz) |
