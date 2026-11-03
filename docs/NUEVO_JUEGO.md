# 🎨 Inventá un juego nuevo para Pitri

En esta guía le vamos a enseñar a Pitri a **asustarse**: va a abrir los ojos re grandes,
temblar y después calmarse. Cuando lo termines, vas a poder inventar los tuyos.

Todos los juegos están en el archivo [components/eyes/eyes.c](../components/eyes/eyes.c).

---

## 1️⃣ Las piezas para armar juegos

Un juego es una lista de pasos. Estos son los "bloques" que podés usar:

| Bloque | Qué hace | Ejemplo |
|---|---|---|
| `mover_a(x, y, ms)` | Mueve los ojos a un lugar | `mover_a(-20, 0, 300);` mira a la izquierda |
| `esperar(ms)` | Se queda quieto un rato | `esperar(1000);` espera 1 segundo |
| `eyes_parpadear()` | Parpadea | |
| `animar(cara, ms)` | Cambia la cara entera (ver abajo) | |
| `azar(min, max)` | Un número al azar | `azar(1, 6)` es como tirar un dado |

**ms** quiere decir **milisegundos**: 1000 ms = 1 segundo.

### ¿Dónde está cada lugar?

`x` e `y` dicen cuánto se corren los ojos **desde el centro**:

```
              y = -14  (arriba)
                  ↑
  x = -24  ←    (0, 0)    →  x = 24
 (izquierda)    centro      (derecha)
                  ↓
              y = 14   (abajo)
```

Si te pasás de esos números, los ojos se salen de la pantalla.

### Cambiar la cara

Para cambiar algo más que la posición, se copia la cara normal y se le cambia lo que uno
quiere:

```c
cara_t d = CARA_NORMAL;   // empiezo con la cara normal
d.feliz = 1;              // le pongo ojitos felices
animar(d, 300);           // y cambio a esa cara en 300 ms
```

Todo lo que se puede cambiar de la cara:

| Qué | Valores | Para qué |
|---|---|---|
| `d.x`, `d.y` | -24 a 24, -14 a 14 | Adónde mira |
| `d.ancho`, `d.alto` | normal: 30 y 36 | Tamaño de los ojos |
| `d.abre_izq`, `d.abre_der` | 0 = cerrado, 1 = abierto | Cuánto abre cada ojo |
| `d.feliz` | 0 a 1 | Ojitos felices ^ ^ y sonrisa (desde 0.3) |
| `d.enojo` | 0 a 1 | Cejas de enojado |
| `d.triste` | 0 a 1 | Cejas de triste |
| `d.espiral` | 0 o 1 | Ojos de mareado |
| `d.corazon` | 0 o 1 | Ojos de corazón (late si cambiás `d.ancho`) |
| `d.corazoncitos` | 0 a 1 | Corazoncitos que suben (0 = abajo, 1 = arriba) |
| `d.lentes` | 0 a 1 | Lentes de sol (0 = arriba afuera, 1 = puestos) |
| `d.asombro` | 0 a 1 | Boca en forma de "O" |
| `d.sospecha` | 0 a 1 | Boca torcida |
| `d.sueno` | 0 a 1 | Boquita de dormido |

La **boca** aparece sola según la emoción: `feliz` pone sonrisa, `triste` la sonrisa al
revés, `enojo` zigzag, `espiral` una ola, `corazon` un beso y `lentes` una sonrisa de
costado. Si querés solo la boca, sin cambiar los ojos, usá `asombro`, `sospecha` o `sueno`.

---

## 2️⃣ Escribí el juego

Buscá en `eyes.c` el juego `juego_llorar` y, **abajo** de donde termina (la última `}`),
escribí esto:

```c
// Se asusta: ojos re grandes, tiembla y se calma
static void juego_susto(void)
{
    // 1. ¡Ojos re grandes, rapidísimo!
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

> 💡 `for (int i = 0; i < 8; i++)` quiere decir **"repetí esto 8 veces"**.

---

## 3️⃣ Agregalo a la bolsa de juegos

Un poco más abajo hay una tabla que se llama `juegos[]`. Agregá tu juego al final, antes
de la `};`:

```c
    { "llorar",         juego_llorar,          1 },
    { "susto",          juego_susto,           5 },   // ← ¡el nuevo!
};
```

El último número es el **peso**: cuántos papelitos tiene en la bolsa. Le pusimos 5 para
que salga seguido y lo puedas ver enseguida. Cuando ya lo viste, bajalo a 1.

---

## 4️⃣ Probalo

1. Compilá y cargá el programa en Pitri (ver
   [Para grandes](PARA_GRANDES.md#compilar-y-cargar)).
2. Mirá a Pitri… ¡y esperá a que se asuste!
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

### ¿No anda?

- **Si al compilar sale un error:** fijate que cada línea termine con `;` y que cada `{`
  tenga su `}`.
- **Si el juego nunca sale:** revisá que lo hayas agregado a la tabla `juegos[]`.
- **Si los ojos desaparecen:** te pasaste de los números de `x` o `y`.
