import rl

cells = rl.Array(320, 240)
cells.random_int(0, 1)

definitions = ['B3/S23', 'B3/S1234', 'B3/S12345', 'B35678/S5678', 'B3678/S34678', 'B368/S245', 'B4678/S35678']
current = 0

def update(event):
    global current
    if event == rl.SPACE:
        current += 1
    elif event == rl.RETURN:
        cells.random_int(0, 1)
    cells.cell_automaton(definitions[current % len(definitions)])
    rl.draw_image(rl.array_to_image(cells, palette=[rl.BLACK, rl.WHITE]), 0, 0)

rl.run(update)
