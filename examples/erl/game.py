# !/usr/bin/env python3
#
# based on libtcod python tutorial
#
 
import math
import sys
import ure
import ujson

import rl
import serialize

rl.set_app_name('expelled-rl')
rl.init('ExpelledRL', 40 * 8, 30 * 8)
rl.poll_event()
rl.present()
rl.load_font('data/04B_03__.TTF', 8, 8)
rl.load_image(0, 'data/tileset.png', 8, 8)

print(sys.argv)
#debug = '-debug' in sys.argv[1:]
debug = False

def capitalize(text):
    return text[0].upper() + text[1:]

def line_iter(x1, y1, x2, y2):
    rl.walk_line_start(x1, y1, x2, y2)
    while True:
	result = rl.walk_line_next()
	if result is None:
	    break
	yield (result[0], result[1])


class Graphics:
    CAVE_ROCK = 0
    CAVE_FLOOR = 1
    CAVE_STAIRS = 2
    BOSS_WALL = 3
    BOSS_FLOOR = 4
    BOSS_CARPET = 5
    BLUE_WALL = 6
    BLUE_FLOOR = 7
    BLUE_STAIRS = 8
    RED_WALL = 9
    RED_FLOOR = 10
    RED_STAIRS = 11
    GRAY_WALL = 12
    GRAY_FLOOR = 13
    WOOD_WALL = 14
    WOOD_FLOOR = 15

    TREE = 16
    GRASS = 17
    ROCK_STAIRS = 18
    LAVA = 19
    WATER = 20

    SWORD = 32
    DAGGER = 33
    SHIELD = 34
    YENDOR_AMULET = 35

    POTION = 49
    SCROLL = 50
    FIREBALL = 51
    BOULDER = 52
    BOULDER_SPLAT = 53

    GHOUL = 64
    ORC = 65
    TROLL = 66
    BAT = 67
    FIRE_ELEMENTAL = 68
    BOSS = 69
    OCTOPUS = 70
    GHOST = 71
    EYE = 72
    DWARF = 73
    WIZARD = 74
    NECROMANCER = 75
    RAT = 76
    SKELETON = 77
    PLAYER = 78
    CORPSE = 79
    TOMB1 = 80
    TOMB2 = 96

    #PLAYER_NOEYES = 80
    #PLAYER = 81

# actual size of the window
SCREEN_WIDTH = 40
SCREEN_HEIGHT = 30
 
# size of the map
MAP_WIDTH = 60
MAP_HEIGHT = 50
 
# sizes and coordinates relevant for the GUI
BAR_WIDTH = 8
PANEL_HEIGHT = 7
PANEL_Y = SCREEN_HEIGHT - PANEL_HEIGHT
MSG_X = BAR_WIDTH + 3
MSG_WIDTH = SCREEN_WIDTH - BAR_WIDTH - 3
MSG_HEIGHT = PANEL_HEIGHT - 1
INVENTORY_WIDTH = 50
ACTION_MENU_WIDTH = 50
CHARACTER_SCREEN_WIDTH = 30
LEVEL_SCREEN_WIDTH = 40
 
# parameters for dungeon generator
ROOM_MAX_SIZE = 10
ROOM_MIN_SIZE = 4
MAX_ROOMS = 20
 
# spell values
#HEAL_AMOUNT = 40
NO_RANGE = 1000
LIGHTNING_DAMAGE = 8
LIGHTNING_RANGE = 3
FEAR_RANGE = 5
FEAR_DURATION = 10
CONFUSE_RANGE = 8
CONFUSE_NUM_TURNS = 10
POSSESS_RANGE = 10
FIREBALL_RADIUS = 2
FIREBALL_RANGE = 7
FIREBALL_DAMAGE = 5
FREEZE_RANGE = 3
FREEZE_DURATION = 20
TELEPORT_RANGE = 9
DIG_RANGE = 10
 
# experience and level-ups
LEVEL_UP_BASE = 200
LEVEL_UP_FACTOR = 150
 
 
FOV_ALGO = 0  # default FOV algorithm
FOV_LIGHT_WALLS = True  # light walls or not
TORCH_RADIUS = 10
 
