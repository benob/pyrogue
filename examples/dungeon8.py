import rl

class Room:
    def __init__(self, x, y, width, height):
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.center_x = self.x + self.width // 2
        self.center_y = self.y + self.height // 2
        self.neighbors = []

    def intersect(self, other):
        if self.x < other.x:
            if self.x + self.width < other.x:
                return False
        else:
            if other.x + other.width < self.x:
                return False
        if self.y < other.y:
            if self.y + self.height < other.y:
                return False
        else:
            if other.y + other.height < self.y:
                return False
        return True



def generate():
    level = rl.Array(320, 240)
    rooms = []
    rooms.append(Room(160 - 3, 120 - 3, 6, 6))
    num_fails = 0
    while num_fails < 1000:
        origin = rl.random_choice(rooms)
        width = rl.random_int(5, 15)
        height = rl.random_int(5, 15)
        x, y = rl.random_choice([(origin.x - width - 1, origin.center_y - height // 2), (origin.x + origin.width + 1, origin.center_y - height // 2), (origin.center_x - width // 2, origin.y - height - 1), (origin.center_x - width // 2, origin.y + origin.height + 1)])
        room = Room(x, y, width, height)
        for other in rooms:
            if other.intersect(room):
                room = None
                num_fails += 1
                break
        if room is not None:
            rooms.append(room)
            origin.neighbors.append(room)
            level.draw_line(room.center_x, room.center_y, origin.center_x, origin.center_y)

    for room in rooms:
        if len(room.neighbors) == 0 or rl.random() < .5:
            level.fill_rect(room.x, room.y, room.width, room.height)
    
    return level

def update(event):
    level = generate()
    rl.draw_image(rl.array_to_image(level, palette=[rl.BLACK, rl.WHITE]), 0, 0)

rl.run(update, rl.ON_KEY)
