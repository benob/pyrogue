import rl

cell_width = cell_height = 16
level = rl.Array(20, 15)
level.draw_line(10, 3, 10, 12)
level.draw_line(3, 8, 17, 8)

dest_x, dest_y = 5, 6
start_x, start_y = 12, 12

font = rl.Font('data/font.ttf', 8)

def update(event):
    global start_x, start_y
    if event == rl.MOUSE_DOWN:
        x, y, button = rl.mouse()
        start_x = x // cell_width
        start_y = y // cell_height

    path = level.shortest_path(start_x, start_y, dest_x, dest_y)

    rl.clear()
    for i, j in path:
        rl.fill_rect(i * cell_width, j * cell_height, cell_width, cell_height, rl.GREEN)

    for j in range(level.height()):
        for i in range(level.width()):
            if level[i, j] == 1:
                rl.fill_rect(i * cell_width, j * cell_height, cell_width, cell_height, rl.WHITE)

    rl.fill_rect(start_x * cell_width, start_y * cell_height, cell_width, cell_height, rl.BLUE)
    rl.fill_rect(dest_x * cell_width, dest_y * cell_height, cell_width, cell_height, rl.RED)

    for j in range(level.height()):
        rl.draw_line(0, j * cell_height, 320, j * cell_height)
    for i in range(level.width()):
        rl.draw_line(i * cell_width, 0, i * cell_width, 240)

rl.run(update, rl.ON_MOUSE)


