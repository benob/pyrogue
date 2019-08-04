import rl

level = rl.Array(320, 240)

def h_corridor(x1, x2, y):
    level.draw_line(x1, y, x2, y)
    i = x1
    while i < x2:
        width = rl.random_int(3, min([x2 - i, 10])) 
        height = rl.random_int(5, width * 2)
        room = level.view(i, y + 2, width, height)
        if not room:
            room.fill(1)
            level[i + width // 2, y + 1] = 1
        i += width + 1
    i = x1
    while i < x2:
        width = rl.random_int(3, min([x2 - i - 1, 10])) 
        height = rl.random_int(5, width * 2)
        room = level.view(i, y - 1 - height, width, height)
        if not room:
            room.fill(1)
            level[i + width // 2, y - 1] = 1
        i += width + 1

def v_corridor(x, y1, y2):
    level.draw_line(x, y1, x, y2)
    i = y1
    while i < y2:
        height = rl.random_int(3, min([y2 - i, 10])) 
        width = rl.random_int(5, height * 2)
        room = level.view(x + 2, i, width, height)
        if not room:
            room.fill(1)
            level[x + 1, i + height // 2] = 1
        i += height + 1
    i = y1
    while i < y2:
        height = rl.random_int(3, min([y2 - i - 1, 10])) 
        width = rl.random_int(5, height * 2)
        room = level.view(x - 1 - width, i, width, height)
        if not room:
            room.fill(1)
            level[x - 1, i + height // 2] = 1
        i += height + 1

def city():
    level.fill(0)
    for i in range(1):
        x1, x2, y = rl.random_int(0, 319), rl.random_int(0, 319), rl.random_int(0, 199)
        level.draw_line(x1, y, x2, y)
    for i in range(1):
        x, y1, y2 = rl.random_int(0, 319), rl.random_int(0, 199), rl.random_int(0, 199)
        level.draw_line(x, y1, x, y2)
    for i in range(200):
        x, y = level.find_random(1)
        if x != -1:
            width, height = rl.random_int(5, 15), rl.random_int(5, 15)
            coords = [(x - width // 2, y + 1), (x - width // 2, y - height), (x + 1, y - height // 2), (x - width, y - height // 2)]
            selected = rl.random_choice(coords)
            view = level.view(selected[0], selected[1], width, height)
            if not view:
                view.view(1, 1, width - 2, height - 2).fill(1)
                #view[width // 2, 0] = 1

def generate():
    level.fill(0)
    for i in range(4):
        x = rl.random_int(20, 320 - 20)
        y = rl.random_int(20, 240 - 20) 
        h_corridor(0, 319, y)
        y = rl.random_int(0, 99)
        v_corridor(x, 0, 239)

def update(event):
    generate()
    rl.draw_image(rl.array_to_image(level, palette=[rl.BLACK, rl.WHITE, rl.RED]), 0, 0)

rl.run(update, rl.ON_KEY)
