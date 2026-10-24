#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/glad.h"
#include "lib/glad.c"
#include <GLFW/glfw3.h>

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#include "config.c"

typedef struct
{
    const char* path;
    const uint8_t* start;
    const uint8_t* end;
} embed_def_t;
#include "../build/embed.c" // Automatically generated during build

#define lengthof(arr) ((int)(sizeof(arr) / sizeof(*(arr))))

static uint8_t cColor[(MAX_COLOR_RES_Z + 1) *
                       MAX_COLOR_RES_X *
                       MAX_COLOR_RES_Y * 3];

static bool gGlfwInited = false;
static GLFWwindow* gWindow = NULL;
static GLuint gVertexArray = 0;
static GLuint gProgram1 = 0;
static GLuint gProgram2 = 0;
static GLuint gProgram3 = 0;
static GLuint gProgram1uPosSize;
static GLuint gProgram2uPx;
static GLuint gProgram3uPx;
static GLuint gBuffers[3] = { 0 };
static GLuint gFramebuffers[2] = { 0 };
static GLuint gFrameTextures[2] = { 0 };
static GLuint gFontTexture = 0;
static GLuint gColorsTexture = 0;
static GLfloat* gGlyphLut = NULL; // Texture positions for each glyph
static bool gHasGaussBlur;
static bool gHasLinearBlur;
static bool gHasBlur;
static int gViewX;
static int gViewY;
static int gViewW;
static int gViewH;
static int gWindowW;
static int gWindowH;
static int gFrameW;
static int gFrameH;

typedef struct
{
    char* data;
    int size;
} buffer_t;
static buffer_t g1 = { 0 };

static void FreeBuffer(buffer_t* buf)
{
    free(buf->data);
    buf->data = NULL;
}

static char* gFragment2 = NULL;
static int gFragment2Len = 0;
static char* gFragment3 = NULL;
static int gFragment3Len = 0;

typedef struct
{
    int period;
    int ctr;
    int y;
} matrix_drop_t;

typedef struct
{
    int randctr;
    int glyph;
    int ctr;
} matrix_char_t;

static int gMatrixW = 0;
static int gMatrixH = 0;
static matrix_drop_t* gMatrixDrops = NULL;
static matrix_char_t* gMatrixChars = NULL;

typedef struct
{
    const char* const name;
    int* const value;
    const int min;
    const int max;
} int_config_def_t;

#define INT_CONFIG_DEF(name, min, max) { #name, &(c##name), min, max }
static int_config_def_t INT_CONFIG_DEFS[] = 
{
    INT_CONFIG_DEF(Log, 0, 3),
    INT_CONFIG_DEF(Screen, -1, 32678),
    INT_CONFIG_DEF(ScreenW, 0, 32678),
    INT_CONFIG_DEF(ScreenH, 0, 32678),
    INT_CONFIG_DEF(Resize, 0, 1),
    INT_CONFIG_DEF(AutoIcon, 0, 1),
    INT_CONFIG_DEF(HideMouse, 0, 1),
    INT_CONFIG_DEF(W, 1, 32768),
    INT_CONFIG_DEF(H, 1, 32768),
    INT_CONFIG_DEF(CharSize, 1, 32768),
    INT_CONFIG_DEF(Border, 0, 32768),
    INT_CONFIG_DEF(Stretch, 0, 1),
    INT_CONFIG_DEF(VSync, 0, 32678),
    INT_CONFIG_DEF(FontNum, 1, 32768),
    INT_CONFIG_DEF(FontCols, 1, 32768),
    INT_CONFIG_DEF(FontRows, 1, 32768),
    INT_CONFIG_DEF(RandMin, 1, 32768),
    INT_CONFIG_DEF(RandMax, 1, 32768),
    INT_CONFIG_DEF(RandPow, 0, 32768),
    INT_CONFIG_DEF(RandMul, 1, 32768),
    INT_CONFIG_DEF(RandDiv, 1, 32768),
    INT_CONFIG_DEF(DropMin, 1, 32768),
    INT_CONFIG_DEF(DropMax, 1, 32768),
    INT_CONFIG_DEF(OnlyInt, 0, 1),
    INT_CONFIG_DEF(Trail, 0, 32768),
    INT_CONFIG_DEF(MinTrail, 0, 32768),
    INT_CONFIG_DEF(Gauss, 1, 256),
    INT_CONFIG_DEF(Linear, 1, 32768),
    INT_CONFIG_DEF(BlurAlpha, 0, INT_MAX),
    INT_CONFIG_DEF(BlurPow, 0, INT_MAX),
    INT_CONFIG_DEF(Noise, 0, 10000),
    INT_CONFIG_DEF(NoisePow, 0, 32768),
    INT_CONFIG_DEF(ColorX, 1, MAX_COLOR_RES_X),
    INT_CONFIG_DEF(ColorY, 1, MAX_COLOR_RES_Y),
    INT_CONFIG_DEF(ColorZ, 1, MAX_COLOR_RES_Z),
    INT_CONFIG_DEF(KeyFullscreen, INT_MIN, INT_MAX),
    INT_CONFIG_DEF(KeyShowMouse, INT_MIN, INT_MAX),
};

typedef struct
{
    const char* const name;
    char** const value;
    bool doFree;
} str_config_def_t;
#define STR_CONFIG_DEF(name) { #name, &(c##name), false }
static str_config_def_t STR_CONFIG_DEFS[] = 
{
    STR_CONFIG_DEF(Font),
    STR_CONFIG_DEF(Title),
};

#define Log(fmt, ...) do { if (cLog >= 3) printf(          fmt, ##__VA_ARGS__); } while(0)
#define Err(fmt, ...) do { if (cLog >= 1) printf("ERROR: " fmt, ##__VA_ARGS__); } while(0)

