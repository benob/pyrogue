import rl
import math
import ujson

rl.set_app_name('example-game')
WIDTH, HEIGHT = 60, 25

class Tile:
    def __init__(self, name, num, fg, bg, blocking=False):
        self.name = name
        self.num = num
        self.fg = fg
        self.bg = bg
        self.blocking = blocking

    WIDTH = 9
    HEIGHT = 16

    FLOOR = 0
    WALL = 1
    STAIRS = 2

Tile.mapping = [
        Tile('floor', ord('.'), rl.DARKGRAY, rl.color(32, 32, 32)),
        Tile('wall', ord('#'), rl.LIGHTGRAY, rl.color(32, 32, 32)),
        Tile('stairs', ord('>'), rl.BROWN, rl.color(32, 32, 32)),
    ]

class Level:
    def __init__(self, width, height):
        self.array = rl.array(width, height)
        self.array.fill(Tile.FLOOR)
        self.bsp(0, 0, WIDTH, HEIGHT, Tile.FLOOR, Tile.WALL)

        self.array.line(0, 0, WIDTH - 1, 0, Tile.WALL)
        self.array.line(0, HEIGHT - 1, WIDTH, HEIGHT - 1, Tile.WALL)
        self.array.line(0, 0, 0, HEIGHT - 1, Tile.WALL)
        self.array.line(WIDTH - 1, 0, WIDTH - 1, HEIGHT - 1, Tile.WALL)

        self.array.place_random(Tile.FLOOR, Tile.STAIRS) # place stairs
        self.width = width
        self.height = height

    def bsp(self, x, y, w, h, floor, wall):
        if w <= rl.random_int(5, 10) or h <= rl.random_int(5, 10):
            return
        if w > h:
            split = rl.random_int(5, w - 5)
            for i in range(1, h):
                self.array[x + split, y + i] = wall
            self.bsp(x        , y, split    , h, floor, wall)
            self.bsp(x + split, y, w - split, h, floor, wall)
            while True:
                l = y + rl.random_int(2, h - 2)
                if self.array[x + split - 1, l] == floor and self.array[x + split + 1, l] == floor:
                    break
            self.array[x + split, l] = 0
        else:
            split = rl.random_int(5, h - 5)
            for i in range(1, w):
                self.array[x + i, y + split] = wall
            self.bsp(x, y        , w,     split, floor, wall)
            self.bsp(x, y + split, w, h - split, floor, wall)
            while True:
                l = x + rl.random_int(2, w - 2)
                if self.array[l, y + split - 1] == floor and self.array[l, y + split + 1] == floor:
                    break
            self.array[l, y + split] = 0

    def is_empty(self, x, y):
        if self.array[x, y] == Tile.WALL:
            return False
        for actor in actors:
            if actor.x == x and actor.y == y:
                return False
        return True

    def __repr__(self):
        return ujson.dumps([[self.array[i, j] for i in range(self.width)] for j in range(self.height)])


class LevelActors(list):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def place(self, actor):
        while True:
            x, y = level.array.find_random(Tile.FLOOR)
            if level.is_empty(x, y):
                actor.x, actor.y = x, y
                break
        self.append(actor)

    def around(self, x, y, radius):
        found = []
        for actor in self:
            distance = (actor.x - x) ** 2 + (actor.y - y) ** 2
            if distance < radius ** 2:
                found.append(actor)
        return found

class Actor:
    def __init__(self, **kwargs):
        self.x = self.y = 0
        self.vision = 10
        self.type = 'actor'
        for name, value in kwargs.items():
            setattr(self, name, value)

    def distance_to(self, other):
        return math.sqrt((self.x - other.x) ** 2 + (self.y - other.y) ** 2)

    def can_see(self, other):
        if self.distance_to(other) > self.vision:
            return False
        return level.array.can_see(self.x, self.y, other.x, other.y)

    def __repr__(self):
        return ujson.dumps(self.__dict__)


class Monster(Actor):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.type = 'monster'

    def move(self, dx, dy):
        x, y = self.x + dx, self.y + dy
        if level.array[x, y] == Tile.WALL:
            return
        found = actors.around(x, y, 1)
        if len(found) == 0:
            self.x = x
            self.y = y

    def move_towards(self, x, y):
        dx = dy = 0
        if self.x < x:
            dx = 1
        elif self.x > x:
            dx = -1
        if self.y < y:
            dy = 1
        elif self.y > y:
            dy = -1
        self.move(dx, dy)

    def attack(self, other):
        messages.append(self.name + ' attack ' + other.name)

    def take_turn(self):
        distance = self.distance_to(player)
        if distance < 2:
            self.attack(player)
        elif distance < 8:
            self.move_towards(player.x, player.y)
        else:
            self.move(rl.random_int(-1, 1), rl.random_int(-1, 1))


class Player(Monster):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.type = 'player'


class Item(Actor):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.type = 'item'

