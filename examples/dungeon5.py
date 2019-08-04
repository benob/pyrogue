import rl

def generate():
    level = rl.Array(320, 240)
    level.random_int(0, 256)
    kernel = rl.Array(5, 5)
    kernel[0, 0] = 1
    kernel[-1, -1] = 1
    level = level.apply_kernel(kernel)
    level = level < 72
    level.cell_automaton('B12/S')
    level.cell_automaton('B8/S2345678')

    level = level.equals(0)

    #smooth for a more glacier style cave
    #level.cell_automaton('B678/S3456789')

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
    rl.clear()
    #rl.draw_image(rl.array_to_image(level, palette=[rl.color(255 - i, i, i) for i in range(256)]), 0, 0)
    rl.draw_image(rl.array_to_image(level, palette=[rl.BLACK, rl.WHITE]), 0, 0)

rl.run(update, rl.ON_KEY)