#define NORETURN __attribute__((noreturn))
static NORETURN void Exit(int code)
{
    FreeBuffer(&g1);
    free(gMatrixChars);
    free(gMatrixDrops);
    free(gFragment2);
    free(gFragment3);
    for (int i = 0; i < lengthof(STR_CONFIG_DEFS); i++)
    {
        str_config_def_t* d = &STR_CONFIG_DEFS[i];
        if (d->doFree)
        {
            free(*d->value);
        }
    }
    if (gFontTexture)
    {
        glDeleteTextures(1, &gFontTexture);
    }
    if (gColorsTexture)
    {
        glDeleteTextures(1, &gColorsTexture);
    }
    if (gBuffers[0]) // Three are created in one operation
    {
        glDeleteBuffers(3, gBuffers);
    }
    if (gFramebuffers[0]) // Two are created in one operation
    {
        glDeleteFramebuffers(2, gFramebuffers);
    }
    if (gFrameTextures[0]) // Two are created in one operation
    {
        glDeleteTextures(2, gFrameTextures);
    }
    if (gProgram1)
    {
        glDeleteProgram(gProgram1);
    }
    if (gProgram2)
    {
        glDeleteProgram(gProgram2);
    }
    if (gProgram3)
    {
        glDeleteProgram(gProgram3);
    }
    if (gVertexArray)
    {
        glDeleteVertexArrays(1, &gVertexArray);
    }
    if (gWindow)
    {
        glfwDestroyWindow(gWindow);
    }
    if (gGlfwInited)
    {
        glfwTerminate();
    }
    Log("Exiting with code %d. Bye!\n", code);
    exit(code);
}
static NORETURN void Fail(void)
{
    Exit(EXIT_FAILURE);
}

#define BENCH(name) static double b##name = 0.0; static int n##name = 0
#define BENCH_START(name) do { b##name -= glfwGetTime(); } while(0)
#define BENCH_END(name)   do { b##name += glfwGetTime(); n##name++; } while(0)
#define BENCH_LOG(name)   do { if (n##name > 0) Log(#name ": %.2fms\n", \
    b##name*1000.0/(double)n##name); } while(0)

static void* Malloc(int size, const char* name)
{
    void* ret = malloc(size);
    if (!ret)
    {
        Err("Failed to allocate %d bytes for %s\n", size, name);
        Fail();
    }
    return ret;
}
static void* Realloc(void* buf, int size, const char* name)
{
    void* ret = realloc(buf, size);
    if (!ret)
    {
        Err("Failed to re-allocate %d bytes for %s\n", size, name);
        Fail();
    }
    return ret;
}

static void BufferEnsure(buffer_t* buf, int size, bool saveContent)
{
    if (buf->size < size)
    {
        if (buf->data)
        {
            while (buf->size < size)
            {
                buf->size *= BUFFER_RESIZE_MULTIPLY;
            }
            if (saveContent)
            {
                buf->data = Realloc(buf->data, buf->size, "g1");
            }
            else
            {
                buf->data = Malloc(buf->size, "g1");
            }
        }
        else
        {
            buf->size = BUFFER_INIT_SIZE;
            while (buf->size < size)
            {
                buf->size *= BUFFER_RESIZE_MULTIPLY;
            }
            buf->data = Malloc(buf->size, "g1");
        }
    }
}

static inline uint8_t* ColorAt(int x, int y, int z)
{
    return (cColor + (z*cColorY*cColorX + y*cColorX + x) * 3);
}

static inline void SetColor(int x, int y, int z, // Used by DEFAULT_COLOR
                               uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t* c = ColorAt(x, y, z);
    c[0] = r;
    c[1] = g;
    c[2] = b;
}

static bool ParseLineCompareKey(const char* a, const char* b, int l)
{
    const char* pa = a;
    const char* pb = b;
    int i = 0;
    while (l < 0 || i < l)
    {
        if (*pa == ' ' || *pa == '-')
        {
            pa++;
            continue;
        }
        if (tolower(*pa) != tolower(*pb))
        {
            return false;
        }
        if (*pa == '\0')
        {
            break;
        }
        pa++;
        pb++;
        i++;
    }
    return true;
}
static bool ParseLineCompareColorKey(const char* line, uint8_t** ret,
                                     const char* str, int k)
{
    int i = strlen(str);
    if (!ParseLineCompareKey(line, str, i))
    {
        return false;
    }
    int p[3] = { 0 };
    int j = 0;
    int n = 0;
    while (true)
    {
        char c = line[i++];
        if (c == '\0')
        {
            break;
        }
        if (c == '.')
        {
            if (n == 0)
            {
                return false;
            }
            j++;
            if (j >= 3)
            {
                return false;
            }
            n = 0;
            continue;
        }
        if (c >= '0' && c <= '9')
        {
            if (n >= 5)
            {
                return false;
            }
            p[j] *= 10;
            p[j] += (c - '0');
            n++;
            continue;
        }
        return false;
    }
    if ((j < 2) ||
        (j == 2 && n == 0))
    {
        return false;
    }
    if (p[0] > MAX_COLOR_RES_X ||
        p[1] > MAX_COLOR_RES_Y ||
        p[2] > MAX_COLOR_RES_Z)
    {
        return false;
    }
    *ret = ColorAt(p[0], p[1], p[2]) + k;
    return true;
}
static int ParseLineParseInt(const char* value, int min, int max,
                             const char* line, const char* file, int lineIndex)
{
    char* end;
    long l = strtol(value, &end, 10);
    if (*value == '\0' || *end != '\0')
    {
        Err("Invalid integer value: '%s=%s' at %s:%d\n",
            line, value, file, lineIndex);
        Fail();
    }
    if (l < (long)min || l > (long)max)
    {
        Err("%s=%ld is out of range [%d-%d] at %s:%d\n",
            line, l, min, max, file, lineIndex);
        Fail();
    }
    return (int)l;
}
static void ParseLineParseHex(const char* value, uint8_t* ret,
                              const char* line, const char* file, int lineIndex)
{
    for (int i = 0; i < 3; i++)
    {
        ret[i] = 0;
    }
    for (int i = 0; i < 6; i++)
    {
        char c = value[i];
        int j = (i / 2);
        int s = (i % 2 ? 0 : 4);
        if (c >= '0' && c <= '9')
        {
            ret[j] |= ((uint8_t)(c - '0')) << s;
            continue;
        }
        if (c >= 'a' && c <= 'f')
        {
            ret[j] |= ((uint8_t)(c - 'a' + 10)) << s;
            continue;
        }
        if (c >= 'A' && c <= 'F')
        {
            ret[j] |= ((uint8_t)(c - 'A' + 10)) << s;
            continue;
        }
        Err("Invalid hex value: '%s=%s' at %s:%d\n",
            line, value, file, lineIndex);
        Fail();
    }
}

