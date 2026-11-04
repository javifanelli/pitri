# 🤖 Pitri

**Pitri es un compañero digital.** Tiene una carita en una pantalla, mira para todos lados,
juega, se pone contento cuando llega alguien, se duerme cuando esta solo y sabe que
temperatura hace.

Lo armamos entre Sofia y su papa, de a poquito, y cada vez le vamos enseñando cosas nuevas.

```
   ┌──────────────────────┐
   │                      │
   │    ████      ████    │
   │    ████      ████    │      ← asi se ven los ojos de Pitri
   │    ████      ████    │
   │                      │
   └──────────────────────┘
```

---

## ✨ Que sabe hacer Pitri

| Cuando… | Pitri… |
|---|---|
| Se prende | Dice **"¡Hola Sofi!"**, **"Soy Pitri"** y, mientras se conecta al WiFi, mira unas **ondas** que se van encendiendo |
| Esta tranquilo | Juega solo: mira para los costados, parpadea, guiña un ojo, se marea, se enoja, llora… |
| Llega alguien | Se sorprende 😮 y se pone contento 😊 |
| Pasa un rato sin ver a nadie | Se duerme 😴 y le salen "Z" |
| Cada 5 minutos en punto (x:00, x:05, x:10...) | Se corre a un costado y mira la **hora** |
| Cada 15 minutos en punto (x:00, x:15, x:30, x:45) | Se corre a un costado y mira la **temperatura y la humedad** |
| Apretas el boton 1 | Abre el **menu**, para elegir un **modo** (ver abajo) |
| Se conecta al WiFi | Dice **"Tengo wifi!"** con la carita de costado y **se pone en hora** |

### Los juegos de Pitri

| Juego | Que hace |
|---|---|
| 👀 mirar | Mira a un lugar al azar |
| 🎯 centro | Vuelve a mirar al frente |
| 😑 doble parpadeo | Parpadea dos veces seguidas |
| 😉 guiño | Cierra un ojo con carita feliz |
| 😮 sorpresa | Abre los ojos muy grandes |
| 😊 contento | Ojitos felices ^ ^, sonrisa y saltitos |
| 🔄 girar | Da una vuelta con los ojos |
| 🤨 sospecha | Entrecierra los ojos y mira de un lado al otro |
| 😪 sueño | Se le cierran los ojos, duerme un poquito y se despierta |
| 🚶 pasear | Recorre la pantalla de punta a punta |
| 😵‍💫 mareado | Los ojos se vuelven espirales que giran |
| 😠 enojado | Cejas para adentro y tiembla de bronca |
| 😢 llorar | Cara triste y le caen lagrimas |
| 😍 enamorado | Los ojos se vuelven corazones que laten y suben corazoncitos |
| 😎 verano | Se pone lentes de sol, baila canchero y se los saca guiñando un ojo |

¡Y cada emocion tiene **su propia boca**! Sonrisa cuando esta contento, una "O" cuando se
sorprende, zigzag cuando se enoja, un "3" de beso cuando esta enamorado, y muchas mas.
Cuando esta tranquilo, lleva una sonrisita chiquita.

---

## 🎭 Los modos

Con el **boton 1** se abre el menu. El **boton 2** baja a la opcion siguiente y el
**boton 1** elige. El menu tiene seis opciones: **Modo**, **Comer**, **Tomar**, **Hora**, **Sensor** y **Salir**.

- **Comer**: aparecen los dibujos de una **torta**, unas **papas fritas** y una
  **hamburguesa** (con el boton 2 se pasa de una a otra, y al final esta **Atras** para
  volver al menu). Al elegir una, Pitri se la come a mordiscos y dice "¡Rico!". Puede
  comer hasta **3 veces por hora**: si ya comio, mueve la cabeza diciendo que no porque
  esta lleno. Si no come, no le pasa nada: **Pitri nunca muere**.
- **Tomar**: aparecen **agua** (en botella), **cafe** (en taza) y **mate** (y **Atras**).
  Pitri toma 3 sorbos, el liquido va bajando y dice "Ahh!" (el mate, al terminarse, hace
  "Slurp"). Tambien puede tomar hasta **3 veces por hora**; si ya
  tomo, dice que no.
- **Sensor** muestra la temperatura grande y la humedad durante 5 segundos.
- **Hora** muestra la hora grande, el dia y la fecha durante 5 segundos, y despues Pitri
  vuelve a lo que estaba haciendo.
