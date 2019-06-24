import rl

WIDTH, HEIGHT = 320, 240

rl.set_seed()

level = rl.Array(WIDTH, HEIGHT)
rooms = [(rl.random_int(0, WIDTH), rl.random_int(0, HEIGHT)) for i in range(40)]

def connect(room1, room2):
    x1, y1 = room1
    x2, y2 = room2
    while not (x1 == x2 and y1 == y2):
        level[x1, y1] = 1
        if x1 < x2:
            x1 += 1
        elif x1 > x2:
            x1 -= 1
        if y1 < y2:
            y1 += 1
        elif y1 > y2:
            y1 -= 1

for i in range(len(rooms) - 1):
    x, y = rooms[i]
    best_d = best = None
    for other in rooms[i + 1:]:
        x2, y2 = other
        d = rl.distance(x, y, x2, y2)
        if best is None or best_d > d:
            best_d = d
            best = other
    connect(rooms[i], best)

for i in range(40):
    room1, room2 = rooms[rl.random_int(0, len(rooms) - 1)], rooms[rl.random_int(0, len(rooms) - 1)]
    d = rl.distance(room1[0], room1[1], room2[0], room2[1])
    if d > WIDTH / 6 and d < WIDTH / 3:
        connect(room1, room2)

for room in rooms:
    x, y = room
    width = rl.random_int(5, 10)
    height = rl.random_int(5, 10)
    level.fill_rect(x - width // 2, y - height // 2, width, height)

def update(event):
    rl.draw_image(rl.array_to_image(level, palette=[rl.BLACK, rl.WHITE]), 0, 0)

rl.run(update, rl.ON_KEY)


