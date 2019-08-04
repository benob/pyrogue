import rl

class Room:
    def __init__(self, x, y, width, height):
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.center_x = self.x + self.width // 2
        self.center_y = self.y + self.height // 2

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

    def distance(self, other):
        return abs(self.center_x - other.center_x) + abs(self.center_y - other.center_y)


def generate():
    level = rl.Array(320, 240)
    rooms = []
    num_fails = 0
    while num_fails < 1000:
        room = Room(rl.random_int(1, 320 - 16), rl.random_int(1, 240 - 16), rl.random_int(5, 15), rl.random_int(5, 15))
        for other in rooms:
            if other.intersect(room):
                room = None
                num_fails += 1
                break
        if room is not None:
            rooms.append(room)
            level.fill_rect(room.x, room.y, room.width, room.height)
    
    def make_key(i, j):
        return (i, j) if i < j else (j, i)
    linked = set()
    for i, room in enumerate(rooms):
        closest = [(0, None) for i in range(4)]
        best_distance = [0, 0, 0, 0]
        for j, other in enumerate(rooms):
            if room != other:
                distance = room.distance(other)
                if room.center_x + room.center_y < other.center_x + other.center_y :
                    if room.center_y - room.center_x < other.center_y - other.center_x:
                        if closest[0][1] is None or distance < best_distance[0]:
                            closest[0] = (make_key(i, j), other)
                            best_distance[0] = distance
                    else:
                        if closest[1][1] is None or distance < best_distance[1]:
                            closest[1] = (make_key(i, j), other)
                            best_distance[1] = distance
                else:
                    if room.center_y - room.center_x < other.center_y - other.center_x:
                        if closest[2][1] is None or distance < best_distance[2]:
                            closest[2] = (make_key(i, j), other)
                            best_distance[2] = distance
                    else:
                        if closest[3][1] is None or distance < best_distance[3]:
                            closest[3] = (make_key(i, j), other)
                            best_distance[3] = distance
        for key, other in closest:
            if other is not None and key not in linked:
                level.draw_line(room.center_x, room.center_y, room.center_x, other.center_y)
                level.draw_line(room.center_x, other.center_y, other.center_x, other.center_y)
                linked.add(key)

    return level

def update(event):
    level = generate()
    rl.draw_image(rl.array_to_image(level, palette=[rl.BLACK, rl.WHITE]), 0, 0)

rl.run(update, rl.ON_KEY)
