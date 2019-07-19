import rl

rl.set_seed()
level = rl.Array(320, 240)
queue = [(160, 120)]
edges = [(-2, 0), (2, 0), (0, -2), (0, 2)]

# maze generation based on prim's algorithm
def prim():
    if len(queue) > 0:
        index = rl.random_int(0, len(queue) - 1)
        x, y = queue.pop(index)
        level[x, y] = 1
        rl.shuffle(edges)
        for dx, dy in edges:
            x2, y2 = x + dx, y + dy
            if x2 >= 0 and x2 < level.width() and y2 >=0 and y2 < level.height():
                if level[x2, y2] == 0:
                    level.draw_line(x, y, x2, y2)
                    queue.append((x2, y2))

def update(event):
    global queue
    if event == rl.KEY:
        level.fill(0)
        queue = [(rl.random_int(0, 319), rl.random_int(0, 239))]
    for i in range(64):
        prim()
    rl.draw_image(rl.array_to_image(level, palette=[rl.LIGHTGRAY, rl.DARKGRAY]), 0, 0)

rl.run(update, rl.ON_REDRAW|rl.ON_KEY)