- En **Modo** se elige que hace Pitri:

| Modo | Pitri hace de… |
|---|---|
| 🙂 **Normal** | Pitri, con todos sus juegos |
| 🎃 **Halloween** | 🧛 vampiro, 👻 fantasma (¡Buu!) y 🎃 calabaza |
| 🎄 **Navidad** | 🎅 Papa Noel (¡Jo jo jo!), 🎄 arbolito con luces y nieve, y 🍪 galleta de jengibre (¡Atrapame!) |
| 🐾 **Animales** | 🐔 gallina (¡Coc!), 🐷 cerdo (¡Oink!) y 🐮 vaca (¡Muu!) |

En los modos especiales, Pitri va cambiando de personaje, y cuando llega alguien lo
recibe con una actuacion. Si nadie toca el menu durante 20 segundos, se cierra solo.

---

## 🧩 De que esta hecho Pitri

| Parte | Para que sirve |
|---|---|
| **ESP32** (la placa negra) | Es el **cerebro**: una computadora chiquita que ejecuta el programa |
| **Pantalla OLED** | Es la **cara**: ahi se dibujan los ojos |
| **Sensor PIR** | Son los **ojos de verdad**: detecta cuando alguien se mueve cerca |
| **Sensor DHT11** | Es la **piel**: siente la temperatura y la humedad del aire |
| **2 botones** | Manejan el menu: el **boton 1** lo abre y elige, el **boton 2** baja a la opcion siguiente |

Como se conecta todo: 👉 [docs/CONEXIONES.md](docs/CONEXIONES.md)

---

## 📚 Para aprender mas

| Documento | Que tiene |
|---|---|
| [Como funciona Pitri](docs/COMO_FUNCIONA.md) | Como dibuja, como se mueve, como "ve" y como "siente" |
| [Inventa un juego nuevo](docs/NUEVO_JUEGO.md) | Paso a paso para enseñarle a Pitri un juego o un personaje nuevo |
| [Conexiones](docs/CONEXIONES.md) | El dibujo de todos los cables |
| [Diario de Pitri](docs/DIARIO.md) | Todo lo que fuimos haciendo y aprendiendo |
| [Para grandes](docs/PARA_GRANDES.md) | Como esta organizado el codigo, como compilarlo y como agregarle cosas |
| [Datasheets](docs/README.md) | Las hojas tecnicas de cada parte |

---

## 🎛️ Cambia como se comporta Pitri

En el archivo [main/pitri_config.h](main/pitri_config.h) hay numeros que se pueden cambiar
sin saber programar. Algunos ejemplos:

```c
#define MENSAJE_ARRANQUE    "¡Hola\nSofi!"               // lo que dice al prenderse
#define MENSAJE_ARRANQUE_2  "Soy\nPitri"                 // y despues
#define TIEMPO_SIN_MOVIMIENTO_PARA_DORMIR_MS  (60 * 1000) // cuando se duerme
#define SENSOR_CADA_MINUTOS  15                           // cada cuanto muestra la temperatura
```

Y en [components/eyes/eyes_interno.h](components/eyes/eyes_interno.h), arriba de todo, esta la
forma de los ojos:

```c
#define OJO_ANCHO        30   // ¿ojos flaquitos o anchos?
#define OJO_ALTO         36   // ¿ojos bajitos o altos?
#define OJO_RADIO        9    // ¿cuadrados o redondos?
#define OJO_SEPARACION   20   // ¿juntitos o separados?
```

> 💡 **Proba:** pone `OJO_RADIO` en `15` y fijate que pasa. Despues proba con `0`.

Cada vez que cambias algo, hay que **compilar** y **cargar** el programa en Pitri
(ver [Para grandes](docs/PARA_GRANDES.md#compilar-y-cargar)).

---

## 🚀 Ideas para el futuro

- [x] Que los botones hagan algo: ¡abren el menu de modos!
- [ ] Mas modos (¿cumpleaños? ¿espacio? ¿dinosaurios?)
- [ ] Que reaccione a la temperatura: que tiemble si hace frio o transpire si hace calor
- [ ] Que hable o haga sonidos (ya dejamos lugar en la placa para el audio)
- [x] Que se conecte al WiFi
- [x] Que sepa la hora (la saca de internet)
- [x] Que muestre la hora (desde el menu)
- [ ] Que se duerma de noche
- [ ] Que sepa el clima de afuera