LIMIT_FPS = 20  # 20 frames-per-second maximum
 
 
def water_effect(monster):
    if monster.alive and len(set(['can_fly', 'can_swim']) & set(monster.skills)) == 0:
        if monster is player or player.can_see(monster):
            message(capitalize(monster.get_name('{is}')) + ' drowning.', rl.RED)
        monster.take_damage(max(1, monster.max_hp // 3))
        return False
    return True

def lava_effect(monster):
    if monster.alive and 'immune_fire' not in monster.skills:
        if monster is player or player.can_see(monster):
            message(capitalize(monster.get_name('{is}')) + ' burning.', rl.RED)
        monster.take_damage(50)
        return False
    return True

def fireball_effect(monster):
    if monster.alive and 'immune_fire' not in monster.skills:
        if monster is player or player.can_see(monster):
            message(capitalize(monster.get_name('{is}')) + ' burning.', rl.RED)
        monster.take_damage(FIREBALL_DAMAGE)
        return False
    return True

class Tile:
    def __init__(self, num, type, blocked, block_sight, graphics, effect=None):
        self.num = num
        self.type = type
        self.blocked = blocked
        self.block_sight = block_sight
        self._graphics = graphics
        self.effect = effect

    def draw(self, x, y):
        rl.draw_tile(0, x, y, self.graphics)

    @property
    def graphics(self):
        if type(self._graphics) == list:
            return self._graphics[dungeon_level]
        return self._graphics
 
    UNUSED = 0
    FLOOR = 1
    ROCK = 2
    WATER = 3
    LAVA = 4
    GRASS = 5
    BOSS_WALL = 6
    BOSS_FLOOR = 7
    BOSS_CARPET = 8

Tile.mapping = [
        Tile(0, 'nothing', True, True, 0),
        Tile(1, 'floor', False, False, [
                None, # level 0
                Graphics.CAVE_FLOOR, # level 1 (no healing)
                Graphics.CAVE_FLOOR, # level 2 (skills)
                Graphics.BLUE_FLOOR, # level 3 (water)
                Graphics.CAVE_FLOOR, # level 4 (invisibles)
                Graphics.RED_FLOOR, # level 5 (lava)
                Graphics.GRAY_FLOOR, # level 6 (necromancers)
                Graphics.WOOD_FLOOR, # level 7 (wizards)
                Graphics.BOSS_FLOOR, # level 8 (boss)
        ]),
        Tile(2, 'rock', True, True, [
                None, # level 0
                Graphics.CAVE_ROCK, # level 1 (no healing)
                Graphics.CAVE_ROCK, # level 2 (skills)
                Graphics.BLUE_WALL, # level 3 (water)
                Graphics.CAVE_ROCK, # level 4 (invisibles)
                Graphics.RED_WALL, # level 5 (lava)
                Graphics.GRAY_WALL, # level 6 (necromancers)
                Graphics.WOOD_WALL, # level 7 (wizards)
                Graphics.BOSS_WALL, # level 8 (boss)
        ]),
        Tile(3, 'water', False, False, Graphics.WATER, water_effect),
        Tile(4, 'lava', False, False, Graphics.LAVA, lava_effect),
        Tile(5, 'grass', False, False, Graphics.GRASS),
        Tile(6, 'wall', True, True, Graphics.BOSS_WALL),
        Tile(7, 'floor', False, False, Graphics.BOSS_FLOOR),
        Tile(8, 'carpet', False, False, Graphics.BOSS_CARPET),
    ]


class Level:
    def __init__(self, width, height):
        self.tiles = rl.array(width, height)
        self.blocked = rl.array(width, height)
        self.visited = rl.array(width, height)
        self.fov = rl.array(width, height)

    def __setitem__(self, index, tile):
        #print('set', index, type(index))
        self.tiles[index] = tile.num
        self.blocked[index] = tile.blocked

    def __getitem__(self, index):
        #print('get', index, type(index))
        num = self.tiles[index]
        if num >= 0 and num < len(Tile.mapping):
            return Tile.mapping[num]
        return None

    def __del__(self):
        self.tiles.free()
        self.blocked.free()
        self.visited.free()
        self.fov.free()

 
class Rect:
    # a rectangle on the map. used to characterize a room.
    def __init__(self, x, y, w, h):
        self.x1 = x
        self.y1 = y
        self.x2 = x + w
        self.y2 = y + h
 
    def center(self):
        center_x = (self.x1 + self.x2) // 2
        center_y = (self.y1 + self.y2) // 2
        return (center_x, center_y)
 
    def intersect(self, other):
        # returns true if this rectangle intersects with another one
        return (self.x1 <= other.x2 and self.x2 >= other.x1 and
                self.y1 <= other.y2 and self.y2 >= other.y1)
 

class Actor:
    def __init__(self, x=0, y=0, tile=0, name=None, color=0, **kwargs):
        self.type = 'actor'
        self.x = x
	self.y = y
        self.tile = tile
        self.name = name
        self.color = color
	self.z = 0
        self.sight_radius = 10
        self.blocks = False
        self.always_visible = False
        for name, value in kwargs.items():
            setattr(self, name, value)

    def __repr__(self):
        return ujson.dumps(self.__dict__)

    @property
    def alive(self):
        return hasattr(self, 'hp') and self.hp > 0

    def can_see(self, other):
        distance = self.distance_to(other)
        if distance > self.sight_radius:
            return False
        if isinstance(other, Monster) and 'invisible' in other.skills and isinstance(self, Monster) and 'see_invisible' not in self.skills:
            return distance < 2
        if self is player:
            return level.fov[other.x, other.y]
        if other is player:
            return level.fov[self.x, self.y]
        # symetrize line-of-sight
        start_x, start_y = self.x, self.y
        end_x, end_y = other.x, other.y
        #if other is player:
        #    start_x, start_y = player.x, player.y
        #    end_x, end_y = self.x, self.y
        if level.blocked.can_see(start_x, start_y, end_x, end_y):
            return True
        return False

    def can_see_tile(self, x, y):
        distance = math.sqrt((x - self.x) ** 2 + (y - self.y) ** 2)
        if distance > self.sight_radius:
            return False
        # TODO: use rl.can_see
        for i, j in line_iter(self.x, self.y, x, y):
            if i == self.x and j == self.y:
                continue
            if level.blocked[i, j]:
                return False
        return True

    def move(self, dx, dy):
        # move by the given amount, if the destination is not blocked
        x = self.x + dx
        y = self.y + dy
        if x < 0 or x >= MAP_WIDTH or y < 0 or y >= MAP_HEIGHT:
            return False
        # TODO: refrain from walking in lava
        if not is_blocked(x, y):
            self.x = x
            self.y = y
            if self is player:
                compute_fov()
            return True
        elif not level.blocked[x, y]: # not blocked by a wall
            for obj in objects:
                if obj.x == x and obj.y == y and isinstance(obj, Monster) and not obj.is_hostile(self):
                    if obj is player:
                        return False
                    # swap places
                    self.x, obj.x = obj.x, self.x
                    self.y, obj.y = obj.y, self.y
                    if self is player:
                        message(self.get_name('swap{s} places with') + ' ' + obj.get_name() + '.', rl.LIGHTGRAY)
                        compute_fov()
                    return True
        return False
 
    def move_towards(self, target_x, target_y):
	path = level.blocked.shortest_path(self.x, self.y, target_x, target_y)
        if len(path) > 0:
            self.move(path[0][0] - self.x, path[0][1] - self.y) 

    def move_away_from(self, target_x, target_y):
        # TODO: use better approach
        max_distance = 0
        argmax = None
        for x in range(self.x - self.sight_radius, self.x + self.sight_radius + 1):
            for y in range(self.y - self.sight_radius, self.y + self.sight_radius + 1):
                if not is_blocked(x, y):
                    distance = (x - target_x) ** 2 + (y - target_y) ** 2
                    if distance > max_distance:
                        max_distance = distance
                        argmax = (x, y)
        if argmax is not None and self.distance(target_x, target_y) < max_distance:
            return self.move_towards(*argmax)
        # fallback, try local escape
        dx = dy = 0
        if target_x < self.x:
            dx = 1
        elif target_x > self.x:
            dx = -1
        if target_y < self.y:
            dy = 1
        elif target_y > self.y:
            dy = -1
        return self.move(dx, dy)
 
    def distance_to(self, other):
        # return the distance to another object
        dx = other.x - self.x
        dy = other.y - self.y
        return math.sqrt(dx ** 2 + dy ** 2)
 
    def distance(self, x, y):
        # return the distance to some coordinates
        return math.sqrt((x - self.x) ** 2 + (y - self.y) ** 2)
 
    def draw(self, x_offset, y_offset):
        # only show if it's visible to the player; or it's set to "always visible" and on an explored tile
        if (tile_in_fov(self.x, self.y) or
                (self.always_visible and level.visited[self.x, self.y] > 0)):
            if game_state == 'dead' or (isinstance(self, Monster) and not self.is_hostile(player)) or not (isinstance(self, Monster) and 'invisible' in self.skills) or ('see_invisible' in player.skills) or self.distance_to(player) <= 1:
                level[self.x, self.y].draw((self.x + x_offset) * 8, (self.y + y_offset) * 8)
                if self is player or (isinstance(self, Monster) and not self.is_hostile(player)):
                    rl.draw_tile(0, (self.x + x_offset) * 8, (self.y + y_offset) * 8, self.tile + 16)
                else:
                    rl.draw_tile(0, (self.x + x_offset) * 8, (self.y + y_offset) * 8, self.tile)
            if not tile_in_fov(self.x, self.y):
                rl.fill_rect((self.x + x_offset) * 8, (self.y + y_offset) * 8, 8, 8, rl.color(0, 0, 0, 128))

    def info(self):
        return str(self)

    def __str__(self):
        return self.get_name(determiner=None)

    def get_pronoun(self, verb=None):
        if self is player:
            if verb is None:
                return 'you'
            else:
                return 'you ' + verb.format(**{'is': 'are', 'has': 'have', 's': ''})
        pronoun = 'it'
        if self.name in ['wizard', 'nuphrindas']:
            pronoun = 'he'
        return pronoun + ' ' + verb.format(**{'is': 'is', 'has': 'has', 's': 's'})

    def get_name(self, verb=None, determiner='the'):
        if self is player:
            if verb is None:
                return 'you'
            else:
                return 'you ' + verb.format(**{'is': 'are', 'has': 'have', 's': ''})
        subject = determiner + ' ' + self.name if determiner else self.name
        if verb:
            return subject + ' ' + verb.format(**{'is': 'is', 'has': 'has', 's': 's'})
        return subject


class Monster(Actor):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.type = 'monster'
        self.z = 1
        self.blocks = True

        self.hp = self.max_hp = 0
        self.mana = self.max_mana = 0
        self.possession_cooldown = 0
        self.current_possession_cooldown = 0
        self.defense = 0
        self.power = 0
        self.target = None
        self.target_location = None
        self.flee = None
        self.skills = []
        self.actions = []
        self.master = None
        self.controllers = [BasicMonster()]

        for name, value in kwargs.items():
            setattr(self, name, value)

        for name in ['hp', 'mana']:
            if name not in kwargs:
                setattr(self, name, getattr(self, 'max_' + name))

    def info(self):
        return ' '.join(['(%d,%d)' % (self.x, self.y), 'HP: %d/%d' % (self.hp, self.max_hp), 'MANA: %d/%d' % (self.mana, self.max_mana), 'target:', str(self.target) + ', flee:', str(self.flee)])

    def draw_hp_bar(self, x_offset, y_offset):
        if tile_in_fov(self.x, self.y) \
                or (self.always_visible and level.visited[self.x, self.y] > 0):
            #if (not 'invisible' in self.skills) or ('see_invisible' in player.skills) or self.distance_to(player) <= 1:
            if game_state == 'dead' or (isinstance(self, Monster) and not self.is_hostile(player)) or not (isinstance(self, Monster) and 'invisible' in self.skills) or ('see_invisible' in player.skills) or self.distance_to(player) <= 1:
                if self.alive and self.hp < self.max_hp:
                    width = 8 * self.hp // self.max_hp
                    if width < 1:
                        width = 1
                    rl.fill_rect((self.x + x_offset) * 8, (self.y + y_offset + 1) * 8, 8, 1, rl.BLACK)
                    rl.fill_rect((self.x + x_offset) * 8, (self.y + y_offset + 1) * 8, width, 1, rl.RED)

    def push_controller(self, controller):
        self.controllers.append(controller)

    def pop_controller(self):
        if len(self.controllers) > 0:
            return self.controllers.pop()

    @property
    def controller(self):
        if len(self.controllers) > 0:
            return self.controllers[-1]
        return None

    def take_turn(self):
        if len(self.controllers) > 0:
            result = self.controllers[-1].take_turn(self)
            if result != 'didnt-take-turn':
                if self.current_possession_cooldown > 0:
                    self.current_possession_cooldown -= 1
                if self.mana < self.max_mana:
                    self.mana += 1
                if level[self.x, self.y].effect:
                    level[self.x, self.y].effect(self)
            return result

    def die(self):
        global player, game_state
        if self is player:
            message('You died! Press ESCAPE to continue.', rl.RED)
            game_state = 'dead'
            self.tile = Graphics.TOMB2
            save_game()
        else:
            if player.can_see(self):
                message(capitalize(self.get_name('{is}')) + ' dead!', rl.ORANGE) 
            self.tile = Graphics.CORPSE
        self.color = rl.RED
        self.blocks = False
        self.controllers = []
        self.name = 'remains of ' + self.name
        self.skills = [x for x in self.skills if x != 'invisible']
        self.master = self.target = self.flee = None
        self.z = 0

        if 'boss' in self.skills:
            # show dead boss and trigger ending
            render_all()
            rl.present()
            rl.wait_key()
            show_ending()

    def attack(self, target):
        if not self.is_hostile(target):
            print('warning, %s trying to attack friend %s' % (self.owner.name, target.name))
            return
        # a simple formula for attack damage
        damage = rl.random_int(0, self.power) - rl.random_int(0, target.defense)
 
        if damage > 0:
            #damage = rl.random_int(1, damage)
            # make the target take some damage
            if player in [self, target] or player.can_see(self):
                message(capitalize(self.get_name('attack{s}')) + ' ' + target.get_name() + ' for ' + str(damage) + ' hit points.')
            else:
                message('You hear combat in the distance', rl.LIGHTGRAY)
            target.take_damage(damage, self)
        else:
            if player in [self, target] or player.can_see(self):
                message(capitalize(self.get_name('attack{s}')) + ' ' + target.get_name() + ' but it has no effect!')
 
    def take_damage(self, damage, perpetrator=None):
        if self.hp <= 0:
            return
        if damage > 0:
            self.hp -= damage
            if self.hp <= 0:
                self.die()
        if perpetrator is not None:
            self.target = perpetrator
 
    def is_hostile(self, other):
        if self is other:
            return False
        if self.target == other or other.target == self:
            return True
        if self.master is other or other.master is self: # friendly with master
            return False
        if self.master is not None and self.master == other.master: # friendly with monsters who have the same master
            return False
        if self.master:
            return self.master.is_hostile(other)
        if self.name == other.name and not(self is player or other is player or self.master is player or other.master is player):
            return False
        return True
 
 
class Transient(Monster):
    def __init__(self, ticks=0, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.ticks = ticks

    def take_turn(self):
        if self.ticks > 0:
            self.ticks -= 1
            super().take_turn()
        else:
            objects.remove(self)
 

class Controller:
    def __init__(self, **kwargs):
        self.type = 'controller'
        for name, value in kwargs.items():
            setattr(self, name, value)
        
    def take_turn(self, monster):
        raise NotImplementedError()

    def __repr__(self):
        return ujson.dumps(self.__dict__)

    @classmethod
    def from_json(_, data):
        cls = {
                'controller': Controller, 
                'do nothing': DoNothing, 
                'basic monster': BasicMonster,
                }
        return cls[data['type']](*[data[x] for x in ['x', 'y', 'tile', 'name', 'color']], **data)



class DoNothing(Controller):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.type = 'do nothing'

    def take_turn(self, monster):
        pass


class SpellEffect(Controller):
    def __init__(self, effect=None, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.type = 'spell effect'
        self.effect = effect

    def take_turn(self, monster):
        for obj in objects:
            if obj.alive and obj is not monster and obj.x == monster.x and obj.y == monster.y:
                self.effect(obj)


class BasicMonster(Controller):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.type = 'basic monster'

    def take_turn(self, monster):
        old_target = monster.target
        old_flee = monster.flee
        def info(*args, **kwargs):
            if debug:
                if (player.can_see(monster) or old_flee is player or old_target is player):
                    print('AI', monster.name, *args, **kwargs)

        info(monster.info())
        target = monster.target
        # dangling target after it becomes friendly
        # no dead target
        # 25% chance to forget target if monster can't see it
        if target \
            and (not target.is_hostile(monster) \
                or target.hp <= 0 \
                or (not monster.can_see(target) and rl.random() < .25)):
            target = monster.target = None
            info('  forget target')

        # save target last seen location to be able to follow it when it leaves fov
        if target:
            monster.target_location = (target.x, target.y)
            info('  save target location %d,%d' % (target.x, target.y))
        else:
            monster.target_location = None

        flee = monster.flee
        # dangling flee after it becomes friendly
        # no dead flee
        # 25% chance to forget flee if monster can't see it
        if flee \
            and (not flee.is_hostile(monster) \
                or flee.hp <= 0 \
                or (not monster.can_see(flee) and rl.random() < .25)):
            flee = monster.flee = None
            info('  forget flee')

        # compute flee based on friends and foes
        friends, foes = friends_and_foes(monster)
        friends += [monster]
        if len(foes) > 0:
            #foes_hp = sum([m.hp for m in foes])
            friends_hp = sum([m.hp for m in friends])
            foes_power = sum([m.power for m in foes])
            #friends_power = sum([m.power for m in friends])
            #foes_defense = sum([m.defense for m in foes])
            friends_defense = sum([m.defense for m in friends])
            if foes_power - friends_defense >= friends_hp:
                flee = monster.flee = closest_hostile(monster, monster.sight_radius)
                info('  outnumbered, flee', str(flee))

        # highest priority is to flee
        if flee and monster.can_see(flee):
            monster.move_away_from(flee.x, flee.y)
            info('  flee: move away from', str(flee))
        elif not target:
            # if no target, move towards master or select new target
            if monster.master and monster.distance_to(monster.master) > 3:
                monster.move_towards(monster.master.x, monster.master.y)
                info('  no target: move towards master')
            else:
                target = monster.target = closest_hostile(monster, monster.sight_radius)
                info('  new target: ', str(target))
        else:
            if monster.can_see(target):
                if monster.distance_to(target) >= 2:
                    potential_actions = [action for action in monster.actions if monster.mana >= action.cost]
                    info('potential-actions', monster.mana, potential_actions)
                    value = rl.random()
                    info(value, value < .5, value > .5)
                    if len(potential_actions) > 0 and value < .5:
                        selected = rl.random_int(0, len(monster.actions) - 1)
                        action = monster.actions[selected]
                        info('selected', action)
                        if monster.distance_to(target) <= action.range:
                            if action in [Action.LIGHTNING, Action.FEAR, Action.DIG, Action.SUMMON, Action.SUMMON_BAT, Action.SUMMON_EYE, Action.SUMMON_RAT, Action.SUMMON_SKELETON, Action.SUMMON]:
                                action.perform(monster)
                            elif action in [Action.CONFUSE, Action.FREEZE, Action.TELEPORT]:
                                action.perform(monster, target)
                            elif action == Action.FIREBALL:
                                action.perform(monster, target.x, target.y)
                            else:
                                raise ValueError('monster cannot handle action ' + str(action))
                            info('  perform action: ', str(action))
                        else:
                            monster.move_towards(target.x, target.y)
                            info('  move towards target: ', str(target))
                    else:
                        monster.move_towards(target.x, target.y)
                        info('  move towards target: ', str(target))
                else:
                    monster.attack(target)
                    info('  move towards target: ', str(target))
            elif monster.target_location is not None and not (monster.x == monster.target_location[0] and monster.y == monster.target_location[1]):
                monster.move_towards(monster.target_location[0], monster.target_location[1])
                info('  move towards old target location %d,%d' % monster.target_location)
            elif target is not None:
                if Action.DIG in monster.actions:
                    Action.DIG.perform(monster, x=target.x, y=target.y)
                    info('  dig to reach target at %d,%d' % (target.x, target.y))
                else:
                    info('  wait, no dig')
            else:
                info('  wait')

        info('  <<', monster.info())
 
 
class ConfusedMonster(Controller):
    def __init__(self, num_turns=CONFUSE_NUM_TURNS, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.type = 'confused monster'
        self.num_turns = num_turns
 
    def take_turn(self, monster):
        if self.num_turns > 0:  # still confused...
            # move in a random direction, and decrease the number of turns confused
            self.num_turns -= 1
            if monster is player:
                result = handle_keys(state='confused')
                monster.move(rl.random_int(-1, 1), rl.random_int(-1, 1))
                return result
            else:
                monster.move(rl.random_int(-1, 1), rl.random_int(-1, 1))
 
        else:  # restore the previous AI (this one will be deleted because it's not referenced anymore)
            monster.pop_controller()
            message(capitalize(monster.get_name('{is}')) + ' no longer confused!', rl.RED)
            if monster is player:
                return handle_keys()
 

class ParalyzedMonster(Controller):
    def __init__(self, num_turns=0, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.type = 'paralized monster'
        self.num_turns = num_turns
    
    def take_turn(self, monster):
        if self.num_turns > 0:
            self.num_turns -= 1
            if monster is player:
                return handle_keys(state='frozen')
        else:
            monster.pop_controller()
            if player.can_see(monster) or player is monster:
                message(capitalize(monster.get_name('start{s}')) + ' moving again', rl.ORANGE)
            if monster is player:
                return handle_keys()


class FrightenedMonster(Controller):
    def __init__(self, flee=None, num_turns=0, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.type = 'frightened monster'
        self.num_turns = num_turns
        self.flee = flee
    
    def take_turn(self, monster):
        if self.num_turns > 0:
            self.num_turns -= 1
            if monster is player:
                result = handle_keys(state='frightened')
                monster.move_away_from(self.flee.x, self.flee.y)
                return result
            else:
                monster.move_away_from(self.flee.x, self.flee.y)
        else:
            monster.pop_controller()
            if player.can_see(monster) or player is monster:
                message(capitalize(monster.get_name('{is}')) + ' not frightened anymore.', rl.ORANGE)
            if monster is player:
                return handle_keys()


class PlayerInput(Controller):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.type = 'player input'

    def take_turn(self, monster):
        return handle_keys()
 
 
def is_blocked(x, y):
    if x < 0 or x >= MAP_WIDTH or y < 0 or y >= MAP_HEIGHT:
        return True
    # first test the map tile
    if level[x, y].blocked or level.blocked[x, y]:
        return True
 
    # now check for any blocking objects
    for object in objects:
        if object.blocks and object.x == x and object.y == y:
            return True
 
    return False
 
 
def fill_circle(x, y, r, tile):
    for i in range(int(x - r), int(x + r)):
        for j in range(int(y - r), int(y + r)):
            if i >= 0 and i < MAP_WIDTH and j >= 0 and j < MAP_HEIGHT:
                distance = math.sqrt((x - i) ** 2 + (y - j) ** 2)
                if distance < r:
                    level[i, j] = Tile.mapping[tile]

def create_river(tile):
    center_x = MAP_WIDTH // 2
    center_y = MAP_HEIGHT // 2
    #configs = [(center_x, 1, 90), (center_x, MAP_HEIGHT - 2, -90), (1, center_y, 0), (MAP_WIDTH - 2, center_y, 180)]
    #choice = rl.random_int(0, len(configs) - 1)
    #x, y, angle = configs[choice]
    x, y, angle = center_x, center_y, rl.random_int(0, 359)
    width = 2
    while x >= 1 and x < MAP_WIDTH - 1 and y >= 1 and y < MAP_HEIGHT - 1:
        if player.distance(x, y) < 10:
            break
        dx = int(math.cos(math.pi * angle / 180) * 5)
        dy = int(math.sin(math.pi * angle / 180) * 5)
        for i, j in line_iter(x, y, x + dx, y + dy):
            fill_circle(i, j, width, tile)
        x += dx
        y += dy
        angle += rl.random_int(0, 60) - 30
        width += (rl.random_int(0, 100) - 50) / 100
        if width < 1:
            width = 1
        if width > 5:
            width = 5

def create_room(room, tile=Tile.FLOOR):
    # go through the tiles in the rectangle and make them passable
    for x in range(room.x1 + 1, room.x2):
        for y in range(room.y1 + 1, room.y2):
            level[x, y] = Tile.mapping[tile]
 
 
def create_h_tunnel(x1, x2, y, tile=Tile.FLOOR):
    for x in range(min(x1, x2), max(x1, x2) + 1):
        level[x, y] = Tile.mapping[tile]
 
 
def create_v_tunnel(y1, y2, x, tile=Tile.FLOOR):
    for y in range(min(y1, y2), max(y1, y2) + 1):
        level[x, y] = Tile.mapping[tile]
 
def make_forest_map():
    global level, objects, stairs

    center_x = MAP_WIDTH // 2
    center_y = MAP_HEIGHT // 2
    radius = min([center_x, center_y])

    def contains_tree(x, y):
        d = math.sqrt((x - center_x) ** 2 + (y - center_y) ** 2)
        if d < 3:
            return False
        if d >= radius:
            return True
        p = 100 * d / radius
        v = rl.random_int(0, 100)
        return v < p

    level = Level(MAP_WIDTH, MAP_HEIGHT)
    level.tiles.fill(Tile.GRASS)
    level.blocked.fill(0)

    objects = [player]
    player.x = center_x
    player.y = center_y
    for y in range(MAP_HEIGHT):
        for x in range(MAP_WIDTH):
            if contains_tree(x, y):
                objects.append(Actor(x, y, Graphics.TREE, 'tree', rl.GREEN, always_visible=True))
                level.blocked[x, y] = 1
    
    angle = rl.random_int(0, 360)
    distance = rl.random_int(2, 3)
    stairs = Actor(center_x + int(distance * math.cos(angle)), center_y + int(distance * math.sin(angle)), Graphics.ROCK_STAIRS, 'stairs', rl.WHITE, always_visible=True)
    objects.append(stairs)
    stairs.z = 0  # so it's drawn below the monsters
    compute_fov()

def make_boss_map():
    global level, objects, player, stairs
    level = Level(MAP_WIDTH, MAP_HEIGHT)
    level.tiles.fill(Tile.BOSS_WALL)
    level.blocked.fill(1)

    center_x = MAP_WIDTH // 2
    center_y = MAP_HEIGHT // 2
    create_room(Rect(center_x - 20, center_y - 20, 40, 40), Tile.BOSS_FLOOR)
    create_room(Rect(center_x - 30, center_y - 3, 6, 6), Tile.BOSS_FLOOR)
    create_h_tunnel(center_x - 24, center_x - 20, center_y, Tile.BOSS_FLOOR)
    fill_circle(center_x, center_y, 6, Tile.BOSS_CARPET)
    for angle in range(0, 360, 15):
        x = math.cos(math.pi * angle / 180) * 12 + center_x + 0.5
        y = math.sin(math.pi * angle / 180) * 12 + center_y + 0.5
        level[int(x), int(y)] = Tile.mapping[Tile.BOSS_WALL]

    for angle in range(0, 360, 45):
        x = math.cos(math.pi * angle / 180) * 6 + center_x + 0.5
        y = math.sin(math.pi * angle / 180) * 6 + center_y + 0.5
        level[int(x), int(y)] = Tile.mapping[Tile.BOSS_WALL]

    player.x = center_x - 27
    player.y = center_y - 1
    amulet = Actor(center_x + 3, center_y, Graphics.YENDOR_AMULET, 'Amulet of Yendor', rl.YELLOW)

    # need stairs to save game
    stairs = Actor(0, 0, Graphics.ROCK_STAIRS, 'stairs', rl.WHITE, always_visible=True)
    objects = [stairs, amulet, player]

    make_monster('original-body', center_x, center_y)
    make_monster('nuphrindas', center_x - 3, center_y)
    compute_fov()

def make_dungeon_map():
    global level, objects, stairs
 
    # the list of objects with just the player
    objects = [player]
 
    # fill map with "blocked" tiles
    level = Level(MAP_WIDTH, MAP_HEIGHT)
    level.tiles.fill(Tile.ROCK)
    level.blocked.fill(1)
 
    rooms = []
    num_rooms = 0
 
    for r in range(MAX_ROOMS):
        # random width and height
        if num_rooms == 0:
            w = rl.random_int(ROOM_MIN_SIZE - 1, ROOM_MIN_SIZE + 2)
            h = rl.random_int(ROOM_MIN_SIZE - 1, ROOM_MIN_SIZE + 2)
        else:
            w = rl.random_int(ROOM_MIN_SIZE, ROOM_MAX_SIZE)
            h = rl.random_int(ROOM_MIN_SIZE, ROOM_MAX_SIZE)
        # random position without going out of the boundaries of the map
        x = rl.random_int(0, MAP_WIDTH - w - 1)
        y = rl.random_int(0, MAP_HEIGHT - h - 1)
 
        # "Rect" class makes rectangles easier to work with
        new_room = Rect(x, y, w, h)
 
        # run through the other rooms and see if they intersect with this one
        failed = False
        for other_room in rooms:
            if new_room.intersect(other_room):
                failed = True
                break
 
        if not failed:
            # this means there are no intersections, so this room is valid
 
            # "paint" it to the map's tiles
            create_room(new_room)
 
            # center coordinates of new room, will be useful later
            (new_x, new_y) = new_room.center()
 
            if num_rooms == 0:
                # this is the first room, where the player starts at
                player.x = new_x
                player.y = new_y
            else:
                # all rooms after the first:
                # connect it to the previous room with a tunnel
 
                # center coordinates of previous room
                (prev_x, prev_y) = rooms[num_rooms - 1].center()
 
                # draw a coin (random number that is either 0 or 1)
                if rl.random_int(0, 1) == 1:
                    # first move horizontally, then vertically
                    create_h_tunnel(prev_x, new_x, prev_y)
                    create_v_tunnel(prev_y, new_y, new_x)
                else:
                    # first move vertically, then horizontally
                    create_v_tunnel(prev_y, new_y, prev_x)
                    create_h_tunnel(prev_x, new_x, new_y)
 
                # add some contents to this room, such as monsters
                place_objects(new_room)
 
            # finally, append the new room to the list
            rooms.append(new_room)
            num_rooms += 1
 
    if dungeon_level == 3:
        create_river(Tile.WATER)
    elif dungeon_level == 5:
        create_river(Tile.LAVA)

    # create stairs at the center of the last room
    stairs_by_level = [
            None, # level 0
            Graphics.CAVE_STAIRS, # level 1 (no healing)
            Graphics.CAVE_STAIRS, # level 2 (skills)
            Graphics.BLUE_STAIRS, # level 3 (water)
            Graphics.CAVE_STAIRS, # level 4 (invisibles)
            Graphics.RED_STAIRS, # level 5 (lava)
            Graphics.ROCK_STAIRS, # level 6 (necromancers)
            Graphics.ROCK_STAIRS, # level 7 (wizards)
            Graphics.ROCK_STAIRS, # level 8 (boss)
    ]
    stairs = Actor(new_x, new_y, stairs_by_level[dungeon_level], 'stairs', rl.WHITE, always_visible=True)
    objects.append(stairs)
    stairs.z = -1  # so it's drawn below everything else
    compute_fov()

 
def random_choice_index(chances):  # choose one option from list of chances, returning its index
    # the dice will land on some number between 1 and the sum of the chances
    dice = rl.random_int(1, sum(chances))
 
    # go through all chances, keeping the sum so far
    running_sum = 0
    choice = 0
    for w in chances:
        running_sum += w
 
        # see if the dice landed in the part that corresponds to this choice
        if dice <= running_sum:
            return choice
        choice += 1
 
 
def random_choice(chances_dict):
    # choose one option from dictionary of chances, returning its key
    chances = list(chances_dict.values())
    strings = list(chances_dict.keys())
 
    return strings[random_choice_index(chances)]
 
 
def from_dungeon_level(table):
    global dungeon_level
    # returns a value that depends on level. the table specifies what value occurs after each level, default is 0.
    for (value, level) in reversed(table):
        if dungeon_level >= level:
            return value
    return 0
 
def make_monster(choice, x, y):
    global objects
    monster = None
    if choice == 'orc':
        monster = Monster(x, y, Graphics.ORC, 'orc', color=rl.GREEN, max_hp=20, defense=0, power=4, possession_cooldown=35)

    elif choice == 'troll':
        monster = Monster(x, y, Graphics.TROLL, 'troll', rl.GREEN, max_hp=40, defense=0, power=8, possession_cooldown=100, max_mana=3, actions=[Action.FEAR])
    elif choice == 'rat':
        monster = Monster(x, y, Graphics.RAT, 'rat', rl.GREEN, max_hp=4, defense=0, power=1, possession_cooldown=10)
    elif choice == 'bat':
        monster = Monster(x, y, Graphics.BAT, 'bat', rl.GREEN, max_hp=5, defense=0, power=1, possession_cooldown=20, skills=['can_fly'])
    elif choice == 'fire_elemental':
        monster = Monster(x, y, Graphics.FIRE_ELEMENTAL, 'fire elemental', rl.RED, max_hp=25, defense=1, power=4, possession_cooldown=150, max_mana=14, actions=[Action.FIREBALL], skills=['immune_fire'])
    elif choice == 'octopus':
        monster = Monster(x, y, Graphics.OCTOPUS, 'octopus', rl.BLUE, max_hp=20, defense=1, power=4, possession_cooldown=150, max_mana=6, actions=[Action.FREEZE], skills=['can_swim'])
    elif choice == 'eye':
        monster = Monster(x, y, Graphics.EYE, 'eye', rl.GREEN, max_hp=5, defense=1, power=1, possession_cooldown=45, max_mana=8, actions=[Action.TELEPORT, Action.CONFUSE], skills=['see_invisible', 'fly'])
    elif choice == 'ghost':
        monster = Monster(x, y, Graphics.GHOST, 'ghost', rl.LIGHTGRAY, max_hp=20, defense=2, power=4, possession_cooldown=30, skills=['invisible', 'see_invisible'])
    elif choice == 'dwarf':
        monster = Monster(x, y, Graphics.DWARF, 'dwarf', rl.YELLOW, max_hp=30, defense=1, power=4, possession_cooldown=150, max_mana=12, actions=[Action.DIG])
    elif choice == 'skeleton':
        monster = Monster(x, y, Graphics.SKELETON, 'skeleton', rl.WHITE, max_hp=4, defense=0, power=3, possession_cooldown=70)
    elif choice == 'necromancer':
        monster = Monster(x, y, Graphics.NECROMANCER, 'necromancer', rl.WHITE, max_hp=20, defense=2, power=3, possession_cooldown=170, max_mana=60, actions=[Action.SUMMON_SKELETON])
    elif choice == 'wizard':
        monster = Monster(x, y, Graphics.WIZARD, 'wizard', rl.BLUE, max_hp=20, defense=2, power=2, possession_cooldown=270, max_mana=50, actions=[Action.SUMMON_RAT, Action.SUMMON_BAT, Action.LIGHTNING, Action.FIREBALL])
    elif choice == 'original-body':
        monster = Monster(x, y, Graphics.PLAYER, 'original body', rl.YELLOW, max_hp=60, defense=4, power=8, possession_cooldown=3500, max_mana=200, skills=['invisible'], actions=[Action.SUMMON, Action.FEAR, Action.DIG, Action.CONFUSE, Action.TELEPORT, Action.FREEZE, Action.LIGHTNING, Action.FIREBALL], controllers=[DoNothing()])

    elif choice == 'nuphrindas':
        monster = Monster(x, y, Graphics.BOSS, 'nuphrindas', rl.WHITE, max_hp=60, defense=3, power=5, possession_cooldown=3500, max_mana=60, actions=[Action.SUMMON], skills=['boss'])
    elif choice == 'ghoul':
        monster = Monster(x, y, Graphics.GHOUL, 'ghoul', rl.WHITE, max_hp=3, defense=0, power=1, possession_cooldown=10)
    else:
        raise ValueError('unknown choice of monster "%s"' % choice)
    objects.append(monster)
    return monster
 
def get_monster_chances():
    monster_chances = {}
    # monsters: rat bat orc troll
    # forest
    monster_chances[0] = {}
    # level 1: find that there is no healing, try possession
    monster_chances[1] = {'rat': 4, 'orc': 1}
    # level 2: find that you can use others competences
    monster_chances[2] = {'rat': 3, 'orc': 4, 'bat': 1}
    # level 3: water
    monster_chances[3] = {'rat': 1, 'orc': 4, 'bat': 3, 'octopus': 2, 'dwarf': 1}
    # level 4: invisibles
    monster_chances[4] = {'rat': 2, 'ghost': 4, 'eye': 1, 'troll': 1, 'dwarf': 1}
    # level 5: lava
    monster_chances[5] = {'rat': 1, 'bat': 1, 'fire_elemental': 4, 'troll': 2, 'dwarf': 1}
    # level 6: necromancers
    monster_chances[6] = {'rat': 1, 'bat': 1, 'necromancer': 4, 'orc': 1}
    # level 7: wizard
    monster_chances[7] = {'orc': 1, 'wizard': 4, 'troll': 2, 'dwarf': 1}
    # level 8: boss
    monster_chances[8] = {'rat': 5, 'bat': 5, 'orc': 1, 'troll': 1, 'fire_elemental': 1, 'octopus': 1, 'necromancer': 1, 'wizard': 1, 'ghost': 5, 'eye': 1, 'ghoul': 1}
    return monster_chances[dungeon_level]

def place_objects(room):
    # this is where we decide the chance of each monster or item appearing.
 
    # maximum number of monsters per room
    max_monsters = from_dungeon_level([[1, 1], [1, 2], [2, 3], [2, 4], [3, 5], [2, 6], [1, 7], [0, 8]])
    #max_monsters = dungeon_level
 
    # maximum number of items per room
    max_items = from_dungeon_level([[4, 1], [12, 2]])
 
    # chance of each item (by default they have a chance of 0 at level 1, which then goes up)
    item_chances = {}
    #item_chances['heal'] = 35  # healing potion always shows up, even if all other items have 0 chance
    item_chances['lightning'] = from_dungeon_level([[25, 1]])
    item_chances['fireball'] = from_dungeon_level([[25, 1]])
    item_chances['confuse'] = from_dungeon_level([[10, 1]])
    item_chances['sword'] = from_dungeon_level([[5, 1]])
    item_chances['shield'] = from_dungeon_level([[15, 1]])
 
    # choose random number of monsters
    num_monsters = rl.random_int(1, max_monsters)
 
    monster_type = random_choice(get_monster_chances())
    for i in range(num_monsters):
        # choose random spot for this monster
        x = rl.random_int(room.x1 + 1, room.x2 - 1)
        y = rl.random_int(room.y1 + 1, room.y2 - 1)
 
        # only place it if the tile is not blocked
        if not is_blocked(x, y):
            make_monster(monster_type, x, y)
 
 
 
def render_bar(x, y, total_width, name, value, maximum, bar_color, back_color, show_value=True):
    bar_width = int(float(value) / maximum * total_width * 8) if maximum != 0 else 0
 
    rl.fill_rect(x * 8, y * 8, total_width * 8, 7, back_color)
    if bar_width > 0:
        rl.fill_rect(x * 8, y * 8, bar_width, 7, bar_color)
 
    if show_value:
        name += ': ' + str(value) + '/' + str(maximum)
    rl.print_text(8 * x + (8 * total_width // 2), y * 8, name, rl.WHITE, rl.ALIGN_CENTER)
 
 
def compute_fov():
    if level.fov is not None:
	level.fov.free()
    level.fov = level.blocked.field_of_view(player.x, player.y, player.sight_radius, 1, True)
    level.tiles.copy_masked(level.visited, level.fov)

def tile_in_fov(x, y):
    if x >= 0 and x < MAP_WIDTH and y >= 0 and y < MAP_HEIGHT:
        return level.fov[x, y] == 1
    return False
 
def render_all():
    x_offset = SCREEN_WIDTH // 2 - player.x
    y_offset = PANEL_Y // 2 - player.y

    for y in range(MAP_HEIGHT):
        for x in range(MAP_WIDTH):
            screen_x = x + x_offset
            screen_y = y + y_offset
            if screen_x < 0 or screen_x >= SCREEN_WIDTH or screen_y < 0 or screen_y >= SCREEN_HEIGHT:
                continue
            visible = tile_in_fov(x, y)
            if not visible:
                if level.visited[x, y]:
                    level[x, y].draw(screen_x * 8, screen_y * 8)
                    rl.fill_rect(screen_x * 8, screen_y * 8, 8, 8, rl.color(0, 0, 0, 128))
            else:
                level[x, y].draw(screen_x * 8, screen_y * 8)
 
    # draw all objects in the list, except the player. we want it to
    # always appear over all other objects! so it's drawn later.
    objects.sort(key=lambda actor: actor.z)
    for object in objects:
        if object is not player:
            object.draw(x_offset, y_offset)
    # draw hp on top of actors
    for object in objects:
        if isinstance(object, Monster) and object is not player:
            object.draw_hp_bar(x_offset, y_offset)
    player.draw(x_offset, y_offset)
    player.draw_hp_bar(x_offset, y_offset)

    rl.fill_rect(0, PANEL_Y * 8, SCREEN_WIDTH * 8, SCREEN_HEIGHT * 8, rl.BLACK)
    # print the game messages, one line at a time
    y = 1
    for (line, color) in game_msgs:
        rl.print_text(MSG_X * 8, (PANEL_Y + y) * 8, line, color)
        y += 1
 
    # show the player's stats
    if player.alive:
        render_bar(2, 1 + PANEL_Y, BAR_WIDTH, 'HP', player.hp, player.max_hp, rl.RED, rl.BLACK) 
        if player.max_mana > 0:
            render_bar(2, 2 + PANEL_Y, BAR_WIDTH, 'MANA', player.mana, player.max_mana, rl.BLUE, rl.BLACK) 
    else:
        render_bar(2, 1 + PANEL_Y, BAR_WIDTH, 'DEAD', 0, 0, rl.RED, rl.BLACK, show_value=False) 
    rl.draw_tile(0, 5, (PANEL_Y + 1) * 8, player.tile + 16)

    if player.current_possession_cooldown > 0:
        render_bar(0, 0, SCREEN_WIDTH, 'Possession cooldown', player.current_possession_cooldown, player.possession_cooldown, rl.INDIGO, rl.BLACK)
    if dungeon_level == 0:
        rl.print_text(1 * 8, (PANEL_Y + 3) * 8, 'Outside', rl.LIGHTGRAY)
    else:
        rl.print_text(1 * 8, (PANEL_Y + 3) * 8, 'Dungeon level ' + str(dungeon_level), rl.LIGHTGRAY)
 
def wrap_text(text, width):
    output = ['']
    x = 0
    for word in text.replace('\n', ' \n ').split(' '):
        if word == '\n':
            x = 0
            output += ['']
            continue
        if x + rl.size_text(word)[0] > width:
            x = 0
            output += ['']
        output[-1] += word + ' '
        x += rl.size_text(word + ' ')[0]
    return output

last_message = None
def message(new_msg, color=rl.WHITE):
    global last_message
    if new_msg == last_message:
        found = ure.search('\((\d+)\)$', game_msgs[-1][0])
        if found:
            value = int(found.group(1)) + 1
            text = ' '.join(game_msgs[-1][0].split(' ')[:-1]) + ' (%d)' % value
            game_msgs[-1] = (text, game_msgs[-1][1])
            #game_msgs[-1] = (ure.sub('\((\d+)\)$', lambda found: '(%d)' % (int(found.group(1)) + 1), game_msgs[-1][0]), game_msgs[-1][1])
        else:
            game_msgs[-1] = (game_msgs[-1][0].rstrip() + ' (2)', game_msgs[-1][1])
    else:
        last_message = new_msg

        # split the message if necessary, among multiple lines
        new_msg_lines = wrap_text(new_msg, MSG_WIDTH * 8)
     
        for line in new_msg_lines:
            # if the buffer is full, remove the first line to make room for the new one
            if len(game_msgs) == MSG_HEIGHT:
                del game_msgs[0]
     
            # add the new line as a tuple, with the text and the color
            game_msgs.append((line, color))
 
 
def player_move_or_attack(dx, dy):
    # the coordinates the player is moving to/attacking
    x = player.x + dx
    y = player.y + dy
 
    # try to find an attackable object there
    target = None
    for object in objects:
        if isinstance(object, Monster) and object.alive and object.x == x and object.y == y and object.is_hostile(player):
            target = object
            break
 
    # attack if target found, move otherwise
    if target is not None:
        player.attack(target)
    else:
        return player.move(dx, dy)
    return True
 
 
# TODO: navigate menu with arrows, numbers
def menu(header, options, width):
    if len(options) > 26:
        raise ValueError('Cannot have a menu with more than 26 options.')
 
    # calculate total height for the header (after auto-wrap) and one line per option
    header_height = len(header.strip().split('\n')) + 1
    height = len(options) + header_height
    #total_width = max([rl.size(text)[0] for text in [header] + options])
    if len(options) > 0:
        option_width = max([rl.size_text(text)[0] for text in options])
    else:
        option_width = 0
    title_width = rl.size_text(header)[0]
    total_width = max([option_width + 16, title_width])

    rl.fill_rect(8 * SCREEN_WIDTH // 2 - total_width // 2 - 4, 8 * (PANEL_Y // 2 - height // 2) - 4, total_width + 8, 8 * height + 8, rl.color(0, 0, 0, 128))
 
    # print the header, with auto-wrap
    rl.print_text(8 * SCREEN_WIDTH // 2, 8 * (PANEL_Y // 2 - height // 2), header, rl.YELLOW, rl.ALIGN_CENTER)
 
    # print all the options
    y = header_height
    letter_index = ord('a')
    for option_text in options:
        text = '(' + chr(letter_index) + ') ' + option_text
        rl.print_text(8 * SCREEN_WIDTH // 2 - option_width // 2 - 8, 8 * (PANEL_Y // 2 - height // 2) + y * 8, text, rl.WHITE)
        y += 1
        letter_index += 1
 
    rl.present()
    key = rl.wait_key()
 
    if key is None:
        return None
    index = key - rl.A
    if index >= 0 and index < len(options):
        return index
    return None
 

class Action:
    def __init__(self, name=None, cost=0, range=0, effect=None, *args, **kwargs):
        self.name = name
        self.effect = effect
        self.cost = cost
        self.extra_args = args
        self.range = range

        for key, value in kwargs.items():
            setattr(self, key, value)

    def __eq__(self, other):
        return str(self) == str(other)

    def __str__(self):
        return 'Action(%s, %d, %d)' % (self.name, self.cost, self.range)
    
    def __repr__(self):
        return str(self)

    def perform(self, performer, *args, **kwargs):
        args = args + tuple(self.extra_args)
	if performer.mana >= self.cost or self.cost == 0:
	    if self.effect(performer, *args, **kwargs) != 'cancelled':
		performer.mana -= self.cost
                return
	elif performer is player:
	    message('Not enough mana', rl.BLUE)
        return 'cancelled'

def action_menu():
    header = 'Press the key next to an action to\nperform it, or any other key to cancel'
    actions = list(player.actions)
    if debug:
        actions += [Action.DEBUG_SPELL, Action.DEBUG_SUMMON, Action.DEBUG_KILL, Action.DEBUG_LEVEL, Action.DEBUG_ENDING, Action.DEBUG_CLEANSE]
    options = [action.name for action in actions]

    if len(options) == 0:
        index = menu(header, ["Nothing"], ACTION_MENU_WIDTH)
    else:
        index = menu(header, options, ACTION_MENU_WIDTH)
 
    # if an item was chosen, return it
    if index is None or len(options) == 0:
        return 'cancelled'
    rl.clear()
    render_all()
    rl.present()
    return actions[index].perform(player)
 
def inventory_menu(header):
    # show a menu with each item of the inventory as an option
    if len(inventory) == 0:
        options = ['Inventory is empty.']
    else:
        options = []
        for item in inventory:
            text = item.name
            # show additional information, in case it's equipped
            if item.equipment and item.equipment.is_equipped:
                text = text + ' (on ' + item.equipment.slot + ')'
            options.append(text)
 
    index = menu(header, options, INVENTORY_WIDTH)
 
    # if an item was chosen, return it
    if index is None or len(inventory) == 0:
        return None
    return inventory[index].item
 
 
def msgbox(text, width=50):
    menu(text, [], width)  # use menu() as a sort of "message box"
 
 
def get_key_direction(key):
    if key == rl.UP or key == rl.KP_8 or key == rl.K:
        return 0, -1
    elif key == rl.DOWN or key == rl.KP_2 or key == rl.J:
        return 0, 1
    elif key == rl.LEFT or key == rl.KP_4 or key == rl.H:
        return -1, 0
    elif key == rl.RIGHT or key == rl.KP_6 or key == rl.L:
        return 1, 0
    elif key == rl.HOME or key == rl.KP_7 or key == rl.Y:
        return -1, -1
    elif key == rl.PAGEUP or key == rl.KP_9 or key == rl.U:
        return 1, -1
    elif key == rl.END or key == rl.KP_1 or key == rl.B:
        return -1, 1
    elif key == rl.PAGEDOWN or key == rl.KP_3 or key == rl.N:
        return 1, 1
    return 0, 0

def handle_keys(state=None):
    key = rl.wait_event()
 
    if key == rl.ESCAPE:
        return 'exit'  
 
    if game_state == 'playing':
        dx, dy = get_key_direction(key)
        if state == 'frightened':
            message('You flee.', rl.RED);
        elif state == 'confused':
            message('You are confused.', rl.RED);
        elif state == 'frozen':
            message('You are frozen.', rl.RED);
        elif not (dx == 0 and dy == 0):
            if not player_move_or_attack(dx, dy):
                return 'didnt-take-turn'
        elif key == rl.KP_5 or key == rl.SPACE:
            pass  # do nothing ie wait for the monster to come to you
        else:
            if key == rl.A:
                if action_menu() == 'cancelled':
                    return 'didnt-take-turn'
            elif key == rl.T:
                show_help()
                return 'didnt-take-turn'
            elif key == rl.C:
                # show character information
                msgbox('Character Information\n\n' +
                       'Your soul currently inhabits a ' + player.name + '\n'
                       'Maximum HP: ' + str(player.max_hp) +
                       '\nAttack: ' + str(player.power) + '\nDefense: ' + str(player.defense), CHARACTER_SCREEN_WIDTH)
                return 'didnt-take-turn'
 
            #if key_char == '>':
            elif key == rl.P:
                cast_possess(player)
            elif key == rl.GREATER:
                # go down stairs, if the player is on them
                if stairs.x == player.x and stairs.y == player.y:
                    next_level()
                return 'didnt-take-turn'
            else:
                return 'didnt-take-turn'
    return 'played-turn'
 
 
def target_tile(max_range=None):
    if max_range is None:
        max_range = SCREEN_WIDTH // 2
    center_x = SCREEN_WIDTH // 2
    center_y = PANEL_Y // 2
    selected_x = center_x
    selected_y = center_y
    while rl.still_running():
        rl.clear()
        render_all()
        for i in range(center_x - max_range, center_x + max_range):
            for j in range(center_y - max_range, center_y + max_range):
                if i >= 0 and i < SCREEN_WIDTH and j >= 0 and j < PANEL_Y:
                    distance = math.sqrt((i - center_x) ** 2 + (j - center_y) ** 2) 
                    if distance < max_range and not (i == selected_x and j == selected_y):
                        rl.fill_rect(i * 8, j * 8, 8, 8, rl.color(0, 128, 0, 128))
        rl.fill_rect(selected_x * 8, selected_y * 8, 8, 8, rl.color(255, 255, 0, 128))
        rl.present()
        key = rl.wait_key()
        dx = dy = 0
        if key == rl.ESCAPE:
            return (None, None)
        if key == rl.RETURN or key == rl.SPACE:
            break
        dx, dy = get_key_direction(key)
        distance = math.sqrt((selected_x + dx - center_x) ** 2 + (selected_y + dy - center_y) ** 2) 
        if distance < max_range:
            selected_x += dx
            selected_y += dy
        
    x_offset = center_x - player.x
    y_offset = center_y - player.y
    return (-x_offset + selected_x, -y_offset + selected_y)
 
 
def target_monster(max_range=None):
    # returns a clicked monster inside FOV up to a range, or None if right-clicked
    while True:
        (x, y) = target_tile(max_range)
        if x is None:  # player cancelled
            return None
 
        # return the first clicked monster, otherwise continue looping
        for obj in objects:
            if obj.x == x and obj.y == y and obj is not player:
                return obj
        message("That's not a monster.")
 
 
def closest_hostile(monster, max_range):
    # find closest enemy, up to a maximum range, and in the player's FOV
    closest_enemy = None
    closest_dist = max_range + 1  # start with (slightly more than) maximum range
 
    for object in objects:
        if isinstance(object, Monster) and object.alive and object.is_hostile(monster) and monster.can_see(object):
            # calculate distance between this object and the monster
            dist = monster.distance_to(object)
            if dist < closest_dist:  # it's closer, so remember it
                closest_enemy = object
                closest_dist = dist
    return closest_enemy

def visible_monsters(target):
    # find closest enemy, up to a maximum range, and in the player's FOV
    enemies = []
 
    for object in objects:
        if isinstance(object, Monster) and object.alive and object is not target and target.can_see(object):
            enemies.append(object)
    return enemies
 
def friends_and_foes(target):
    monsters = visible_monsters(target)
    friends = []
    foes = []
    for monster in monsters:
        if target.is_hostile(monster):
            foes.append(monster)
        else:
            friends.append(monster)
    return friends, foes

 
def cast_summon(caster, name=None):
    if name is None:
        if caster is player:
            options = ['rat', 'bat', 'orc', 'troll', 'ghost', 'eye', 'fire_elemental', 'octopus', 'necromancer', 'wizard']
            index = menu('What monster do you want to summon?', options, 24)
            if index is None:
                message('No summoning today?')
                return 'cancelled'
            name = options[index]
        else:
            name = random_choice(get_monster_chances())
    x, y = 0, 0
    i = 0
    while i < 100:
        angle = rl.random_int(0, 359)
        distance = rl.random_int(2, 4)
        x = int(caster.x + .5 + math.cos(math.pi * angle / 180) * distance)
        y = int(caster.y + .5 + math.sin(math.pi * angle / 180) * distance)
        if not is_blocked(x, y) and caster.can_see_tile(x, y):
            break
        i += 1
    if i == 100:
        if caster is player:
            message('You fail to cast the summon spell.', rl.BLUE)
        return 'cancelled'
    monster = make_monster(name, x, y)
    monster.master = caster
    if caster is player or player.can_see(caster) or player.can_see(monster):
        message(capitalize(caster.get_name('summon{s}')) + ' ' + monster.get_name(determiner='a') + '.', rl.BLUE)
    return monster

def cast_lightning(caster, target=None):
    # find closest enemy (inside a maximum range) and damage it
    monster = closest_hostile(caster, LIGHTNING_RANGE)
    if monster is None:  # no enemy found within maximum range
        if caster is player:
            message('No enemy is close enough to strike.', rl.RED)
        return 'cancelled'
    if target is not None and target is not monster:
        return 'cancelled'
 
    # zap it!
    message('A lighting bolt strikes ' + monster.get_name() + ' with a loud thunder! The damage is '
            + str(LIGHTNING_DAMAGE) + ' hit points.', rl.BLUE)
    monster.take_damage(LIGHTNING_DAMAGE, player)
 
def cast_fear(caster):
    # find closest enemy (inside a maximum range) and damage it
    monster = closest_hostile(caster, FEAR_RANGE)
    if monster is None:  # no enemy found within maximum range
        if caster is player:
            message('No enemy is close enough to strike.', rl.RED)
        return 'cancelled'
 
    # zap it!
    if caster is player or monster is player:
        message(capitalize(caster.get_name('frighten{s}')) + ' ' + monster.get_name() + '. ' + capitalize(monster.get_name('flee{s}')) + '.', rl.BLUE)
    if type(monster.controller) is FrightenedMonster:
            monster.pop_controller()
    monster.push_controller(FrightenedMonster(caster, FEAR_DURATION))
 
def cast_at_location(text, caster, range, effect, x=None, y=None, any_tile=False):
    if caster is player and x is None:
        message('Select where to cast {name} (ARROWS+RETURN to select; ESCAPE to cancel).'.format(**text), rl.BLUE)
        (x, y) = target_tile(range)
    if x is None:
        return 'cancelled'
    if not any_tile and not caster.can_see_tile(x, y):
        if caster is player:
            message('Invalid target', rl.BLUE)
        return 'cancelled'
    if player.can_see_tile(x, y):
        message(text['result'].format(caster=caster.name), rl.ORANGE)
    effect(caster, x, y)
 
def cast_fireball(caster, x=None, y=None):
    def effect(caster, x, y):
        global objects
        #for obj in objects:  # damage every monster in range, including the player
        #    if isinstance(obj, Monster) and obj.distance(x, y) < FIREBALL_RADIUS and 'immune_fire' not in obj.skills:
        #        message(capitalize(obj.get_name('get{s}')) + ' burned for ' + str(FIREBALL_DAMAGE) + ' hit points.', rl.ORANGE)
        #        obj.take_damage(FIREBALL_DAMAGE, player)
        for i in range(x - FIREBALL_RADIUS, x + FIREBALL_RADIUS + 1):
            for j in range(y - FIREBALL_RADIUS, y + FIREBALL_RADIUS + 1):
                distance = math.sqrt((x - i) ** 2 + (y - j) ** 2)
                if distance < FIREBALL_RADIUS and not is_blocked(i, j):
                    objects.append(Transient(rl.random_int(1, 3), i, j, Graphics.FIREBALL, 'fireball', rl.RED, z=3, blocks=False, controllers=[SpellEffect(fireball_effect)]))
    return cast_at_location({
        'name': 'fireball',
        'result': capitalize(caster.get_name('cast{s}')) + 'a fireball spell. The fireball explodes, burning everything within ' + str(FIREBALL_RADIUS) + ' tiles!',
        }, caster, FIREBALL_RANGE, effect, x, y)
 
def cast_dig(caster, x=None, y=None):
    def effect(caster, x, y):
        dx = x - caster.x
        dy = y - caster.y
        num = 0
        for i, j in line_iter(caster.x, caster.y, caster.x + dx * DIG_RANGE, caster.y + dy * DIG_RANGE):
            if i >= 0 and i < MAP_WIDTH and j >= 0 and j < MAP_HEIGHT:
                if level[i, j].type == 'rock':
                    level[i, j] = Tile.mapping[Tile.FLOOR]
            if num > DIG_RANGE:
                break
            num += 1
        compute_fov()
    return cast_at_location({
        'name': 'dig',
        'result': 'Good diggning makes nice corridors.',
        }, caster, DIG_RANGE, effect, x, y, any_tile=True)

def cast_possess(caster):
    global player
    if caster.current_possession_cooldown > 0:
        if caster is player:
            message('You need to wait for your possess spell to cool down', rl.BLUE)
        return 'cancelled'
    # ask the player for a target to confuse
    in_fov = visible_monsters(caster)

    if len(in_fov) == 0:
        if caster is player:
            message('No monster in sight to possess.', rl.BLUE)
    elif len(in_fov) == 1:
        monster = in_fov[0]
        if monster.name == 'nuphrindas':
            if caster is player:
                message('Nuphrindas is too powerfull to be possessed.', rl.YELLOW)
            return 'cancelled'
        monster.push_controller(caster.pop_controller())
        monster.target = None
        monster.flee = None
        monster.master = None
        caster.push_controller(ParalyzedMonster(3))
        if caster is player:
            if monster.name == 'original body':
                message('You possess your original body. It feels like home.', rl.GREEN)
            else:
                message('You possess ' + monster.get_name(), rl.GREEN)
            player, old_monster = monster, player
            value = rl.random()
            if value < .2:
                old_monster.hp = 0
                old_monster.die()
            elif value < .4:
                old_monster.take_damage(old_monster.hp // 3)
                if caster is old_monster or player.can_see(old_monster):
                    message(capitalize(old_monster.get_name('get{s} hurt and turn{s}')) + ' against ' + monster.get_name() + '.', rl.ORANGE)
            else:
                old_monster.master = player
                if caster is old_monster or player.can_see(old_monster):
                    message(capitalize(old_monster.get_name('{is} subdued and start{s} helping')) + ' ' + monster.get_name() + '.', rl.ORANGE)

            compute_fov()
        monster.current_possession_cooldown = monster.possession_cooldown
    elif caster is player:
        message('You need to be alone with a monster to possess it.', rl.BLUE)
    return 'cancelled'

def cast_at_monster(text, spell_range, immune, effect, caster, target):
    if target is None:
        if caster is player:
            message('Select a monster to {verb} it (ARROWS+RETURN to select; ESCAPE to cancel).'.format(**text), rl.BLUE)
        target = target_monster(spell_range)
        if target is None:
            if caster is player:
                message('Cancelled', rl.BLUE)
            return 'cancelled'
    if player is caster or player.can_see(caster):
        message(capitalize(caster.get_name('cast{s}')) + ' a {spell}'.format(**text))
 
    if len(set(immune) & set(target.skills)) == 0:
        effect(caster, target)
    else:
        if player.can_see(caster) or caster is player:
            message(capitalize(target.get_name('resist{s}')) + '.')
 
def cast_confuse(caster, target=None):
    def effect(caster, target):
        if type(target.controller) is ConfusedMonster:
            target.pop_controller()
        target.push_controller(ConfusedMonster())
        if target is player or player.can_see(target): 
            message(capitalize(target.get_name()) + ' is confused. ' + capitalize(target.get_pronoun('start{s}')) + ' to stumble around!', rl.BLUE)
    return cast_at_monster({
        'verb': 'confuse', 
        'spell': 'spell of confusion', 
        }, CONFUSE_RANGE, ['immune_mind'], effect, caster, target)

def cast_freeze(caster, target=None):
    def effect(caster, target):
        if type(target.controller) is ParalyzedMonster:
            target.pop_controller()
        target.push_controller(ParalyzedMonster(FREEZE_DURATION))
        if target is player or player.can_see(target):
            message(target.get_name('freeze{s}') + '!', rl.BLUE)
    return cast_at_monster({
        'verb': 'feeze', 
        'spell': 'spell of freezing', 
        }, FREEZE_RANGE, ['immune_cold'], effect, caster, target)

def cast_teleport(caster, target=None):
    def effect(caster, target):
        target.x, caster.x = caster.x, target.x
        target.y, caster.y = caster.y, target.y
        if target is player or caster is player or player.can_see(target) or player.can_see(caster):
            message('Teleportation: ' + caster.get_name('exchange{s}') + ' location with ' + target.get_name() + '!', rl.BLUE)
    return cast_at_monster({
        'verb': 'teleport', 
        'spell': 'spell of teleporation', 
        }, TELEPORT_RANGE, ['immune_teleport'], effect, caster, target)

def save_game():
    # open a new empty shelve (possibly overwriting an old one) to write the game data
    print('saving game')
    data = {} 
    data['tiles'] = level.tiles.to_string()
    data['blocked'] = level.blocked.to_string()
    data['visited'] = level.visited.to_string()
    data['objects'] = objects
    data['player'] = player
    data['stairs'] = stairs
    data['game_msgs'] = game_msgs
    data['game_state'] = game_state
    data['dungeon_level'] = dungeon_level
    registry = {}
    rl.save_pref('savegame', ujson.dumps(serialize.encode(data, registry)))
    rl.save_pref('registry', ujson.dumps(registry))
 
 
def load_game():
    global level, objects, player, stairs, game_msgs, game_state, dungeon_level
    print('loading game')
    data = ujson.loads(rl.load_pref('savegame'))
    registry = ujson.loads(rl.load_pref('registry'))
    data = serialize.decode(data, globals(), registry)
    level = Level(MAP_WIDTH, MAP_HEIGHT)
    level.tiles = rl.array_from_string(data['tiles'])
    level.blocked = rl.array_from_string(data['blocked'])
    level.visited = rl.array_from_string(data['visited'])
    objects = data['objects'] 
    player = data['player'] 
    stairs = data['stairs']
    game_msgs = data['game_msgs']
    game_state = data['game_state']
    dungeon_level = data['dungeon_level']

    compute_fov()
 
 
def show_help():
    rl.clear()
    text = '''
Description:
  ExpelledRL is a roguelike built for the 2019 7DRL gamejam. It is set in a traditional RL theme, except for the fact that you can control any monster, through the possess spell. The objective is to recover your original body, and if time permits, the amulet of Yendor.

Controls:
  escape: menu
  h j k l y u b n, arrows or keypad: move
  5 or space: wait
  a: perform an action (such as casting a spell)
  p: cast the possess spell
  >: descent to the next level (on stairs)
  c: see character information
  t: this tutorial
  use arrow keys to select spell targets

                    press a key to continue
    '''
    lines = wrap_text(text, SCREEN_WIDTH * 8 - 64)
    rl.print_text(32, ((SCREEN_HEIGHT - len(lines)) // 2) * 8, '\n'.join(lines), rl.LIGHTGRAY)
    rl.present()
    key = rl.wait_key()

def show_story():
    story= [["You enter the final room of the dungeon, quite confident that all the power you have accumulated in your descent will lead you to victory.\n\nThe amulet of Yendor is right there, standing on an altar on the opposite side of the room. You start to move carefully, wary of any trap that could trigger...", Graphics.PLAYER + 16, Graphics.YENDOR_AMULET, Graphics.BOSS_FLOOR],
            ["Suddenly you see a shadow forming between you and the altar. Nuphrindas, the damned king, is barring the access to the ultimate treasure.\n\nHe mumbles words in an unheard language, and you feel the very life starting to escape your body.\n\nYou try to resist but you are quickly overwhelmed by the unconquerable power of the spell. You feel your soul slowly being expelled from your body. Your vision blurs and you lose consciousness...", Graphics.BOSS, Graphics.PLAYER, Graphics.BOSS_FLOOR],
            ["You wake up in the forest, just outside the dungeon entrance. Your sensations are strange, sluggish, inappropriate.\n\nYou are in complete dismay when you realize that the body you are incarnating is not yours, but rather that of an ugly, feeble creature. Yet, you feel that you have a special connection to the living and the animate...", Graphics.GHOUL + 16, Graphics.TREE, Graphics.GRASS]
            ]

    for text, tile1, tile2, bg in story:
        rl.clear()
        lines = wrap_text(text, SCREEN_WIDTH * 8 - 64)
        rl.print_text(32, ((SCREEN_HEIGHT - len(lines)) // 2) * 8, '\n'.join(lines), rl.LIGHTGRAY)
        x = 8 * SCREEN_WIDTH // 2
        y = (1 + (SCREEN_HEIGHT + len(lines)) // 2) * 8
        for i in range(4):
            rl.draw_tile(0, x - 12 + i * 8, y + 2, bg)
        rl.draw_tile(0, x - 8, y, tile1)
        rl.draw_tile(0, x + 8, y, tile2)
        rl.present()
        key = rl.wait_key()
        if not rl.still_running() or key == rl.ESCAPE:
            break

    #message('ExpelledRL', rl.YELLOW)

    message('Putting yourself together, you decide to go back down the dungeon and settle this mess.', rl.LIGHTGRAY)
    message('Press t for a tutorial. Press p to possess another monster.', rl.GREEN)
 
def show_ending():
    global game_state
    holding = True
    tile1 = player.tile + 16
    tile2 = Graphics.YENDOR_AMULET
    bg = Graphics.GRASS
    text = 'The hideous creature breaths its last non-breath, and falls to the ground. Now that Nuphrindas is no more, your revenge is complete.\n\n'
    if player.name == 'original body':
        text += '''You have finally recovered your adored body. You feel all skilled, and powerful. You raise your hand towards the amulet of Yendor, imagining the fame that you will soon enjoy. Your hand stops as you are having second thoughts. Possessing all those bodies has changed your mind, and you are not sure you want the life of a hero. You turn back and leave the prized amulet for generations to come...'''
        tile2 = None
    elif player.name == 'orc':
        text += '''You grunt in victory. Your journey within the orc tribe has made you more aware of the fraternity that those creatures exhibit to each other. As part of the big orc family you will be able to enjoy large feasts, and adventurer squishing parties. This really warms your heart.'''
        tile2 = Graphics.ORC
        bg = Graphics.CAVE_FLOOR
        holding = False
    elif player.name == 'rat':
        text += '''You are a rat. Not the kind of rat that stumbles giant organizations, but the kind of rat that likes to eat cheese. Maybe the weakest are stronger than they seem. Numbers are what make feeble creatures standout. And your plan to conquer the world is to have a chat with your peers from the pantry. Together, you can do crazy things, and the amulet will be a great asset.'''
        bg = Graphics.BOSS_FLOOR
        holding = False
    elif player.name == 'bat':
        text += 'You echo-locate your prey on the floor. It will no-longer move. You wonder whether you would like the taste of blood. Maybe one day you will become a vampire. You grab the amulet and fly towards a dark cave where you will rest and prepare new adventures.'
        bg = Graphics.CAVE_FLOOR
        holding = False
    elif player.name == 'octopus':
        text += 'You grab the amulet with one of your tentacles. Touching its surface makes ripples of colored stripes crawl your tentacle towards your body as it touches your brains. You feel its power surge through you and a range of powers suddenly be available. You fire one of them and the dungeon is suddenly flowed in water, decorated with algae and filled with fish of various colors. What could be interpreted as a smile draw on your face...'
        bg = Graphics.WATER
    elif player.name == 'skeleton':
        text += 'You feel the air flowing between your bones as the power of the old king flows through the room, towards mysterious eons. Magic slowly dissipates from the room... After grabbing the amulet, you stride towards the exit. Your joints start to feel unfit, and you fall on a knee while the rest of your leg crumbles. The energy that was holding you together is running away and your last though is that you should not have killed that necromancer.'
        tile1 = Graphics.CORPSE + 16
        bg = Graphics.BOSS_FLOOR
        holding = False
    elif player.name == 'ghost':
        text += 'As a ghost, your interactions with the real world are rather abstract. Holding the amulet is something that you can only contemplate. So you decide to just gaze at it for the eternity to come.'
        bg = Graphics.BOSS_FLOOR
        holding = False
    elif player.name == 'ghoul':
        text += 'You have defeated the old king as the feeble creature he though would never be able to reach him. You contemplate the irony of the situation and fail to reckon the large stone falling above you. The distinctive splat noise of your head exploding is the last sound you hear.'
        tile1 = Graphics.BOULDER_SPLAT
        bg = Graphics.BOSS_FLOOR
        holding = False
    elif player.name == 'necromancer':
        text += '''Death is crawling under your fingers. You feel it twirling in the room, taking its toll and leaving the area for all it has been busy in the dungeon. Necromancy is a discipline, a science, no, an art which you no longer feel like leaving. Now that the room is all quiet, the idea grows in you to finally meet your destiny. You take Nuphrindas scepter from the ground and raise it to mark the eons with your new role in a new home. Finally the dungeon has a leader who will make terror reign as it should always have been.'''
        bg = Graphics.BOSS_FLOOR
    elif player.name == 'troll':
        text += '''Trolls are strange creatures. At first they seem brutal, selfless and greedy. Yet, you realize that troll legends are well overstated and that trolls are actually very nice and sensitive creatures. While incarnating one, you have been given to meet a few which were actually kind of interesting beings and you can't wait to deepen your relationship with them. A lot of fun awaits.'''
        tile2 = Graphics.TROLL
        holding = False
    elif player.name == 'wizard':
        text += '''All your life, you have been seeking knowledge and wisdom. Now, you have finally found it and your new wizard life will be full of spells and magic. If only you had more mana... Maybe the amulet provides that kind of power. You reach for it with the satisfaction that this time nothing will prevent you from succeeding your quest.'''
    elif player.name == 'eye':
        text += '''Seeing it all is what you prefer. Your latest adventures have opened your eyes to a new world of sightedness. Clearly, your power is way beyond those creatures blind to the real world. Can finally see the meaning of life. You can see it all, way above anything you could imagine, in its beauty and creepiness. As they say, seeing is believing.'''
        holding = False
    elif player.name == 'fire elemental':
        text += '''You take the amulet, and suddenly it catches fire. You realize that you should have been a tiny bit more careful and that it is now destroyed forever. Beyond all the magic that makes you, sorrow takes over and you start crying. Tears of lava.'''
        tile2 = Graphics.FIREBALL
        bg = Graphics.BOSS_FLOOR
    else:
        text += '''Your body was never worth it. You abandon it and take the amulet. Its immense power surges through you and you finally get to know that subtle feeling of accomplishment. '''
    rl.clear()
    lines = wrap_text(text, SCREEN_WIDTH * 8 - 64)
    rl.print_text(32, ((SCREEN_HEIGHT - len(lines)) // 2) * 8, '\n'.join(lines), rl.LIGHTGRAY)
    x = 8 * SCREEN_WIDTH // 2
    y = (1 + (SCREEN_HEIGHT + len(lines)) // 2) * 8
    for i in range(4):
        rl.draw_tile(0, x - 12 + i * 8, y + 2, bg)
    if holding:
        rl.draw_tile(0, x - 2, y, tile1)
        if tile2 is not None:
            rl.draw_tile(0, x + 2, y - 4, tile2)
    else:
        rl.draw_tile(0, x - 8, y, tile1)
        rl.draw_tile(0, x + 8, y, tile2)
    rl.present()
    rl.wait_key()
    message('Congratulations, you have finished ExpelledRL. Game over.', rl.PINK)
    game_state = 'game-over'

def new_game(no_story=False):
    global player, inventory, game_msgs, game_state, dungeon_level, objects

    game_msgs = []

    # create object representing the player
    objects = []
    player = make_monster('ghoul', 0, 0)
    player.push_controller(BasicMonster())
    player.push_controller(PlayerInput())
 
    # generate map (at this point it's not drawn to the screen)
    if no_story:
        dungeon_level = 1
        make_dungeon_map()
    else:
        dungeon_level = 0
        make_forest_map()
    #make_boss_map()
 
    game_state = 'playing'
    inventory = []
 
    # create the list of game messages and their colors, starts empty
    game_msgs = []
 
    if not no_story:
        show_story()
    else:
        message('You enter the dungeon to recover your body and, if time permits, get the amulet.', rl.BLUE)
 
 
def next_level():
    # advance to the next level
    global dungeon_level
 
    level_message = [
            '',
            'You enter the dungeon. Strangely, it feels good to be back...',
            'You descent to the next level. You hear wings flapping in the distance...',
            'You descent to the next level. You hear the sound of a river flowing...',
            'You descent to the next level. Your vision is playing tricks on you...',
            'You descent to the next level. The atmosphere is pretty hot here...',
            'You descent to the next level. The atmosphere is putrid...',
            'You descent to the next level. The air is charged with magic...',
            'You descent to the next level. You feel the presence of a powerful entity...',
    ]

    dungeon_level += 1
    message(level_message[dungeon_level], rl.BLUE)
    if dungeon_level < 8:
        make_dungeon_map()  # create a fresh new level!
    else:
        make_boss_map()
 
 
def play_game():
    while rl.still_running():
        rl.clear()
        render_all()
        rl.present()
 
        if game_state == 'playing':
            player_action = player.take_turn()
            if player_action == 'didnt-take-turn':
                continue
            elif player_action == 'exit':
                save_game()
                break
            elif game_state == 'playing':
                for object in objects:
                    if isinstance(object, Monster) and object is not player:
                        object.take_turn()
                    if game_state != 'playing':
                        break
        else:
            if handle_keys() == 'exit':
                break
 
 
def main_menu():
    try:
        load_game()
    except Exception as e:
        sys.print_exception(e)
        new_game()
    if game_state == 'dead':
        new_game()
    play_game()
    while rl.still_running():
        rl.clear()
        rl.print_text(8 * SCREEN_WIDTH // 2, 8 * (SCREEN_HEIGHT - 2), 'By Benob (version 7DRL 2019-04-20)\nhttps://benob.itch.io/expelledrl', rl.YELLOW, rl.ALIGN_CENTER)
 
        # show options and wait for the player's choice
        choice = menu('ExpelledRL', ['Continue', 'New game', 'New game (no story)', 'Save & quit'], 24)
 
        if choice == 1:  # new game
            new_game()
            play_game()
        if choice == 2:  # new game
            new_game(no_story=True)
            play_game()
        elif choice == 3:  # quit
            break
        else:
            play_game()
 
 
Action.LIGHTNING = Action('Create lightning', 7, LIGHTNING_RANGE, cast_lightning)
Action.FEAR = Action('Induce fear', 3, FEAR_RANGE, cast_fear)
Action.FIREBALL = Action('Launch fireball', 12, FIREBALL_RANGE, cast_fireball)
Action.CONFUSE = Action('Confuse other', 2, CONFUSE_RANGE, cast_confuse)
Action.FREEZE = Action('Freeze other', 6, FREEZE_RANGE, cast_freeze)
Action.TELEPORT = Action('Teleport', 2, TELEPORT_RANGE, cast_teleport)
#Action.POSSESS = Action('Cast possess', 0, cast_possess)
Action.DIG = Action('Dig', 4, NO_RANGE, cast_dig)
Action.SUMMON = Action('Summon monster', 30, NO_RANGE, cast_summon)
Action.SUMMON_BAT = Action('Summon bat', 10, NO_RANGE, cast_summon, 'bat')
Action.SUMMON_EYE = Action('Summon eye', 10, NO_RANGE, cast_summon, 'eye')
Action.SUMMON_RAT = Action('Summon rat', 10, NO_RANGE, cast_summon, 'rat')
Action.SUMMON_SKELETON = Action('Summon skeleton', 30, NO_RANGE, cast_summon, 'skeleton')

def debug_any_spell(caster):
    actions = [Action.LIGHTNING, Action.FEAR, Action.FIREBALL, Action.CONFUSE, Action.FREEZE, Action.TELEPORT, Action.DIG, Action.SUMMON]
    options = [action.name for action in actions]
    index = menu('Which spell?', options, ACTION_MENU_WIDTH)
    if index is not None:
        rl.clear()
        render_all()
        rl.present()
        actions[index].effect(player)

def debug_summon_hostile(caster):
    monster = cast_summon(caster)
    if monster:
        monster.master = None

def debug_kill_monster(caster):
    monster = target_monster(player.sight_radius)
    print('KILL', monster)
    if monster:
        monster.take_damage(1000)

def debug_choose_level(caster):
    global dungeon_level
    options = ['Level %d' % i for i in range(1, 9)]
    index = menu('Choose level', options, ACTION_MENU_WIDTH)
    if index is not None:
        dungeon_level = index 
        next_level()

def debug_see_ending(caster):
    global player
    options = ['rat', 'bat', 'orc', 'troll', 'ghost', 'eye', 'fire_elemental', 'octopus', 'necromancer', 'wizard', 'original-body', 'ghoul']
    index = menu('Choose character', options, ACTION_MENU_WIDTH)
    if index is not None:
        player = make_monster(options[index], 0, 0)
        show_ending()

def debug_cleanse_self(caster):
    caster.current_possession_cooldown = 0
    caster.hp = caster.max_hp
    caster.mana = caster.max_mana
 
Action.DEBUG_SUMMON = Action('Summon hostile', 0, NO_RANGE, effect=debug_summon_hostile)
Action.DEBUG_LEVEL = Action('Choose level', 0, NO_RANGE, effect=debug_choose_level)
Action.DEBUG_ENDING = Action('See ending', 0, NO_RANGE, effect=debug_see_ending)
Action.DEBUG_SPELL = Action('Cast spell', 0, NO_RANGE, effect=debug_any_spell)
Action.DEBUG_KILL = Action('Kill monster', 0, NO_RANGE, effect=debug_kill_monster)
Action.DEBUG_CLEANSE = Action('Drink a refresher', 0, NO_RANGE, effect=debug_cleanse_self)

main_menu()
