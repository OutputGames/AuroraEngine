#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "postprocess.hpp"
#include "utils/utils.hpp"

struct CameraBase;
struct DeferredLightingData;

struct Texture
{
    unsigned int ID;

    std::string type;

    bool isCubemap;

    std::string path;

    int height, width;

    AURORA_API static Texture Load(std::string path, bool flip=false);

    AURORA_API virtual void Unload();
};

struct CubemapTexture : Texture
{

    static CubemapTexture Load(std::vector<std::string> faces);
    static CubemapTexture LoadFromPath(std::string p);

};

struct RenderTexture : Texture
{
    static RenderTexture* Load(int w, int h, CameraBase* cam);

    AURORA_API void Resize(int w, int h);

    AURORA_API void Unload() override;

    void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, fbo); }

    static void Unbind(int prebuf) { glBindFramebuffer(GL_FRAMEBUFFER, prebuf); };

    void Render(bool igv);

    DeferredLightingData* GetLightingData() { return lightingData; }

    AURORA_API GLuint GetID();

private:

    void Regenerate(int w, int h);

    GLuint fbo=0, rbo=0;
    GLuint RealID;
    DeferredLightingData* lightingData=nullptr;
    GraphicsPostProcessor* post_processing;

    friend class CameraBase;

    CameraBase* camera;

};

#endif