# 🎨 Inventa un juego nuevo para Pitri

En esta guia le vamos a enseñar a Pitri a **asustarse**: va a abrir los ojos re grandes,
temblar y despues calmarse. Cuando lo termines, vas a poder inventar los tuyos.

Todos los juegos estan en el archivo [components/eyes/eyes.c](../components/eyes/eyes.c).

---

## 1️⃣ Las piezas para armar juegos

Un juego es una lista de pasos. Estos son los "bloques" que podes usar:

| Bloque | Que hace | Ejemplo |
|---|---|---|
| `mover_a(x, y, ms)` | Mueve los ojos a un lugar | `mover_a(-20, 0, 300);` mira a la izquierda |
| `esperar(ms)` | Se queda quieto un rato | `esperar(1000);` espera 1 segundo |
| `eyes_parpadear()` | Parpadea | |
| `animar(cara, ms)` | Cambia la cara entera (ver abajo) | |
| `azar(min, max)` | Un numero al azar | `azar(1, 6)` es como tirar un dado |

**ms** quiere decir **milisegundos**: 1000 ms = 1 segundo.

### ¿Donde esta cada lugar?

`x` e `y` dicen cuanto se corren los ojos **desde el centro**:

```
              y = -14  (arriba)
                  ↑
  x = -24  ←    (0, 0)    →  x = 24
 (izquierda)    centro      (derecha)
                  ↓
              y = 14   (abajo)
```

Si te pasas de esos numeros, los ojos se salen de la pantalla.

### Cambiar la cara

Para cambiar algo mas que la posicion, se copia la cara normal y se le cambia lo que uno
quiere:

```c
cara_t d = CARA_NORMAL;   // empiezo con la cara normal
d.feliz = 1;              // le pongo ojitos felices
animar(d, 300);           // y cambio a esa cara en 300 ms
```

Todo lo que se puede cambiar de la cara:

| Que | Valores | Para que |
|---|---|---|
| `d.x`, `d.y` | -24 a 24, -14 a 14 | Adonde mira |
| `d.ancho`, `d.alto` | normal: 30 y 36 | Tamaño de los ojos |
| `d.abre_izq`, `d.abre_der` | 0 = cerrado, 1 = abierto | Cuanto abre cada ojo |
| `d.feliz` | 0 a 1 | Ojitos felices ^ ^ y sonrisa (desde 0.3) |
| `d.enojo` | 0 a 1 | Cejas de enojado |
| `d.triste` | 0 a 1 | Cejas de triste |
| `d.espiral` | 0 o 1 | Ojos de mareado |
| `d.corazon` | 0 o 1 | Ojos de corazon (late si cambias `d.ancho`) |
| `d.corazoncitos` | 0 a 1 | Corazoncitos que suben (0 = abajo, 1 = arriba) |
| `d.lentes` | 0 a 1 | Lentes de sol (0 = arriba afuera, 1 = puestos) |
| `d.asombro` | 0 a 1 | Boca en forma de "O" |
| `d.sospecha` | 0 a 1 | Boca torcida |
| `d.sueno` | 0 a 1 | Boquita de dormido |
| `d.mira` | -2 a 2 | Hacia donde mira, aparte de donde estan los ojos (negativo = izquierda) |

La **boca** aparece sola segun la emocion (si no hay ninguna, una sonrisita tranquila): `feliz` pone sonrisa, `triste` la sonrisa al
reves, `enojo` zigzag, `espiral` una ola, `corazon` un beso y `lentes` una sonrisa de
costado. Si queres solo la boca, sin cambiar los ojos, usa `asombro`, `sospecha` o `sueno`.

---

## 2️⃣ Escribi el juego

Busca en `eyes.c` el juego `juego_llorar` y, **abajo** de donde termina (la ultima `}`),
escribi esto:

```c
// Se asusta: ojos re grandes, tiembla y se calma
static void juego_susto(void)
{
    // 1. ¡Ojos re grandes, rapidisimo!
    cara_t d = CARA_NORMAL;
    d.ancho = 36;
    d.alto  = 46;
    animar(d, 90);

    // 2. Tiembla 8 veces
    for (int i = 0; i < 8; i++) {
        mover_a(-2, 0, 40);
        mover_a(2, 0, 40);
    }

    // 3. Se queda mirando asustado
    esperar(800);

    // 4. Suspira y vuelve a la normalidad
    animar(CARA_NORMAL, 600);
    eyes_parpadear();
}
```

