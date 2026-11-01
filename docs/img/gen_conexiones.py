"""Genera docs/img/conexiones.svg

Uso:  python docs/img/gen_conexiones.py docs/img/conexiones.svg

Placa: ESP32 DevKit de 30 pines (estilo DOIT DevKit V1), vista desde arriba,
antena arriba y USB abajo. Si se cambia un pin, actualizar también
components/board_config/include/board_config.h
"""
import sys

OUT = sys.argv[1]

T = 70                      # margen superior (título)
def py(i): return T + 120 + 32 * i

W = 1040
BX0, BX1 = 360, 640         # bordes de la placa
DL, DR = 376, 624           # columnas de pines
board_top = py(0) - 70
board_bot = py(14) + 40
GND_X, V33_X = 975, 995     # rieles verticales de la derecha
BOT_GND, BOT_3V3 = py(14) + 42, py(14) + 62
PX0, PX1 = 790, 950         # periféricos de la derecha

C = dict(v33='#d62728', v5='#ff7f0e', gnd='#222222', sda='#1f77b4', scl='#17becf',
         pir='#b8860b', dht='#e377c2', btn='#8c564b', adc='#2ca02c', dac='#9467bd',
         spi='#4a6fa5', avoid='#9a9a9a', free='#ffffff')

# Lo que está impreso en la placa, de arriba (antena) hacia abajo (USB)
left = ['EN', 'VP·36', 'VN·39', 'D34', 'D35', 'D32', 'D33', 'D25', 'D26', 'D27', 'D14', 'D12',
        'D13', 'GND', 'VIN']
right = ['D23', 'D22', 'TX0', 'RX0', 'D21', 'D19', 'D18', 'D5', 'TX2·17', 'RX2·16', 'D4', 'D2',
         'D15', 'GND', '3V3']
lcat = ['avoid', 'adc', 'adc', 'adc', 'adc', 'adc', 'adc', 'dac', 'dac', 'pir', 'free', 'avoid',
        'free', 'gnd', 'v5']
rcat = ['spi', 'scl', 'avoid', 'avoid', 'sda', 'spi', 'spi', 'spi', 'btn', 'btn', 'dht', 'avoid',
        'avoid', 'gnd', 'v33']
ltag = {0: 'reset', 1: 'ADC1', 2: 'ADC1', 3: 'ADC1', 4: 'ADC1', 5: 'ADC1', 6: 'ADC1',
        7: 'DAC1', 8: 'DAC2', 10: 'libre', 11: 'arranque', 12: 'libre'}
rtag = {0: 'SPI', 2: 'USB', 3: 'USB', 5: 'SPI', 6: 'SPI', 7: 'SPI', 11: 'arranque',
        12: 'arranque'}
used = {('L', 9), ('L', 13), ('L', 14), ('R', 1), ('R', 4), ('R', 8), ('R', 9), ('R', 10),
        ('R', 13), ('R', 14)}

# Posiciones de los periféricos
disp_top = T + 60
SCL_Y, SDA_Y = disp_top + 50, disp_top + 100
B2_Y = py(8) - 76           # botón 2 -> TX2 (GPIO17)
B1_Y = py(9)                # botón 1 -> RX2 (GPIO16), línea recta
DATA_Y = py(10) + 75        # DHT22
pir_top, pir_bot = py(9) - 40, py(14) + 30
H = BOT_3V3 + 170

o = []
a = o.append
a(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}" '
  f'font-family="Helvetica, Arial, sans-serif">')
a(f'<rect width="{W}" height="{H}" fill="#ffffff"/>')
a('<text x="20" y="34" font-size="22" font-weight="bold" fill="#111">Pitri — conexiones</text>')
a('<text x="20" y="56" font-size="13" fill="#555">ESP32 DevKit 30 pines (eMakers, CH9102X) · '
  'vista desde arriba, antena arriba, USB abajo</text>')

