#include "OpenGLRenderer.hpp"
#include "IWindowSystem.hpp"
#include "PaletteManager.hpp"

#include <cstring>
#include <cstddef>
#include <cstdio>
#include <cmath>
#include <vector>

// ---------------------------------------------------------------------------
// Minimal OpenGL declarations.
//
// We deliberately avoid including any GL header so this compiles everywhere
// (Linux/Windows/Android/Emscripten). Types and constants follow the GL 2.1 /
// GLES 2.0 ABI, which is identical across those platforms. Entry points are
// resolved at runtime via OpenGLRenderer::SetProcLoader().
// ---------------------------------------------------------------------------
namespace {

using GLenum = unsigned int;
using GLbitfield = unsigned int;
using GLuint = unsigned int;
using GLint = int;
using GLsizei = int;
using GLfloat = float;
using GLchar = char;
using GLboolean = unsigned char;
using GLsizeiptr = std::ptrdiff_t;

constexpr GLenum GL_NO_ERROR = 0;
constexpr GLenum GL_TRIANGLES = 0x0004;
constexpr GLenum GL_TRIANGLE_FAN = 0x0006;
constexpr GLenum GL_SRC_ALPHA = 0x0302;
constexpr GLenum GL_ONE_MINUS_SRC_ALPHA = 0x0303;
constexpr GLenum GL_FRONT = 0x0404;
constexpr GLenum GL_BACK = 0x0405;
constexpr GLenum GL_ONE = 1;
constexpr GLenum GL_ZERO = 0;
constexpr GLenum GL_TEXTURE_2D = 0x0DE1;
constexpr GLenum GL_UNSIGNED_BYTE = 0x1401;
constexpr GLenum GL_UNSIGNED_SHORT = 0x1403;
constexpr GLenum GL_FLOAT = 0x1406;
constexpr GLenum GL_RGBA = 0x1908;
constexpr GLenum GL_TEXTURE0 = 0x84C0;
constexpr GLenum GL_ARRAY_BUFFER = 0x8892;
constexpr GLenum GL_DYNAMIC_DRAW = 0x88E8;
constexpr GLenum GL_COMPILE_STATUS = 0x8B81;
constexpr GLenum GL_LINK_STATUS = 0x8B82;
constexpr GLenum GL_INFO_LOG_LENGTH = 0x8B84;
constexpr GLenum GL_VERTEX_SHADER = 0x8B31;
constexpr GLenum GL_FRAGMENT_SHADER = 0x8B30;
constexpr GLenum GL_LINEAR = 0x2601;
constexpr GLenum GL_NEAREST = 0x2600;
constexpr GLenum GL_CLAMP_TO_EDGE = 0x812F;
constexpr GLenum GL_TEXTURE_MIN_FILTER = 0x2801;
constexpr GLenum GL_TEXTURE_MAG_FILTER = 0x2800;
constexpr GLenum GL_TEXTURE_WRAP_S = 0x2802;
constexpr GLenum GL_TEXTURE_WRAP_T = 0x2803;
constexpr GLenum GL_BLEND = 0x0BE2;
constexpr GLenum GL_DEPTH_TEST = 0x0B71;
constexpr GLenum GL_CULL_FACE = 0x0B44;
constexpr GLbitfield GL_COLOR_BUFFER_BIT = 0x00004000;
constexpr GLenum GL_UNPACK_ALIGNMENT = 0x0CF5;
constexpr GLenum GL_PACK_ALIGNMENT = 0x0D05;
constexpr GLenum GL_RGB = 0x1907;

// GL entry points (resolved at runtime).
struct GLFunctions {
    void (*glGenTextures)(GLsizei, GLuint*);
    void (*glDeleteTextures)(GLsizei, const GLuint*);
    void (*glBindTexture)(GLenum, GLuint);
    void (*glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
    void (*glTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*);
    void (*glTexParameteri)(GLenum, GLenum, GLint);
    void (*glActiveTexture)(GLenum);
    void (*glPixelStorei)(GLenum, GLint);
    void (*glReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*);
    void (*glReadBuffer)(GLenum);

    void (*glGenBuffers)(GLsizei, GLuint*);
    void (*glDeleteBuffers)(GLsizei, const GLuint*);
    void (*glBindBuffer)(GLenum, GLuint);
    void (*glBufferData)(GLenum, GLsizeiptr, const void*, GLenum);

    void (*glEnableVertexAttribArray)(GLuint);
    void (*glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
    void (*glUseProgram)(GLuint);

    GLuint (*glCreateShader)(GLenum);
    void (*glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*);
    void (*glCompileShader)(GLuint);
    void (*glGetShaderiv)(GLuint, GLenum, GLint*);
    void (*glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
    void (*glDeleteShader)(GLuint);

    GLuint (*glCreateProgram)();
    void (*glAttachShader)(GLuint, GLuint);
    void (*glLinkProgram)(GLuint);
    void (*glGetProgramiv)(GLuint, GLenum, GLint*);
    void (*glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
    void (*glDeleteProgram)(GLuint);

    GLint (*glGetUniformLocation)(GLuint, const GLchar*);
    GLint (*glGetAttribLocation)(GLuint, const GLchar*);
    void (*glUniform1i)(GLint, GLint);
    void (*glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*);

    void (*glDrawArrays)(GLenum, GLint, GLsizei);

    void (*glBlendFunc)(GLenum, GLenum);
    void (*glEnable)(GLenum);
    void (*glDisable)(GLenum);
    void (*glClear)(GLbitfield);
    void (*glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat);
    void (*glViewport)(GLint, GLint, GLsizei, GLsizei);
    GLenum (*glGetError)();
};

GLFunctions g_gl = {};
void* (*g_procLoader)(const char* name) = nullptr;

template <typename T>
void LoadProc(T& slot, const char* name) {
    slot = reinterpret_cast<T>(g_procLoader(name));
}

const char* const kVertexShaderSrc =
    "attribute vec4 a_position;\n"
    "attribute vec4 a_color;\n"
    "attribute vec2 a_texCoord;\n"
    "uniform mat4 u_projMatrix;\n"
    "varying vec4 v_color;\n"
    "varying vec2 v_texCoord;\n"
    "void main() {\n"
    "    gl_Position = u_projMatrix * vec4(a_position.xy, 0.0, 1.0);\n"
    "    v_color = a_color.bgra;\n"
    "    v_texCoord = a_texCoord;\n"
    "}\n";

const char* const kFragmentShaderSrc =
    "#ifdef GL_ES\n"
    "precision mediump float;\n"
    "#endif\n"
    "varying vec4 v_color;\n"
    "varying vec2 v_texCoord;\n"
    "uniform sampler2D u_texture;\n"
    "uniform int u_useTexture;\n"
    "uniform int u_blendMode;\n"
    "void main() {\n"
    "    vec4 texColor = (u_useTexture != 0) ? texture2D(u_texture, v_texCoord) : vec4(1.0);\n"
    "    vec4 finalColor;\n"
    "    if (u_blendMode == 0) {\n"
    "        finalColor = vec4(texColor.rgb, 1.0) * v_color;\n"
    "    } else {\n"
    "        finalColor = texColor * v_color;\n"
    "        if (finalColor.a < 0.05) discard;\n"
    "    }\n"
    "    gl_FragColor = finalColor;\n"
    "}\n";

bool LoadAllGL() {
    if (!g_procLoader) {
        return false;
    }
    LoadProc(g_gl.glGenTextures, "glGenTextures");
    LoadProc(g_gl.glDeleteTextures, "glDeleteTextures");
    LoadProc(g_gl.glBindTexture, "glBindTexture");
    LoadProc(g_gl.glTexImage2D, "glTexImage2D");
    LoadProc(g_gl.glTexSubImage2D, "glTexSubImage2D");
    LoadProc(g_gl.glTexParameteri, "glTexParameteri");
    LoadProc(g_gl.glActiveTexture, "glActiveTexture");
    LoadProc(g_gl.glPixelStorei, "glPixelStorei");
    LoadProc(g_gl.glReadPixels, "glReadPixels");
    LoadProc(g_gl.glReadBuffer, "glReadBuffer");

    LoadProc(g_gl.glGenBuffers, "glGenBuffers");
    LoadProc(g_gl.glDeleteBuffers, "glDeleteBuffers");
    LoadProc(g_gl.glBindBuffer, "glBindBuffer");
    LoadProc(g_gl.glBufferData, "glBufferData");

    LoadProc(g_gl.glEnableVertexAttribArray, "glEnableVertexAttribArray");
    LoadProc(g_gl.glVertexAttribPointer, "glVertexAttribPointer");
    LoadProc(g_gl.glUseProgram, "glUseProgram");

    LoadProc(g_gl.glCreateShader, "glCreateShader");
    LoadProc(g_gl.glShaderSource, "glShaderSource");
    LoadProc(g_gl.glCompileShader, "glCompileShader");
    LoadProc(g_gl.glGetShaderiv, "glGetShaderiv");
    LoadProc(g_gl.glGetShaderInfoLog, "glGetShaderInfoLog");
    LoadProc(g_gl.glDeleteShader, "glDeleteShader");

    LoadProc(g_gl.glCreateProgram, "glCreateProgram");
    LoadProc(g_gl.glAttachShader, "glAttachShader");
    LoadProc(g_gl.glLinkProgram, "glLinkProgram");
    LoadProc(g_gl.glGetProgramiv, "glGetProgramiv");
    LoadProc(g_gl.glGetProgramInfoLog, "glGetProgramInfoLog");
    LoadProc(g_gl.glDeleteProgram, "glDeleteProgram");

    LoadProc(g_gl.glGetUniformLocation, "glGetUniformLocation");
    LoadProc(g_gl.glGetAttribLocation, "glGetAttribLocation");
    LoadProc(g_gl.glUniform1i, "glUniform1i");
    LoadProc(g_gl.glUniformMatrix4fv, "glUniformMatrix4fv");

    LoadProc(g_gl.glDrawArrays, "glDrawArrays");

    LoadProc(g_gl.glBlendFunc, "glBlendFunc");
    LoadProc(g_gl.glEnable, "glEnable");
    LoadProc(g_gl.glDisable, "glDisable");
    LoadProc(g_gl.glClear, "glClear");
    LoadProc(g_gl.glClearColor, "glClearColor");
    LoadProc(g_gl.glViewport, "glViewport");
    LoadProc(g_gl.glGetError, "glGetError");

    return g_gl.glCreateProgram && g_gl.glDrawArrays && g_gl.glBindTexture;
}

GLuint CompileShader(GLenum type, const char* src) {
    GLuint shader = g_gl.glCreateShader(type);
    g_gl.glShaderSource(shader, 1, &src, nullptr);
    g_gl.glCompileShader(shader);
    GLint status = 0;
    g_gl.glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        char infoLog[1024] = {};
        GLsizei logLen = 0;
        if (g_gl.glGetShaderInfoLog) {
            g_gl.glGetShaderInfoLog(shader, sizeof(infoLog), &logLen, infoLog);
        }
        std::fprintf(stderr, "[OpenGL] Shader compilation failed (type 0x%X): %s\n", type, infoLog);
        g_gl.glDeleteShader(shader);
        return 0;
    }
    return shader;
}

} // namespace

namespace gta2::hal {

void OpenGLRenderer::SetProcLoader(void* (*pLoader)(const char* name)) {
    g_procLoader = pLoader;
}

bool OpenGLRenderer::Initialize(IWindowSystem* pWindow) {
    mWindow = pWindow;
    mReady = false;

    if (!LoadAllGL()) {
        return false;
    }

    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVertexShaderSrc);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFragmentShaderSrc);
    if (!vs || !fs) {
        if (vs) g_gl.glDeleteShader(vs);
        if (fs) g_gl.glDeleteShader(fs);
        return false;
    }

    mProgram = g_gl.glCreateProgram();
    g_gl.glAttachShader(mProgram, vs);
    g_gl.glAttachShader(mProgram, fs);
    g_gl.glLinkProgram(mProgram);

    GLint linked = 0;
    g_gl.glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
    g_gl.glDeleteShader(vs);
    g_gl.glDeleteShader(fs);
    if (linked == 0) {
        g_gl.glDeleteProgram(mProgram);
        mProgram = 0;
        return false;
    }

    mPositionAttrib = g_gl.glGetAttribLocation(mProgram, "a_position");
    mColorAttrib = g_gl.glGetAttribLocation(mProgram, "a_color");
    mTexCoordAttrib = g_gl.glGetAttribLocation(mProgram, "a_texCoord");
    mProjUniform = g_gl.glGetUniformLocation(mProgram, "u_projMatrix");
    mTextureUniform = g_gl.glGetUniformLocation(mProgram, "u_texture");
    mUseTextureUniform = g_gl.glGetUniformLocation(mProgram, "u_useTexture");
    mBlendModeUniform = g_gl.glGetUniformLocation(mProgram, "u_blendMode");

    g_gl.glGenBuffers(1, &mVbo);

    g_gl.glDisable(GL_DEPTH_TEST);
    g_gl.glDisable(GL_CULL_FACE);
    g_gl.glEnable(GL_BLEND);
    g_gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mReady = true;
    return true;
}

void OpenGLRenderer::Shutdown() {
    if (!mReady) {
        return;
    }
    for (auto& entry : mTextures) {
        g_gl.glDeleteTextures(1, &entry.second.glId);
    }
    mTextures.clear();
    if (mVbo) {
        g_gl.glDeleteBuffers(1, &mVbo);
        mVbo = 0;
    }
    if (mProgram) {
        g_gl.glDeleteProgram(mProgram);
        mProgram = 0;
    }
    mReady = false;
}

void OpenGLRenderer::SetViewport(s32 x, s32 y, s32 width, s32 height) {
    if (!mReady) {
        return;
    }
    s32 winW = 640, winH = 480;
    if (mWindow) {
        mWindow->GetWindowSize(winW, winH);
    }
    if (winW <= 0) winW = 1;
    if (winH <= 0) winH = 1;

    float scale = std::min(static_cast<float>(winW) / 640.0f, static_cast<float>(winH) / 480.0f);
    if (scale <= 0.0f) scale = 1.0f;
    GLint baseVpX = (winW - static_cast<GLint>(640.0f * scale)) / 2;
    GLint baseVpY = (winH - static_cast<GLint>(480.0f * scale)) / 2;

    GLint vpX = baseVpX + static_cast<GLint>(static_cast<float>(x) * scale);
    GLint vpY = baseVpY + static_cast<GLint>(static_cast<float>(480 - (y + height)) * scale);
    GLsizei vpW = static_cast<GLsizei>(static_cast<float>(width) * scale);
    GLsizei vpH = static_cast<GLsizei>(static_cast<float>(height) * scale);

    g_gl.glViewport(vpX, vpY, vpW, vpH);
}

void OpenGLRenderer::BeginScene() {
    if (!mReady) {
        return;
    }
    g_gl.glUseProgram(mProgram);

    s32 winW = 640, winH = 480;
    if (mWindow) {
        mWindow->GetWindowSize(winW, winH);
    }
    if (winW <= 0) winW = 1;
    if (winH <= 0) winH = 1;

    float scale = std::min(static_cast<float>(winW) / 640.0f, static_cast<float>(winH) / 480.0f);
    if (scale <= 0.0f) scale = 1.0f;
    GLsizei vpW = static_cast<GLsizei>(640.0f * scale);
    GLsizei vpH = static_cast<GLsizei>(480.0f * scale);
    GLint vpX = (winW - vpW) / 2;
    GLint vpY = (winH - vpH) / 2;

    g_gl.glViewport(vpX, vpY, vpW, vpH);

    // Fixed 640x480 orthographic projection mapping virtual coordinates to clip space.
    if (mProjUniform >= 0) {
        const GLfloat sx = 2.0f / 640.0f;
        const GLfloat sy = -2.0f / 480.0f;
        const GLfloat proj[16] = {
            sx, 0.0f, 0.0f, 0.0f,
            0.0f, sy, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 1.0f,
        };
        g_gl.glUniformMatrix4fv(mProjUniform, 1, 0, proj);
    }
    if (mTextureUniform >= 0) {
        g_gl.glUniform1i(mTextureUniform, 0);
    }
    if (mBlendModeUniform >= 0) {
        g_gl.glUniform1i(mBlendModeUniform, 0);
    }
}

void OpenGLRenderer::EndScene() {
    // Immediate-mode renderer: nothing is batched, so there is nothing to flush.
}

void OpenGLRenderer::Clear(u32 colorARGB) {
    if (!mReady) {
        return;
    }
    const GLfloat r = static_cast<GLfloat>((colorARGB >> 16) & 0xFF) / 255.0f;
    const GLfloat g = static_cast<GLfloat>((colorARGB >> 8) & 0xFF) / 255.0f;
    const GLfloat b = static_cast<GLfloat>(colorARGB & 0xFF) / 255.0f;
    const GLfloat a = static_cast<GLfloat>((colorARGB >> 24) & 0xFF) / 255.0f;
    g_gl.glClearColor(r, g, b, a);
    g_gl.glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLRenderer::DrawPrimitive(const HalVertex* verts, int count, unsigned int mode,
                                   TextureHandle texture, u32 flags) {
    if (!mReady || !verts || count <= 0) {
        return;
    }

    auto it = mTextures.find(texture);
    const bool hasTexture = (it != mTextures.end());

    // Normalize texture coordinates from pixel space [0, texW]x[0, texH] to [0.0, 1.0]
    std::vector<HalVertex> normVerts(verts, verts + count);
    if (hasTexture && it->second.width > 0 && it->second.height > 0) {
        const float invW = 1.0f / static_cast<float>(it->second.width);
        const float invH = 1.0f / static_cast<float>(it->second.height);
        for (int i = 0; i < count; ++i) {
            normVerts[i].u *= invW;
            normVerts[i].v *= invH;
        }
    }

    if (mUseTextureUniform >= 0) {
        g_gl.glUniform1i(mUseTextureUniform, hasTexture ? 1 : 0);
    }
    if (hasTexture) {
        g_gl.glActiveTexture(GL_TEXTURE0);
        g_gl.glBindTexture(GL_TEXTURE_2D, it->second.glId);
    } else {
        g_gl.glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (flags & 0x200) {
        // Additive blending
        if (mBlendModeUniform >= 0) g_gl.glUniform1i(mBlendModeUniform, 2);
        g_gl.glEnable(GL_BLEND);
        g_gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    } else if ((flags & 0x180) || (flags & 0x80)) {
        // Alpha blending / transparent sprites
        if (mBlendModeUniform >= 0) g_gl.glUniform1i(mBlendModeUniform, 1);
        g_gl.glEnable(GL_BLEND);
        g_gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        // Opaque drawing
        if (mBlendModeUniform >= 0) g_gl.glUniform1i(mBlendModeUniform, 0);
        g_gl.glDisable(GL_BLEND);
    }

    g_gl.glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    g_gl.glBufferData(GL_ARRAY_BUFFER,
                      static_cast<GLsizeiptr>(count) * static_cast<GLsizeiptr>(sizeof(HalVertex)),
                      normVerts.data(), GL_DYNAMIC_DRAW);

    const GLsizei stride = static_cast<GLsizei>(sizeof(HalVertex));
    g_gl.glEnableVertexAttribArray(static_cast<GLuint>(mPositionAttrib));
    g_gl.glVertexAttribPointer(static_cast<GLuint>(mPositionAttrib), 4, GL_FLOAT, 0, stride,
                               reinterpret_cast<const void*>(0));
    g_gl.glEnableVertexAttribArray(static_cast<GLuint>(mColorAttrib));
    g_gl.glVertexAttribPointer(static_cast<GLuint>(mColorAttrib), 4, GL_UNSIGNED_BYTE, 1, stride,
                               reinterpret_cast<const void*>(offsetof(HalVertex, diff)));
    g_gl.glEnableVertexAttribArray(static_cast<GLuint>(mTexCoordAttrib));
    g_gl.glVertexAttribPointer(static_cast<GLuint>(mTexCoordAttrib), 2, GL_FLOAT, 0, stride,
                               reinterpret_cast<const void*>(offsetof(HalVertex, u)));

    g_gl.glDrawArrays(mode, 0, static_cast<GLsizei>(count));
}

void OpenGLRenderer::DrawQuad(const HalVertex* verts, TextureHandle texture, u32 flags, u32 /*diffuseColor*/) {
    DrawPrimitive(verts, 4, GL_TRIANGLE_FAN, texture, flags);
}

void OpenGLRenderer::DrawTriangle(const HalVertex* verts, TextureHandle texture, u32 flags, u32 /*diffuseColor*/) {
    DrawPrimitive(verts, 3, GL_TRIANGLES, texture, flags);
}

static void CheckTextureDiagnostics(TextureHandle handle, u16 width, u16 height, u8 bpp, const void* pixels, size_t pixelCount, const char* op) {
    if (width == 0 || height == 0) {
        std::printf("[DIAG-TEX] %s: Empty texture handle=%u (0x0)\n", op, handle);
        std::fflush(stdout);
        return;
    }
    if (!pixels) {
        std::printf("[DIAG-TEX] %s: NULL pixels handle=%u (%dx%d, bpp=%d)\n", op, handle, width, height, bpp);
        std::fflush(stdout);
        return;
    }
    size_t nonZeroCount = 0;
    const u32* p32 = static_cast<const u32*>(pixels);
    for (size_t i = 0; i < pixelCount; ++i) {
        if (p32[i] != 0) {
            nonZeroCount++;
        }
    }
    if (nonZeroCount == 0) {
        std::printf("[DIAG-TEX] %s: All-black texture handle=%u (%dx%d, bpp=%d)\n", op, handle, width, height, bpp);
        std::fflush(stdout);
    } else {
        static int sLoadedLogCount = 0;
        if (sLoadedLogCount < 30) {
            sLoadedLogCount++;
            std::printf("[DIAG-TEX] %s: handle=%u (%dx%d, bpp=%d, nonZero=%zu/%zu)\n", op, handle, width, height, bpp, nonZeroCount, pixelCount);
            std::fflush(stdout);
        }
    }
}

TextureHandle OpenGLRenderer::RegisterTexture(const TextureDesc& desc) {
    if (!mReady) {
        return kInvalidTextureHandle;
    }
    GLuint id = 0;
    g_gl.glGenTextures(1, &id);
    g_gl.glBindTexture(GL_TEXTURE_2D, id);
    g_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    g_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    g_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    g_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    g_gl.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const void* uploadPixels = desc.pPixels;
    std::vector<u32> rgbaBuffer;
    if (desc.pPixels && desc.bitsPerPixel == 8) {
        const size_t pixelCount = static_cast<size_t>(desc.width) * desc.height;
        rgbaBuffer.resize(pixelCount);
        const u8* src = static_cast<const u8*>(desc.pPixels);
        const size_t srcStride = (desc.width <= 256) ? 256 : desc.width;

        for (u16 y = 0; y < desc.height; ++y) {
            const u8* srcRow = src + static_cast<size_t>(y) * srcStride;
            u32* dstRow = rgbaBuffer.data() + static_cast<size_t>(y) * desc.width;
            for (u16 x = 0; x < desc.width; ++x) {
                const u8 val = srcRow[x];
                if (val == 0) {
                    dstRow[x] = 0;
                } else if (desc.pPaletteRGBA) {
                    dstRow[x] = desc.pPaletteRGBA[val];
                } else if (desc.pPalette) {
                    dstRow[x] = Convert16BitTo32Bit(desc.pPalette[val], false);
                } else {
                    dstRow[x] = 0xFF000000 | (val << 16) | (val << 8) | val;
                }
            }
        }
        uploadPixels = rgbaBuffer.data();
    }

    g_gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                      static_cast<GLsizei>(desc.width), static_cast<GLsizei>(desc.height), 0,
                      GL_RGBA, GL_UNSIGNED_BYTE, uploadPixels);

    const TextureHandle handle = mNextTextureHandle++;
    TexInfo info{};
    info.glId = id;
    info.width = desc.width;
    info.height = desc.height;
    info.bitsPerPixel = desc.bitsPerPixel;
    info.bTransparent = desc.bTransparent;
    info.pPalette = desc.pPalette;
    info.pPaletteRGBA = desc.pPaletteRGBA;
    info.pPixels = desc.pPixels;
    mTextures[handle] = info;

    return handle;
}

void OpenGLRenderer::UpdateTexture(TextureHandle handle, const void* pPixels, const u16* pPalette,
                                   const u32* pPaletteRGBA, u16 width, u16 height) {
    if (!mReady) {
        return;
    }
    auto it = mTextures.find(handle);
    if (it == mTextures.end()) {
        return;
    }
    TexInfo& info = it->second;
    if (width > 0) info.width = width;
    if (height > 0) info.height = height;
    if (pPixels) {
        info.pPixels = pPixels;
    }
    if (pPalette) {
        info.pPalette = pPalette;
    }
    if (pPaletteRGBA) {
        info.pPaletteRGBA = pPaletteRGBA;
    }
    g_gl.glBindTexture(GL_TEXTURE_2D, info.glId);
    g_gl.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const void* uploadPixels = info.pPixels;
    std::vector<u32> rgbaBuffer;
    if (info.pPixels && info.bitsPerPixel == 8) {
        const size_t pixelCount = static_cast<size_t>(info.width) * info.height;
        rgbaBuffer.resize(pixelCount);
        const u8* src = static_cast<const u8*>(info.pPixels);
        const size_t srcStride = (info.width <= 256) ? 256 : info.width;

        for (u16 y = 0; y < info.height; ++y) {
            const u8* srcRow = src + static_cast<size_t>(y) * srcStride;
            u32* dstRow = rgbaBuffer.data() + static_cast<size_t>(y) * info.width;
            for (u16 x = 0; x < info.width; ++x) {
                const u8 val = srcRow[x];
                if (val == 0) {
                    dstRow[x] = 0;
                } else if (info.pPaletteRGBA) {
                    dstRow[x] = info.pPaletteRGBA[val];
                } else if (info.pPalette) {
                    dstRow[x] = Convert16BitTo32Bit(info.pPalette[val], false);
                } else {
                    dstRow[x] = 0xFF000000 | (val << 16) | (val << 8) | val;
                }
            }
        }
        uploadPixels = rgbaBuffer.data();
    }

    g_gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                      static_cast<GLsizei>(info.width), static_cast<GLsizei>(info.height), 0,
                      GL_RGBA, GL_UNSIGNED_BYTE, uploadPixels);
}

void OpenGLRenderer::UnregisterTexture(TextureHandle handle) {
    auto it = mTextures.find(handle);
    if (it == mTextures.end()) {
        return;
    }
    if (mReady) {
        GLuint id = it->second.glId;
        g_gl.glDeleteTextures(1, &id);
    }
    mTextures.erase(it);
}

void OpenGLRenderer::SetPalette(TextureHandle texture, const u16* pPalette, const u32* pPaletteRGBA) {
    if (!mReady) {
        return;
    }
    auto it = mTextures.find(texture);
    if (it == mTextures.end()) {
        return;
    }
    UpdateTexture(texture, it->second.pPixels, pPalette, pPaletteRGBA);
}

void OpenGLRenderer::SetGamma(f32 gamma) {
    // Gamma correction is not applied in the shader path yet; accepted for API
    // compatibility. A future revision can fold it into the fragment shader.
    (void)gamma;
}

bool OpenGLRenderer::CaptureScreenshot(const char* ppmPath) {
    if (!mReady || !mWindow || !g_gl.glReadPixels) {
        std::fprintf(stderr, "[Screenshot] FAILED: mReady=%d, mWindow=%p, glReadPixels=%p\n",
                     mReady ? 1 : 0, static_cast<void*>(mWindow), reinterpret_cast<void*>(g_gl.glReadPixels));
        return false;
    }
    s32 w = 640, h = 480;
    mWindow->GetWindowSize(w, h);
    if (w <= 0 || h <= 0) {
        return false;
    }
    std::vector<u8> pixels(static_cast<size_t>(w) * h * 3);
    g_gl.glPixelStorei(GL_PACK_ALIGNMENT, 1);
    g_gl.glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // If GL_BACK was empty/black (e.g. called after buffer swap), try reading from GL_FRONT
    size_t nonZero = 0;
    for (size_t i = 0; i < pixels.size(); ++i) {
        if (pixels[i] != 0) { nonZero++; break; }
    }
    if (nonZero == 0 && g_gl.glReadBuffer) {
        g_gl.glReadBuffer(GL_FRONT);
        g_gl.glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        g_gl.glReadBuffer(GL_BACK);
    }

    FILE* fp = std::fopen(ppmPath, "wb");
    if (!fp) {
        std::fprintf(stderr, "[Screenshot] FAILED: fopen('%s') failed\n", ppmPath);
        return false;
    }
    std::fprintf(fp, "P6\n%d %d\n255\n", w, h);
    // glReadPixels reads bottom-to-top; flip vertically for PPM
    const size_t rowBytes = static_cast<size_t>(w) * 3;
    for (int y = h - 1; y >= 0; --y) {
        std::fwrite(&pixels[static_cast<size_t>(y) * rowBytes], 1, rowBytes, fp);
    }
    std::fclose(fp);
    std::fprintf(stderr, "[Screenshot] Captured %dx%d to %s\n", w, h, ppmPath);
    return true;
}

} // namespace gta2::hal