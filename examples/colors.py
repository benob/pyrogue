import rl

rl.init_display('Colors', 256, 256)

colors = rl.Array(256, 256)
for j in range(256):
    for i in range(256):
        if j < 128:
            colors[i, j] = rl.hsv_color(i, j * 2, 255)
        else:
            colors[i, j] = rl.hsv_color(i, 255, 255 - j * 2)

image = rl.array_to_image(colors)

def update(event):
    rl.draw_image(image, 0, 0)
    rl.fill_rect(50, 50, 40, 40, rl.RED)
    rl.fill_rect(100, 50, 40, 40, rl.color('#a00')) # 3-digit hex color
    rl.fill_rect(50, 100, 40, 40, rl.color('#1e84d4')) # 6-digit hex color
    rl.fill_rect(100, 100, 40, 40, rl.color('#1f1e84d4')) # 6-digit with alpha channel

rl.run(update)
