#ifndef RENDER_HPP
#define RENDER_HPP

#include "utils/utils.hpp"

struct AURORA_API Material;
class Mesh;

struct AURORA_API RenderData
{
    Mesh* mesh;
    mat4 matrix = mat4(1.0);
    bool castShadow = false, useDepthMask=true, cullBack=false, deferred=true, instanced=false, editorOnly=false;
    Material* material;
    int instances;
}; 

struct AURORA_API RenderMgr {
    static void InitGraphicsDevice(string windowName, unsigned windowWidth, unsigned windowHeight);
    static void UpdateGraphicsDevice();
    static void DestroyGraphicsDevice();
    static bool CheckCloseWindow();
    static void RenderScene();
    static void RenderEngineSpace();
    static glm::vec2 GetWindowSize();
    static glm::vec2 GetSceneWinSize();
    static bool CheckMouseInputs();
    static std::vector<RenderData*> renderObjs;
    static float GetDeltaTime();
    static float GetAspect();
    static void SwapBuffers();
    static ImGuiContext* GetIMGUIContext();
};

struct AURORA_API FrameBuffer {
    FrameBuffer();
    void Bind();
    unsigned int id;
};

struct AURORA_API TextureColorBuffer : FrameBuffer
{
    TextureColorBuffer();
    void Resize(glm::vec2 size);
    void Unload();
    unsigned int texture;
    unsigned int renderBuffer;
};

#endif
