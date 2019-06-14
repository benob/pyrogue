import rl

WIDTH = 80
HEIGHT = 25
TILE_WIDTH = 9
TILE_HEIGHT = 16

rl.init_display('fire', WIDTH * TILE_WIDTH, HEIGHT * TILE_HEIGHT)
tiles = rl.image('data/cp437.png', TILE_WIDTH, TILE_HEIGHT)

def gradient(start, stop, steps):
    r1 = rl.color_r(start)
    g1 = rl.color_g(start)
    b1 = rl.color_b(start)
    r2 = rl.color_r(stop)
    g2 = rl.color_g(stop)
    b2 = rl.color_b(stop)
    return [rl.color(r1 + (r2 - r1) * i // steps, g1 + (g2 - g1) * i // steps, b1 + (b2 - b1) * i // steps) for i in range(steps)]

palette = list(reversed(gradient(rl.WHITE, rl.YELLOW, 3) + gradient(rl.YELLOW, rl.ORANGE, 5) + gradient(rl.ORANGE, rl.RED, 5) + gradient(rl.RED, rl.color(0, 0, 128), 9) + gradient(rl.color(0, 0, 128), rl.BLACK, 5)))
chars = [ord(x) for x in reversed('ABCDEFGHIJKLMNOPQRSTUVWXYZ#')]

fire = rl.array(WIDTH, HEIGHT)

def redraw(event):
    for i in range(1, WIDTH - 1):
        fire[i, HEIGHT - 1] = len(palette) - 1 - rl.random_int(0, 3)
    for j in range(HEIGHT - 1):
        for i in range(1, WIDTH - 1):
            value = fire[i + rl.random_int(-1, 1), j + 1] - rl.random_int(0, 3)
            if value < 0:
                value = 0
            fire[i, j] = value

    rl.clear()
    rl.draw_array(fire, tiles, 0, 0, mapping=chars, fg=palette)

rl.run(redraw, rl.UPDATE_LOOP)


