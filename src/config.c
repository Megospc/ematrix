// See README.md for these
#define MAX_COLOR_RES_X 32
#define MAX_COLOR_RES_Y 32
#define MAX_COLOR_RES_Z 32
#define DEFAULT_COLOR \
    SetColor(0, 0, 0, 0x30, 0xD0, 0x18); \
    SetColor(0, 0, 1, 0xD0, 0xFF, 0xC0);
static int cLogLevel = 2;
static int cFullscreen = -1;
static int cFullscreenW = 0;
static int cFullscreenH = 0;
static int cResizable = 1;
static int cAutoIconify = 0;
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
static int cMinRandPeriod = 8;
static int cMaxRandPeriod = 16;
static int cPowMulRandPeriod = 5;
static int cPowDivRandPeriod = 4;
static int cPowRandPeriod = 8;
static int cMinDropPeriod = 200;
static int cMaxDropPeriod = 400;
static int cOnlyIntPeriod = 1;
static int cTrailLen = 8000;
static int cMinTrailLen = 6;
static int cBlurGauss = 12;
static int cBlurLinear = 3;
static int cBlurAlpha = 5000;
static int cBlurPow = 10000;
static int cBlurNoise = 2000;
static int cColorResX = 1;
static int cColorResY = 1;
static int cColorResZ = 1;
static int cKeyFullscreen = GLFW_KEY_F11;
static int cKeyShowMouse = GLFW_KEY_ESCAPE;
static int cKeyQuit = GLFW_KEY_Q;
static char* cFont = "";
static char* cTitle = "ematrix";

// Idk why may you want to change these three
#define GL_INFOLOG_BUFFER_SIZE 4096 // maximum size of OpenGL error log
#define BUFFER_INIT_SIZE 4096 // initial size of dynamic buffer, min: 1
#define BUFFER_RESIZE_MULTIPLY 2 // how fast the buffer is growing, min: 1
