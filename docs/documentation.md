# Pyrogue documentation

Pyrogue is a python interpreter based on [micropython](https://www.micropython.org) with a minimalist library for making roguelikes.

Pyrogue comes with the `rl` module which covers basic roguelike functionalities.
In addition, it comes with the following micropython modules: [math](https://docs.micropython.org/en/latest/library/math.html), [sys](https://docs.micropython.org/en/latest/library/sys.html), [ujson](https://docs.micropython.org/en/latest/library/ujson.html), [ure](https://docs.micropython.org/en/latest/library/ure.html). Other micropython modules are not available and the python language implemented by micropython [differs](https://docs.micropython.org/en/latest/reference/index.html) a bit from python. 

A typical roguelike programs first imports the `rl` module, setups a display and invokes run with an update callback.

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

### `rl.run(update_callback, event_filter=rl.UPDATE_LOOP)`

Pyrogue's display functions are only honored if called from an update callback which is called every time events occur and the screen needs to be repainted. `rl.run()` is a blocking function which calls the update function repeatedly according to the given event filter.

```python
def update(event):
	rl.fill_rect(0, 0, 50, 50, rl.INDIGO)
rl.run(update)
```

Valid values for the filter are `rl.UPDATE_LOOP` to recieve update continuously, `rl.UPDATE_KEY` to recieve updates when a key is pressed, and `rl.UPDATE_MOUSE` to receive updates on mouse events. The last two can be combined (`rl.UPDATE_KEYS|rl.UPDATE_MOUSE`) to recieve both kinds of events.

The update callback is a function taking one parameter which represents the event that triggered it. Events can be:
* a value > 0 is recieved when a key is pressed. The constants for key identities are the same than [SDL](https://wiki.libsdl.org/SDL_Keycode) without the `SDL_` prefix.
* `rl.QUIT` is recieved when the `rl.quit()` function is called or the window is closed.
* `rl.MOUSE` is received when a mouse event occured. `rl.mouse()` can be used to retrive mouse coordinates.
* `rl.REDRAW` is recieved for other events that require a redraw (such as window size changes)

### `rl.quit()`

Terminates a `rl.run()` call.

```python
def update(event):
	if event > 0:
		rl.quit() # quit on any key
rl.run(update, rl.UPDATE_KEY)
```

### `rl.mouse()`

Returns the coordinates of the mouse, as well as the button being pressed. The coordinates are in pixels according to the resolution of the display (see `rl.init_display()`).

```python
x, y, button = rl.mouse()
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

### `rl.font(filename, size)`

Load a TTF font for writing text on the display. The text renderer is fast but does not support sub-pixel positionning of letters, which sometimes leads to weird kernings. The returned font can be passed to the `rl.draw_text()` function. Note that only ascii characters in range 32-127 can be drawn. The font object has two read-only attributes: `size` which is the size passed to the constructor, and `line_height` which is the line height stored in the font.

```python
font = rl.font('monospace.ttf', 14)
print(font.line_height)
```

Note that system resources behind a font (such as the character atlas) are only disposed when the garbage collector is run after memory gets low. When manipulating many fonts, it is a good idea to call `del font`, where `font` is a variable holding the font, to explicitly release the resources.

### `rl.image(filename, tile_width=8, tile_height=8)`

Load a png/jpg image from resources and return an image object. Optionnaly, the size of tiles can be specified. It defaults to 8x8. The returned image object can be passed to drawing functions such as `rl.draw_image()`. It has two read-only members: `width` and `height` and two read-write members: `tile_width` and `tile_height`.

```python
image = rl.image('tiles.png', 16, 16)
print(image.width, image.height)
image.tile_width = 8
print(image.tile_width)
```

Note that system resources behind an image (such as the pixels uploaded to the GPU) are only disposed when the garbage collector kicks in when memory gets low. When using many images, it is a good idea to call `del image` when it is not used anymore.

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
In addition, if specified, the tile background is first painted with `bg` color, and the non-transparent pixels of the tile are colorized with `fg` color. Each can be `0` to skip colorization. 

```python
rl.draw_tile(0, 32, 55, 3) # draw tile number 3 at coord (32, 55) from image 0
rl.draw_tile(tileset, 5, 5, 17, fg=rl.BLUE, bg=rl.RED) # colorize in addition
```

Note that the alpha channel of the image is used to determine transparent pixels.

### `rl.draw_text(font, x, y, text, color=rl.WHITE, align=rl.ALIGN_LEFT, line_height=0)`

Draw text on the screen at coordinates (x, y) using the given color. The font can be either a TTF font loaded with `rl.font()` or an image loaded with `rl.image()`. If it is an image, it is assumed to be a tileset with ASCII characters as letters.

The align parameter selects the anchoring point of the text compared to the coordinates (x, y). Valid values are `rl.ALIGN_LEFT`, `rl.ALIGN_RIGHT` and `rl.ALIGN_CENTER`.

Note that only characters from 32 to 127 can be printed (both when using a TTF font or a tileset).

```python
font = rl.font('font.ttf', 10)
image = rl.image('font.png', 10, 10)
rl.draw_text(font, 0, 0, 'Hello world')
rl.draw_text(image, 0, 20, 'Hello world')
```

### `rl.size_text(font, text)`

Compute the width and height of text according to the TTF font.

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

### `rl.color(r, g, b, a=255)`

Make an integer representing an RGBA color from 8-bit components. The values for red (`r`), green (`g`), blue (`b`) and alpha (`a`) must be between 0 and 255 inclusive. The alpha channel represents the opacity of the color, with 255 the most opaque, and 0 transparent.

```python
blue = rl.color(0, 0, 255)
magenta = rl.color(255, 0, 255)
transparent_green = rl.color(0, 255, 0, 128)
```

In addition, pyrogue defines constants for the pico8 palette:
```python
pico8_colors = [rl.BLACK, rl.DARKBLUE, rl.DARKPURPLE, rl.DARKGREEN,
	rl.BROWN, rl.DARKGRAY, rl.LIGHTGRAY, rl.WHITE,
	rl.RED, rl.ORANGE, rl.YELLOW, rl.GREEN, 
	rl.BLUE, rl.INDIGO, rl.PINK, rl.PEACH,
]
```

### `rl.hsv_color(h, s, v, a=255)`

Make an integer representing an RGBA color from HSV components. This function is similar to `rl.color()` but it takes a hue, saturation and value components. Unlike typical implementations, the hue component is not in degrees, but rather sits between 0 and 255 (corresponding to a 2PI angle).

```python
red = rl.hsv_color(0, 255, 255)
```

### `rl.color_r(c)`, `rl.color_g(c)`, `rl.color_b(c)`, `rl.color_a(c)`

Return indvidual components from an RGBA color.

```python
c = rl.color(255, 0, 0)
print(rl.color_r(c) == 255)
```

## <a name="random"></a>Random

The random generator in pyrogue is a [PCG](http://www.pcg-random.org/) with a single 64-bit state.
Setting the seed changes the state to a given value. Drawing a random number with any of the functions below changes the state.

### `rl.random()`

Generate a random floating point number between 0 and 1.

```python
x = rl.random()
```

### `rl.random_int(a, b)`

Generate a random integer between `a` and `b` inclusive.

For example, to generate `5 <= x <= 7`:
```python
x = rl.random_int(5, 7)
```

### `rl.roll(dice)`

Roll `n` dices with `m` faces according to the `dice` string specification.
The specification the number of rolls followed by 'd' or 'D', followed by the number of faces of the dice. The result is the sum of all rolls. 0 is returned if the specification is invalid.

```python
x = rl.roll('3d5')
x = rl.roll('1d12')
```

### `rl.set_seed(s)`

Set the random number state to the 64-bit integer `s`. If `s` is 0, then the state is set according to the current time.

```python
rl.set_seed(123) # set a fixed seed
rl.set_seed(0)   # seed randomly
```

### `rl.get_seed()`

Get the 64-bit state of the random generator. Is useful to resume generation after, for example, loading a level.

```python
x = rl.get_seed()
```

### `rl.random_color()`

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

### `rl.array(width, height)`

Create a new array filled with zeros.

```python
a = rl.array(80, 25)
```

### `rl.array_from_string(text)`, `array.to_string()`

Deserialize and serialize and array from and to a string. Useful for saving and restoring array data.

```python
a = rl.array(10, 10)
text = a.to_string()
print(text)
b = rl.array_from_string(text)
print(b)
```

### `array.get(i, j)`, `array.set(i, j, value)`, `array[i, j]`

Access individual elements of the array. The bracket operator is a shortcut.

```python
a = rl.array(5, 5)
a.set(3, 3, 42)
a[2, 1] = a.get(3, 3)
```

### `array.width()`, `array.height()`

Return the size of the array.

```python
a = rl.array(5, 7)
print(a.width()) # should print 5
```

### `array.print_ascii(chars, fg=None, bg=None)`

Print the content of an array to the terminal. Mainly useful for debugging.
Only positive values are displayed. `chars` is a string where each character corresponds
to the symbol printed for value of the array equals to its index.
`bg` and `fg` are optional strings for selecting colors for the corresponding characters.
The colors are denoted by characters '1-9a-f' corresponding to basic 16 ANSI terminal colors.

```python
a = rl.array(3, 5)
a[1, 1] = 1
a.print_ascii('.#', '12', 'ba')
```

### `print(array)`

Print the content of an array using the standard python print. The array is presented as a list of lists.

```python
a = rl.array(2, 2)
print(a)
```

### `array.copy()`

Return a copy of the array which can be modified independently.

```python
b = a.copy()
b[1, 1] = 2
print(a, b) # prints different content for a and b
```

### `array.copy_to(dest)`

Copy the values of an array to another array.

```python
a.copy_to(b)
```

### `array.copy_masked(dest, mask, keep=1)`

Copy the values of an array to another array, but only copy those values for which the mask array is equal to `keep`. All arrays must be the same size.

```python
b = rl.array(a.width(), a.height())
a.copy_masked(b, mask)
print(b)
```

### `array.equals(value)`

Returns a new array with 1 in every cell containing the value in the original array, 0 otherwise.

```python
walls = level.equals(1)
```

### `array.fill(value)`

Fill an array with a given integer value.

```python
a = rl.array(10, 10)
a.fill(5)
```

### `array.replace(value, other)`

Replace a value by another in an array.

```python
a.replace(5, 7) # replaces all occurrences of 5 with 7.
```

### `array.add(value, blocking=INT_MAX)`, `array.mul(value, blocking=INT_MAX)`

Add (respectively multiply) a scalar value to each cell of an array.
Values which equal `blocking` are not modified.

```python
a.add(3) 
a.mul(2)
```

### `array.min()`, `array.max()`

Return the mininum (respectively maximum) value of an array.

```python
min = a.min()
max = a.max()
```

### `array.argmin()`, `array.argmax()`

Returns the location of the first encountered smallest (respectively largest) value in the array.

```python
x_min, y_min = a.argmin()
x_max, y_max = a.argmax()
```

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

### `array.line(x1, y1, x2, y2, value)`

Set all elements of an array on a straight line with a given value. Coordinates of the line ends are cell indices in the array.

```python
a.line(1, 3, 5, 7, -2) # draw a line with the value -2
```

### `array.rect(x, y, width, height, value)`

Fill a rectangular area in a rectangle with a given value.

```python
a.rect(0, 0, 2, 3, 42) # fill a retangle with value 42
```

### `array.can_see(x1, y1, x2, y2, blocking=1)`

Tests whether cell (x1, y1) has line of sight to cell (x2, y2) in the given array, using `blocking` as wall value. Note that the algorithm used for computing lines is not symmetric, that is `array.can_see(x1, y1, x2, y2)` is not necessarily the same as `array.can_see(x2, y2, x1, y1)`. Returns `True` if the destination can be seen from the source, `False` otherwise.

```python
if level.can_see(player.x, player.y, monster.x, monster.y):
	print('I can see a monster.')
```

### `array.field_of_view(x, y, radius, blocking=1)`

Computes the field of view around a point in an array, considering that all cells with value `blocking`, and returns an array where all cells with value 1 are visible, other cells have value 0. 

```python
fov = level.field_of_view(player.x, player.y, 10)
```

### `array.dijkstra()`

Applies the [djikstra map](http://roguebasin.roguelikedevelopment.org/index.php?title=The_Incredible_Power_of_Dijkstra_Maps) algorithm to the array until convergence.

```python
a.fill(rl.MAX_INT)
a[5, 5] = 0
a.djikstra()
# a now contains the distance from each of its 
# cells to (5, 5) using nonblocking paths
```

### `array.shortest_path(x1, y1, x2, y2, blocking=1)`

Computes the shortest path between two points in an array, considering that cells containing the `blocking` value cannot be moved through. Diagonal movements are authorized. If no path cannot be found, `None` is returned.

```python
for x, y in a.shortest_path(3, 2, 5, 6):
	print(x, y)
```

### `array.find_random(needle, tries=100)`

Return the location of a value (the needle) in an array. Locations are tested randomly until one is found up to the number of specified tries.

```python
x, y = a.find_random(1)
```

### `array.place_random(needle, value, tries=100)`

Replace an occurrence of the needle with the given value in an array. Works similarly to `array.find_random()`.

```python
a.place_random(1, 42)
```

### `rl.draw_array(x, y, image=0, x_shift=0, y_shift=0, mapping=None, fg=None, bg=None)`

Draw an array of tiles on the screen at coordinates x, y. By default, coordinates are shifted by the `tile_width` and `tile_height` of the image, but other values can be specified in `x_shift` and `y_shift`. Optionaly, values can be mapped for selecting tiles and coloring them with foreground (`fg`) and background (`bg`) colors.

```python
rl.draw_array(x, y, mapping=[1,2,3], fg=[rl.RED, rl.BLUE, rl.GREEN])
```

### `rl.array_to_image(array, image, tile_width=8, tile_height=8)`

Fill the image at index `image` with the values of an array intepreted as RGBA colors.

```python
a = rl.array(320, 240)
a.fill(rl.RED)
rl.array_to_image(a, 0)
```

### `rl.image_to_array(image)`

Create an array with the pixels of an image. Each value of the array represents an RGBA color. 
The image index must contain an image.

```python
rl.load_image(0, 'tileset.png')
a = rl.image_to_array(0)
```

## <a name="files"></a>Files

Pyrogue assumes two locations for storing files. The "ressources" location is a bundle of read-only assets such as images, fonts, game data and python scripts. It can be a directory, a zip or can be embedded in the pyrogue executable. The "preferences" location is a directory where game saves or high-scores can be saved and loaded later. It sits in the user directory returned by `SDL_GetPrefPath()` under a `pyrogue` directory.

### `rl.open_resources(location)`

Set the location of resources for loading scripts and assets. Generally, the interpreter already sets that correct value according to how the script was loaded. Currently supports directories and zips and the main exe. Directories must end with a '/', zips must end with '.zip', otherwise resources are looked for at the end of the specified exe.

```python
rl.open_resources('data.zip')
rl.open_resources('directory/')
rl.open_resources('pyrogue.exe')
```

It is generally not required to call this function.

### `rl.load_asset(filename)`

Load asset from resources. Filename can be a path including directories. Returns a bytes object with the loaded data or `None` if the asset could not be loaded.

```python
data = rl.load_asset('data.txt')
```

Note that assets from the resources file are read-only.

### `rl.set_app_name(name)`

Set the name of the app for loading and saving preferences in the user directory.

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

