import rl

rl.init_display("Image", 512, 512)
tileset = rl.Image("data/kenney_1bit_16x16.png")
tileset.tile_width = 16
tileset.tile_height = 16
print(tileset.tile_width, tileset.tile_height)

def update(event):
    rl.draw_image(tileset, 0, 0)
    for i in range(32):
        for j in range(16, 32):
            rl.draw_tile(tileset, i * 16, j * 16, i + j * 16, rl.BLUE, rl.WHITE)

rl.run(update)
