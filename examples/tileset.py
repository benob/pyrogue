import rl

TILE_WIDTH = 16
TILE_HEIGHT = 16
WIDTH = 24
HEIGHT = 15

rl.init_display('Tileset', WIDTH * TILE_WIDTH, HEIGHT * TILE_HEIGHT)
tileset = rl.image('data/kenney_1bit_16x16.png', TILE_WIDTH, TILE_HEIGHT)

def update(event):
    rl.clear()
    a = rl.array(WIDTH, HEIGHT)
    a.random_int(0, 1023)
    fg = [rl.color(rl.random_int(0, 255), rl.random_int(0, 255), rl.random_int(0, 255)) for i in range(1024)]
    bg = [rl.color(rl.random_int(0, 255), rl.random_int(0, 255), rl.random_int(0, 255)) for i in range(1024)]
    rl.draw_array(a, tileset, 0, 0, fg=fg, bg=bg)

rl.run(update, rl.UPDATE_KEY)