static void ParseFile(const char* path, const char* atFile, int atLine);

static void ParseLine(char* line, const char* atFile, int atLine)
{
    const char* value = "";
    for (int i = 0; true; i++)
    {
        char c = line[i];
        if (c == '\0')
        {
            break;
        }
        if (c == '#')
        {
            line[i] = '\0';
            break;
        }
        if (c == '=')
        {
            line[i] = '\0';
            value = (line + i + 1);
            break;
        }
    }
    if (*line == '\0')
    {
        return;
    }

    if (ParseLineCompareKey(line, "Echo", -1))
    {
        printf("%s\n", value);
        return;
    }
    if (ParseLineCompareKey(line, "Include", -1))
    {
        int len = 0;
        for (int i = 0; true; i++)
        {
            char c = atFile[i];
            if (c == '/')
            {
                len = i+1;
            }
            if (c == '\0')
            {
                break;
            }
        }
        int valueLen = strlen(value);
        char* path = (char*)malloc(len + valueLen + 1);
        for (int i = 0; i < len; i++)
        {
            path[i] = atFile[i];
        }
        for (int i = 0; i <= valueLen; i++)
        {
            path[i+len] = value[i];
        }
        ParseFile(path, atFile, atLine);
        free(path);
        return;
    }
    if (ParseLineCompareKey(line, "IncludeAbs", -1))
    {
        ParseFile(value, atFile, atLine);
        return;
    }
    for (int i = 0; i < lengthof(INT_CONFIG_DEFS); i++)
    {
        int_config_def_t* d = &INT_CONFIG_DEFS[i];
        if (!ParseLineCompareKey(line, d->name, -1))
        {
            continue;
        }
        *d->value = ParseLineParseInt(value, d->min, d->max,
                                      line, atFile, atLine);
        Log("int config: %s=%d\n", d->name, *d->value);
        return;
    }
    for (int i = 0; i < lengthof(STR_CONFIG_DEFS); i++)
    {
        str_config_def_t* d = &STR_CONFIG_DEFS[i];
        if (!ParseLineCompareKey(line, d->name, -1))
        {
            continue;
        }
        if (d->doFree)
        {
            free(*d->value);
        }
        int len = (strlen(value) + 1);
        *d->value = Malloc(len, "configString");
        d->doFree = true;
        memcpy(*d->value, value, len);
        Log("str config: %s=%s\n", d->name, *d->value);
        return;
    }
    uint8_t* color;
    if (ParseLineCompareColorKey(line, &color, "R", 0) ||
        ParseLineCompareColorKey(line, &color, "G", 1) ||
        ParseLineCompareColorKey(line, &color, "B", 2))
    {
        *color = ParseLineParseInt(value, 0, 255, line, atFile, atLine);
        Log("int config: %s=%d\n", line, *color);
        return;
    }
    if (ParseLineCompareColorKey(line, &color, "Hex", 0))
    {
        ParseLineParseHex(value, color, line, atFile, atLine);
        Log("hex config: %s=%02X%02X%02X\n", line,
            color[0], color[1], color[2]);
        return;
    }
    
    Err("Invalid key: '%s' at %s:%d\n", line, atFile, atLine);
    Fail();
}

static bool CheckEmbed(const char* path,
                       const uint8_t** start, const uint8_t** end)
{
    const char* pref = "embed://";
    while (true)
    {
        if (*pref == '\0')
        {
            break;
        }
        if (*pref != *path)
        {
            return false;
        }
        pref++;
        path++;
    }
    for (int i = 0; i < lengthof(EMBED); i++)
    {
        const embed_def_t* d = &EMBED[i];
        if (strcmp(path, d->path) == 0)
        {
            *start = d->start;
            *end   = d->end;
            return true;
        }
    }
    return false;
}

static void ParseFile(const char* path, const char* atFile, int atLine)
{
    const uint8_t* embedStart = NULL;
    const uint8_t* embedEnd = NULL;
    Log(atFile ?
        "Including file: %s\n" :
        "Loading file: %s\n", path);
    FILE* file = NULL;
    if (!CheckEmbed(path, &embedStart, &embedEnd))
    {
        file = fopen(path, "r");
        if (!file)
        {
            if (atFile)
            {
                Err("Failed to include file '%s' at %s:%d\n",
                    path, atFile, atLine);
            }
            else
            {
                Err("Failed to open file '%s'\n", path);
            }
            Fail();
        }
    }
    int i = 0;
    int line = 1;
    buffer_t buf = { 0 };
    while (true)
    {
        char c;
        if (file)
        {
            if (fread(&c, 1, 1, file) == 0)
            {
                break;
            }
        }
        else
        {
            if (embedStart == embedEnd)
            {
                break;
            }
            c = (char)(*embedStart++);
        }
        BufferEnsure(&buf, i+1, (i > 0));
        if (c == '\n')
        {
            buf.data[i] = '\0';
            ParseLine(buf.data, path, line);
            line++;
            i = 0;
            continue;
        }
        buf.data[i] = c;
        i++;
    }
    if (i > 0)
    {
        BufferEnsure(&buf, i+1, true);
        buf.data[i] = '\0';
        ParseLine(buf.data, path, line);
    }
    FreeBuffer(&buf);
    if (file)
    {
        fclose(file);
    }
}

