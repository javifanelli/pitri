# 🔍 Cómo funciona Pitri

Acá explicamos cómo hace Pitri cada cosa. Las partes marcadas con 🧑‍🔧 **Para grandes**
tienen más detalles técnicos: se pueden saltear.

---

## 🧠 El cerebro: un programa que nunca termina

El programa de Pitri es como una lista de instrucciones que se repite **para siempre**:

```
Al prenderse:
   1. Preparar la pantalla y los sensores
   2. Mostrar "¡Hola Sofi!" y "Soy Pitri"
   3. Abrir los ojos y saludar

Y después, una y otra vez:
   ¿Hace mucho que no veo a nadie?  →  me duermo hasta que llegue alguien
   ¿Llegó alguien?                  →  ¡lo saludo!
   ¿Pasó un minuto?                 →  muestro la temperatura
   Elijo un juego al azar y lo hago
   Descanso un ratito
```

Eso está en [main/main.c](../main/main.c), en la parte que dice `while (1)`.
`while (1)` quiere decir "repetir para siempre".

Mientras tanto, hay **ayudantes** que trabajan al mismo tiempo, sin que el programa
principal se tenga que acordar de ellos:

- uno vigila el **sensor de movimiento** todo el tiempo,
- otro lee el **sensor de temperatura** cada 10 segundos.

> 🧑‍🔧 **Para grandes:** los "ayudantes" son *tareas* de FreeRTOS, el sistema operativo que
> trae el ESP32. Cada una tiene su propio `while (1)` y el ESP32 las va alternando muy rápido
> (además tiene 2 núcleos, así que algunas corren de verdad al mismo tiempo).

---

## 🖥️ La pantalla: un tablero de lucecitas

La pantalla tiene **128 lucecitas de ancho y 64 de alto**. Cada lucecita se llama
**píxel** y puede estar **prendida** (blanca) o **apagada** (negra). Nada más.

```
  x →  0 1 2 3 4 5 6 7 ...                     127
y 0    · · · · · · · · · · · · · · · · · · · · ·
↓ 1    · · · █ █ █ · · · · · · · · · █ █ █ · · ·
  2    · · · █ █ █ · · · · · · · · · █ █ █ · · ·
  3    · · · █ █ █ · · · · · · · · · █ █ █ · · ·
  ...
  63
```

Cada píxel tiene una dirección, como en la batalla naval: **x** dice qué tan a la derecha
está y **y** qué tan abajo. ¡Ojo! En las pantallas, **y crece para abajo**.

### Dibujar sin que se note

Si Pitri dibujara directo en la pantalla, veríamos cómo va pintando de a pedacitos.
Por eso hace como los dibujantes de dibujos animados:

1. Dibuja todo en un **papel escondido** (en la memoria del ESP32).
2. Cuando el dibujo está listo, lo **manda entero** a la pantalla de una sola vez.

Esas dos cosas son `display_clear()` (borrar el papel) y `display_flush()` (mandarlo).

> 🧑‍🔧 **Para grandes:** el "papel escondido" es un *framebuffer* de 1024 bytes
> (128 × 64 ÷ 8, un bit por píxel). Se manda por **I2C**, un protocolo de 2 cables
> (SDA = datos, SCL = reloj) a 400 kHz. Cada cuadro tarda unos 25 ms en viajar.
> La pantalla está montada al revés, así que justo antes de mandarlo
> [display.c](../components/display/display.c) gira el dibujo 180°.

### Las formas que sabe dibujar

| Función | Dibuja |
|---|---|
| `display_set_pixel` | Un solo píxel |
| `display_fill_rect` | Un rectángulo |
| `display_fill_round_rect` | Un rectángulo con las esquinas redondeadas (¡los ojos!) |
| `display_fill_circle` | Un círculo |
| `display_fill_triangle` | Un triángulo (las cejas de enojado y triste) |
| `display_draw_text` | Letras |

### Las letras

Cada letra es un dibujito de **5 píxeles de ancho y 7 de alto**. Por ejemplo, la **A**:

```
 · █ █ █ ·
 █ · · · █
 █ · · · █
 █ █ █ █ █
 █ · · · █
 █ · · · █
 █ · · · █
```

Todas las letras están guardadas en [font5x7.h](../components/display/font5x7.h).
Para que se vean más grandes, cada píxel de la letra se dibuja como un cuadradito de
2×2 o 3×3.

---

## 👀 Los ojos: dibujos animados

Los ojos de Pitri son **dos rectángulos con las esquinas redondeadas**. Para que parezca
que se mueven, Pitri hace lo mismo que en los dibujos animados: dibuja **muchos dibujos
apenas distintos, uno atrás del otro, muy rápido**. Cada dibujo se llama **cuadro**.

Pitri dibuja unos **33 cuadros por segundo** (uno cada 30 milisegundos). Tu ojo no llega
a ver los saltos, ¡y parece movimiento!

### Parpadear

Un parpadeo es achicar la altura de los ojos hasta que quedan como una rayita, y después
volver a agrandarla:

