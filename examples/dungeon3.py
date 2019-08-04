import rl

rl.set_seed(0)

def bsp(level, x, y, w, h):
    if w <= rl.random_int(5, 10) or h <= rl.random_int(5, 10):
        return
    if w > h:
        split = rl.random_int(5, w - 5)
        for i in range(1, h):
            level[x + split, y + i] = 1
        bsp(level, x        , y, split    , h)
        bsp(level, x + split, y, w - split, h)
        for i in range(10):
            l = y + rl.random_int(2, h - 2)
            if level[x + split - 1, l] == 0 and level[x + split + 1, l] == 0:
                break
        level[x + split, l] = 0
    else:
        split = rl.random_int(5, h - 5)
        for i in range(1, w):
            level[x + i, y + split] = 1
        bsp(level, x, y        , w,     split)
        bsp(level, x, y + split, w, h - split)
        for i in range(10):
            l = x + rl.random_int(2, w - 2)
            if level[l, y + split - 1] == 0 and level[l, y + split + 1] == 0:
                break
        level[l, y + split] = 0

def generate():
    level = rl.Array(320, 240)
    level.draw_rect(0, 0, level.width(), level.height())
    bsp(level, 0, 0, level.width() - 1, level.height() - 1)
    return level

def update(event):
    level = generate()
    rl.draw_image(rl.array_to_image(level, palette=[rl.BLACK, rl.WHITE, rl.RED]), 0, 0)

rl.run(update, rl.ON_KEY)
