import rl

rl.init_display('Keys', 640, 480)
font = rl.Font('data/font.ttf', 24)

keys = []

def update(event):
    if event > 0:
        keys.append(event)
    rl.clear()
    rl.draw_text(font, 0, 0, 'events> ' + str(keys))
    rl.draw_text(font, 0, 24, 'text> ' + ''.join([chr(x) if x >= 32 and x < 128 else '#' for x in keys]))

rl.run(update, rl.UPDATE_KEY)
