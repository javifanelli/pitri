# 📔 Diario de Pitri

Aca anotamos todo lo que le fuimos enseñando a Pitri y lo que aprendimos en el camino
(¡incluso cuando algo salio mal!). Lo mas nuevo va arriba.

> **Para anotar algo nuevo**, copia esto arriba de todo y completalo:
>
> ```
> ## AAAA-MM-DD — Titulo
>
> **Que hicimos:** …
> **Que aprendimos:** …
> ```

---

## 2026-11-03 — La hora en el menu y chau tildes 🕐

**Que hicimos:**
- El menu ahora tiene tres opciones: **Modo**, **Hora** y **Salir**. Con **Hora**, Pitri muestra la hora grande, el dia y la fecha durante 5 segundos, y despues vuelve a lo que estaba haciendo.
- Sacamos las tildes de **todo** el proyecto: codigo, documentacion y pantalla.

**Que aprendimos:** Los dos puntos de la hora titilan una vez por segundo: se dibujan cuando el segundo es par y se borran cuando es impar.

---

## 2026-11-03 — Pitri sabe la hora 🕐

**Que hicimos:** Apenas tiene WiFi, Pitri le pregunta la hora a internet y pone en hora su relojito. Todavia no la muestra en ningun lado: ya le vamos a buscar un lugar.

Tambien decidimos escribir los textos de la pantalla **sin tildes** ("Menu", "¡Atrapame!").

**Que aprendimos:**
- En internet hay relojes **muy exactos** que contestan la hora con un idioma llamado **NTP**.
- Esos relojes dan la **hora mundial**; para tener la hora de casa hay que decir cuantas horas de diferencia hay (la **zona horaria**).

---

## 2026-11-03 — ¡Menu y modos! 🎃🎄🐾

**Que hicimos:** ¡Los botones por fin tienen trabajo! El **boton 1** abre un **menu** y el **boton 2** baja por las opciones. En el menu se elige un **modo**:
- 🎃 **Halloween:** Pitri hace de vampiro, fantasma y calabaza.
- 🎄 **Navidad:** hace de Papa Noel, muestra un arbolito con luces y nieve, y hace de galleta de jengibre.
- 🐾 **Animales:** hace de gallina, cerdo y tortuga.

Ademas, la pantalla aprendio las **letras con tilde y la ñ**, aunque al final decidimos no usar tildes y las sacamos.

**Que aprendimos:**
- Si un boton tarda en responder, parece que esta roto. Por eso Pitri se fija **33 veces por segundo** si lo apretaron, y si es asi deja todo y abre el menu.
- Con las mismas formas de siempre (circulos, triangulos, rectangulos y lineas) se puede dibujar casi cualquier cosa: una estrella son 5 triangulos, y el borde del fantasma son circulos negros que le "muerden" la sabana.
- Simulamos los 9 personajes en la compu antes de cargarlos en Pitri.

---

## 2026-11-02 — Siempre con boca 🙂

**Que hicimos:** Ahora Pitri **siempre** tiene boca. Cuando esta tranquilo (mirando, paseando, parpadeando) lleva una sonrisita chiquita, y cuando siente algo, la boca de esa emocion.

**Que aprendimos:** La boca va justo abajo de los ojos, pero cuando Pitri parpadea los ojos se achican… ¡y la boca saltaba para arriba! Lo arreglamos haciendo que la sonrisita tranquila se ubique como si los ojos estuvieran siempre abiertos.

---

## 2026-11-02 — Botones y Pitri mirando los datos 🔘👀

**Que hicimos:**
- Conectamos los **2 botones**. Por ahora, cuando los apretas, Pitri lo avisa en la consola.
- Ahora Pitri muestra la temperatura y la humedad **cada 5 minutos**, con letra chica. Su carita se achica, se corre a un costado y **mira los datos**, que entran deslizandose.

**Que aprendimos:**
- Los botones **rebotan**: al apretarlos, el metal hace contacto varias veces seguidas. Para no contar de mas, Pitri espera a que el boton se quede quieto.
- Para que Pitri pareciera mirar a un costado sin moverse para ese lado, separamos "donde estan los ojos" de "hacia donde miran".

---

## 2026-11-02 — ¡Pitri tiene WiFi! 📶

**Que hicimos:** Conectamos a Pitri a la red WiFi de casa. La primera vez que se conecta, dice **"¡Tengo WiFi!"** y se pone contento. Si se corta, lo vuelve a intentar solo.

**Que aprendimos:**
- **Los secretos no se publican.** El codigo de Pitri esta en internet, pero la contraseña del WiFi es un secreto de la casa. Por eso vive en un archivo que se queda en nuestra compu y nunca se sube. En internet solo hay una plantilla con datos de mentira.
- Pitri tampoco escribe la contraseña ni el nombre de la red en la consola, por si alguna vez compartimos lo que aparece ahi.

---

## 2026-11-02 — Una boca para cada emocion 👄

**Que hicimos:** ¡A Sofia le encanto la sonrisa! Asi que le pusimos boca a **todas** las emociones, cada una con su forma: triste (sonrisa al reves), enojado (zigzag), sorpresa (una O), mareado (una ola que se mueve), enamorado (un 3 de beso), verano (sonrisa de costado), sospecha (rayita torcida) y dormido (una o chiquita).

**Que aprendimos:**
- Con solo tres herramientas (pedazos de circulo, lineas y anillos) se pueden dibujar un monton de bocas distintas.
- Para que la boca en "O" entrara debajo de los ojos gigantes de la sorpresa, hubo que **subir un poco la cara**.
- En la simulacion, la boca de beso salio tan chiquita que parecia una rayita: la agrandamos hasta que se vio el "3".

---

## 2026-11-02 — Pitri sonrie 😊

