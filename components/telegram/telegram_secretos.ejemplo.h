// ─────────────────────────────────────────────────────────────────────────────
//  PLANTILLA de los datos del bot de Telegram
// ─────────────────────────────────────────────────────────────────────────────
//
//  1. Copia este archivo en la misma carpeta con el nombre:  telegram_secretos.h
//  2. En la copia, pone el token del bot y tu numero de usuario de Telegram.
//
//  telegram_secretos.h esta en el .gitignore: se queda en tu compu y NUNCA se
//  sube al repo. Este archivo de ejemplo si se sube, asi que aca no pongas nada real.
//
//  Como conseguir cada dato: ver docs/PARA_GRANDES.md (seccion Telegram).
//
#ifndef TELEGRAM_SECRETOS_H
#define TELEGRAM_SECRETOS_H

// El token que te dio @BotFather al crear el bot (es la "llave" del bot)
#define TELEGRAM_TOKEN  "123456789:ABCdefGhIJKlmNoPQRsTUVwxyZ"

// Los numeros de usuario de Telegram de las personas que le pueden escribir a
// Pitri. Los mensajes de cualquier otra persona se ignoran.
// Si todavia no sabes tu numero, deja { 0 }: escribile algo al bot y Pitri te
// lo va a mostrar en el monitor (idf.py monitor).
// Para agregar a alguien mas: { 111111111, 222222222 }
#define TELEGRAM_IDS_PERMITIDOS  { 0 }

#endif // TELEGRAM_SECRETOS_H
