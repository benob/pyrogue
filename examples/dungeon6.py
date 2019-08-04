import math
import rl

def generate():
    level = rl.Array(320, 240)
    level.random_int(0, 5)
    for i in range(320):
        for j in range(240):
            level[i, j] &= int(math.sqrt((i - 160) * (i - 160) + (j - 120) * (j - 120)))
    level.cell_automaton('B234/S78')
    level.cell_automaton('B/S3456')
    level.cell_automaton('B/S13456')

    return level

def update(event):
    level = generate()
    rl.clear()
    rl.draw_image(rl.array_to_image(level, palette=[rl.BLACK, rl.WHITE]), 0, 0)

rl.run(update, rl.ON_KEY)
