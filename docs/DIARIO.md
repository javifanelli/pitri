# 📔 Diario de Pitri

Acá anotamos todo lo que le fuimos enseñando a Pitri y lo que aprendimos en el camino
(¡incluso cuando algo salió mal!). Lo más nuevo va arriba.

> **Para anotar algo nuevo**, copiá esto arriba de todo y completalo:
>
> ```
> ## AAAA-MM-DD — Título
>
> **Qué hicimos:** …
> **Qué aprendimos:** …
> ```

---

## 2026-11-02 — Una boca para cada emoción 👄

**Qué hicimos:** ¡A Sofía le encantó la sonrisa! Así que le pusimos boca a **todas** las emociones, cada una con su forma: triste (sonrisa al revés), enojado (zigzag), sorpresa (una O), mareado (una ola que se mueve), enamorado (un 3 de beso), verano (sonrisa de costado), sospecha (rayita torcida) y dormido (una o chiquita).

**Qué aprendimos:**
- Con solo tres herramientas (pedazos de círculo, líneas y anillos) se pueden dibujar un montón de bocas distintas.
- Para que la boca en "O" entrara debajo de los ojos gigantes de la sorpresa, hubo que **subir un poco la cara**.
- En la simulación, la boca de beso salió tan chiquita que parecía una rayita: la agrandamos hasta que se vio el "3".

---

## 2026-11-02 — Pitri sonríe 😊

**Qué hicimos:** Sofía pidió que Pitri tenga una **boca sonriente** cuando está contento. Ahora la sonrisa aparece cada vez que pone ojitos felices: cuando está contento, cuando guiña, cuando saluda y cuando se saca los lentes de sol.

**Qué aprendimos:**
- Una sonrisa es **un pedazo de círculo**: la parte de abajo.
- Cuanto más contento está Pitri, más grande es la sonrisa.
- Simulamos el dibujo y vimos que, con media sonrisa, las puntas tocaban los ojos. Lo arreglamos bajando un poquito la boca cuando está menos contento.

---

## 2026-11-02 — Enamorado y verano 😍😎

**Qué hicimos:** Para empezar noviembre, Sofía pidió tres cosas:
- Que después de "¡Hola Sofi!" Pitri se presente: **"Soy Pitri"**.
- Una emoción de **enamorado**: los ojos se vuelven corazones que laten y suben corazoncitos flotando.
- Una emoción **veraniega**: bajan unos lentes de sol, Pitri baila canchero y después se los saca guiñando un ojo.

**Qué aprendimos:**
- Un corazón se puede dibujar con **dos círculos y un triángulo**.
- Antes de cargarlo en Pitri, simulamos el dibujo en la compu y vimos que los ojos asomaban por debajo de los lentes. Lo arreglamos haciendo que Pitri **entrecierre los ojos** cuando se pone los lentes, como alguien tomando sol.

---

## 2026-11-01 — Documentación

**Qué hicimos:** Escribimos estas guías para que cualquiera pueda entender cómo funciona
Pitri y enseñarle cosas nuevas.

---

## 2026-11-01 — Temperatura y humedad 🌡️

**Qué hicimos:** Conectamos un sensor de temperatura. Cada un minuto Pitri muestra cuántos
grados hace y cuánta humedad hay. Le enseñamos a la pantalla el símbolo de grados (°).

**Qué aprendimos:**
- El primer sensor (un DHT22) mandaba temperaturas rarísimas: ¡decía que hacían 1 °C con
  20 °C de verdad! Revisamos los mensajes bit por bit y estaban perfectos, así que el
  sensor estaba midiendo mal. Lo cambiamos por un **DHT11**.
- Los DHT11 no contestaban nada… y era porque **los cables estaban al revés** en la plaquita
  del sensor. Moraleja: **siempre leer lo que dice impreso al lado de cada pata**, no
  guiarse por la posición.
- Para encontrarlo, le enseñamos al programa a **probar el pin solo**: lo intentaba poner
  en 0 y no podía, porque estaba conectado a la alimentación.

---

## 2026-11-01 — La pantalla da vuelta 🔄

**Qué hicimos:** Como la pantalla quedó montada al revés en la placa, le enseñamos a Pitri
a dibujar todo dado vuelta (180°).

---

## 2026-11-01 — ¡Hola Sofi! 👋

**Qué hicimos:** Al prenderse, Pitri dice "¡Hola Sofi!". Para eso le enseñamos a la
pantalla a escribir **letras**: cada letra es un dibujito de 5×7 puntitos.

**Qué aprendimos:** "¡Hola Sofi!" en una sola línea no entraba en la pantalla (le faltaban
2 puntitos), así que lo pusimos en dos renglones y bien grande.

---

## 2026-11-01 — Pitri se duerme 😴

**Qué hicimos:** Si pasa un minuto sin que nadie se mueva, Pitri cierra los ojos, respira
despacito y le salen "Z". Cuando alguien llega, se despierta y saluda.

---

## 2026-11-01 — Placa nueva y orden 🔌

**Qué hicimos:**
- Rearmamos la placa pensando en el futuro (WiFi y audio) y dibujamos todas las
  conexiones en [CONEXIONES.md](CONEXIONES.md).
- Juntamos las hojas técnicas (datasheets) de cada parte.
- Ordenamos el código en "componentes": una carpeta para cada cosa.

**Qué aprendimos:**
- No todos los pines del ESP32 sirven para todo: algunos deciden cómo arranca la placa,
  otros los usa el WiFi, y la salida de audio solo puede ir en dos pines.
- Antes de dibujar el plano, hay que mirar **la placa de verdad**: la primera vez lo
  dibujamos para otra placa parecida y no coincidía.
- El display a veces se "trababa" por los cables; le enseñamos a reiniciarse solo.

---

## 2026-11-01 — Sensor de movimiento 🚶

**Qué hicimos:** Le pusimos un sensor PIR. Cuando alguien llega, Pitri se sorprende y se
pone contento.

**Qué aprendimos:** El sensor PIR ve el **calor** de las personas (luz infrarroja). Al
prenderse necesita casi un minuto para "calentarse" y en ese tiempo inventa movimientos.

---

## 2026-11-01 — Mareado, enojado y llorando 😵‍💫😠😢

**Qué hicimos:** Sofía pidió tres juegos nuevos: ojos en espiral, cejas de enojado y
lágrimas. Para eso le enseñamos a la pantalla a dibujar **círculos y triángulos**.

---

## 2026-11-01 — Los ojos se mueven 👀

**Qué hicimos:** Pitri empezó a mirar para todos lados, guiñar, sorprenderse, ponerse
contento, sospechar, dar vueltas y pasear por la pantalla. Elige qué hacer al azar.

**Qué aprendimos:** Para que algo parezca moverse, hay que dibujarlo muchas veces por
segundo, cada vez un poquito distinto (como en los dibujos animados).

---

## 2026-11-01 — ¡Nace Pitri! 🐣

**Qué hicimos:** Reciclamos la placa de un proyecto anterior (una estación de
temperatura) y dibujamos los primeros ojitos, que parpadeaban cada 2 segundos.
