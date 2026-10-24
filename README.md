# ematrix
ematrix is a highly customizable digital rain effect.

Written in C99 using OpenGL 3.3 ([GLAD](https://github.com/Dav1dde/glad)), [GLFW](https://github.com/glfw/glfw) and [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h). \
Own files of ematrix (files not located in `src/lib/` directory) is distributed under The MIT License (See [LICENSE](LICENSE) for details).

## Compiling
+ Install:
    - C99-compliant compiler and linker
    - GNU `make`
    - [GLFW](https://github.com/glfw/glfw)
+ Open a terminal in this directory and run `make`. The executable will be located in the `build/` directory.
    - Alternatively, run `make test` to compile and run ematrix immediately.

## About the effect
![Screenshot](screenshot.png)

The effect utilizes glyphs provided by a font (See [Fonts](#fonts)) to render the rain (matrix of characters). \
Each character glyph changes randomly. Each character has brightness counter that decreases over time. \
A drop moves down each column of the rain. It increases the brightness counter of the characters it touches. Characters currently being touched by a drop are active and use separate layer of color map (See [Color map](#color-map)). \
Once a drop reaches the bottom edge of the window, it disappears for a random time before jumping to the top of window.

## Configuration
ematrix is configured using `key=value` options. 

You can pass options directly via the command-line argument `-key=value`, or specify a configuration file containing options using the `path/to/config` argument. \
**Note:** You can NOT add spaces after the equation mark (`=`).

### List of Options
| Key | Type | Description | Default | Min | Max |
| - | - | - | - | - | - |
| `Log` | int | Logging verbosity level.<br>0: none, 1: errors, 2: errors+version, 3: verbose. | 2 | 0 | 3 |
| `Screen` | int<br>index | The index of the screen to open in fullscreen. If set to -1, fullscreen is disabled. | -1 | -1 | 32768 |
| `ScreenW` | int<br>pixels | Fullscreen width (0 for default). | 0 | 0 | 32768 |
| `ScreenH` | int<br>pixels | Fullscreen height (0 for default). | 0 | 0 | 32768 |
| `Resize` | bool | If enabled, allows resizing the window and toggling fullscreen via the `KeyFullscreen` keybind. | 1 | 0 | 1 |
| `AutoIcon` | bool | If enabled, the fullscreen window will iconify when you click on another screen. | 0 | 0 | 1 |
| `HideMouse` | bool | If enabled, the mouse cursor will be hidden when clicked. It can be shown again via `KeyShowMouse` keybind. | 0 | 0 | 1 |
| `W` | int<br>pixels | Window width. | 800 | 1 | 32768 |
| `H` | int<br>pixels | Window height. | 600 | 1 | 32768 |
| `Title` | string | Window title. | `"ematrix"` | | |
| `CharSize` | int<br>pixels | Size of the rain characters. | 20 | 1 | 32768 |
| `Border` | int<br>pixels | Empty gap from window borders. | 4 | 0 | 32768 |
| `Stretch` | bool | Allows glyphs to be stretched for an ideal fit. | 1 | 0 | 1 |
| `VSync` | int | If set to 0, VSync is disabled. other values act as a divisor for the monitor's frame rate. Can be ignored if the GPU does not support VSync. | 1 | 0 | 32768 |
| `Font` | string<br>path | Path to a font file (PNG MSDF).<br>See [Fonts](#fonts). | `"embed://font.png"` | | |
| `FontNum` | int | The number of glyphs contained in the font file.<br>See [Fonts](#fonts). | 57 | 1 | 32768 |
| `FontCols` | int | The number of columns the glyphs are grouped into.<br>See [Fonts](#fonts). | 8 | 1 | 32768 |
| `FontRows` | int | The number of rows the glyphs are grouped into.<br>See [Fonts](#fonts). | 8 | 1 | 32768 |
| `RandMin` | int<br>frames | Minimum time between glyph randomization. | 200 | 1 | 32768 |
| `RandMax` | int<br>frames | Maximum time between glyph randomization. | 400 | 1 | 32768 |
| `RandPow` | int | Maximum steps of pow'ing. | 8 | 0 | 32768 |
| `RandMul` | int<br>frames | Each pow'ing step the time is multiplied by this. | 5 | 1 | 32768 |
| `RandDiv` | int<br>frames | Each pow'ing step the time is divided by this. | 4 | 1 | 32768 |
| `DropMin` | int<br>frames | Minimum period between falling drops. | 200 | 1 | 32768 |
| `DropMax` | int<br>frames | Maximum period between falling drops. | 400 | 1 | 32768 |
| `OnlyInt` | bool | If enabled, the drop period will be rounded to an integer ratio. | 1 | 0 | 1 |
| `Trail` | int<br>0.01% | Average trail length relative to window height. Faster drops have longer trails. | 8000 | 0 | 32768 |
| `MinTrail` | int<br>chars | Minimum trail length. | 6 | 0 | 32768 |
| `Gauss` | int<br>pixels | Gaussian blur factor. | 12 | 1 | 256 |
| `Linear` | int<br>pixels | Linear blur factor. | 3 | 1 | 32768 |
| `BlurAlpha` | int<br>0.01% | The brightness of the blur. | 5000 | 0 | INT_MAX |
| `BlurPow` | int<br>0.01% | The power applied to the blur. Higher values result in a sharper blur. | 10000 | 0 | INT_MAX |
| `Noise` | int<br>0.01% | The strength of blur noise. | 2000 | 0 | 10000 |
| `NoisePow` | int<br>0.01% | The power applied to the blur noise. | 20000 | 0 | 32768 |
| `ColorX` | int<br>0.01% | X resolution of the color map. | 1 | 1 | 32 |
| `ColorY` | int<br>0.01% | Y resolution of the color map. | 1 | 1 | 32 |
| `ColorZ` | int<br>0.01% | Z resolution of the color map. | 1 | 1 | 32 |
| `R{X}.{Y}.{Z}` | int | Red component of the color map pixel.<br>See [Color map](#color-map). | | 0 | 255 |
| `G{X}.{Y}.{Z}` | int | Green component of the color map pixel.<br>See [Color map](#color-map). | | 0 | 255 |
| `B{X}.{Y}.{Z}` | int | Blue component of the color map pixel.<br>See [Color map](#color-map). | | 0 | 255 |
| `Hex{X}.{Y}.{Z}` | hex color | Color map pixel.<br>See [Color map](#color-map). | | | |
| `KeyFullscreen` | int<br>GLFW key | Keybinding for fullscreen mode. | 300 (F11) | | |
| `KeyShowMouse` | int<br>GLFW key | Keybinding to show the mouse (which can be hidden via `HideMouse`). | 256 (Escape) | | |
| `KeyQuit` | int<br>GLFW key | Keybinding to quit the application. | 81 (Q) | | |

*All keys are case-insensitive. You can also add spaces and dashes (`-`) anywhere within a key.* \
*Boolean fields are represented as integers (`0` or `1`) rather than `false` or `true`.* \
*Default values and MAX_COLOR_RES_\* can be modified in `src/config.c`.*

### Color map
The color map is a `ColorMapResX * ColorMapResY * (ColorMapResZ + 1)` 3D texture used to define character colors. \
Its X and Y dimensions match the character position, and the Z dimension represents brightness. An extra Z layer is used for active characters.

Default color map:
```
ColorResX=1
ColorResY=1
ColorResZ=1
Hex0.0.0=30D018
Hex0.0.1=D0FFC0 # Active characters
```

## Fonts
![embed/font.png](embed/font.png) \
*`embed/font.png`*

Font is a MSDF (Multi-channel Signed Distance Field) and must be PNG images. \
Glyphs should be laid out from left to right and top to bottom. \
If you provide custom font, you should also provide the total number of glyphs and the number of columns. \
Unused image space (free from glyphs) can contain anything.

## Code structure
+ `src/`: C sources of the project.
    + `src/ematrix.c`: ematrix itself.
    + `src/config.c`: Some configurable constants.
    + `src/lib/`: External libraries.
+ `embed/`: All files inside are embeded into the executable during build.
