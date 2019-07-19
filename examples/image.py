import rl

rl.init_display("Image", 512, 512)
tileset = rl.Image("data/kenney_1bit_16x16.png")
tileset.tile_width = 16
tileset.tile_height = 16

def update(event):
    rl.draw_image(tileset, 0, 0)
    for j in range(32):
        for i in range(32):
            if j > 16:
                if i < 16:
                    rl.draw_tile(tileset, i * 16, j * 16, i + j * 32, rl.random_color())
                else:
                    rl.draw_tile(tileset, i * 16, j * 16, i + j * 32, rl.random_color(), rl.random_color())
            elif i > 16:
                rl.draw_tile(tileset, i * 16, j * 16, i + j * 32, rl.BLACK, rl.random_color())

rl.run(update, rl.ON_KEY)
