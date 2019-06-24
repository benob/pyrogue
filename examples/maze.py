import rl

# this maze is not guaranteed to be fully connected
def update(event):
    rl.fill_rect(0, 0, 320, 240, rl.DARKGRAY)
    for i in range(320 // 4):
        for j in range(240 // 4):
            if rl.random_int(0, 1) == 1:
                rl.draw_line(i * 4, j * 4, (i + 1) * 4, (j + 1) * 4, rl.LIGHTGRAY)
            else:
                rl.draw_line((i + 1) * 4, j * 4, i * 4, (j + 1) * 4, rl.LIGHTGRAY)

rl.run(update, rl.ON_KEY)
