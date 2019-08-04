import rl

WALL = 0
FLOOR = 1
DOOR = 2
OUTSIDE = 3
mapping = {'.': FLOOR, '#': WALL, '+': DOOR}

room_templates = [[
'###+#',
'+...#',
'###.#',
'#...+',
'#+###',
],[
'#+#',
'+.+',
'#+#',
],[
'#+########',
'#........#',
'####..##.+',
'#........#',
'+.##..####',
'#........#',
'########+#',
],[
'######+++++######',
'#...............#',
'#......###......#',
'#...............#',
'#...............#',
'+..#####.#####..+',
'+..#.........#..+',
'+..#.........#..+',
'+..#.........#..+',
'+..#####.#####..+',
'#...............#',
'#...............#',
'#...............#',
'#.....#####.....#',
'#.....#...#.....#',
'#.....##.##.....#',
'#...............#',
'######+++++######',
],[
'#+++#',
'+...+',
'+...+',
'+...+',
'#+++#',
],[
'###+###',
'#..#..#',
'#.#.#.#',
'+#.#.#+',
'#.#.#.#',
'#..#..#',
'###+###',
],[
'###+###',
'#.....#',
'#.....#',
'+.##..+',
'#.....#',
'#.....#',
'###+###',
],[
'###+###',
'##....#',
'#.....#',
'+.....+',
'#.....#',
'#.....#',
'###+###',
],[
' ##+## ',
'##...##',
'#.....#',
'+.....+',
'#...#.#',
'##...##',
' ##+## ',
],[
' ##+## ',
'##...##',
'#.....#',
'+..#..+',
'#..#..#',
'##...##',
' ##+## ',
],[
'  ##+##  ',
' ##...## ',
'##.....##',
'#....#..#',
'+...#...+',
'#.......#',
'##.....##',
' ##...## ',
'  ##+##  ',
],[
'  ##+##  ',
' ##...## ',
'##.....##',
'#.......#',
'+.......+',
'#.......#',
'###...###',
' ##...## ',
'  ##+##  ',
],[
'  #++++++#  ',
' ##......## ',
'##........##',
'+..#.#..#..+',
'+.......#..+',
'+..#.#..#..+',
'+.....#....+',
'+..#.......+',
'+..###.##..+',
'##........##',
' ##......## ',
'  #++++++#  ',
],[
'  #++++++#  ',
' ##......## ',
'##........##',
'+..........+',
'+..........+',
'+..###.....+',
'+..#..#....+',
'+..#..#....+',
'+...###....+',
'###.......##',
' ##......## ',
'  #++++++#  ',
],[
'####+##  ',
'#.....## ',
'#......##',
'###.....#',
'+.......+',
'###.....#',
'#......##',
'#.....## ',
'####+##  ',
],[
'#+######',
'#......+',
'+......#',
'####...#',
'   #...#',
'   +...+',
'   ##+##',
],[
'###+###',
'#.....+',
'#.....#',
'+....##',
'###+###',
],[
'###+#    ',
'#...#    ',
'#...#    ',
'+...#    ',
'#...#####',
'#.......+',
'#.......#',
'#+#######',
],[
'###+#######',
'#.........#',
'+....#....+',
'#...#.....#',
'#..#......#',
'#.#.......#',
'#.........#',
'#####+#####',
],[
'######++++######',
'#..............#',
'#..............#',
'#..............#',
'#....####......#',
'+......##......+',
'+..#...........+',
'+..#...........+',
'#..##..##......#',
'#..##..##......#',
'#..............#',
'#..............#',
'######++++######',
],[
'###+#######',
'#.........#',
'+.........+',
'#...#.#.#.#',
'#.........#',
'#...#.....#',
'#.........#',
'######+####',
],[
'###+#######',
'#.........#',
'+.........+',
'#...#######',
'#...#.....#',
'#...#.....#',
'#.........#',
'######+####',
],[
'##+##     ##+##',
'#...#######...#',
'+.............+',
'#.............#',
'###..##.##..###',
'  #..#...#..#  ',
'###..#...#..###',
'#....#...#....#',
'+....#...#....+',
'#...###+###...#',
'##+##     ##+##',
],[
'###+###',
'#.....#',
'#.....#',
'+.##..+',
'#.....#',
'#.....#',
'#.....#',
'+..##.+',
'#.....#',
'#..#..#',
'##+####',
],[
'###+###',
'##...##',
'#.....#',
'#.....#',
'#..#..#',
'+.##..+',
'#..#..#',
'#.....#',
'#.....#',
'##...##',
'###+###',
],[
'#+##+##+#+##',
'#..........+',
'#..######..#',
'+..#....#..+',
'#..#..#.#..#',
'#..####.#..+',
'+..........#',
'##+###+##+##',
],[
'#+##+##+#+##',
'+..........+',
'#..##..##..#',
'#..#....#..#',
'+..#....#..+',
'#..#....#..#',
'#.......#..#',
'#..######..+',
'+..........#',
'##+###+##+##',
],[
'###+########',
'#..........#',
'#..........+',
'+......#####',
'#......#    ',
'#......#    ',
'#...####    ',
'+...#       ',
'#...#       ',
'##+##       ',
],[
'###+####+####',
'#...........+',
'#..........##',
'#.........## ',
'+....#...##  ',
'#....#####   ',
'#....#       ',
'#....+       ',
'+....#       ',
'#...##       ',
'#..##        ',
'#.##         ',
'#+#          ',
],[
'#+#   ',
'#.#   ',
'+.##  ',
'#..###',
'#....+',
'#..###',
'+.##  ',
'#.#   ',
'#+#   ',
],[
'  ##+##  ',
'###...###',
'+...#...+',
'#.....###',
'####+##  ',
],[
'###+###',
'#.....#',
'#..#..#',
'+.....+',
'#...#.#',
'#.....#',
'###+###',
]]