static sig_atomic_t gHasSigint = false;
static void SigHandler(int sig)
{
    if (sig == SIGINT)
    {
        gHasSigint = true;
    }
}

static GLuint CreateShader(const char* pname, const char* name,
                           GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    if (!shader)
    {
        Err("Failed to create shader %s:%s\n", pname, name);
        return 0;
    }
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    int status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        BufferEnsure(&g1, GLINFOLOG_BUFFER_SIZE, false);
        glGetShaderInfoLog(shader, g1.size, NULL, g1.data);
        glDeleteShader(shader);
        Err("Failed to compile shader %s:%s\n%s", pname, name, g1.data);
        return 0;
    }
    return shader;
}
static GLuint CreateProgram(const char* pname,
                            const char* vertSrc, const char* fragSrc)
{
    GLuint prog = glCreateProgram();
    if (!prog)
    {
        Err("Failed to create program %s\n", pname);
        Fail();
    }
    GLuint vert = CreateShader(pname, "vert", GL_VERTEX_SHADER, vertSrc);
    if (!vert)
    {
        glDeleteProgram(prog);
        Fail();
    }
    GLuint frag = CreateShader(pname, "frag", GL_FRAGMENT_SHADER, fragSrc);
    if (!frag)
    {
        glDeleteProgram(prog);
        glDeleteShader(vert);
        Fail();
    }
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    glDeleteShader(vert);
    glDeleteShader(frag);
    int status = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (!status)
    {
        BufferEnsure(&g1, GLINFOLOG_BUFFER_SIZE, false);
        glGetProgramInfoLog(prog, g1.size, NULL, g1.data);
        glDeleteProgram(prog);
        Err("Failed to link program %s\n%s", pname, g1.data);
        Fail();
    }
    return prog;
}

static void SetUniformInt(GLuint prog, const char* name, int value)
{
    glUniform1i(glGetUniformLocation(prog, name), value);
}

static void TextureParameters(void)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

static void CheckFramebuffer(const char* name)
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        Err("Framebuffer %s is not complete: %04X\n", name, status);
        Fail();
    }
}

static void GlCheckpoint(const char* name)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        Err("OpenGL error 0x%04X at %s\n", err, name);
        Fail();
    }
}

static const char VERTEX1[] =
    "#version 330 core\n"
    "layout (location = 0) in vec2 aVert;\n"
    "layout (location = 1) in vec2 aPos;\n"
    "layout (location = 2) in vec2 aTexPos;\n"
    "layout (location = 3) in float aZ;\n"
    "uniform vec2 uPosSize;\n"
    "uniform vec2 uTexPosSize;\n"
    "uniform sampler3D tColors;\n"
    "out vec2 vTexPos;\n"
    "out vec4 vColor;\n"
    "void main()\n"
    "{\n"
    "    vec2 p = aPos+aVert*uPosSize;\n"
    "    gl_Position = vec4(p*2.0-1.0, 1.0, 1.0);\n"
    "    gl_Position.y = -gl_Position.y;\n"
    "    vTexPos = aTexPos+aVert*uTexPosSize;\n"
    "    vec2 c = p*(1.0-uPosSize*1.0)+uPosSize*0.5;\n"
    "    vColor = texture(tColors, vec3(c, aZ));\n"
    "}\n";
static const char FRAGMENT1[] =
    "#version 330 core\n"
    "in vec2 vTexPos;\n"
    "in vec4 vColor;\n"
    "out vec4 fColor;\n"
    "uniform sampler2D tFont;\n"
    "void main()\n"
    "{\n"
    "    vec3 t = texture(tFont, vTexPos).rgb;\n"
    "    float s = min(max(t.r, t.g),\n"
    "                  min(max(t.g, t.b),\n"
    "                      max(t.r, t.b)));\n"
    "    fColor = vColor*s;\n"
    "}\n";

static const char VERTEX2[] =
    "#version 330 core\n"
    "layout (location = 0) in vec2 aVert;\n"
    "out vec2 vTexPos;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aVert*2.0-1.0, 1.0, 1.0);\n"
    "    vTexPos = aVert;\n"
    "}\n";

static const char VERTEX3[] =
    "#version 330 core\n"
    "layout (location = 0) in vec2 aVert;\n"
    "out vec2 vTexPos;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aVert*2.0-1.0, 1.0, 1.0);\n"
    "    vTexPos = aVert;\n"
    "}\n";

static void ShaderSourceAppend(char** source, int* len, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int s = vsnprintf(NULL, 0, fmt, va);
    va_end(va);
    int l = *len;
    *len += s;
    if (l > 0)
    {
        *source = Realloc(*source, *len + 1, "shaderSource");
    }
    else
    {
        *source = Malloc(*len + 1, "shaderSource");
    }
    va_start(va, fmt);
    int z = vsnprintf((*source + l), (s + 1), fmt, va);
    if (s != z)
    {
        Err("ShaderSourceAppend: length mismatch.\n"
            "Something is really wrong.\n");
        Fail();
    }
    va_end(va);
}

static const GLfloat QUAD_VERTS[] = 
{
    0.0F, 0.0F,
    1.0F, 0.0F,
    0.0F, 1.0F,
    1.0F, 1.0F,
    1.0F, 0.0F,
    0.0F, 1.0F,
};

static const char* GetGlfwError(void)
{
    const char* ret;
    glfwGetError(&ret);
    return ret;
}

static int Rand(int max)
{
    return rand() % max;
}
static int RandRange(int min, int max)
{
    return Rand(max - min + 1) + min;
}

