# 🤖 Pitri

**Pitri es un compañero digital.** Tiene una carita en una pantalla, mira para todos lados,
juega, se pone contento cuando llega alguien, se duerme cuando está solo y sabe qué
temperatura hace.

Lo armamos entre Sofía y su papá, de a poquito, y cada vez le vamos enseñando cosas nuevas.

```
   ┌──────────────────────┐
   │                      │
   │    ████      ████    │
   │    ████      ████    │      ← así se ven los ojos de Pitri
   │    ████      ████    │
   │                      │
   └──────────────────────┘
```

---

## ✨ Qué sabe hacer Pitri

| Cuando… | Pitri… |
|---|---|
| Se prende | Dice **"¡Hola Sofi!"**, **"Soy Pitri"** y después abre los ojos |
| Está tranquilo | Juega solo: mira para los costados, parpadea, guiña un ojo, se marea, se enoja, llora… |
| Llega alguien | Se sorprende 😮 y se pone contento 😊 |
| Pasa un rato sin ver a nadie | Se duerme 😴 y le salen "Z" |
| Pasa un minuto | Muestra la temperatura y la humedad |

### Los juegos de Pitri

| Juego | Qué hace |
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
| 😢 llorar | Cara triste y le caen lágrimas |
| 😍 enamorado | Los ojos se vuelven corazones que laten y suben corazoncitos |
| 😎 verano | Se pone lentes de sol, baila canchero y se los saca guiñando un ojo |

¡Y cada emoción tiene **su propia boca**! Sonrisa cuando está contento, una "O" cuando se
sorprende, zigzag cuando se enoja, un "3" de beso cuando está enamorado, y muchas más.

---

## 🧩 De qué está hecho Pitri

| Parte | Para qué sirve |
|---|---|
| **ESP32** (la placa negra) | Es el **cerebro**: una computadora chiquita que ejecuta el programa |
| **Pantalla OLED** | Es la **cara**: ahí se dibujan los ojos |
| **Sensor PIR** | Son los **ojos de verdad**: detecta cuando alguien se mueve cerca |
| **Sensor DHT11** | Es la **piel**: siente la temperatura y la humedad del aire |
| **2 botones** | Todavía no los usamos… ¡ya les vamos a encontrar un trabajo! |

Cómo se conecta todo: 👉 [docs/CONEXIONES.md](docs/CONEXIONES.md)

---

## 📚 Para aprender más

| Documento | Qué tiene |
|---|---|
| [Cómo funciona Pitri](docs/COMO_FUNCIONA.md) | Cómo dibuja, cómo se mueve, cómo "ve" y cómo "siente" |
| [Inventá un juego nuevo](docs/NUEVO_JUEGO.md) | Paso a paso para enseñarle a Pitri algo nuevo |
| [Conexiones](docs/CONEXIONES.md) | El dibujo de todos los cables |
| [Diario de Pitri](docs/DIARIO.md) | Todo lo que fuimos haciendo y aprendiendo |
| [Para grandes](docs/PARA_GRANDES.md) | Cómo está organizado el código, cómo compilarlo y cómo agregarle cosas |
| [Datasheets](docs/README.md) | Las hojas técnicas de cada parte |

---

## 🎛️ Cambiá cómo se comporta Pitri

En el archivo [main/pitri_config.h](main/pitri_config.h) hay números que se pueden cambiar
sin saber programar. Algunos ejemplos:

```c
#define MENSAJE_ARRANQUE    "¡Hola\nSofi!"               // lo que dice al prenderse
#define MENSAJE_ARRANQUE_2  "Soy\nPitri"                 // y después
#define TIEMPO_SIN_MOVIMIENTO_PARA_DORMIR_MS  (60 * 1000) // cuándo se duerme
#define MOSTRAR_TEMPERATURA_CADA_MS  (60 * 1000)          // cada cuánto muestra la temperatura
```

Y en [components/eyes/eyes.c](components/eyes/eyes.c), arriba de todo, está la forma de
los ojos:

```c
#define OJO_ANCHO        30   // ¿ojos flaquitos o anchos?
#define OJO_ALTO         36   // ¿ojos bajitos o altos?
#define OJO_RADIO        9    // ¿cuadrados o redondos?
#define OJO_SEPARACION   20   // ¿juntitos o separados?
```

> 💡 **Probá:** poné `OJO_RADIO` en `15` y fijate qué pasa. Después probá con `0`.

Cada vez que cambiás algo, hay que **compilar** y **cargar** el programa en Pitri
(ver [Para grandes](docs/PARA_GRANDES.md#compilar-y-cargar)).

---

## 🚀 Ideas para el futuro

- [ ] Que los botones hagan algo (¿hacerle cosquillas? ¿darle de comer?)
- [ ] Que reaccione a la temperatura: que tiemble si hace frío o transpire si hace calor
- [ ] Que hable o haga sonidos (ya dejamos lugar en la placa para el audio)
- [ ] Que se conecte al WiFi
- [ ] Letras con tilde y ñ en la pantalla
