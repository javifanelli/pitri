# 🔍 Como funciona Pitri

Aca explicamos como hace Pitri cada cosa. Las partes marcadas con 🧑‍🔧 **Para grandes**
tienen mas detalles tecnicos: se pueden saltear.

---

## 🧠 El cerebro: un programa que nunca termina

El programa de Pitri es como una lista de instrucciones que se repite **para siempre**:

```
Al prenderse:
   1. Preparar la pantalla y los sensores
   2. Mostrar "¡Hola Sofi!" y "Soy Pitri"
   3. Mirar las ondas de WiFi hasta conectarse, y decir "Tengo wifi!"
   4. Abrir los ojos y saludar

Y despues, una y otra vez:
   ¿Apretaron el boton 1?           →  abro el menu
   ¿Hace mucho que no veo a nadie?  →  me duermo hasta que llegue alguien
   ¿Llego alguien?                  →  ¡lo saludo!
   ¿Es en punto (x:00, x:05...)?    →  miro la hora (y cada 15, la temperatura)
   Elijo un juego al azar y lo hago (o, en un modo especial, hago de un personaje)
   Descanso un ratito
```

Eso esta en [main/main.c](../main/main.c), en la parte que dice `while (1)`.
`while (1)` quiere decir "repetir para siempre".

Mientras tanto, hay **ayudantes** que trabajan al mismo tiempo, sin que el programa
principal se tenga que acordar de ellos:

- uno vigila el **sensor de movimiento** todo el tiempo,
- otro lee el **sensor de temperatura** cada 10 segundos,
- y otro vigila los **botones**.

> 🧑‍🔧 **Para grandes:** los "ayudantes" son *tareas* de FreeRTOS, el sistema operativo que
> trae el ESP32. Cada una tiene su propio `while (1)` y el ESP32 las va alternando muy rapido
> (ademas tiene 2 nucleos, asi que algunas corren de verdad al mismo tiempo).

---

## 🖥️ La pantalla: un tablero de lucecitas

La pantalla tiene **128 lucecitas de ancho y 64 de alto**. Cada lucecita se llama
**pixel** y puede estar **prendida** (blanca) o **apagada** (negra). Nada mas.

```
  x →  0 1 2 3 4 5 6 7 ...                     127
y 0    · · · · · · · · · · · · · · · · · · · · ·
↓ 1    · · · █ █ █ · · · · · · · · · █ █ █ · · ·
  2    · · · █ █ █ · · · · · · · · · █ █ █ · · ·
  3    · · · █ █ █ · · · · · · · · · █ █ █ · · ·
  ...
  63
```

Cada pixel tiene una direccion, como en la batalla naval: **x** dice que tan a la derecha
esta y **y** que tan abajo. ¡Ojo! En las pantallas, **y crece para abajo**.

### Dibujar sin que se note

Si Pitri dibujara directo en la pantalla, veriamos como va pintando de a pedacitos.
Por eso hace como los dibujantes de dibujos animados:

1. Dibuja todo en un **papel escondido** (en la memoria del ESP32).
2. Cuando el dibujo esta listo, lo **manda entero** a la pantalla de una sola vez.

Esas dos cosas son `display_clear()` (borrar el papel) y `display_flush()` (mandarlo).

> 🧑‍🔧 **Para grandes:** el "papel escondido" es un *framebuffer* de 1024 bytes
> (128 × 64 ÷ 8, un bit por pixel). Se manda por **I2C**, un protocolo de 2 cables
> (SDA = datos, SCL = reloj) a 400 kHz. Cada cuadro tarda unos 25 ms en viajar.
> La pantalla esta montada al reves, asi que justo antes de mandarlo
> [display.c](../components/display/display.c) gira el dibujo 180°.

### Las formas que sabe dibujar

| Funcion | Dibuja |
|---|---|
| `display_set_pixel` | Un solo pixel |
| `display_fill_rect` | Un rectangulo |
| `display_fill_round_rect` | Un rectangulo con las esquinas redondeadas (¡los ojos!) |
| `display_fill_circle` | Un circulo |
| `display_fill_triangle` | Un triangulo (las cejas de enojado y triste) |
| `display_draw_text` | Letras |