```
 ████    ████    ▄▄▄▄    ────    ▄▄▄▄    ████    ████
 ████    ████    ▀▀▀▀            ▀▀▀▀    ████    ████
 ████  →        →       →       →       →        → ████
abierto                cerrado                   abierto
```

### Moverse suave

Para ir de un lugar a otro, Pitri **no salta**: calcula todos los puntos del medio.
Y además arranca despacito, acelera y frena despacito, como cuando movés la cabeza.

> 🧑‍🔧 **Para grandes:** la "cara" es una estructura (`cara_t`) con posición, tamaño,
> apertura de cada ojo, y cuánto de feliz, enojado, triste o mareado está. La función
> `animar(destino, ms)` interpola **todos** esos valores a la vez entre la cara actual y la
> de destino, con una curva *smoothstep* (`t² · (3 − 2t)`) para arrancar y frenar suave.
> Por eso cualquier combinación funciona sola: moverse mientras se cierra un ojo, etc.

### Las expresiones

Las caras distintas se hacen **tapando partes de los ojos con negro**:

| Expresión | Truco |
|---|---|
| 😊 Feliz | Una "loma" negra tapa la parte de abajo del ojo → queda como `^`. Y abajo aparece una **sonrisa**: un pedazo de círculo que crece cuanto más contento está |
| 😠 Enojado | Un triángulo negro corta la esquina de arriba que da a la nariz |
| 😢 Triste | Un triángulo negro corta la esquina de arriba del lado de afuera |
| 😵‍💫 Mareado | En vez del ojo se dibuja una espiral que gira |
| 😍 Enamorado | En vez del ojo se dibuja un **corazón**: dos círculos y un triángulo. Late porque se agranda y se achica |
| 😎 Verano | Se dibujan unos **lentes** blancos arriba de los ojos, con dos rayitas negras que parecen el brillo del vidrio |

### ¿Cómo elige qué juego hacer?

Cada juego tiene un **peso**. Es como poner papelitos en una bolsa: el juego "mirar" tiene
10 papelitos y "llorar" tiene 1, así que "mirar" sale mucho más seguido. Pitri saca un
papelito al azar y hace ese juego.

---

## 🚶 El sensor de movimiento: ver el calor

Todo lo que está vivo da **calor**, y el calor brilla con una luz que nuestros ojos no
ven: la luz **infrarroja**. El sensor **PIR** sí la ve.

Cuando una persona camina delante del sensor, el calor que ve **cambia**, y el sensor
avisa poniendo su cable de salida en "1". Por eso detecta **movimiento**: si te quedás
muy quieto, ¡deja de verte!

La cúpula blanca del sensor es una **lente** que junta el calor de toda la habitación.

> 🧑‍🔧 **Para grandes:** el HC-SR501 usa el chip BISS0001. Necesita 5 V, pero su salida es
> de 3,3 V (segura para el ESP32). Tarda casi un minuto en estabilizarse al prenderse, así
> que [motion.c](../components/motion/motion.c) ignora los primeros 30 segundos. Una tarea
> lo revisa cada 50 ms, avisa por consola y guarda la hora del último movimiento: con eso
> Pitri sabe cuándo dormirse.

---

## 🌡️ El sensor de temperatura: mensajes en código

El sensor **DHT11** mide la temperatura y la humedad del aire. Para contarle al ESP32 lo
que midió, usa **un solo cable** y un código parecido al **Morse**:

- un pulso **cortito** quiere decir **0**
- un pulso **largo** quiere decir **1**

```
        cortito = 0           largo = 1
       ┌──┐                 ┌───────┐
  ─────┘  └─────       ─────┘       └─────
```

Manda **40** de estos ceros y unos, todos juntos, en menos de 5 milésimas de segundo.
Con 40 ceros y unos se arman 5 números:

| Número | Qué dice |
|---|---|
| 1 y 2 | La humedad |
| 3 y 4 | La temperatura |
| 5 | Un número de **control**: la suma de los otros cuatro |

El número de control sirve para saber si el mensaje llegó bien: si la suma no da, algo
se perdió en el camino y Pitri descarta el mensaje.

> 🧑‍🔧 **Para grandes:** el protocolo está explicado paso a paso en
> [dht.c](../components/dht/dht.c). El driver sirve para el DHT11 y el DHT22 (se elige con
> `SENSOR_TH_MODELO` en `board_config.h`); cambian el pulso de inicio (18 ms vs 1 ms) y el
> formato de los números. Como los tiempos son de microsegundos, mientras escucha bloquea
> las interrupciones, y por eso corre en el otro núcleo del ESP32 que el display.

---

## 😴 Dormir

El ayudante del sensor de movimiento anota **la hora de la última vez que vio a alguien**.
El programa principal pregunta todo el tiempo: *"¿cuánto hace que no ves a nadie?"*.
Si pasó más de un minuto, Pitri cierra los ojos y se queda "respirando" (los ojos cerrados
suben y bajan y sale una Z) hasta que el sensor vuelva a ver a alguien.
