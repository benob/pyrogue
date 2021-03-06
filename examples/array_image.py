import math
import rl

WIDTH, HEIGHT = 80, 60

rl.init_display("Array image", WIDTH, HEIGHT)

pixels = rl.Array(WIDTH, HEIGHT)
angle = 0

def update(event):
    global angle

    pixels.random_2d(int(math.cos(angle) * 128), int(math.sin(angle) * 128))

    image = rl.array_to_image(pixels)
    rl.draw_image(image, 0, 0)

    angle += 0.01


rl.run(update, rl.ON_REDRAW)
