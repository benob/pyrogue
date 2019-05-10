import rl
import math

WIDTH = 80
HEIGHT = 24
TILE_WIDTH = 9
TILE_HEIGHT = 16

rl.init("plasma", WIDTH * TILE_WIDTH, HEIGHT * TILE_HEIGHT)
rl.load_image(0, "data/cp437.png", TILE_WIDTH, TILE_HEIGHT)

shift = 0.0
colors = [rl.hsv_color(i, 255, 255, 255) for i in range(256)]

array = rl.array(WIDTH, HEIGHT)

def redraw(event):
    global shift
    for y in range(HEIGHT):
        for x in range(WIDTH):
            value = math.sin(shift / 100 + x / 4.0)
            value += math.sin((shift / 100 + y / 4.0))
            value += math.sin(((shift / 100 + x + y) / 8.0))
            value += math.sin(math.sqrt(shift / 100 + x * x + y * y) / 16.0)
            array[x, y] = int(128 + 128 * value / 4)

    rl.clear()
    rl.draw_array(array, 0, 0, fg=colors)
    shift += 10

rl.run(redraw, rl.UPDATE_LOOP)