### Las letras

Cada letra es un dibujito de **5 pixeles de ancho y 7 de alto**. Por ejemplo, la **A**:

```
 · █ █ █ ·
 █ · · · █
 █ · · · █
 █ █ █ █ █
 █ · · · █
 █ · · · █
 █ · · · █
```

Todas las letras estan guardadas en [font5x7.h](../components/display/font5x7.h).
Para que se vean mas grandes, cada pixel de la letra se dibuja como un cuadradito de
2×2 o 3×3.

---

## 👀 Los ojos: dibujos animados

Los ojos de Pitri son **dos rectangulos con las esquinas redondeadas**. Para que parezca
que se mueven, Pitri hace lo mismo que en los dibujos animados: dibuja **muchos dibujos
apenas distintos, uno atras del otro, muy rapido**. Cada dibujo se llama **cuadro**.

Pitri dibuja unos **33 cuadros por segundo** (uno cada 30 milisegundos). Tu ojo no llega
a ver los saltos, ¡y parece movimiento!

### Parpadear

Un parpadeo es achicar la altura de los ojos hasta que quedan como una rayita, y despues
volver a agrandarla:

```
 ████    ████    ▄▄▄▄    ────    ▄▄▄▄    ████    ████
 ████    ████    ▀▀▀▀            ▀▀▀▀    ████    ████
 ████  →        →       →       →       →        → ████
abierto                cerrado                   abierto
```

### Moverse suave

Para ir de un lugar a otro, Pitri **no salta**: calcula todos los puntos del medio.
Y ademas arranca despacito, acelera y frena despacito, como cuando moves la cabeza.

> 🧑‍🔧 **Para grandes:** la "cara" es una estructura (`cara_t`) con posicion, tamaño,
> apertura de cada ojo, y cuanto de feliz, enojado, triste o mareado esta. La funcion
> `animar(destino, ms)` interpola **todos** esos valores a la vez entre la cara actual y la
> de destino, con una curva *smoothstep* (`t² · (3 − 2t)`) para arrancar y frenar suave.
> Por eso cualquier combinacion funciona sola: moverse mientras se cierra un ojo, etc.

### Las expresiones

Las caras distintas se hacen **tapando partes de los ojos con negro**:

| Expresion | Truco |
|---|---|
| 😊 Feliz | Una "loma" negra tapa la parte de abajo del ojo → queda como `^`. Y abajo aparece una **sonrisa**: un pedazo de circulo que crece cuanto mas contento esta |
| 😠 Enojado | Un triangulo negro corta la esquina de arriba que da a la nariz |
| 😢 Triste | Un triangulo negro corta la esquina de arriba del lado de afuera |
| 😵‍💫 Mareado | En vez del ojo se dibuja una espiral que gira |
| 😍 Enamorado | En vez del ojo se dibuja un **corazon**: dos circulos y un triangulo. Late porque se agranda y se achica |
| 😎 Verano | Se dibujan unos **lentes** blancos arriba de los ojos, con dos rayitas negras que parecen el brillo del vidrio |

### La boca

Cada emocion tiene su boca, y todas se dibujan con tres herramientas: **pedazos de
circulo**, **lineas rectas** y **anillos** (un circulo blanco con un agujero negro).

| Emocion | Boca | Como se dibuja |
|---|---|---|
| 🙂 Tranquilo | Sonrisita | Un pedacito de circulo, chiquito y finito |
| 😊 Contento | Sonrisa | La parte de **abajo** de un circulo. Crece cuanto mas contento esta |
| 😢 Triste | Sonrisa al reves | La parte de **arriba** de un circulo |
| 😠 Enojado | Zigzag | Seis rayitas que suben y bajan, como dientes apretados |
| 😮 Sorpresa | O | Un anillo |
| 😵‍💫 Mareado | Ola | Una onda que se mueve al mismo ritmo que las espirales |
| 😍 Enamorado | 3 | Dos pedazos de circulo, uno arriba del otro |
| 😎 Verano | Sonrisa de costado | Una linea recta con una punta que sube |
| 🤨 Sospecha | Rayita torcida | Una linea cortita inclinada |
| 😴 Dormido | o chiquita | Un anillo chiquito, como roncando |

