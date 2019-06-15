import rl

rl.init_display("Text example", 320, 240)
font = rl.Font('data/font.ttf', 8)

def update(event):
    rl.draw_text(font, 0, 0, 'Hello', rl.BLUE)
    rl.draw_text(font, 10, 10, 'world', rl.RED)
    rl.draw_text(font, 20, 20, 'from', rl.PINK)
    rl.draw_text(font, 30, 30, 'pyrogue', rl.GREEN)
    rl.draw_text(font, 40, 40, '!!!', rl.YELLOW)

rl.run(update, rl.UPDATE_KEY)
