# TODO:
# text with shadow
# player attack other
# pickup items
# use item
# actor index

import rl
import math
import ujson

rl.set_app_name('example-game')
WIDTH, HEIGHT = 40, 20

class Tile:
    def __init__(self, name, num, fg=0, bg=0):
        self.name = name
        self.num = num
        self.fg = fg
        self.bg = bg

    WIDTH = 9
    HEIGHT = 16

    NOTHING = 0
    FLOOR = 1
    WALL = 2
    STAIRS = 3

Tile.minirogue = [
        Tile('nothing', 0),
        Tile('floor', 0, rl.DARKGRAY, rl.color(32, 32, 32)),
        Tile('wall', 2, rl.DARKGREEN, rl.color(32, 32, 32)),
        Tile('stairs', 13, rl.LIGHTGRAY, rl.color(32, 32, 32)),

        Tile('player', 88, rl.PEACH),
        Tile('bat', 74, rl.INDIGO),
        Tile('cat', 75, rl.ORANGE),
        Tile('rat', 76, rl.BROWN),
        Tile('orc', 83, rl.DARKGREEN),

        Tile('key', 22, rl.YELLOW),
        Tile('heart', 38, rl.RED),
        Tile('sword', 69, rl.WHITE),
        Tile('potion', 52, rl.GREEN),

        Tile('corpse', 17, rl.RED),
    ]

Tile.dcss = [
        Tile('nothing', 0),
        Tile('floor', 13),
        Tile('wall', 1),
        Tile('stairs', 14),

        Tile('player', 2),
        Tile('bat', 11),
        Tile('cat', 4),
        Tile('rat', 5),
        Tile('orc', 6),

        Tile('key', 10),
        Tile('heart', 12),
        Tile('sword', 8),
        Tile('potion', 9),

        Tile('corpse', 7),
    ]

Tile.ascii = [
        Tile('nothing', 0),
        Tile('floor', 0, rl.DARKGRAY, rl.color(32, 32, 32)),
        Tile('wall', ord('#'), rl.DARKGREEN, rl.color(32, 32, 32)),
        Tile('stairs', ord('>'), rl.LIGHTGRAY, rl.color(32, 32, 32)),

        Tile('player', ord('@'), rl.PEACH),
        Tile('bat', ord('b'), rl.INDIGO),
        Tile('cat', ord('c'), rl.ORANGE),
        Tile('rat', ord('r'), rl.BROWN),
        Tile('orc', ord('o'), rl.DARKGREEN),

        Tile('key', ord('*'), rl.YELLOW),
        Tile('heart', ord('$'), rl.RED),
        Tile('sword', ord('|'), rl.WHITE),
        Tile('potion', ord('!'), rl.GREEN),

        Tile('corpse', ord('%'), rl.RED),
    ]

def load_theme():
    global tileset, font
    tilesets = [
            ['data/ascii_8x8.png', 8, 8, Tile.ascii, 8],
            ['data/minirogue-c64-all.png', 8, 8, Tile.minirogue, 8],
            ['data/cp437.png', 9, 16, Tile.ascii, 16],
            ['data/polyducks_12x12.png', 12, 12, Tile.ascii, 8],
            ['data/dcss.png', 32, 32, Tile.dcss, 32],
        ]
    filename, Tile.WIDTH, Tile.HEIGHT, Tile.mapping, font_size = tilesets[game.theme % len(tilesets)]

    rl.init_display('Example roguelike [' + filename + ']', WIDTH * Tile.WIDTH, (HEIGHT + 4) * Tile.HEIGHT)
    tileset = rl.Image(filename, Tile.WIDTH, Tile.HEIGHT)
    if Tile.mapping is Tile.ascii:
        font = tileset
    else:
        font = rl.Font('data/font.ttf', font_size)

class Level:
    def __init__(self, width, height):
        self.array = rl.Array(width, height)
        self.array.fill(Tile.FLOOR)
        self.bsp(0, 0, WIDTH, HEIGHT, Tile.FLOOR, Tile.WALL)
        self.memory = rl.Array(width, height)

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
            self.array[x + split, l] = floor
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
            self.array[l, y + split] = floor

    def is_empty(self, x, y):
        if self.array[x, y] != Tile.FLOOR:
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

    def around(self, x, y, radius, blocking=False):
        found = []
        for actor in self:
            if not blocking or (blocking and actor.blocking):
                distance = (actor.x - x) ** 2 + (actor.y - y) ** 2
                if distance < radius ** 2:
                    found.append(actor)
        return found

class Actor:
    def __init__(self, **kwargs):
        self.x = self.y = self.z = 0
        self.vision = 10
        self.blocking = False
        self.type = 'actor'
        for name, value in kwargs.items():
            setattr(self, name, value)

    def distance_to(self, other):
        return math.sqrt((self.x - other.x) ** 2 + (self.y - other.y) ** 2)

    def can_see(self, other):
        if self.distance_to(other) > self.vision:
            return False
        return level.array.can_see(self.x, self.y, other.x, other.y, Tile.WALL)

    def __repr__(self):
        return ujson.dumps(self.__dict__)