Pitri **siempre** tiene boca: cuando no siente nada especial (mirando, paseando,
parpadeando), lleva la sonrisita tranquila. Y si tiene dos emociones a la vez (por ejemplo,
mientras pasa de enamorado a contento), muestra la boca de la emocion **mas fuerte**.

### Mirar para un costado

Para que parezca que Pitri mira hacia un lado, el ojo de ese lado se dibuja **un poquito
mas grande**, como cuando giras la cabeza. Normalmente mira hacia donde estan sus ojos,
pero cuando muestra la temperatura hace algo especial: se achica, se corre a la
**izquierda** y mira hacia la **derecha**, donde aparecen los datos.

### Darle de comer

En el menu, **Comer** muestra las comidas de a una, con su dibujo grande: torta, papas
fritas y hamburguesa. Estan hechas con las formas de siempre: la hamburguesa es medio
circulo (el pan), una linea ondulada (la lechuga) y dos rectangulos redondeados (la carne
y el pan de abajo). Las semillas son puntitos negros.

Cada dibujo se puede hacer **grande o chiquito**: todas sus medidas se multiplican por un
numero (el tamano). En el menu se ve grande y cuando Pitri come, chiquito.

Cuando Pitri come:

1. La comida entra por la derecha y Pitri la mira con la boca en "O".
2. Se la acerca a la boca y le da **3 mordiscos**. Cada mordisco es un **circulo negro**
   que le "borra" un pedazo a la comida. Con el tercero, ¡se la termina!
3. Mastica, pone cara feliz y dice "¡Rico!".

**La pancita de Pitri:** Pitri anota cuando comio. Si ya comio **3 veces en la ultima
hora**, esta lleno: mueve la cabeza diciendo que no. A medida que pasa el tiempo, las
comidas viejas dejan de contar y puede volver a comer. Si no le dan de comer, no le pasa
nada malo: **Pitri nunca muere**.

### Disfraces: los personajes de los modos

En los modos especiales, Pitri se "disfraza". Hay dos clases de disfraces:

- **Encima de los ojos de Pitri:** el vampiro (pelo con pico, cuello de capa y colmillos),
  Papa Noel (gorro, barba y bigote), la gallina (cresta, pico y moquito) y el cerdo
  (orejas y hocico). Pitri sigue parpadeando y mirando con sus ojos de siempre.
- **Un dibujo entero:** el fantasma, la calabaza, el arbolito, la galleta y la tortuga.
  Tienen sus propios ojos (huecos, triangulares, puntitos...) que igual parpadean.

Todos estan hechos con las mismas formas de siempre: circulos, rectangulos redondeados,
triangulos y lineas. Por ejemplo, la **estrella** del arbolito son 5 triangulos (las
puntas) mas el centro, y el borde ondulado del **fantasma** son circulos negros que le
"muerden" la parte de abajo.

Para cambiar de disfraz, Pitri **cierra los ojos**, se cambia y los vuelve a abrir.

### ¿Como elige que juego hacer?

Cada juego tiene un **peso**. Es como poner papelitos en una bolsa: el juego "mirar" tiene
10 papelitos y "llorar" tiene 1, asi que "mirar" sale mucho mas seguido. Pitri saca un
papelito al azar y hace ese juego.

---

## 🚶 El sensor de movimiento: ver el calor

Todo lo que esta vivo da **calor**, y el calor brilla con una luz que nuestros ojos no
ven: la luz **infrarroja**. El sensor **PIR** si la ve.

Cuando una persona camina delante del sensor, el calor que ve **cambia**, y el sensor
avisa poniendo su cable de salida en "1". Por eso detecta **movimiento**: si te quedas
muy quieto, ¡deja de verte!

La cupula blanca del sensor es una **lente** que junta el calor de toda la habitacion.

