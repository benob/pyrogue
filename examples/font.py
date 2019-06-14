import rl

font1 = rl.font('data/font.ttf', 8)
font2 = rl.font('data/font.ttf', 16)
font3 = rl.font('data/font.ttf', 32)
font4 = rl.image('data/cp437.png', 9, 16)
font5 = rl.image('data/ascii_8x8.png', 8, 8)

def update(event):
    y = 0
    rl.draw_text(font1, 0, 0, 'hello', rl.INDIGO)
    y += int(font1.line_height)
    rl.draw_text(font2, 0, y, 'world', rl.BLUE)
    y += int(font2.line_height)
    rl.draw_text(font3, 0, y, 'from', rl.GREEN)
    y += int(font3.line_height)
    rl.draw_text(font2, 0, y, 'pyrogue', rl.YELLOW)
    y += int(font2.line_height)
    rl.draw_text(font1, 0, y, 'the', rl.ORANGE)
    y += int(font1.line_height)
    rl.draw_text(font4, 0, y, 'roguelike', rl.RED)
    y += int(font4.tile_height)
    rl.draw_text(font5, 0, y, 'library', rl.PINK)

rl.run(update)
