#define MAX_COLOR_RES_X 32 // maximum X resolution of color map, unit: pixels
#define MAX_COLOR_RES_Y 32 // maximum Y resolution of color map, unit: pixels
#define MAX_COLOR_RES_Z 32 // maximum Z resolution of color map, unit: pixels

// Default config values
static int cLogLevel = 2; // how much logs to print, 0:no, 1:errors, 2:errors+version, 3:verbose
static int cFullscreen = -1; // monitor to open in fullscreen on, -1:noFullscreen
static int cFullscreenW = 0; // width of fullscreen, 0:widthOfScreen
static int cFullscreenH = 0; // height of fullscreen, 0:heightOfScreen
static int cResizable = 1; // is window resizeable (also controls fullscreen toggling), boolean
static int cAutoIconify = 0; // auto iconify when clicked outside fullscreen window, boolean
static int cHideMouse = 0; // hide mouse (KeyShowMouse to show it again, click to hide again), boolean
static int cW = 800; // initial window width, unit: pixels
static int cH = 600; // initial window height, unit: pixels
static int cCharSize = 20; // size of one char, unit: pixels
static int cBorder = 4; // gap from window borders, unit: pixels
static int cStretch = 1; // allow glyph stretching for ideal window fit, boolean
static int cVSync = 1; // enable vsync, boolean
static int cFontNumGlyphs = 57; // number of glyphs in font file
static int cFontNumCols = 8; // number of columns in font file
static int cMinRandPeriod = 8; // period between glyph randomizing, unit: frames
static int cMaxRandPeriod = 16; // ^ the same as cMinGlyphPeriod ^
static int cPowMulRandPeriod = 5; // ^ the same as cMinGlyphPeriod ^
static int cPowDivRandPeriod = 4; // ^ the same as cMinGlyphPeriod ^
static int cPowRandPeriod = 8; // ^ the same as cMinGlyphPeriod ^
static int cMinDropPeriod = 200; // how slow drops are, unit: 0.01 frames/char
static int cMaxDropPeriod = 400; // ^ the same as cMinDropPeriod ^
static int cOnlyIntPeriod = 1; // round drop period to int, boolean
static int cTrailLen = 8000; // average length of drop trail, unit: 0.01% from window height, faster drops will have longer trail
static int cMinTrailLen = 6; // ^ minmal trail length, unit: chars, hint: combine this option with TrailLen=0 to set exact value
static int cBlurGauss = 12; // gaussian blur factor, unit: pixels
static int cBlurLinear = 3; // linear blur factor, unit: pixels
static int cBlurAlpha = 5000; // how bright blur is, unit: 0.01%
static int cBlurPow = 10000; // power to apply on blur texture, unit: 0.01%
static int cBlurNoise = 2000; // how noise blur is, unit: 0.01%
static int cColorResX = 1; // resolution of color map by X
static int cColorResY = 1; // resolution of color map by Y
static int cColorResZ = 1; // resolution of color map by Z
#define DEFAULT_COLORS SetColorMap(0, 0, 0, 0x30, 0xD0, 0x18); SetColorMap(0, 0, 1, 0xD0, 0xFF, 0xC0); // colors of chars by three dimensions: X and Y are position of char and Z is brightness, unit: pair (bg and fg) of RGB
static int cKeyFullscreen = GLFW_KEY_F11; // keyboard shortcut to toggle fullscreen
static int cKeyShowMouse = GLFW_KEY_ESCAPE; // keyboard shortcut to show mouse fullscreen
static int cKeyQuit = GLFW_KEY_Q; // keyboard shortcut to quit the app
static char* cFont = ""; // path to font, "":embedded
static char* cTitle = "ematrix"; // title of window

// Idk why may you want to change these three
#define GL_INFOLOG_BUFFER_SIZE 4096 // maximum size of OpenGL error log
#define BUFFER_INIT_SIZE 4096 // initial size of dynamically allocated buffer, min: 1
#define BUFFER_RESIZE_MULTIPLY 2 // how fast the buffer is growing, min: 2
