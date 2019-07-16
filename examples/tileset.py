import rl

TILE_WIDTH = 16
TILE_HEIGHT = 16
WIDTH = 50
HEIGHT = 25

rl.init_display('Tileset', WIDTH * TILE_WIDTH, HEIGHT * TILE_HEIGHT)
tileset = rl.Image('data/kenney_1bit_16x16.png', TILE_WIDTH, TILE_HEIGHT)

level = rl.Array(WIDTH, HEIGHT)
fg = [rl.random_color() for i in range(256)]
bg = [rl.random_color() for i in range(256)]

def update(event):
    global level

    # Fill array with random tiles (0..1023), random foregroud and background colors (0..255)
    # 1) fill with completely random bits
    # 2) make sure only valid bits remain (0xff000000 for background, 0xff0000 for foreground and 0x3ff for tiles)
    #
    # Equivalent loop:
    # for j in range(a.height()):
    #    for i in range(a.width()):
    #        level[i, j] = rl.pack_tile(rl.random_int(0, 1023), rl.random_int(0, 255), rl.random_int(0, 255))
    #
    level.random()
    level &= 0xffff03ff

    rl.clear()
    rl.draw_array(level, 0, 0, image=tileset, fg_palette=fg, bg_palette=bg, packed=True)

rl.run(update, rl.ON_KEY)
