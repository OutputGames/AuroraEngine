#ifndef POST_PROCESS
#define POST_PROCESS

#include "rendering/shader.hpp"
#include "utils/utils.hpp"

struct DeferredLightingData;

struct SSAOProcessor
{

    SSAOProcessor(int w, int h);

    GLuint ssaoBuffer;

    GLuint GenerateSSAOTex(DeferredLightingData* lighting_data, mat4 projection);

    void Resize(int w, int h);

    void Unload();

    void Regenerate(int w, int h);

    GLuint GetSSAOTex();

private:
    GLuint outTex, blurTex;
    GLuint blurFBO;
    GLuint noiseTex;
    vector<vec3> kernel;
};

struct BloomProcessor
{

    struct BloomMipMap
    {
        vec2 size;
        ivec2 intSize;
        GLuint texture;
    };

    struct BloomFBO
    {

        BloomFBO(int w, int h, int chainLength);

        void Bind();

        vector<BloomMipMap*> GetMipChain() { return MipChain; }

        void Unload();

        void Resize(int w, int h) { Regenerate(w, h); };

    private:

        void Regenerate(int w, int h);
        int chainLength;
        GLuint FBO=0;
        vector<BloomMipMap*> MipChain;
    };

    BloomProcessor(int w, int h);
    void Unload();

    GLuint RenderBloom(GLuint source, float filterRadius);
    GLuint GetTexture();


    void Resize(int w, int h);

private:

    void RenderDownsamples(GLuint source);
    void RenderUpsamples(float filterRadius);


    BloomFBO* FBO;
    ivec2 srcViewportSize;
    vec2 srcViewportSizeFlt;
    Shader* downsampleShader;
	Shader* upsampleShader;
};

struct GraphicsPostProcessor
{

    SSAOProcessor* ssao_processor=nullptr;
    BloomProcessor* bloom_processor;

    GraphicsPostProcessor(int w, int h);

    void Resize(int w, int h);

};


#endif
