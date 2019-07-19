import rl

cell_width = cell_height = 16
level = rl.Array(320 // cell_width, 240 // cell_height)

palette = [rl.color(255 - i, 0, 0) for i in range(256)]

level.fill(rl.INT_MAX)
level.draw_line(3, 4, 10, 4, -1)
level.draw_line(10, 4, 10, 15, -1)
level[3, 7] = 0
level.dijkstra()

font = rl.Font('data/font.ttf', 8)

def update(event):
    global level
    if event == rl.MOUSE_DOWN:
        x, y, button = rl.mouse()
        if button:
            x //= cell_width
            y //= cell_height
            if x >= 0 and x < level.width() and y >=0 and y < level.height():
                if button & rl.MOUSE_LEFT:
                    level[x, y] = -1
                else:
                    level[x, y] = 0
                level = level.equals(-1)
                level.replace(1, -1)
                level.replace(0, rl.INT_MAX)
                level[3, 7] = 0
                level.dijkstra()
    value_max = level.max()
    for j in range(level.height()):
        for i in range(level.width()):
            value = level[i, j] * 255 // value_max
            if value >= 0 and value < len(palette):
                rl.fill_rect(i * cell_width, j * cell_height, cell_width, cell_height, palette[value])
                rl.draw_text(font, i * cell_width + cell_width // 2, j * cell_height + cell_height // 2, str(level[i, j]), align=rl.ALIGN_CENTER|rl.ALIGN_MIDDLE)
            elif level[i, j] == -1:
                rl.fill_rect(i * cell_width, j * cell_height, cell_width, cell_height, rl.WHITE)
            else:
                rl.fill_rect(i * cell_width, j * cell_height, cell_width, cell_height, rl.BLUE)
        rl.draw_line(0, j * cell_height, 320, j * cell_height)
    for i in range(level.width()):
        rl.draw_line(i * cell_width, 0, i * cell_width, 240)

rl.run(update, rl.ON_MOUSE)


