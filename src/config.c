// See README.md for these
#define MAX_COLOR_RES_X 32
#define MAX_COLOR_RES_Y 32
#define MAX_COLOR_RES_Z 32
#define DEFAULT_COLOR \
    SetColor(0, 0, 0, 0x30, 0xD0, 0x18); \
    SetColor(0, 0, 1, 0xD0, 0xFF, 0xC0);
static int cLog = 2;
static int cScreen = -1;
static int cScreenW = 0;
static int cScreenH = 0;
static int cResize = 1;
static int cAutoIcon = 0;
static int cHideMouse = 0;
static int cW = 800;
static int cH = 600;
static int cCharSize = 20;
static int cBorder = 4;
static int cStretch = 1;
static int cVSync = 1;
static int cFontNum = 57;
static int cFontCols = 8;
static int cFontRows = 8;
static int cRandMin = 8;
static int cRandMax = 16;
static int cRandMul = 5;
static int cRandDiv = 4;
static int cRandPow = 8;
static int cDropMin = 200;
static int cDropMax = 400;
static int cOnlyInt = 1;
static int cTrail = 8000;
static int cMinTrail = 6;
static int cGauss = 12;
static int cLinear = 3;
static int cBlurAlpha = 5000;
static int cBlurPow = 10000;
static int cNoise = 2000;
static int cNoisePow = 20000;
static int cColorX = 1;
static int cColorY = 1;
static int cColorZ = 1;
static int cKeyFullscreen = GLFW_KEY_F11;
static int cKeyShowMouse = GLFW_KEY_ESCAPE;
static int cKeyQuit = GLFW_KEY_Q;
static char* cFont = "embed://font.png";
static char* cTitle = "ematrix";

// Idk why may you want to change these three
#define GL_INFOLOG_BUFFER_SIZE 4096 // maximum size of OpenGL error log
#define BUFFER_INIT_SIZE 4096 // initial size of dynamic buffer, min: 1
#define BUFFER_RESIZE_MULTIPLY 2 // how fast the buffer is growing, min: 1