def identity(template):
    return template

def mirror1(template):
    return list(reversed(template))

def mirror2(template):
    return [list(reversed(row)) for row in template]

def mirror3(template):
    return mirror1(mirror2(template))

def transpose(template):
    N, M = len(template[0]), len(template)
    result = [['.'] * M for i in range(N)]
    for j in range(M):
        for i in range(N):
            result[i][j] = template[j][i]
    return result

def rotate1(template):
    N, M = len(template), len(template[0])
    result = [[' '] * N for i in range(M)]
    for j in range(N):
        for i in range(M):
            a = template[j][i]
            result[i][N - j - 1] = a
    return result

def rotate2(template):
    return rotate1(rotate1(template))

def rotate3(template):
    return rotate1(rotate2(template))

transforms = [identity, mirror1, mirror2, mirror3, transpose, rotate1, rotate2, rotate3]

def apply_transform(template):
    transform = rl.random_choice(transforms)
    return transform(template)

def choose_template(templates):
    return templates[rl.random_int(0, len(templates) - 1)]

def intersect(a, b):
    i1 = False
    if b.x < a.x:
        a, b = b, a
    if a.x + a.width - 1 > b.x:
        i1 = True
    i2 = False
    if b.y < a.y:
        a, b = b, a
    if a.y + a.height - 1 > b.y:
        i2 = True
    return i1 and i2

class Door:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.used = False

class Room:
    def __init__(self, template_subset):
        template = apply_transform(rl.random_choice(template_subset))
	self.template = template
	self.height = len(template)
	self.width = len(template[0])
	self.x = 0
	self.y = 0

	self.doors = []
        for j, row in enumerate(template):
            for i, c in enumerate(row):
                if c == '+':
                    self.doors.append(Door(i, j))

    def bake(self, level, offset_x, offset_y):
        for j in range(self.height):
            for i in range(self.width):
                if self.template[j][i] != ' ':
                    level[self.x + i + offset_x, self.y + j + offset_y] = mapping[self.template[j][i]]

def generate():
    num_rooms = 50
    rooms = [Room(room_templates)]
    fails = 0
    while len(rooms) < num_rooms:
        a = rl.random_choice(rooms)
        door = rl.random_choice(a.doors)
        if not door.used:
            b = Room(room_templates)
            other_door = rl.random_choice(b.doors)
            b.x = a.x + door.x - other_door.x
            b.y = a.y + door.y - other_door.y
            ok = True
            for i, room in enumerate(rooms):
                if intersect(room, b):
                    ok = False
            if ok:
                door.used = True
                other_door.used = True
                rooms.append(b)
                fails = 0
            else:
                fails += 1
            if fails > 1000:
                break
    min_x = min([room.x for room in rooms])
    min_y = min([room.y for room in rooms])
    max_x = max([room.x + room.width for room in rooms])
    max_y = max([room.y + room.height for room in rooms])
    width = max_x - min_x
    height = max_y - min_y

    level = rl.Array(320, 240)
    level.fill(OUTSIDE)

    for room in rooms:
        room.bake(level, - 160, - 120)

    for j in range(level.height()):
        for i in range(level.width()):
            if level[i, j] == DOOR:
                if i == 0 or i == width - 1 or j == 0 or j == height - 1 \
                    or level[i - 1, j] == OUTSIDE or level[i + 1, j] == OUTSIDE or level[i, j - 1] == OUTSIDE or level[i, j + 1] == OUTSIDE \
                    or level[i - 1, j - 1] == OUTSIDE or level[i + 1, j - 1] == OUTSIDE or level[i - 1, j + 1] == OUTSIDE or level[i + 1, j + 1] == OUTSIDE:
                    level[i, j] = WALL
    level.replace(DOOR, FLOOR)
    level.replace(OUTSIDE, WALL)
    return level

def update(event):
    level = generate()
    rl.clear()
    rl.draw_image(rl.array_to_image(level, palette=[rl.BLACK, rl.WHITE, rl.RED, rl.BLUE]), 0, 0)

rl.run(update, rl.ON_KEY)