class Game:
    def new(self):
        global level, actors, player, messages, fov
        rl.set_seed(0)
        messages = []

        level = Level(WIDTH, HEIGHT)
        actors = LevelActors()

        player = Player(name='player', tile=ord('@'), color=rl.PEACH)
        actors.place(player)
        for i in range(3):
            actors.place(Monster(name='bat', tile=ord('b'), color=rl.INDIGO))
            actors.place(Monster(name='cat', tile=ord('c'), color=rl.ORANGE))
            actors.place(Monster(name='rat', tile=ord('r'), color=rl.BROWN))
            actors.place(Monster(name='orc', tile=ord('o'), color=rl.DARKGREEN))

        actors.place(Item(name='key', tile=ord('*'), color=rl.YELLOW))
        actors.place(Item(name='heart', tile=ord('!'), color=rl.RED))
        actors.place(Item(name='sword', tile=ord('|'), color=rl.WHITE))
        actors.place(Item(name='potion', tile=ord('!'), color=rl.GREEN))

        fov = level.array.field_of_view(player.x, player.y, 10, 1, True)

    def save(self):
        data = {}
        data['seed'] = rl.get_seed()
        data['actors'] = actors
        data['messages'] = messages
        data['player'] = actors.index(player)
        data['level'] = level
        rl.save_pref('state.json', ujson.dumps(data))

    def load(self):
        global level, actors, player, messages, fov
        data = ujson.loads(rl.load_pref('state.json'))
        rl.seed(data['seed'])
        actors = LevelActors()
        mapping = {'actor': Actor, 'player': Player, 'monster': Monster, 'item': Item}
        for actor in data['actors']:
            actors.append(mapping[actor['type']](**actor))
        messages = data['messages']
        level = Level(WIDTH, HEIGHT)
        for j, row in enumerate(data['level']):
            for i, value in enumerate(row):
                level[i, j] = value
        player = actors[data['player']]
        fov = level.field_of_view(player.x, player.y, 10)


game = Game()
try:
    game.load()
except:
    game.new()


rl.load_font('data/font.ttf', 8)

def rotate_tileset():
    global current_tileset
    current_tileset += 1
    tilesets = [
            ['data/ascii_8x8.png', 8, 8],
            ['data/polyducks_gloop_8x8.png', 8, 8],
            ['data/cp437.png', 9, 16],
        ]
    filename, Tile.WIDTH, Tile.HEIGHT = tilesets[current_tileset % len(tilesets)]
    messages.append('loading tileset ' + filename)

    rl.init('Pyrogue: Example roguelike [' + filename + ']', WIDTH * Tile.WIDTH, (HEIGHT + 4) * Tile.HEIGHT)
    rl.load_image(0, filename, Tile.WIDTH, Tile.HEIGHT)

current_tileset = 0
rotate_tileset()

def handle_input():
    global fov
    key = rl.wait_key()
    print(key, rl.LEFT, rl.RIGHT, rl.UP, rl.DOWN)
    dx = dy = 0
    if key == rl.ESCAPE or key == rl.QUIT:
        game.save()
        rl.quit()
    elif key == ord('N'):
        game.new()
    elif key == ord('h') or key == rl.LEFT:
        dx = -1
    elif key == ord('j') or key == rl.DOWN:
        dy = 1
    elif key == ord('k') or key == rl.UP:
        dy = -1
    elif key == ord('l') or key == rl.RIGHT:
        dx = 1
    elif key == ord('b'):
        dx, dy = -1, 1
    elif key == ord('n'):
        dx, dy = 1, 1
    elif key == ord('y'):
        dx, dy = -1, -1
    elif key == ord('u'):
        dx, dy = 1, -1
    elif key == ord('t'):
        rotate_tileset()
    if dx != 0 or dy != 0:
        player.move(dx, dy)
        fov = level.array.field_of_view(player.x, player.y, 10, Tile.WALL, True)

def update():
    for actor in actors:
        if actor.type == 'monster':
            actor.take_turn()

def redraw():
    rl.clear()
    rl.draw_array(level.array, 0, 0, 
            mapping=[tile.num for tile in Tile.mapping],
            fg=[tile.fg for tile in Tile.mapping],
            bg=[tile.bg for tile in Tile.mapping])
    rl.draw_array(fov, 0, 0, 
            mapping=[0, 0],
            bg=[rl.color(0, 0, 0, 255), rl.color(0, 0, 0, 0)],
            fg=None)
    for actor in actors:
        if player.can_see(actor):
            rl.colorize_tile(0, actor.x * Tile.WIDTH, actor.y * Tile.HEIGHT, actor.tile, actor.color, 0)
    #rl.draw_image(0, 8 * WIDTH - 80, 0)
    rl.print_text(0, Tile.HEIGHT * HEIGHT, '\n'.join(messages[-4:]))
    rl.present()

while rl.still_running():
    redraw()
    handle_input()
    update()