static inline matrix_drop_t* MatrixDropAt(int x)
{
    return &gMatrixDrops[x];
}
static inline matrix_char_t* MatrixCharAt(int x, int y)
{
    return &gMatrixChars[x + y * gMatrixW];
}
static inline int MatrixGenRandPeriod(void)
{
    int ret = RandRange(cRandMin, cRandMax);
    int toMul = Rand(cRandPow + 1);
    int toDiv = toMul;
    while (toMul-- > 0)
    {
        while (toDiv > 0 && ret > (INT_MAX / cRandMul))
        {
            ret /= cRandDiv;
            toDiv--;
        }
        ret *= cRandMul;
    }
    while (toDiv-- > 0)
    {
        ret /= cRandDiv;
    }
    return ret;
}
static inline int MatrixGenDropPeriod(void)
{
    int ret = RandRange(cDropMin, cDropMax);
    if (cOnlyInt)
    {
        if (ret >= 75)
        {
            int d = ((ret + 50) / 100);
            ret = d * 100;
        }
        else if (ret >= 37)
        {
            ret = 50;
        }
        else
        {
            ret = 25;
        }
    }
    return ret;
}

static void MatrixResize(int w, int h)
{
    if (gMatrixW == w && gMatrixH == h)
    {
        return;
    }
    free(gMatrixDrops);
    free(gMatrixChars);
    gMatrixDrops = Malloc(w     * sizeof(matrix_drop_t), "gMatrixDrops");
    gMatrixChars = Malloc(w * h * sizeof(matrix_char_t), "gMatrixChars");
    gMatrixW = w;
    gMatrixH = h;

    for (int x = 0; x < gMatrixW; x++)
    {
        matrix_drop_t* d = MatrixDropAt(x);
        d->period = MatrixGenDropPeriod();
        d->ctr = 0;
        d->y = -Rand(gMatrixH + (gMatrixH+1)/2)-1;
    }
    for (int y = 0; y < gMatrixH; y++)
    {
        for (int x = 0; x < gMatrixW; x++)
        {
            matrix_char_t* c = MatrixCharAt(x, y);
            c->randctr = MatrixGenRandPeriod();
            c->glyph = Rand(cFontNum);
            c->ctr = 0;
        }
    }

    int size = (gMatrixW * gMatrixH * 2 * sizeof(GLfloat));
    BufferEnsure(&g1, size, false);
    GLfloat* buf = (GLfloat*)g1.data;
    for (int y = 0; y < gMatrixH; y++)
    {
        for (int x = 0; x < gMatrixW; x++)
        {
            GLfloat* b = &buf[(x + y * gMatrixW) * 2];
            b[0] = (GLfloat)x / (GLfloat)w;
            b[1] = (GLfloat)y / (GLfloat)h;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, gBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, size, g1.data, GL_DYNAMIC_DRAW);
    glUseProgram(gProgram1);
    glUniform2f(gProgram1uPosSize, 1.0F/(GLfloat)w, 1.0F/(GLfloat)h);
}

BENCH(MatrixUpdate);
static void MatrixUpdate(void)
{
    assert(gMatrixW > 0 && gMatrixH > 0);

    int trailLen = (gMatrixH * cTrail / 10000);
    if (trailLen < cMinTrail)
    {
        trailLen = cMinTrail;
    }
    int fadeTime = (trailLen * (cDropMin + cDropMax) / 200);
    for (int x = 0; x < gMatrixW; x++)
    {
        matrix_drop_t* d = MatrixDropAt(x);
        d->ctr += 100;
        while (d->ctr >= d->period)
        {
            d->ctr -= d->period;
            d->y++;
            if (d->y >= gMatrixH)
            {
                d->period = MatrixGenDropPeriod();
                d->y = -Rand((gMatrixH+1)/2)-1;
                break;
            }
            if (d->y >= 0)
            {
                matrix_char_t* c = MatrixCharAt(x, d->y);
                c->ctr = fadeTime;
            }
        }
    }

    BENCH_START(MatrixUpdate);
    if (fadeTime < 1)
    {
        fadeTime = 1;
    }
    float mulZ = 1.0F / (GLfloat)fadeTime *
                 (GLfloat)(cColorZ+1) / (GLfloat)(cColorZ+3);
    int size = (gMatrixW * gMatrixH * 3 * sizeof(GLfloat));
    BufferEnsure(&g1, size, false);
    GLfloat* buf = (GLfloat*)g1.data;
    for (int y = 0; y < gMatrixH; y++)
    {
        for (int x = 0; x < gMatrixW; x++)
        {
            GLfloat* b = &buf[(x + y * gMatrixW) * 3];
            matrix_drop_t* d = MatrixDropAt(x);
            matrix_char_t* c = MatrixCharAt(x, y);
            c->randctr--;
            if (c->randctr == 0)
            {
                c->glyph = Rand(cFontNum);
                c->randctr = MatrixGenRandPeriod();
            }
            b[0] = gGlyphLut[c->glyph*2+0];
            b[1] = gGlyphLut[c->glyph*2+1];
            if (y == d->y)
            {
                b[2] = 1.0F;
            }
            else
            {
                b[2] = (GLfloat)c->ctr * mulZ;
            }
            if (c->ctr > 0)
            {
                c->ctr--;
            }
        }
    }
    BENCH_END(MatrixUpdate);
    glBindBuffer(GL_ARRAY_BUFFER, gBuffers[2]);
    glBufferData(GL_ARRAY_BUFFER, size, g1.data, GL_DYNAMIC_DRAW);
}

static void ResizeCallback(GLFWwindow* window, int w, int h)
{
    (void)window;

    gWindowW = w;
    gWindowH = h;
    w -= cBorder*2;
    h -= cBorder*2;
    if (w < 0)
    {
        w = 0;
    }
    if (h < 0)
    {
        h = 0;
    }
    int mw = (w / cCharSize);
    int mh = (h / cCharSize);
    if (mw < 1)
    {
        mw = 1;
    }
    if (mh < 1)
    {
        mh = 1;
    }
    MatrixResize(mw, mh);

    if (cStretch)
    {
        gViewW = w;
        gViewH = h;
    }
    else
    {
        gViewW = (mw * cCharSize);
        gViewH = (mh * cCharSize);
        if (gViewW > w)
        {
            gViewW = w;
        }
        if (gViewH > h)
        {
            gViewH = h;
        }
    }
    gViewX = (gWindowW - gViewW) / 2;
    gViewY = (gWindowH - gViewH) / 2;

    if (gHasBlur)
    {
        gFrameW = (gWindowW / cLinear);
        gFrameH = (gWindowH / cLinear);
        if (gFrameW < 1)
        {
            gFrameW = 1;
        }
        if (gFrameH < 1)
        {
            gFrameH = 1;
        }

        glActiveTexture(GL_TEXTURE2);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gWindowW, gWindowH,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glBindFramebuffer(GL_FRAMEBUFFER, gFramebuffers[0]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, gFrameTextures[0], 0);
        GlCheckpoint("ResizeFramebuffer0");
        CheckFramebuffer("Framebuffer0");

        glActiveTexture(GL_TEXTURE3);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gFrameW, gFrameH,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glBindFramebuffer(GL_FRAMEBUFFER, gFramebuffers[1]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, gFrameTextures[1], 0);
        GlCheckpoint("ResizeFramebuffer1");
        CheckFramebuffer("Framebuffer1");
        
        glUseProgram(gProgram2);
        glUniform2f(gProgram2uPx, 1.0F/(GLfloat)gWindowW, 1.0F/(GLfloat)gWindowH);
        glUseProgram(gProgram3);
        glUniform2f(gProgram3uPx, 1.0F/(GLfloat)gWindowW, 1.0F/(GLfloat)gWindowH);
    }
    else
    {
        glViewport(gViewX, gViewY, gViewW, gViewH);
    }
}

static GLFWmonitor* GetMonitor(int index, int* total)
{
    GLFWmonitor** monitors = glfwGetMonitors(total);
    if (index >= *total)
    {
        return NULL;
    }
    return monitors[index];
}

static void ToggleFullscreen(GLFWmonitor* monitor)
{
    static bool isFullscreen = false;
    if (isFullscreen)
    {
        glfwSetWindowMonitor(gWindow, NULL, 0, 0, cW, cH, GLFW_DONT_CARE);
        isFullscreen = false;
    }
    else
    {
        int w = cScreenW;
        int h = cScreenH;
        if (w <= 0 || h <= 0)
        {
            const GLFWvidmode* video = glfwGetVideoMode(monitor);
            w = video->width;
            h = video->height;
        }
        glfwSetWindowMonitor(gWindow, monitor, 0, 0, w, h, GLFW_DONT_CARE);
        isFullscreen = true;
    }
}

static void HideMouse(bool hide)
{
    glfwSetInputMode(gWindow, GLFW_CURSOR, hide ?
        GLFW_CURSOR_HIDDEN :
        GLFW_CURSOR_NORMAL); 
}

static void KeyCallback(GLFWwindow* window, int key, int scancode,
                        int action, int mods)
{
    (void)window; (void)scancode; (void)mods;
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        if (key == cKeyFullscreen && cResize)
        {
            int mon = 0;
            for (int i = 0; i < 9; i++)
            {
                if (glfwGetKey(gWindow, GLFW_KEY_1 + i))
                {
                    mon = i;
                    break;
                }
            }
            int total;
            GLFWmonitor* monitor = GetMonitor(mon, &total);
            if (monitor)
            {
                ToggleFullscreen(monitor);
            }
        }
        if (key == cKeyShowMouse && cHideMouse)
        {
            HideMouse(false);
        }
        if (key == cKeyQuit)
        {
            glfwSetWindowShouldClose(gWindow, GLFW_TRUE);
        }
    }
}

