#ifndef RENDER_HPP
#define RENDER_HPP

#include "utils/utils.hpp"

class Mesh;

struct RenderData
{
    Mesh* mesh;
    mat4 matrix = mat4(1.0);
    bool castShadow = false, useDepthMask=true, cullBack=false;
};

struct RenderMgr {
    static void InitGraphicsDevice();
    static void UpdateGraphicsDevice();
    static void DestroyGraphicsDevice();
    static bool CheckCloseWindow();
    static void RenderScene();
    static void RenderEngineSpace();
    static glm::vec2 GetWindowSize();
    static glm::vec2 GetSceneWinSize();
    static bool CheckMouseInputs();
    static std::vector<RenderData*> renderObjs;
};

struct FrameBuffer {
    FrameBuffer();
    void Bind();
    unsigned int id;
};

struct TextureColorBuffer : FrameBuffer
{
    TextureColorBuffer();
    void Resize(glm::vec2 size);
    void Unload();
    unsigned int texture;
    unsigned int renderBuffer;
};

#endif
