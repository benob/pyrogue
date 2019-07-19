import rl

rl.init_display('Mouse', 320, 240)
rl.fill_rect(0, 0, 320, 240, rl.BLUE)

pressed = False
previous = (0, 0)

def update(event):
    global pressed, previous
    x, y, button = rl.mouse()
    if event == rl.MOUSE_DOWN:
        pressed = True
        previous = x, y
    elif event == rl.MOUSE_UP:
        pressed = False
    elif pressed:
        rl.draw_line(previous[0], previous[1], x, y, rl.WHITE)
        previous = x, y

rl.run(update, rl.ON_MOUSE)