static void MouseButtonCallback(GLFWwindow* window, int button,
                           int action, int mods)
{
    (void)window; (void)button; (void)mods;
    if (action == GLFW_PRESS)
    {
        if (cHideMouse)
        {
            HideMouse(true);
        }
    }
}

int main(int argc, char** argv)
{
    DEFAULT_COLOR

    for (int i = 1; i < argc; i++)
    {
        char* arg = argv[i];
        if (arg[0] == '-')
        {
            ParseLine(arg + 1, "ARG", i);
        }
        else
        {
            ParseFile(arg, NULL, 0);
        }
    }
    if (cLog >= 2)
    {
        printf("%s v1.0+dev by Megospc\n", (argc < 1 ? "ematrix" : argv[0]));
#ifndef NDEBUG
        printf("DEBUG BUILD\n");
#endif
    }

    for (int i = 0; i < lengthof(INT_CONFIG_DEFS); i++)
    {
        const int_config_def_t* d = &INT_CONFIG_DEFS[i];
        int v = *d->value;
        if (v < d->min || v > d->max)
        {
            Err("%s=%d is out of allowed range [%d-%d]\n",
                d->name, v, d->min, d->max);
            Fail();
        }
    }

    gGlyphLut = Malloc(cFontNum * 2 * sizeof(GLfloat), "gGlyphLut");
    for (int i = 0; i < cFontNum; i++)
    {
        int col = (i % cFontCols);
        int row = (i / cFontCols);
        gGlyphLut[i*2+0] = (GLfloat)col / (GLfloat)cFontCols;
        gGlyphLut[i*2+1] = (GLfloat)row / (GLfloat)cFontRows;
    }
    gHasGaussBlur  = (cGauss  > 1 && cBlurAlpha > 0);
    gHasLinearBlur = (cLinear > 1 && cBlurAlpha > 0);
    gHasBlur = (gHasGaussBlur || gHasLinearBlur);

    if (!glfwInit())
    {
        Err("Failed to init glfw: %s\n", GetGlfwError());
        Fail();
    }
    gGlfwInited = true;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, cResize);
    glfwWindowHint(GLFW_AUTO_ICONIFY, cAutoIcon);
    gWindow = glfwCreateWindow(cW, cH, cTitle, NULL, NULL);
    if (!gWindow)
    {
        Err("Failed create window: %s\n", GetGlfwError());
        Fail();
    }
    GLFWmonitor* monitor = NULL;
    if (cScreen >= 0)
    {
        int total;
        monitor = GetMonitor(cScreen, &total);
        if (monitor)
        {
            ToggleFullscreen(monitor);
        }
        else
        {
            Err("Invalid monitor %d: this device only has %d\n",
                cScreen, total);
            Fail();
        }
    }
    if (cHideMouse)
    {
        HideMouse(true);
    }
    glfwSetFramebufferSizeCallback(gWindow, ResizeCallback);
    glfwSetKeyCallback(gWindow, KeyCallback);
    glfwSetMouseButtonCallback(gWindow, MouseButtonCallback);
    glfwMakeContextCurrent(gWindow);
    glfwSwapInterval(cVSync);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        Err("Failed to load GL\n");
        Fail();
    }

    if (gHasBlur)
    {
        ShaderSourceAppend(&gFragment2, &gFragment2Len,
            "#version 330 core\n"
            "in vec2 vTexPos;\n"
            "out vec4 fColor;\n"
            "uniform vec2 uPx;\n"
            "uniform sampler2D t0;\n"
            "void main()\n"
            "{\n");

        ShaderSourceAppend(&gFragment3, &gFragment3Len,
            "#version 330 core\n"
            "in vec2 vTexPos;\n"
            "out vec4 fColor;\n"
            "uniform vec2 uPx;\n"
            "uniform sampler2D t0;\n"
            "uniform sampler2D t1;\n");

        if (cNoise > 0)
        {
            ShaderSourceAppend(&gFragment3, &gFragment3Len,
                "float rand(vec2 p)\n"
                "{\n"
                "    p = fract(p * vec2(123.34, 456.21));\n"
                "    p += dot(p, p + 45.32);\n"
                "    return fract(p.x * p.y);\n"
                "}\n");
        }
        
        ShaderSourceAppend(&gFragment3, &gFragment3Len,
            "void main()\n"
            "{\n");
        
        GLfloat ws[cGauss];
        GLfloat sw = 0.0F;
        GLfloat sigma = (GLfloat)cGauss / 3.0F;
        for (int i = 0; i < cGauss; i++)
        {
            GLfloat x = (GLfloat)i / (GLfloat)(cGauss - 1);
            GLfloat w = expf(-(x * x) / (2.0F * sigma * sigma));
            ws[i] = w;
            sw += w;
        }
        for (int i = 0; i < cGauss; i++)
        {
            GLfloat w = (ws[i] / sw);
            Log("Gauss weight%d %f\n", i, w);
            if (i == 0)
            {
                ShaderSourceAppend(&gFragment2, &gFragment2Len,
                    "    fColor = texture(t0, vTexPos) * %f", w);
                ShaderSourceAppend(&gFragment3, &gFragment3Len,
                    "    fColor = texture(t1, vTexPos) * %f", w);
                continue;
            }
            ShaderSourceAppend(&gFragment2, &gFragment2Len,
                " +\n             texture(t0, vTexPos+uPx*vec2(%d.0, 0.0)) * %f"
                " +\n             texture(t0, vTexPos-uPx*vec2(%d.0, 0.0)) * %f",
                i, w, i, w);
            ShaderSourceAppend(&gFragment3, &gFragment3Len,
                " +\n             texture(t1, vTexPos+uPx*vec2(0.0, %d.0)) * %f"
                " +\n             texture(t1, vTexPos-uPx*vec2(0.0, %d.0)) * %f",
                i, w, i, w);
        }

        ShaderSourceAppend(&gFragment2, &gFragment2Len, ";\n"
            "}\n");

        ShaderSourceAppend(&gFragment3, &gFragment3Len,
                           ";\n    fColor = texture(t0, vTexPos) + ");
        if (cBlurPow == 10000)
        {
            ShaderSourceAppend(&gFragment3, &gFragment3Len, "fColor");
        }
        else
        {
            ShaderSourceAppend(&gFragment3, &gFragment3Len,
                "pow(fColor, vec4(%f))",
                (GLfloat)cBlurPow / 10000.0F);
        }
        if (cBlurAlpha != 10000)
        {
            ShaderSourceAppend(&gFragment3, &gFragment3Len, " * %f",
                (GLfloat)cBlurAlpha / 10000.0F);
        }
        if (cNoise > 0)
        {
            float n = (GLfloat)cNoise / 10000.0F;
            ShaderSourceAppend(&gFragment3, &gFragment3Len,
                " * pow(rand(floor(vTexPos / uPx)) * %f + %f, %f)",
                n, 1.0F-n, (GLfloat)cNoisePow / 10000.0F);
        }
        ShaderSourceAppend(&gFragment3, &gFragment3Len, ";\n}\n");

        Log("Generated gFragment2:\n%s\n", gFragment2);
        Log("Generated gFragment3:\n%s\n", gFragment3);

        gProgram2 = CreateProgram("2", VERTEX2, gFragment2);
        glUseProgram(gProgram2);
        SetUniformInt(gProgram2, "t0", 2);
        gProgram2uPx = glGetUniformLocation(gProgram2, "uPx");
        GlCheckpoint("Program2");

        gProgram3 = CreateProgram("3", VERTEX3, gFragment3);
        glUseProgram(gProgram3);
        SetUniformInt(gProgram3, "t0", 2);
        SetUniformInt(gProgram3, "t1", 3);
        gProgram3uPx = glGetUniformLocation(gProgram3, "uPx");
        GlCheckpoint("Program3");

        glGenFramebuffers(2, gFramebuffers);
        GlCheckpoint("Framebuffers");
        glGenTextures(2, gFrameTextures);
        GlCheckpoint("FrameTextures");

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gFrameTextures[0]);
        TextureParameters();
        GlCheckpoint("FrameTextures0");
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gFrameTextures[1]);
        TextureParameters();
        GlCheckpoint("FrameTextures1");
    }
    gProgram1 = CreateProgram("1", VERTEX1, FRAGMENT1);
    glUseProgram(gProgram1);
    SetUniformInt(gProgram1, "tFont", 0);
    SetUniformInt(gProgram1, "tColors", 1);
    gProgram1uPosSize = glGetUniformLocation(gProgram1, "uPosSize");
    glUniform2f(glGetUniformLocation(gProgram1, "uTexPosSize"),
                1.0F/(GLfloat)cFontCols, 1.0F/(GLfloat)cFontRows);
    GlCheckpoint("Program1");

    glGenVertexArrays(1, &gVertexArray);
    glBindVertexArray(gVertexArray);
    GlCheckpoint("VertexArray");

    glGenBuffers(3, gBuffers);
    GlCheckpoint("Buffers");

    glBindBuffer(GL_ARRAY_BUFFER, gBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTS), QUAD_VERTS, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 2*sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    GlCheckpoint("Buffer0");

    glBindBuffer(GL_ARRAY_BUFFER, gBuffers[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 2*sizeof(GLfloat), (void*)0);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(1);
    GlCheckpoint("Buffer1");

    glBindBuffer(GL_ARRAY_BUFFER, gBuffers[2]);
    glVertexAttribPointer(2, 2, GL_FLOAT, false, 3*sizeof(GLfloat), (void*)0);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, false, 3*sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
    glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(3);
    GlCheckpoint("Buffer2");

    int w, h, nc;
    uint32_t* pixels = NULL;
    const uint8_t* embedStart;
    const uint8_t* embedEnd;
    if (CheckEmbed(cFont, &embedStart, &embedEnd))
    {
        pixels = (uint32_t*)stbi_load_from_memory(
            embedStart,
            (int)(embedEnd - embedStart),
            &w, &h, &nc, 4);
        Log("Loading embedded font: %s (%p:%p)\n",
            cFont, embedStart, embedEnd);
    }
    else
    {
        pixels = (uint32_t*)stbi_load(cFont, &w, &h, &nc, 4);
        Log("Loading font: %s\n", cFont);
    }
    if (!pixels)
    {
        Err("Failed to load font: %s\n"
            "Note: Only PNG is supported.\n", cFont);
        Fail();
    }
    glGenTextures(1, &gFontTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gFontTexture);
    TextureParameters();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    GlCheckpoint("FontTexture");
    free(pixels);

    BufferEnsure(&g1, cColorX * cColorY * (cColorZ + 2) * 4, false);
    uint8_t* buf = (uint8_t*)g1.data;
    for (int z = 0; z < (cColorZ + 2); z++)
    {
        for (int y = 0; y < cColorY; y++)
        {
            for (int x = 0; x < cColorX; x++)
            {
                uint8_t* b = &buf[(z*cColorY*cColorX + y*cColorX + x) * 4];
                if (z == 0)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        b[i] = 0;
                    }
                }
                else
                {
                    uint8_t* c = ColorAt(x, y, z-1);
                    for (int i = 0; i < 3; i++)
                    {
                        b[i] = c[i];
                    }
                }
                b[3] = 255;
            }
        }
    }
    glGenTextures(1, &gColorsTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, gColorsTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, cColorX, cColorY, (cColorZ + 2),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, g1.data);
    GlCheckpoint("ColorsTexture");
    
    ResizeCallback(gWindow, cW, cH);
    if (signal(SIGINT, SigHandler) == SIG_ERR)
    {
        Log("Failed to register SigHandler\n");
    }
    double start = glfwGetTime();
    int frames = 0;
    while (!glfwWindowShouldClose(gWindow))
    {
        if (gHasSigint)
        {
            glfwSetWindowShouldClose(gWindow, GLFW_TRUE);
            continue;
        }
        MatrixUpdate();

        if (gHasBlur)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, gFramebuffers[0]);
            glViewport(gViewX, gViewY, gViewW, gViewH);
            glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
            glClear(GL_COLOR_BUFFER_BIT);
            GlCheckpoint("Clear1");
            glUseProgram(gProgram1);
            glDrawArraysInstanced(GL_TRIANGLES, 0, lengthof(QUAD_VERTS),
                                  gMatrixW*gMatrixH);
            GlCheckpoint("Draw1");

            glBindFramebuffer(GL_FRAMEBUFFER, gFramebuffers[1]);
            glViewport(0, 0, gFrameW, gFrameH);
            glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
            GlCheckpoint("Clear2");
            glUseProgram(gProgram2);
            GlCheckpoint("Draw2");
            glDrawArrays(GL_TRIANGLES, 0, lengthof(QUAD_VERTS));

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, gWindowW, gWindowH);
            glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
            GlCheckpoint("Clear3");
            glUseProgram(gProgram3);
            GlCheckpoint("Draw3");
            glDrawArrays(GL_TRIANGLES, 0, lengthof(QUAD_VERTS));
        }
        else
        {
            glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
            glClear(GL_COLOR_BUFFER_BIT);
            GlCheckpoint("Clear");
            glDrawArraysInstanced(GL_TRIANGLES, 0, lengthof(QUAD_VERTS),
                                  gMatrixW*gMatrixH);
            GlCheckpoint("Draw");
        }

        glfwSwapBuffers(gWindow);
        glfwPollEvents();

        frames++;
    }
    double end = glfwGetTime();
    Log("FPS: %.2F\n", (double)frames / (end - start));
    BENCH_LOG(MatrixUpdate);
    Exit(EXIT_SUCCESS);
}
