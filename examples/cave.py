import rl

level = rl.Array(320, 240)

rl.set_seed(0)

blocked = rl.RED

def init():
    global to_process
    to_process = set()
    level.fill(0)
    for i in range(30):
        rl.walk_line_start(rl.random_int(20, 299), rl.random_int(20, 219), rl.random_int(0, 299), rl.random_int(0, 219))
        while True:
            result = rl.walk_line_next()
            if result is None:
                break
            x, y = result
            level[x, y] = blocked
            to_process.add((x, y))

paused = False
def update(event):
    global paused, image
    if event == rl.SPACE:
        paused = not paused
    elif event == rl.RETURN:
        init()
    elif event == rl.ESCAPE:
        rl.quit()
    if not paused:
        for i in range(50):
            choice = rl.random_int(0, len(to_process) - 1)
            x, y = list(to_process)[choice]
            to_process.remove((x, y))
            
            if level[x, y - 1] == 0:
                level[x, y - 1] = blocked
                to_process.add((x, y - 1))
            if level[x, y + 1] == 0:
                level[x, y + 1] = blocked
                to_process.add((x, y + 1))
            if level[x - 1, y] == 0:
                level[x - 1, y] = blocked
                to_process.add((x - 1, y))
            if level[x + 1, y] == 0:
                level[x + 1, y] = blocked
                to_process.add((x + 1, y))
        rl.clear()
        image = rl.array_to_image(level)

    rl.draw_image(image, 0, 0)

init()
rl.init_display('Cave', 320, 240)
rl.run(update)