class Monster(Actor):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.type = 'monster'
        self.z = 1
        self.blocking = True

    def move(self, dx, dy):
        x, y = self.x + dx, self.y + dy
        if level.array[x, y] == Tile.WALL:
            return
        found = actors.around(x, y, 1, blocking=True)
        if len(found) == 0:
            self.x = x
            self.y = y
        elif found[0].type == 'monster' and self is player:
            self.attack(found[0])

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
        messages.append(self.name + ' attacks ' + other.name + ' for ' + str(self.damage) + ' hp')
        other.hp -= self.damage
        if other.hp <= 0:
            other.hp = 0
            other.tile = 13
            other.blocking = False
            messages.append(other.name + ' dies')
            if other is player:
                game.state = 'dead'
                messages.append('game over')

    def take_turn(self):
        if self.hp <= 0:
            return
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
        self.z = 2


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

        player = Player(name='player', tile=4, hp=30, damage=3)
        actors.place(player)
        for i in range(3):
            actors.place(Monster(name='bat', tile=5, hp=1, damage=1))
            actors.place(Monster(name='cat', tile=6, hp=5, damage=2))
            actors.place(Monster(name='rat', tile=7, hp=3, damage=1))
            actors.place(Monster(name='orc', tile=8, hp=8, damage=2))

        actors.place(Item(name='key', tile=9))
        actors.place(Item(name='heart', tile=10))
        actors.place(Item(name='sword', tile=11))
        actors.place(Item(name='potion', tile=12))

        fov = level.array.field_of_view(player.x, player.y, 10, Tile.WALL, True)
        level.array.copy_masked(level.memory, fov)

        self.state = 'playing'
        self.theme = 0

    def save(self):
        data = {}
        data['seed'] = rl.get_seed()
        data['actors'] = actors
        data['messages'] = messages
        data['player'] = actors.index(player)
        data['level'] = level.array.to_string()
        data['memory'] = level.memory.to_string()
        data['state'] = self.state
        data['theme'] = self.theme
        rl.save_pref('state.json', ujson.dumps(data))

    def load(self):
        global level, actors, player, messages, fov
        data = ujson.loads(rl.load_pref('state.json'))
        actors = LevelActors()
        mapping = {'actor': Actor, 'player': Player, 'monster': Monster, 'item': Item}
        for actor in data['actors']:
            actors.append(mapping[actor['type']](**actor))
        messages = data['messages']
        level = Level(WIDTH, HEIGHT)
        level.array = rl.array_from_string(data['level'])
        level.memory = rl.array_from_string(data['memory'])
        player = actors[data['player']]
        self.state = data['state']
        self.theme = data['theme']
        fov = level.array.field_of_view(player.x, player.y, 10, Tile.WALL, True)
        rl.set_seed(data['seed'])

game = Game()

try:
    game.load()
except Exception as e:
    print(e)
    game.new()

load_theme()

def handle_input(key):
    global fov
    dx = dy = 0
    if key == rl.ESCAPE:
        game.save()
        rl.quit()
    elif key == ord('t'):
        game.theme += 1
        load_theme()
        return rl.REDRAW
    elif key == ord('N'):
        game.new()

    if game.state == 'dead':
        return rl.REDRAW

    if key == ord('h') or key == rl.LEFT:
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
    if dx != 0 or dy != 0:
        player.move(dx, dy)
        fov = level.array.field_of_view(player.x, player.y, 10, Tile.WALL, True)
        level.array.copy_masked(level.memory, fov)
    return key

def update():
    for actor in actors:
        if actor.type == 'monster':
            actor.take_turn()

def redraw():
    rl.clear()
    # draw the full level
    '''rl.draw_array(level.array, tileset, 0, 0, 
            mapping=[tile.num for tile in Tile.mapping],
            fg=[tile.fg for tile in Tile.mapping],
            bg=[tile.bg for tile in Tile.mapping])'''

    # draw memorized tiles
    rl.draw_array(level.memory, tileset, 0, 0, 
            mapping=[tile.num for tile in Tile.mapping],
            fg=[rl.color(255, 255, 255, 192) for tile in Tile.mapping])
    # draw level masked with fov
    to_draw = rl.Array(WIDTH, HEIGHT)
    level.array.copy_masked(to_draw, fov)
    rl.draw_array(to_draw, tileset, 0, 0, 
            mapping=[tile.num for tile in Tile.mapping],
            fg=[tile.fg for tile in Tile.mapping],
            bg=[tile.bg for tile in Tile.mapping])

    # draw actors
    actors.sort(key=lambda x: x.z)
    for actor in actors:
        if player.can_see(actor):
            tile = Tile.mapping[actor.tile]
            floor = Tile.mapping[level.array[actor.x, actor.y]]
            rl.draw_tile(tileset, actor.x * Tile.WIDTH, actor.y * Tile.HEIGHT, floor.num, floor.fg, floor.bg)
            rl.draw_tile(tileset, actor.x * Tile.WIDTH, actor.y * Tile.HEIGHT, tile.num, tile.fg, tile.bg)
    #rl.draw_image(tileset, Tile.WIDTH * WIDTH - 128, 0)

    rl.draw_text(font, Tile.WIDTH // 8, Tile.HEIGHT // 8, 'hp: %d  damage: %d' % (player.hp, player.damage), color=rl.color(0, 0, 0, 192))
    rl.draw_text(font, 0, 0, 'hp: %d  damage: %d' % (player.hp, player.damage))

    rl.draw_text(font, 0, Tile.HEIGHT * HEIGHT, '\n'.join(messages[-4:]))

def handler(event):
    if handle_input(event) != rl.REDRAW:
        update()
    redraw()

rl.run(handler, rl.UPDATE_KEY)
game.save()
