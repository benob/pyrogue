import rl

level = rl.Array(320, 240)

def bsp(x, y, w, h):
    if w <= rl.random_int(5, 10) or h <= rl.random_int(5, 10):
        return
    if w > h:
        split = rl.random_int(5, w - 5)
        for i in range(1, h):
            level[x + split, y + i] = 1
        bsp(x        , y, split    , h)
        bsp(x + split, y, w - split, h)
        while True:
            l = y + rl.random_int(2, h - 2)
            if level[x + split - 1, l] == 0 and level[x + split + 1, l] == 0:
                break
        level[x + split, l] = 0
    else:
        split = rl.random_int(5, h - 5)
        for i in range(1, w):
            level[x + i, y + split] = 1
        bsp(x, y        , w,     split)
        bsp(x, y + split, w, h - split)
        while True:
            l = x + rl.random_int(2, w - 2)
            if level[l, y + split - 1] == 0 and level[l, y + split + 1] == 0:
                break
        level[l, y + split] = 0

bsp(0, 0, 320, 240)

def update(event):
    rl.draw_image(rl.array_to_image(level, palette=[rl.BLACK, rl.WHITE]), 0, 0)

rl.run(update)