def wire(pts, color, w=3):
    d = ' '.join(f'{x},{y}' for x, y in pts)
    a(f'<polyline points="{d}" fill="none" stroke="{color}" stroke-width="{w}" '
      f'stroke-linejoin="round" stroke-linecap="round"/>')

def wire_hop(x1, x2, y, hop_x, color):
    a(f'<path d="M{x1},{y} H{hop_x - 6} a6,6 0 0 1 12,0 H{x2}" fill="none" stroke="{color}" '
      f'stroke-width="3" stroke-linecap="round"/>')

def dot(x, y, color):
    a(f'<circle cx="{x}" cy="{y}" r="4.5" fill="{color}"/>')

def text(x, y, s, size=13, color='#111', anchor='start', weight='normal'):
    a(f'<text x="{x}" y="{y}" font-size="{size}" fill="{color}" text-anchor="{anchor}" '
      f'font-weight="{weight}" dominant-baseline="middle">{s}</text>')

def box(x0, y0, x1, y1, title, sub=None):
    a(f'<rect x="{x0}" y="{y0}" width="{x1 - x0}" height="{y1 - y0}" rx="8" fill="#f4f6f8" '
      f'stroke="#555" stroke-width="1.5"/>')
    text((x0 + x1) / 2, y0 + 18, title, 13, '#111', 'middle', 'bold')
    if sub:
        text((x0 + x1) / 2, y1 - 14, sub, 11, '#555', 'middle')

def pad(x, y):
    a(f'<rect x="{x - 4}" y="{y - 4}" width="8" height="8" fill="#c9a227" stroke="#7a6212"/>')

# ─── Cables ───────────────────────────────────────────────────────────────────
# Rieles: salen de 3V3 y GND (abajo a la derecha) y suben por la derecha
wire([(BX1, py(14)), (650, py(14)), (650, BOT_3V3), (V33_X, BOT_3V3), (V33_X, SCL_Y)], C['v33'])
wire([(BX1, py(13)), (665, py(13)), (665, BOT_GND), (GND_X, BOT_GND), (GND_X, SDA_Y)], C['gnd'])

# Display
wire([(BX1, py(1)), (670, py(1)), (670, SCL_Y), (PX0, SCL_Y)], C['scl'])
wire([(BX1, py(4)), (690, py(4)), (690, SDA_Y), (PX0, SDA_Y)], C['sda'])
wire_hop(PX1, V33_X, SCL_Y, GND_X, C['v33'])
wire([(PX1, SDA_Y), (GND_X, SDA_Y)], C['gnd'])
dot(V33_X, SCL_Y, C['v33']); dot(GND_X, SDA_Y, C['gnd'])

# Botones
wire([(BX1, py(8)), (700, py(8)), (700, B2_Y), (PX0, B2_Y)], C['btn'])
wire([(BX1, B1_Y), (PX0, B1_Y)], C['btn'])
for y in (B2_Y, B1_Y):
    wire([(PX1, y), (GND_X, y)], C['gnd'])
    dot(GND_X, y, C['gnd'])

# DHT22
wire([(BX1, py(10)), (715, py(10)), (715, DATA_Y), (PX0, DATA_Y)], C['dht'])
wire_hop(PX1, V33_X, DATA_Y - 20, GND_X, C['v33'])
wire([(PX1, DATA_Y + 20), (GND_X, DATA_Y + 20)], C['gnd'])
dot(V33_X, DATA_Y - 20, C['v33']); dot(GND_X, DATA_Y + 20, C['gnd'])

# PIR (izquierda, líneas rectas)
wire([(BX0, py(9)), (250, py(9))], C['pir'])
wire([(BX0, py(13)), (250, py(13))], C['gnd'])
wire([(BX0, py(14)), (250, py(14))], C['v5'])

# ─── Placa ESP32 ──────────────────────────────────────────────────────────────
a(f'<rect x="{BX0}" y="{board_top}" width="{BX1 - BX0}" height="{board_bot - board_top}" rx="10" '
  f'fill="#15171c" stroke="#000" stroke-width="2"/>')
