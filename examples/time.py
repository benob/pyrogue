import rl

rl.init_display('Time example', 320, 240)

class Action:
    def __init__(self, actor, cost):
        self.actor = actor
        self.cost = cost

    def can_perform(self):
        return self.actor.energy >= self.cost

    def perform(self):
        return self.actor.consume_energy(self.cost)


class Move(Action):
    def __init__(self, actor, dx, dy):
        super().__init__(actor, 10)
        self.dx = dx
        self.dy = dy

    def perform(self):
        if super().perform():
            x, y = self.actor.x + self.dx, self.actor.y + self.dy
            if x >= 0 and y >= 0 and x < 320 and y < 240:
                self.actor.locations.append((self.actor.x, self.actor.y))
                self.actor.x, self.actor.y = x, y
                return True
        return False


class Controller:
    def control(self):
        raise NotImplemented


class PlayerController:
    keys = []

    @classmethod
    def add_key(cls, key):
        cls.keys.append(key)

    @classmethod
    def get_key(cls):
        if len(cls.keys) > 0:
            return cls.keys.pop(0)

    def control(self, actor):
        key = PlayerController.get_key()
        dx = dy = None
        if key == rl.LEFT:
            dx, dy = -1, 0
        elif key == rl.RIGHT:
            dx, dy = 1, 0
        elif key == rl.UP:
            dx, dy = 0, -1
        elif key == rl.DOWN:
            dx, dy = 0, 1
        if dx is not None:
            actor.set_action(Move(actor, 8 * dx, 8 * dy))


class MonsterController:
    def control(self, actor):
        dx, dy = rl.random_int(-1, 1), rl.random_int(-1, 1)
        actor.set_action(Move(actor, 8 * dx, 8 * dy))


class Actor:
    def __init__(self, name, color, speed, controller):
        self.action = None
        self.energy = 0
        self.locations = []
        self.x = rl.random_int(0, 319) & ~7
        self.y = rl.random_int(0, 239) & ~7
        self.name = name
        self.color = color
        self.controller = controller
        self.speed = speed

    def draw(self):
        locations = self.locations + [(self.x, self.y)]
        for (x1, y1), (x2, y2) in [locations[i: i + 2] for i in range(len(locations) - 1)]:
            rl.draw_line(x1 + 4, y1 + 4, x2 + 4, y2 + 4, self.color)
        rl.fill_rect(self.x, self.y, 7, 7, self.color)

    def set_action(self, action):
        self.action = action

    def get_action(self):
        self.controller.control(self)
        action = self.action
        self.action = None
        return action

    def add_energy(self, amount):
        self.energy += amount

    def consume_energy(self, amount):
        if self.energy < amount:
            return False
        self.energy -= amount
        return True


class Game:
    def __init__(self):
        self.actors = []
        self.current = 0

    def process(self):
        for actor in self.actors:
            actor.add_energy(actor.speed)
        while len(self.actors) > 0:
            actor = self.actors[self.current % len(self.actors)]
            action = actor.get_action()
            while isinstance(action, Action) and action.can_perform():
                action = action.perform()
            if action:
                self.current += 1
            elif actor.name == 'player':
                return

    def add_actor(self, actor):
        self.actors.append(actor)

game = Game()
for i in range(10):
    speed = rl.random_int(1, 16)
    game.add_actor(Actor('monster', rl.color(127 + speed * 8, 0, 0), speed, MonsterController()))
game.add_actor(Actor('player', rl.BLUE, 10, PlayerController()))

def update(key):
    if key == rl.ESCAPE:
        rl.quit()
    elif key > 0:
        PlayerController.add_key(key)
    game.process()
    rl.clear()
    for actor in game.actors:
        actor.draw()

rl.run(update, rl.UPDATE_KEY)