> 🧑‍🔧 **Para grandes:** el HC-SR501 usa el chip BISS0001. Necesita 5 V, pero su salida es
> de 3,3 V (segura para el ESP32). Tarda casi un minuto en estabilizarse al prenderse, asi
> que [motion.c](../components/motion/motion.c) ignora los primeros 30 segundos. Una tarea
> lo revisa cada 50 ms, avisa por consola y guarda la hora del ultimo movimiento: con eso
> Pitri sabe cuando dormirse.

---

## 🌡️ El sensor de temperatura: mensajes en codigo

El sensor **DHT11** mide la temperatura y la humedad del aire. Para contarle al ESP32 lo
que midio, usa **un solo cable** y un codigo parecido al **Morse**:

- un pulso **cortito** quiere decir **0**
- un pulso **largo** quiere decir **1**

```
        cortito = 0           largo = 1
       ┌──┐                 ┌───────┐
  ─────┘  └─────       ─────┘       └─────
```

Manda **40** de estos ceros y unos, todos juntos, en menos de 5 milesimas de segundo.
Con 40 ceros y unos se arman 5 numeros:

| Numero | Que dice |
|---|---|
| 1 y 2 | La humedad |
| 3 y 4 | La temperatura |
| 5 | Un numero de **control**: la suma de los otros cuatro |

El numero de control sirve para saber si el mensaje llego bien: si la suma no da, algo
se perdio en el camino y Pitri descarta el mensaje.

> 🧑‍🔧 **Para grandes:** el protocolo esta explicado paso a paso en
> [dht.c](../components/dht/dht.c). El driver sirve para el DHT11 y el DHT22 (se elige con
> `SENSOR_TH_MODELO` en `board_config.h`); cambian el pulso de inicio (18 ms vs 1 ms) y el
> formato de los numeros. Como los tiempos son de microsegundos, mientras escucha bloquea
> las interrupciones, y por eso corre en el otro nucleo del ESP32 que el display.

---

## 🔘 Los botones: un cable que se une a tierra

Cada boton tiene dos patas: una va a un pin del ESP32 y la otra a **GND** (tierra, el "0").
Adentro del ESP32 hay una resistencia (el *pull-up*) que, cuando nadie toca el boton,
mantiene el pin en **1**. Al apretar, el boton une el pin con GND y el pin pasa a **0**.

```
  suelto:   pin ── 1        apretado:   pin ──┐
                                              └── GND  →  0
```

### El rebote

Cuando apretas un boton, las chapitas de metal de adentro **rebotan** y durante unos
milisegundos se tocan y se separan varias veces. Si Pitri no tuviera cuidado, ¡contaria
varias apretadas! Por eso solo cree que el boton cambio cuando lo ve igual **3 veces
seguidas** (30 milisegundos).

> 🧑‍🔧 **Para grandes:** [botones.c](../components/botones/botones.c) tiene una tarea que
> lee los pines cada 10 ms con *debounce* por contador, loguea cada apretada y deja una
> marca que se consume con `botones_se_apreto(n)` (o se mira sin consumir con
> `botones_pendiente(n)`).

### El menu

El **boton 1** abre el menu. Las opciones aparecen una abajo de la otra, y la elegida
tiene una **barra blanca con letras negras** (como un resaltador). El **boton 2** mueve la
barra a la opcion siguiente; al llegar a la ultima, vuelve a la primera. El **boton 1**
elige la opcion de la barra.

El menu principal no tiene titulo (ya se sabe que es el menu), asi que entran sus 4
opciones juntas. Los submenus, como **Modo**, tienen titulo arriba y entran 3 opciones: si
hay mas, aparecen flechitas ▲▼ y la lista se corre a medida que bajas.

### Que el boton responda al instante

Algunos juegos de Pitri duran varios segundos. Si el boton tuviera que esperar a que
terminen, ¡pareceria que no anda! Por eso, en **cada cuadro** de la animacion (33 veces por
segundo), Pitri se pregunta: *"¿apretaron el boton?"*. Si la respuesta es si, deja todo lo
que estaba haciendo y abre el menu.

---

## 📶 El WiFi: hablar sin cables