**Que hicimos:** Sofia pidio que Pitri tenga una **boca sonriente** cuando esta contento. Ahora la sonrisa aparece cada vez que pone ojitos felices: cuando esta contento, cuando guiña, cuando saluda y cuando se saca los lentes de sol.

**Que aprendimos:**
- Una sonrisa es **un pedazo de circulo**: la parte de abajo.
- Cuanto mas contento esta Pitri, mas grande es la sonrisa.
- Simulamos el dibujo y vimos que, con media sonrisa, las puntas tocaban los ojos. Lo arreglamos bajando un poquito la boca cuando esta menos contento.

---

## 2026-11-02 — Enamorado y verano 😍😎

**Que hicimos:** Para empezar noviembre, Sofia pidio tres cosas:
- Que despues de "¡Hola Sofi!" Pitri se presente: **"Soy Pitri"**.
- Una emocion de **enamorado**: los ojos se vuelven corazones que laten y suben corazoncitos flotando.
- Una emocion **veraniega**: bajan unos lentes de sol, Pitri baila canchero y despues se los saca guiñando un ojo.

**Que aprendimos:**
- Un corazon se puede dibujar con **dos circulos y un triangulo**.
- Antes de cargarlo en Pitri, simulamos el dibujo en la compu y vimos que los ojos asomaban por debajo de los lentes. Lo arreglamos haciendo que Pitri **entrecierre los ojos** cuando se pone los lentes, como alguien tomando sol.

---

## 2026-11-01 — Documentacion

**Que hicimos:** Escribimos estas guias para que cualquiera pueda entender como funciona
Pitri y enseñarle cosas nuevas.

---

## 2026-11-01 — Temperatura y humedad 🌡️

**Que hicimos:** Conectamos un sensor de temperatura. Cada un minuto Pitri muestra cuantos
grados hace y cuanta humedad hay. Le enseñamos a la pantalla el simbolo de grados (°).

**Que aprendimos:**
- El primer sensor (un DHT22) mandaba temperaturas rarisimas: ¡decia que hacian 1 °C con
  20 °C de verdad! Revisamos los mensajes bit por bit y estaban perfectos, asi que el
  sensor estaba midiendo mal. Lo cambiamos por un **DHT11**.
- Los DHT11 no contestaban nada… y era porque **los cables estaban al reves** en la plaquita
  del sensor. Moraleja: **siempre leer lo que dice impreso al lado de cada pata**, no
  guiarse por la posicion.
- Para encontrarlo, le enseñamos al programa a **probar el pin solo**: lo intentaba poner
  en 0 y no podia, porque estaba conectado a la alimentacion.

---

## 2026-11-01 — La pantalla da vuelta 🔄

**Que hicimos:** Como la pantalla quedo montada al reves en la placa, le enseñamos a Pitri
a dibujar todo dado vuelta (180°).

---

## 2026-11-01 — ¡Hola Sofi! 👋

**Que hicimos:** Al prenderse, Pitri dice "¡Hola Sofi!". Para eso le enseñamos a la
pantalla a escribir **letras**: cada letra es un dibujito de 5×7 puntitos.

**Que aprendimos:** "¡Hola Sofi!" en una sola linea no entraba en la pantalla (le faltaban
2 puntitos), asi que lo pusimos en dos renglones y bien grande.

---

## 2026-11-01 — Pitri se duerme 😴

**Que hicimos:** Si pasa un minuto sin que nadie se mueva, Pitri cierra los ojos, respira
despacito y le salen "Z". Cuando alguien llega, se despierta y saluda.

---

## 2026-11-01 — Placa nueva y orden 🔌

**Que hicimos:**
- Rearmamos la placa pensando en el futuro (WiFi y audio) y dibujamos todas las
  conexiones en [CONEXIONES.md](CONEXIONES.md).
- Juntamos las hojas tecnicas (datasheets) de cada parte.
- Ordenamos el codigo en "componentes": una carpeta para cada cosa.

**Que aprendimos:**
- No todos los pines del ESP32 sirven para todo: algunos deciden como arranca la placa,
  otros los usa el WiFi, y la salida de audio solo puede ir en dos pines.
- Antes de dibujar el plano, hay que mirar **la placa de verdad**: la primera vez lo
  dibujamos para otra placa parecida y no coincidia.
- El display a veces se "trababa" por los cables; le enseñamos a reiniciarse solo.

---

## 2026-11-01 — Sensor de movimiento 🚶

**Que hicimos:** Le pusimos un sensor PIR. Cuando alguien llega, Pitri se sorprende y se
pone contento.

**Que aprendimos:** El sensor PIR ve el **calor** de las personas (luz infrarroja). Al
prenderse necesita casi un minuto para "calentarse" y en ese tiempo inventa movimientos.

---

## 2026-11-01 — Mareado, enojado y llorando 😵‍💫😠😢

**Que hicimos:** Sofia pidio tres juegos nuevos: ojos en espiral, cejas de enojado y
lagrimas. Para eso le enseñamos a la pantalla a dibujar **circulos y triangulos**.

---

## 2026-11-01 — Los ojos se mueven 👀

**Que hicimos:** Pitri empezo a mirar para todos lados, guiñar, sorprenderse, ponerse
contento, sospechar, dar vueltas y pasear por la pantalla. Elige que hacer al azar.

**Que aprendimos:** Para que algo parezca moverse, hay que dibujarlo muchas veces por
segundo, cada vez un poquito distinto (como en los dibujos animados).

---

## 2026-11-01 — ¡Nace Pitri! 🐣

**Que hicimos:** Reciclamos la placa de un proyecto anterior (una estacion de
temperatura) y dibujamos los primeros ojitos, que parpadeaban cada 2 segundos.