a(f'<rect x="455" y="{board_top + 8}" width="90" height="42" rx="3" fill="#22252c" stroke="#8892a8"/>')
zz = ' '.join(f'{462 + i * 8},{board_top + (16 if i % 2 == 0 else 42)}' for i in range(10))
a(f'<polyline points="{zz}" fill="none" stroke="#c9a227" stroke-width="2"/>')
text(500, board_top + 62, 'antena WiFi', 10, '#c8d0e0', 'middle')
a(f'<rect x="458" y="{py(1) - 10}" width="84" height="{py(7) - py(1)}" rx="3" fill="#b9bec7" stroke="#80858f"/>')
text(500, py(3), 'ESP32', 14, '#222', 'middle', 'bold')
text(500, py(4) - 4, 'WROOM', 12, '#222', 'middle')
text(500, py(5) - 12, '-32', 12, '#222', 'middle')
text(500, py(9), 'ESP32 DevKit', 12, '#e0e6f0', 'middle', 'bold')
text(500, py(10) - 6, '30 pines', 11, '#aab4c8', 'middle')
a(f'<rect x="482" y="{py(11) - 2}" width="36" height="36" fill="#2a2d33" stroke="#555"/>')
text(500, py(12) + 18, 'CH9102X', 9, '#aab4c8', 'middle')
for bx, lbl in ((450, 'EN'), (550, 'BOOT')):
    a(f'<rect x="{bx - 10}" y="{board_bot - 30}" width="20" height="20" rx="3" fill="#ccc" stroke="#777"/>')
    text(bx, board_bot - 40, lbl, 9, '#c8d0e0', 'middle')
a(f'<rect x="475" y="{board_bot - 8}" width="50" height="26" rx="3" fill="#a0a4ab" stroke="#666"/>')
text(500, board_bot + 5, 'USB', 10, '#222', 'middle', 'bold')

def pins(labels, cats, tags, side):
    x = DL if side == 'L' else DR
    for i, lbl in enumerate(labels):
        y = py(i)
        is_used = (side, i) in used
        a(f'<circle cx="{x}" cy="{y}" r="7" fill="{C[cats[i]]}" stroke="#ffffff" stroke-width="2"/>')
        col = '#ffffff' if is_used else '#aab4c8'
        wt = 'bold' if is_used else 'normal'
        if side == 'L':
            text(x + 14, y, lbl, 13, col, 'start', wt)
        else:
            text(x - 14, y, lbl, 13, col, 'end', wt)
        if i in tags:
            tcol = C.get({'ADC1': 'adc', 'DAC1': 'dac', 'DAC2': 'dac', 'SPI': 'spi'}.get(tags[i], ''), '#777')
            if side == 'L':
                text(BX0 - 8, y, tags[i], 11, tcol, 'end')
            else:
                text(BX1 + 8, y, tags[i], 11, tcol, 'start')

pins(left, lcat, ltag, 'L')
pins(right, rcat, rtag, 'R')

# ─── Periféricos ──────────────────────────────────────────────────────────────
box(PX0, disp_top, PX1, disp_top + 150, 'Display OLED 1.3"', 'SH1106 · I2C 0x3C')
a(f'<rect x="832" y="{disp_top + 34}" width="76" height="84" rx="4" fill="#000"/>')
a(f'<rect x="843" y="{disp_top + 58}" width="22" height="30" rx="7" fill="#fff"/>')
a(f'<rect x="875" y="{disp_top + 58}" width="22" height="30" rx="7" fill="#fff"/>')
for y, l in ((SCL_Y, 'SCL'), (SDA_Y, 'SDA')):
    pad(PX0, y); text(PX0 + 8, y, l, 11, '#111', 'start', 'bold')
for y, l in ((SCL_Y, 'VCC'), (SDA_Y, 'GND')):
    pad(PX1, y); text(PX1 - 8, y, l, 11, '#111', 'end', 'bold')

