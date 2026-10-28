"""Render host runner_test PGM frames; captions approximate the OLED font."""
from pathlib import Path
import sys
from PIL import Image, ImageDraw, ImageFont

source = Path(sys.argv[1])
dest = Path(__file__).resolve().parents[1] / 'preview'
font = ImageFont.truetype('/usr/share/fonts/noto/NotoSans-Regular.ttf', 9)
small = ImageFont.truetype('/usr/share/fonts/noto/NotoSans-Regular.ttf', 7)
digits = ImageFont.truetype('/usr/share/fonts/noto/NotoSans-Regular.ttf', 24)
for name, prefix, count, seconds in [('countdown-seconds', 'frame', 60, 2163600), ('finish-seconds', 'finish', 134, 0)]:
    frames = []
    for i in range(count):
        im = Image.open(source / f'{prefix}_{i}.pgm').convert('RGB')
        draw = ImageDraw.Draw(im)
        for y, text, face in [(2, 'ДО ПОДАЧИ ILR', small), (14, str(max(0, seconds-i*90//1000)), digits),
                              (50, 'СЕКУНД', font), (61, '17.11.2026', small)]:
            draw.text((64, y), text, font=face, anchor='mt', fill='white')
        for mark in range(17):
            box = (5+mark*7, 125, 9+mark*7, 127)
            if seconds == 0:
                draw.rectangle(box, fill='white')
            else:
                draw.rectangle(box, outline='white')
        frames.append(im.resize((512, 512), Image.Resampling.NEAREST))
    dest.mkdir(exist_ok=True)
    frames[0].save(dest / f'{name}.png')
    if name == 'finish-seconds':
        frames[55].save(dest / 'fireworks.png')
    frames[0].save(dest / f'{name}.gif', save_all=True, append_images=frames[1:], duration=90, loop=0)
