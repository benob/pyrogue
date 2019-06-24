import rl

rl.init_display('Mouse', 320, 240)
rl.fill_rect(0, 0, 320, 240, rl.BLUE)

prev_x, prev_y = None, None

def update(event):
    global prev_x, prev_y
    if event == rl.MOUSE:
        x, y, button = rl.mouse()
        if button != 0:
            if prev_x is None or prev_y is None:
                prev_x, prev_y = x, y
            rl.draw_line(prev_x, prev_y, x, y, rl.WHITE)
            prev_x, prev_y = x, y
        else:
            prev_x = prev_y = None

rl.run(update, rl.ON_MOUSE)