for y, name in ((B2_Y, 'Botón 2'), (B1_Y, 'Botón 1')):
    a(f'<rect x="{PX0}" y="{y - 22}" width="{PX1 - PX0}" height="44" rx="8" fill="#f4f6f8" stroke="#555" stroke-width="1.5"/>')
    a(f'<circle cx="{(PX0 + PX1) / 2 + 10}" cy="{y}" r="11" fill="#d33" stroke="#822" stroke-width="2"/>')
    text(PX0 + 12, y, name, 12, '#111', 'start', 'bold')
    pad(PX0, y); pad(PX1, y)

box(PX0, DATA_Y - 70, PX1, DATA_Y + 50, 'DHT22 / AM2302', 'temperatura y humedad')
for k in range(4):
    a(f'<rect x="828" y="{DATA_Y - 44 + k * 14}" width="40" height="8" rx="2" fill="#dfe5ea" stroke="#9aa"/>')
pad(PX0, DATA_Y); text(PX0 + 8, DATA_Y, 'DATA', 11, '#111', 'start', 'bold')
pad(PX1, DATA_Y - 20); text(PX1 - 8, DATA_Y - 20, 'VCC (+)', 11, '#111', 'end', 'bold')
pad(PX1, DATA_Y + 20); text(PX1 - 8, DATA_Y + 20, 'GND (−)', 11, '#111', 'end', 'bold')

box(70, pir_top, 250, pir_bot, 'Sensor PIR', 'HC-SR501')
cy = (py(10) + py(12)) // 2 + 6
a(f'<circle cx="150" cy="{cy}" r="38" fill="#fafafa" stroke="#aaa" stroke-width="2"/>')
for r in (27, 16, 5):
    a(f'<circle cx="150" cy="{cy}" r="{r}" fill="none" stroke="#ccc" stroke-width="1.5"/>')
for y, l in ((py(9), 'OUT'), (py(13), 'GND'), (py(14), 'VCC (5V)')):
    pad(250, y); text(242, y, l, 11, '#111', 'end', 'bold')

text(V33_X + 2, BOT_3V3 + 16, '3V3', 12, C['v33'], 'end', 'bold')
text(GND_X - 6, BOT_GND - 10, 'GND', 12, C['gnd'], 'end', 'bold')
text(305, py(14) - 12, 'VIN = 5V del USB', 10, C['v5'], 'middle')

# ─── Leyenda ──────────────────────────────────────────────────────────────────
ly = BOT_3V3 + 45
a(f'<line x1="20" y1="{ly - 20}" x2="{W - 20}" y2="{ly - 20}" stroke="#ddd"/>')
rows = [
    [('v33', '3V3'), ('v5', '5V (VIN)'), ('gnd', 'GND'), ('sda', 'SDA display'),
     ('scl', 'SCL display'), ('pir', 'PIR'), ('dht', 'DHT22'), ('btn', 'Botones')],
    [('adc', 'ADC1: reservado entrada de audio (funciona con WiFi)'),
     ('dac', 'DAC: reservado salida de audio')],
    [('spi', 'SPI: reservado (tarjeta SD / I2S)'), ('free', 'libre'),
     ('avoid', 'no usar: USB, reset o arranque')],
]
for r, items in enumerate(rows):
    xx, yy = 30, ly + 30 * r
    for k, lbl in items:
        a(f'<circle cx="{xx}" cy="{yy}" r="7" fill="{C[k]}" stroke="#666" stroke-width="1"/>')
        text(xx + 14, yy, lbl, 12, '#222')
        xx += 125 if r == 0 else 30 + len(lbl) * 7
text(30, ly + 95, 'Las líneas que se cruzan con un "saltito" NO están conectadas. Los puntos indican unión.', 12, '#555')

a('</svg>')
open(OUT, 'w', encoding='utf-8').write('\n'.join(o))
print('ok', W, H)
