import rl

level = rl.Array(320, 240)
kernel = rl.Array(3, 3)
kernel.fill(1)

level.random_int(0, 1)
level *= 255

palette = [rl.color(i, i, i) for i in range(256)]

def update(event):
    global level
    level = level.apply_kernel(kernel)
    rl.draw_image(rl.array_to_image(level, palette=palette), 0, 0)

rl.run(update)