> 💡 `for (int i = 0; i < 8; i++)` quiere decir **"repeti esto 8 veces"**.

---

## 3️⃣ Agregalo a la bolsa de juegos

Un poco mas abajo hay una tabla que se llama `juegos[]`. Agrega tu juego al final, antes
de la `};`:

```c
    { "llorar",         juego_llorar,          1 },
    { "susto",          juego_susto,           5 },   // ← ¡el nuevo!
};
```

El ultimo numero es el **peso**: cuantos papelitos tiene en la bolsa. Le pusimos 5 para
que salga seguido y lo puedas ver enseguida. Cuando ya lo viste, bajalo a 1.

---

## 4️⃣ Probalo

1. Compila y carga el programa en Pitri (ver
   [Para grandes](PARA_GRANDES.md#compilar-y-cargar)).
2. Mira a Pitri… ¡y espera a que se asuste!
3. En la consola de la compu, cada vez que hace un juego aparece su nombre:
   ```
   I (12345) OJOS: Juego: susto
   ```

---

## 💡 Ideas para inventar

- 😎💕 **Canchero enamorado:** ¡lentes de sol y corazoncitos al mismo tiempo!
- 🥱 **Bostezo:** ojos que se cierran despacito hasta la mitad y se abren de golpe
- 🤪 **Loco:** un ojo cerrado y el otro mirando para cualquier lado (`d.abre_izq = 0`)
- 🙄 **Revolear los ojos:** mirar arriba a la derecha, arriba a la izquierda y volver
- 👻 **Escondida:** los ojos se van fuera de la pantalla (`mover_a(60, 0, 500)`) y vuelven

---

## 🎭 Inventa un personaje para un modo

Los personajes estan en [components/eyes/disfraces.c](../components/eyes/disfraces.c).
Cada uno tiene dos funciones: `dibujar_xxx()` (como se ve) y `actuar_xxx()` (que hace).
Para agregar uno nuevo, por ejemplo un **gato** 🐱 para el modo Animales:

1. En [eyes.h](../components/eyes/include/eyes.h), agrega `DISFRAZ_GATO` a la lista de
   disfraces.
2. En `disfraces.c`, copia un personaje parecido (el cerdo, que usa los ojos de Pitri) y
   cambialo: orejas puntiagudas (triangulos), nariz (un triangulito) y bigotes (lineas).
   Que diga `decir("¡Miau!", ...)`.
3. Agregalo a la lista `personajes[]` al final de `disfraces.c`.
4. En [main.c](../main/main.c), agregalo a la tabla `personajes` del modo que quieras
   (cambiando el 3 por un 4).

Para el dibujo, `mx` es el centro de la cara (se mueve con `cara.x`) y `cy` la altura de
los ojos. `cara.fase` es un "reloj" para cosas que se mueven solas (como las orejas del
cerdo): si en la actuacion le sumas 0.25 en cada paso, da una vuelta cada 4 pasos.

---

## 🍕 Inventa una comida

Las comidas estan en [components/eyes/comida.c](../components/eyes/comida.c). Para
agregar una, por ejemplo una **pizza**:

1. En [eyes.h](../components/eyes/include/eyes.h), agrega `COMIDA_PIZZA` a la lista de comidas.
2. En `comida.c`, escribi `dibujar_pizza()` copiando otra comida: un triangulo (la
   porcion), una linea gruesa arriba (el borde) y circulitos negros (el salame). Acordate
   de multiplicar todas las medidas por `s`.
3. Agregala en `eyes_dibujar_comida()` y en `medio_ancho()`.
4. En [main.c](../main/main.c), agregala a `comidas[]` y `nombres_comidas[]`, y cambia
   `CANT_COMIDAS` de 3 a 4.

Para una **bebida** (por ejemplo un vaso de jugo) es parecido: copia `dibujar_taza()`, usa
el numero de sorbos para la altura del liquido, agregala a `eyes_dibujar_comida()` y a
`es_bebida()`, y en `main.c` a `bebidas[]`, `nombres_bebidas[]` y `CANT_BEBIDAS`. Si el
liquido no se ve (como en el mate, `dibujar_mate()`), los sorbos no cambian el dibujo.

### ¿No anda?

- **Si al compilar sale un error:** fijate que cada linea termine con `;` y que cada `{`
  tenga su `}`.
- **Si el juego nunca sale:** revisa que lo hayas agregado a la tabla `juegos[]`.
- **Si los ojos desaparecen:** te pasaste de los numeros de `x` o `y`.