El ESP32 tiene una **antena** (el zigzag dorado de arriba de la placa) con la que puede
hablar con el **router** de la casa por ondas de radio, igual que un celular. A traves del
router, Pitri puede llegar a **internet**.

Para entrar a una red WiFi hacen falta dos cosas: el **nombre de la red** y la
**contraseña**. Y aca aprendimos algo importante:

> 🔒 **Los secretos no se publican.** El codigo de Pitri esta en internet para que
> cualquiera lo pueda ver y aprender. Pero la contraseña del WiFi es un secreto de la casa.
> Por eso la guardamos en un archivo aparte, `wifi_secretos.h`, que **se queda en nuestra
> compu y nunca se sube**. En internet solo esta una plantilla con datos de mentira.
> Por la misma razon, Pitri nunca escribe la contraseña ni el nombre de la red en la consola.

Al prenderse, mientras se conecta, Pitri se corre a un costado y mira un **simbolo de
WiFi**: un puntito y tres ondas que se van encendiendo de a una, como cuando el celular
busca señal. Cuando se conecta, las ondas se cambian por "Tengo wifi!".

Si el WiFi se corta, Pitri lo vuelve a intentar solo: primero al segundo, despues a los 2,
a los 4, a los 8… hasta probar una vez por minuto, para no cansarse (ni cansar al router).

> 🧑‍🔧 **Para grandes:** [wifi.c](../components/wifi/wifi.c) usa el modo *estacion*
> (se conecta a un router), escucha los eventos de conexion, desconexion y obtencion de IP,
> y reintenta con *backoff* exponencial usando un `esp_timer`. Los datos de la red quedan
> solo en RAM (`WIFI_STORAGE_RAM`), no se graban en la flash.

---

## 🕐 La hora: preguntarle a internet

El ESP32 tiene un relojito adentro, pero cuando se apaga se olvida la hora, y ademas no
es muy exacto. Por eso, apenas tiene WiFi, Pitri **le pregunta la hora a internet**.

En internet hay servidores que tienen relojes **muy, muy exactos** (algunos conectados a
relojes atomicos). Hablan un idioma que se llama **NTP**. Pitri les pregunta "¿que hora
es?", ellos le contestan, y Pitri pone en hora su relojito. Cada tanto vuelve a preguntar
para corregirlo.

Los servidores contestan la **hora mundial** (UTC). Para saber la hora de casa hay que
decirle a Pitri cuantas horas de diferencia hay: eso es la **zona horaria**
(`ZONA_HORARIA` en [pitri_config.h](../main/pitri_config.h)).

Pitri muestra la hora solo, **en punto**: cada 5 minutos (x:00, x:05, x:10...) se corre a
un costado y la mira. Y cada 15 minutos (x:00, x:15, x:30, x:45) mira tambien la
temperatura y la humedad. Para saber si es "en punto", se fija si los minutos se pueden
dividir justo por 5 (o por 15).

Tambien se puede ver cuando uno quiere: en el menu, **Hora** la muestra grande, con el
dia y la fecha, y los dos puntos titilan una vez por segundo, como en un reloj de verdad.
Y **Sensor** muestra la temperatura grande y la humedad, con el mismo estilo.

> 🧑‍🔧 **Para grandes:** [reloj.c](../components/reloj/reloj.c) espera en una tarea a que
> `wifi_conectado()` sea true, inicia `esp_netif_sntp` contra `pool.ntp.org` y marca
> `reloj_sincronizado()` en el callback. La zona horaria se aplica con `setenv("TZ")` +
> `tzset()`, asi `localtime_r()` devuelve la hora local.

---

## 😴 Dormir

El ayudante del sensor de movimiento anota **la hora de la ultima vez que vio a alguien**.
El programa principal pregunta todo el tiempo: *"¿cuanto hace que no ves a nadie?"*.
Si paso mas de un minuto, Pitri cierra los ojos y se queda "respirando" (los ojos cerrados
suben y bajan y sale una Z) hasta que el sensor vuelva a ver a alguien. Esto pasa en
**cualquier modo**: si estaba disfrazado, primero se saca el disfraz.
