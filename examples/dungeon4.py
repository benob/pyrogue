import rl

def generate():
    level = rl.Array(320, 240)
    level.random_int(0, 1)
    level *= 256
    kernel = rl.Array(3, 3)
    kernel.fill(1)
    for i in range(2):
        level = level.apply_kernel(kernel)
    level = level < 128
    #level.cell_automaton('B/S678')

    # search for large connected component
    best = 0
    for i in range(10):
        x, y = level.find_random(0)
        while x == -1:
            x, y = level.find_random(0)
        num = level.flood_fill(x, y, i + 2)
        if num > best:
            best = num
            selected = i + 2

    level = level.not_equals(selected)
    return level

def update(event):
    level = generate()
    rl.draw_image(rl.array_to_image(level, palette=[rl.BLACK, rl.WHITE, rl.BLUE]), 0, 0)

rl.run(update, rl.ON_KEY)
