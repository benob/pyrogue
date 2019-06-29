# Pyrogue documentation

Pyrogue is a python interpreter based on [micropython](https://www.micropython.org) with a minimalist library for making roguelikes.

Pyrogue comes with the `rl` module which covers basic roguelike functionalities.
In addition, it comes with the following micropython modules: [math](https://docs.micropython.org/en/latest/library/math.html), [sys](https://docs.micropython.org/en/latest/library/sys.html), [ujson](https://docs.micropython.org/en/latest/library/ujson.html), [ure](https://docs.micropython.org/en/latest/library/ure.html). Other micropython modules are not available and the python language implemented by micropython [differs](https://docs.micropython.org/en/latest/genrst/index.html) a bit from standard python. For efficiency, most functions in the `rl` module do not support keyword arguments.

A typical pyrogue program first imports the `rl` module, setups a display and invokes run with an update callback.

```python
import rl

WIDTH, HEIGHT = 320, 240

def update(event):
	x, y = rl.random_int(0, WIDTH - 1), rl.random_int(0, HEIGHT - 1)
	rl.fill_rect(x, y, 50, 50, rl.random_color())

rl.init_display('example', WIDTH, HEIGHT)
rl.run(update)
```

`rl` module contents:
* [Input](#input)
* [Drawing](#drawing)
* [Random](#random)
* [Arrays](#arrays)
* [Files](#files)
* [Utils](#utils)

## <a name="input"></a> Input

### `rl.run(update_callback, when=rl.CONTINUOUSLY)`

Pyrogue's display functions are only honored if called from an update callback which is called every time events occur and the screen needs to be repainted. `rl.run()` is a blocking function which calls the update function repeatedly according to the `when` parameter.

Valid values for `when` are `rl.CONTINUOUSLY` to recieve updates continuously, `rl.ON_KEY` to recieve updates when a key is pressed, and `rl.ON_MOUSE` to receive updates on mouse events. The last two can be combined (`rl.ON_KEYS|rl.ON_MOUSE`) to recieve both kinds of events.

The update callback is a function taking one parameter which represents the event that triggered it. Events can be:
* a value > 0 is recieved when a key is pressed. The constants for key identities are the same than [SDL](https://wiki.libsdl.org/SDL_Keycode) without the `SDL_` prefix.
* `rl.QUIT` is recieved when the `rl.quit()` function is called or the window is closed.
* `rl.MOUSE` is received when a mouse event occured. `rl.mouse()` can be used to retrive mouse coordinates and clicks.
* `rl.REDRAW` is recieved for other events that require a redraw (such as window size changes)

```python
import rl

x, y = 50, 50

def update(event):
	global x, y

	# move rectangle by 10 pixels in the direction of the pressed key
	if event == rl.LEFT:
		x -= 10
	elif event == rl.RIGHT:
		x += 10
	elif event == rl.UP:
		y -= 10
	elif event == rl.DOWN:
		y += 10
	# or use mouse to move the rectangle
	elif event == rl.MOUSE:
		x, y, button = rl.mouse()

	rl.clear()
	rl.fill_rect(x, y, 50, 50, rl.GREEN)

rl.run(update)
```

### `rl.quit()`

Terminates a `rl.run()` call.

```python
def update(event):
	if event > 0:
		rl.quit() # quit on any key
rl.run(update, rl.ON_KEY)
```

### `x, y, button = rl.mouse()`

Returns the coordinates of the mouse, as well as the button being pressed. The coordinates are in pixels according to the resolution of the display (see `rl.init_display()`). The button can be `rl.NO_BUTTON`, `rl.BUTTON1_DOWN`, `rl.BUTTON2_DOWN`, `rl.BUTTON3_DOWN` (button pressed), `rl.BUTTON1_UP`, `rl.BUTTON2_UP`, `rl.BUTTON3_UP` (button released). Button 1, 2 and 3 refer to left, center and right mouse buttons.

```python
x, y, button = rl.mouse()
if button == rl.BUTTON1_DOWN:
	print('pressed left mouse button')
```

### `rl.shift_pressed()`, `rl.alt_pressed()`, `rl.ctrl_pressed()`, `rl.win_pressed()`

Returns true if the corresponding modifier key is pressed.

## <a name="drawing"></a>Drawing

The display is a pixel surface resized to fit the pyrogue window. Alt-Enter can put the game in fullscreen. Alt-q force-quits. When starting pyrogue, a default resolution of 320x240 is already setup.

### `rl.init_display(title, width, height)`

Initialize the display with a given window title, and width and height in pixels.
Coordinates follow the standard in graphics: (0, 0) is at the top-left corner, (width, height) is as the bottom-right corner. The display is a 32bit RGB surface which is automatically resized to fit the window.

```python
rl.init_display('snake', 320, 240)
```

### `font = rl.Font(filename, size)`

Load a TTF font for writing text on the display. The returned font can be passed to the `rl.draw_text()` function. Note that only ascii characters in range 32-127 can be drawn. The font object has two read-only attributes: `size` which is the size passed to the constructor, and `line_height` which is the line height stored in the font.

```python
font = rl.Font('monospace.ttf', 14)
print(font.line_height)
```

Note that system resources behind a font (such as the character atlas) are only disposed when the garbage collector is run after memory gets low. When manipulating many fonts, it is a good idea to call `del font`, where `font` is a variable holding the font, to explicitly release the resources.

### `image = rl.Image(filename, tile_width=8, tile_height=8)`

Load a png/jpg image from resources and return an image object. Optionnaly, the size of tiles can be specified. It defaults to 8x8. The returned image object can be passed to drawing functions such as `rl.draw_image()`. It has two read-only members: `width` and `height` and two read-write members: `tile_width` and `tile_height`.

```python
image = rl.Image('tiles.png', 16, 16)
print(image.width, image.height)
image.tile_width = 8
print(image.tile_width)
```

Note that system resources behind an image (such as the pixels uploaded to the GPU) are only disposed when the garbage collector kicks in when memory gets low. When using many images, it is a good idea to call `del image` when the image is not used anymore.

### `rl.clear()`

Clear the screen by painting it in black.

```python
rl.clear()
```

### `rl.draw_image(image, x, y)`

Draw an image on the screen. The image parameter is the index of the image loaded in memory. The coordinates are in screen coordinates.

```python
rl.draw_image(image, 50, 50)
```

### `rl.draw_tile(image, x, y, tile, fg=0, bg=0)`

Draw a tile from an image representing a tileset. The integer index of the image is passed as first parameter. The coordinates of the tile in the image are computed as follows:
```python
tiles_per_line = image.width / image.tile_width
tile.x = image.tile_width * (tile % tiles_per_line)
tile.y = image.tile_height * (tile / tiles_per_line)
```
The retangle `(tile.x, tile.y, image.tile_width, image.tile_height)` is then copied at coordinates (x, y) on the screen.
In addition, if specified, the tile background is first painted with `bg` color, and the non-transparent pixels of the tile are colorized with `fg` color. Either can be `0` to skip colorization. 

```python
tileset = rl.Image('tileset.png', 16, 16)
rl.draw_tile(tileset, 32, 55, 3) # draw tile number 3 at coord (32, 55)
rl.draw_tile(tileset, 5, 5, 17, fg=rl.BLUE, bg=rl.RED) # colorize in addition
```

Note that the alpha channel of the image is used to determine transparent pixels.

### `rl.draw_text(font, x, y, text, color=rl.WHITE, align=rl.ALIGN_LEFT|rl.ALIGN_TOP, line_height=0)`

Draw text on the screen at coordinates (x, y) using the given color. The font can be either a TTF font loaded with `rl.Font()` or an image loaded with `rl.Image()`. If it is an image, it is assumed to be a tileset with ASCII characters as letters.

The align parameter selects the anchoring point of the text compared to the coordinates (x, y). Valid values are `rl.ALIGN_LEFT`, `rl.ALIGN_RIGHT` and `rl.ALIGN_CENTER` for horizontal alignment, and `rl.ALIGN_TOP`, `rl.ALIGN_BOTTOM` and `rl.ALIGN_MIDDLE` for vertical alignment. Both types of alignment can be mixed with the `|` operator (`rl.ALIGN_CENTER|rl.ALIGN_MIDDLE` to center the text vertically and horizontaly).

Note that only characters from 32 to 127 can be printed from TTF fonts. All tiles from 1 to 255 can be printed from a tileset (except `\n`).

```python
font = rl.Font('font.ttf', 10)
image = rl.Image('font.png', 10, 10)
rl.draw_text(font, 0, 0, 'Hello world', rl.ALIGH_RIGHT)
rl.draw_text(image, 0, 20, 'Hello world')
```

### `width, height = rl.size_text(font, text)`

Compute the width and height of text according to a TTF font.

```python
width, height = rl.size_text(font, 'hello world')
```

### `rl.fill_rect(x, y, width, height, color)`

Fill a rectangle `(x, y, width, height)` with the given color.

```python
rl.fill_rect(20, 10, 50, 60, rl.color(255, 0, 0))
```
 
### `rl.draw_rect(x, y, width, height, color)`

Draw the outline of a rectangle `(x, y, width, height)` with the given color.
```python
rl.draw_rect(10, 20, 30, 90, rl.color(255, 255, 0))
```

### `rl.draw_line(x1, y1, x2, y2, color)`

Draw a stright line between `(x1, y1)` and `(x2, y2)` with the given color.
```python
rl.draw_line(5, 13, 120, 92, rl.PINK)
```

### `color = rl.color(r, g, b, a=255)`, `color = rl.color(hex_color)`

Make an integer representing an RGBA color from 8-bit components or an hexadecimal string. The values for red (`r`), green (`g`), blue (`b`) and alpha (`a`) must be between 0 and 255 inclusive. The alpha channel represents the opacity of the color, with 255 the most opaque, and 0 transparent. If only one argument is given to the function, it is interpreted as a string representing a hex color, starting with a `#` and followed by 3, 6 or 8 hexadecimal digits (`0-9a-f`). The specification can be found [here](https://www.quackit.com/css/color/values/css_hex_color_notation_3_digits.cfm). 8-digit colors are the same as 6-digits, prefixed with two digits for the alpha channel.

```python
blue = rl.color(0, 0, 255)
magenta = rl.color(255, 0, 255)
transparent_green = rl.color(0, 255, 0, 128)
red = rl.color('#ff0000') # 6-digit hex color
green = rl.color('#0f0') # 3-digit hex color
transparent_blue = rl.color('#1f0000ff') # 6-digit with transparency
```

In addition, pyrogue defines constants for the [pico8 palette](https://www.romanzolotarev.com/pico-8-color-palette/):
```python
pico8_colors = [rl.BLACK, rl.DARKBLUE, rl.DARKPURPLE, rl.DARKGREEN,
	rl.BROWN, rl.DARKGRAY, rl.LIGHTGRAY, rl.WHITE,
	rl.RED, rl.ORANGE, rl.YELLOW, rl.GREEN, 
	rl.BLUE, rl.INDIGO, rl.PINK, rl.PEACH,
]
```

### `color = rl.hsv_color(h, s, v, a=255)`

Make an integer representing an RGBA color from HSV components. This function is similar to `rl.color()` but it takes a hue, saturation and value components. Unlike typical implementations, the hue component is not in degrees, but rather sits between 0 and 255 (corresponding to a 2PI angle).

```python
red = rl.hsv_color(0, 255, 255)
```

### `r, g, b, a = rl.color_components(color)`

Return indvidual components from an RGBA color.

```python
color = rl.color(255, 128, 64, 32)
r, g, b, a = rl.color_components(color)
print(r == 255, g == 128, b == 64, a == 32)
```

## <a name="random"></a>Random

The random generator in pyrogue is a [PCG](http://www.pcg-random.org/) with a single 64-bit state.
Setting the seed changes the state to a given value. Drawing a random number with any of the functions below changes the state.

### `number = rl.random()`

Generate a random floating point number between 0 and 1.

```python
x = rl.random()
```

### `number = rl.random_int(a, b)`

Generate a random integer between `a` and `b` inclusive.

For example, to generate `5 <= x <= 7`:
```python
x = rl.random_int(5, 7)
```

### `number = rl.roll(dice)`

Roll `n` dices with `m` faces according to the `dice` string specification.
The specification the number of rolls followed by 'd' or 'D', followed by the number of faces of the dice. The result is the sum of all rolls. 0 is returned if the specification is invalid.

```python
x = rl.roll('3d5')
x = rl.roll('1d12')
```

### `element = rl.random_choice(list)`

Choose a random element from a list and return it. 

```python
chosen = rl.random_choice([1, 2, 3])
print(chosen)
```

### `element = rl.shuffle(list)`

Shuffle elements from a list, modifying it inplace.

```python
numbers = [1, 2, 3]
rl.shuffle(numbers)
print(numbers)
```

### `rl.set_seed(seed=0)`

Set the random number state to the 64-bit integer `seed`. If `seed` is 0, then the state is set according to the current time.

```python
rl.set_seed(123) # set a fixed seed
rl.set_seed()   # seed randomly
```

### `seed = rl.get_seed()`

Get the 64-bit state of the random generator. Is useful to resume generation after, for example, loading a level.

```python
x = rl.get_seed()
```

### `color = rl.random_color()`

Generate a random RGB color.
```python
c = rl.random_color()
```

### `rl.random_next()`, `rl.random_1d(x)`, `rl.random_2d(x, y)`, `rl.random_3d(x, y, z)`

Generate 32-bit random integers accoring to the internal seed and up to 3 integer dimensions.
These functions yield deterministic noise for a given point in space.
```python
x = rl.random_next()
x = rl.random_1d(5)
x = rl.random_2d(5, 42)
x = rl.random_3d(5, 42, 33)
```

## <a name="arrays"></a>Arrays

Arrays are fixed-size 2D data structures which allow access to individual elements and have a number of facilities for performing bulk operations. The class also has a few roguelike-focused methods such as computation of field of vision or path finding.

The underlying data type for each cell of the array is a 32-bit signed integer. It can therefore accommodate RGB colors.

Two useful constants are defined, `rl.INT_MAX` and `rl.INT_MIN`, which correspond to the largest and smallest values that can be stored in an array.

### `array = rl.Array(width, height)`

Create a new array of a given size filled with zeros.

```python
a = rl.Array(80, 25)
```

### `array = rl.array_from_string(text)`, `text = array.to_string()`

Deserialize and serialize and array from and to a string. Useful for saving and restoring array data.

```python
a = rl.Array(10, 10)
text = a.to_string()
print(text)
b = rl.array_from_string(text)
print(b)
```

### `array = array.view(x, y, width, height)`

Create a new array (a view) which refers to values from another array. 
Modifying the new array also modifies the old array. Other than that,
a view can be used as a regular array.

```python
a = rl.Array(80, 25)
b = a.view(10, 10, 20, 30)
b[0, 0] = 1
print(a[10, 10]) # prints 1
```

### `array[i, j]`

Access individual elements of the array for reading or writing. 

```python
a = rl.Array(5, 5)
a[1, 3] = 42
a[2, 1] = a[1, 3]
```

### `array[start: end, start: end]` 

Slice indexing is supported similarly to numpy and yields a view into the
array.  Either or both indexes can be a slice which result in a view to the
corresponding indices. Note that slices of step != 1 are not supported. This
kind of indexing supports assignment from an integer or an appropriately shaped
array.

```python
a = rl.Array(80, 25)
b = a[10: 30, 10: 40] # identical to previous view
print(a[:,-1]) # last row
a[:2, :2] = 42 # fill slice with value
b = rl.Array(2, 2)
b.fill(33)
a[:2, :2] = b # fill slice with other array
```

### Unary and binary operators

Arrays support the following unary operators: `+` (does nothing), `-` (negative), `~` (bitwise complement of values), abs(x) (absolute value of elements). In addition, in boolean context, arrays are `True` if they contain any non-zero value, `False` otherwise.

Arrays support the following binary opeartors with arrays and integers as operands: `+`, `-`, `*`, `/`, `&` (bitwise and), `|` (bitwise or), `^` (bitwise xor), `<<` (left bitshift), `>>` (right bitshift). They work on each integer values of the array following C conventions (so division is like // in python) and return an array with the result. Inplace operators such as `+=` work too.

Arrays support the following operators with arrays and integers as operands: `a.equals(b)`, `a.not_equals(b)`, `>`, `>=`, `<`, `<=`. They return an array of truth value encoded as integers (1 is true, 0 is false). 

```python
a = rl.Array(2, 2)
a.random_int(2, 6)
b = a.equals(2)
c = b * 2 + 3
print(c << 1)
```

Note that `==` and `!=` are not supported due to a bug in micropython.

### `number = array.width()`, `number = array.height()`

Return the size of the array.

```python
a = rl.Array(5, 7)
print(a.width()) # should print 5
```

### `array.print_ascii(chars, fg=None, bg=None)`

Print the content of an array to the terminal. Mainly useful for debugging.
Only positive values are displayed. `chars` is a string where each character corresponds
to the symbol printed for value of the array equals to its index.
`bg` and `fg` are optional strings for selecting colors for the corresponding characters.
The colors are denoted by characters '1-9a-f' corresponding to 16 basic ANSI terminal colors.

```python
a = rl.Array(3, 5)
a[1, 1] = 1
a.print_ascii('.#', '12', 'ba')
```

### `print(array)`

Print the content of an array using the standard python print. The array is presented as a list of lists.

```python
a = rl.Array(2, 2)
print(a)
```

### `array = array.copy(mask=None)`

Return a copy of the array which can be modified independently. If a `mask` array is specified, only values where the mask is non-null are copied, the rest are zeroed.

```python
b = a.copy()
b[1, 1] = 2
print(a, b) # prints different content for a and b
```

### `array.copy_to(dest, mask=None)`

Copy the values of an array to another array. Both arrays must have the same size. If a `mask` array is specified, only values where the mask is non-null are copied, the rest are left unchanged.

```python
a = rl.Array(2, 2)
a.random_int(0, 1)
b = rl.Array(2, 2)
mask = rl.Array(2, 2)
mask[0, 0] = 1
mask[1, 1] = 1
a.copy_to(b, mask) # only copy values on the diagonal of a to b.
```

### `array.fill(value)`

Fill an array with a given integer value.

```python
a = rl.Array(10, 10)
a.fill(5)
```

### `array.replace(value, other)`

Replace a value by another in an array.

```python
a.replace(5, 7) # replaces all occurrences of 5 with 7.
```

### `value = array.min()`, `value = array.max()`

Return the mininum (respectively maximum) value of an array.

```python
min = a.min()
max = a.max()
```

### `x, y = array.argmin()`, `x, y = array.argmax()`

Returns the location of the first encountered smallest (respectively largest) value in the array.

```python
x_min, y_min = a.argmin()
x_max, y_max = a.argmax()
```

### `result = array.sum()`

Return the sum of the array elements.

### `result = array.count(value)`

Count the number of times a value appears in the array.

### `array.random_int(a, b)`

Fill an array with random numbers between `a` and `b` inclusive.

```python
a.random_int(0, 42)
```

### `array.random_2d(x, y)`

Fill an array with deterministic noise at `(x, y)` offsetted by the corresponding cell index.
See `rl.random_2d()` for more information.

```python
a.random_2d(5, 3) 
```

### `array.random()`

Fill an array with random 32-bit values.

```python
a.random()
```

### `array.draw_line(x1, y1, x2, y2, value)`

Set all elements of an array on a straight line with a given value. Coordinates of the line ends are cell indices in the array. Note that lines are not symmetric, so `line(a, b)` will not necessary modify the same cells as `line(b, a)`.

```python
a.draw_line(1, 3, 5, 7, -2) # draw a line with the value -2
```

### `array.draw_rect(x, y, width, height, value=1)`, `array.fill_rect(x, y, width, height, value=1)`

Draw or fill a rectangular area with a given value.

```python
a.fill_rect(0, 0, 2, 3, 42) # fill a rectangule with value 42
```

### `array.can_see(x1, y1, x2, y2, blocking=1)`

Tests whether cell (x1, y1) has line of sight to cell (x2, y2) in the given array, using `blocking` as wall value. Note that the algorithm used for computing lines is not symmetric, that is `array.can_see(x1, y1, x2, y2)` is not necessarily the same as `array.can_see(x2, y2, x1, y1)`. Returns `True` if the destination can be seen from the source, `False` otherwise.

```python
if level.can_see(player.x, player.y, monster.x, monster.y):
	print('I can see a monster.')
```

### `fov = array.field_of_view(x, y, radius, blocking=1)`

Computes the field of view around a point in an array, considering that all cells with value `blocking`, and returns an array where all cells with value 1 are visible, other cells have value 0. 

```python
fov = level.field_of_view(player.x, player.y, 10)
```

### `array.dijkstra()`

Applies the [djikstra map](http://roguebasin.roguelikedevelopment.org/index.php?title=The_Incredible_Power_of_Dijkstra_Maps) algorithm to the array until convergence. Each cell is set to the value of its lowest neighbor plus one until convergence. Negative cells are ignored. To build a path finder, set targets to 0, walls to -1, and passable floor to `rl.INT_MAX`. 

```python
a.fill(rl.MAX_INT)
a[5, 5] = 0
a.djikstra()
# a now contains the distance from each of its 
# cells to (5, 5) using nonblocking paths
```

This algorithm loops over the array as many times as there are steps in the longest path in the level.

### `array.cell_automaton(definition, warp=False)`

Applies one step of a [cellular automaton](https://en.wikipedia.org/wiki/Life-like_cellular_automaton) to the array. The function assumes that the array is filled with 0 (dead) or 1 (alive). Then, for each cell it computes its new state (dead or alive) depending on the definition which states how the cell changes depending on its neighbors. It uses the Golly notation: `Bx/Sy` where `x` and `y` are strings of digits from 0 to 8. `B` stands for birth and `S` stands for survival. If a cell is dead and its number of alive neighbors is in `x`, then it is set to alive. If a cell is alive and its number of alive neighbors is not in `y`, then it is set to dead.

```python
a = rl.Array(100, 100)
a.random_int(0, 1)

# 10 steps of the game of life automaton
for i in range(10):
	a.cell_automaton('B3/S23') 
```

### `path = array.shortest_path(x1, y1, x2, y2, blocking=1)`

Computes the shortest path between two points in an array, considering that cells containing the `blocking` value cannot be moved through. Diagonal movements are authorized. If a path cannot be found, `None` is returned.

```python
for x, y in a.shortest_path(3, 2, 5, 6):
	print(x, y)
```

### `result = array.apply_kernel(kernel)`

Apply a kernel to each cell of the array (sum of kernel elements multiplied by array elements shifted by position in kernel, and divide by the sum of kernel elements).

```python
a = rl.Array(10, 10)
a.random_int(0, 10)
kernel = rl.Array(3, 3)
kernel.fill(1) # blur kernel, average of element and neighbors
b = a.apply_kernel(kernel)
```

### `x, y = array.find_random(needle, tries=100)`

Return the location of a value (the needle) in an array. Locations are tested randomly until one is found up to the number of specified tries. Returns (-1, -1) when unsuccessful.

```python
x, y = a.find_random(1)
```

### `rl.draw_array(image, x, y, x_shift=0, y_shift=0, mapping=None, fg=None, bg=None)`

Draw an array of tiles on the screen at coordinates x, y. By default, coordinates are shifted by the `tile_width` and `tile_height` of the image, but other values can be specified in `x_shift` and `y_shift`. Optionaly, values can be mapped for selecting tiles and coloring them with foreground (`fg`) and background (`bg`) colors.

```python
rl.draw_array(tileset, x, y, mapping=[1,2,3], fg=[rl.RED, rl.BLUE, rl.GREEN])
```

### `image = rl.array_to_image(array, tile_width=8, tile_height=8, palette=None)`

Create an image from the values of an array intepreted as RGBA colors. The `tile_width` and `tile_height` properties of the image are set according to the given arguments. Optionally, a list of colors can be given as argument to convert array values with the corresponding colors in the palette.

```python
a = rl.Array(320, 240)
a.fill(rl.RED)
image1 = rl.array_to_image(a)
a = rl.Array(320, 240)
a.random_int(0, 1)
image2 = rl.array_to_image(a, palette=[rl.BLACK, rl.RED])
```

### `array = rl.image_to_array(image)`

Create an array from the pixels of an image. Each value of the array represents an RGBA color. 

```python
image = rl.load_image('tileset.png')
a = rl.image_to_array(image)
print(a[42, 31]) # color of pixel at (41, 31)
```

## <a name="files"></a>Files

Pyrogue assumes two locations for storing files. The "ressources" location is a bundle of read-only assets such as images, fonts, game data and python scripts. It can be a directory, a zip or can be embedded in the pyrogue executable. The "preferences" location is a directory where game saves or high-scores can be saved and loaded later. It sits in the user directory returned by `SDL_GetPrefPath()` under a `pyrogue` directory.

### `data = rl.load_asset(filename)`

Load asset from resources. Filename can be a path including directories. Returns a bytes object with the loaded data or `None` if the asset could not be loaded.

```python
data = rl.load_asset('data.txt')
```

Note that assets cannot be written.

### `rl.set_app_name(name)`

Set the name of the app for loading and saving preferences in the user directory. Special characters such as '/' are mapped to '_'.

```python
rl.set_app_name('my-game')
```

### `rl.load_pref(filename)`

Load a file from the user directory. As with `rl.load_assets()`, the content of the file is returned as a bytes object or `None` if the file could not be loaded.

```python
high_scores = rl.load_pref('high_scores.txt') 
```

On linux, that directory is `$XDG_CONFIG_DIR/pyrogue/<app-name>` where `<app-name>` is changed with `rl.set_app_name(name)`.

### `rl.save_pref(filename, content)`

Save a file to the user directory. The content passed to the function must be a bytes-like object such as a string. Python objects can be encoded to string with ujson.

```python
import ujson
scores = [1, 2, 3, 4]
rl.save_pref('high_scores.txt', ujson.dumps(scores))
```

## <a name="utils"></a>Utils

### `rl.walk_line_start(x1, y1, x2, y2)`, `rl.walk_line_next()`

Iterate over a straight line from (x1, y1) to (x2, y2). `rl.walk_line_next()` returns a tuple of coordinates for each point and `None` when it is finished iterating.

```python
rl.walk_line_start(10, 10, 20, 30)
while True:
	result = rl.walk_line_next()
	if result is None:
		break
	x, y = result
	# use (x, y)
```

The same algorithm as in `array.line()` is used.
