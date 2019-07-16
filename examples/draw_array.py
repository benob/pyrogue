import rl

rl.init_display('draw_array example', 9 * 80, 16 * 25)

font = rl.Image('data/cp437.png', 9, 16)

fg_palette = [rl.color(255, i, 0) for i in range(256)]
bg_palette = [rl.color(0, 255 - i, i) for i in range(256)]
tile_map = [i % 26 + 65 for i in range(256)]

def update(event):
    rl.clear()

    width = 9 * 9
    height = 9 * 16

    console = rl.Array(8, 8)
    console.random_int(0, 255)

    # specify tiles
    rl.draw_array(console, width * 0, 0, image=font)

    # specify tiles with mapping
    rl.draw_array(console, width * 0, height, image=font, tile_map=tile_map)

    # specify fg color
    rl.draw_array(console, width * 1, 0, image=font, fg_palette=fg_palette)

    # specify bg color
    rl.draw_array(console, width * 2, 0, image=font, bg_palette=bg_palette)

    # both fg & bg color
    rl.draw_array(console, width * 3, 0, image=font, fg_palette=fg_palette, bg_palette=bg_palette)

    # now with tile-specific colors
    console_packed = rl.Array(8, 8)
    for j in range(8):
        for i in range(8):
            console_packed[i, j] = rl.pack_tile(console[i, j], i * 32, j * 32)

    # specify fg color
    rl.draw_array(console_packed, width * 1, height, image=font, fg_palette=fg_palette, packed=True)

    # specify bg color
    rl.draw_array(console_packed, width * 2, height, image=font, bg_palette=bg_palette, packed=True)

    # both fg & bg color
    rl.draw_array(console_packed, width * 3, height, image=font, fg_palette=fg_palette, bg_palette=bg_palette, packed=True)

rl.run(update, rl.ON_KEY)
