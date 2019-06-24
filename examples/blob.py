import rl
import math

WIDTH = 80
HEIGHT = 25
TILE_WIDTH = 9
TILE_HEIGHT = 16

rl.init_display('Blob', WIDTH * TILE_WIDTH, HEIGHT * TILE_HEIGHT)
tileset = rl.Image('data/cp437.png', TILE_WIDTH, TILE_HEIGHT)

screen = rl.Array(WIDTH, HEIGHT)

class Blob:
    def __init__(self):
        self.scale_x = rl.random() * 0.6
        self.scale_y = rl.random() * 0.6
        self.speed = rl.random() * math.pi * 8 - math.pi
        self.x = rl.random() * WIDTH
        self.y = rl.random() * HEIGHT

    def update(self, time, shift):
        self.x = math.sin((time + shift) * math.pi * self.speed) * WIDTH * self.scale_x + WIDTH / 2
        self.y = math.cos((time + shift) * math.pi * self.speed) * HEIGHT * self.scale_y + HEIGHT / 2

blobs = [Blob() for i in range(5)]
time = 0

def redraw(event):
    global time

    for i, blob in enumerate(blobs):
        blob.update(time, i / 2)

    for j in range(HEIGHT - 1):
        for i in range(1, WIDTH - 1):
            value = 1
            for blob in blobs:
                dx = (i - blob.x) ** 2
                dy = (j - blob.y) ** 2
                value *= math.sqrt(dx + dy)

            value = max(min(255 - (value // 4000), 255), 1)
            screen[i, j] = int(value)

    rl.clear()
    rl.draw_array(screen, tileset, 0, 0, mapping=[0, ord('.')] + list(range(2, 256)), fg=[rl.color(i, - i // 2, i) for i in range(256)])
    time += 0.005

rl.run(redraw)

