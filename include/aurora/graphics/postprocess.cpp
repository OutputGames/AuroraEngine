#include "postprocess.hpp"

#include <random>

#include "lighting.hpp"
#include "shape.hpp"
#include "rendering/render.hpp"
#include "rendering/shader.hpp"
#include "utils/math.hpp"

SSAOProcessor::SSAOProcessor(int w, int h)
{
    // also create framebuffer to hold SSAO processing stage 
// -----------------------------------------------------

    Regenerate(w, h);

    // generate sample kernel
    // ----------------------
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    for (int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale = Math::Lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        kernel.push_back(sample);
    }

    // generate noise texture
    // ----------------------
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    unsigned int noiseTexture; glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    noiseTex = noiseTexture;

}

GLuint SSAOProcessor::GenerateSSAOTex(DeferredLightingData* lighting_data, mat4 projection)
{

    static Shader* shaderSSAO;

    if (shaderSSAO == nullptr)
    {
        shaderSSAO = new Shader("Assets/Editor/shaders/10/");
    }

    static Shader* shaderSSAOBlur;

    if (shaderSSAOBlur == nullptr)
    {
        shaderSSAOBlur = new Shader("Assets/Editor/shaders/11/");
    }


    shaderSSAO->use();
    shaderSSAO->setInt("gPosition", 0);
    shaderSSAO->setInt("gNormal", 1);
    shaderSSAO->setInt("texNoise", 2);
    shaderSSAOBlur->use();
    shaderSSAOBlur->setInt("ssaoInput", 0);


    glViewport(0, 0, lighting_data->size.x, lighting_data->size.y);
    // 2. generate SSAO texture
// ------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderSSAO->use();
    // Send kernel + rotation 
    for (unsigned int i = 0; i < 64; ++i)
        shaderSSAO->setVec3("samples[" + std::to_string(i) + "]", kernel[i]);
    shaderSSAO->setMat4("projection", projection);
    shaderSSAO->setVec2("noiseScale", { lighting_data->size.x / 4.0, lighting_data->size.y / 4.0 });
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lighting_data->gPos);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, lighting_data->gNrm);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTex);
    ShapeMgr::DrawQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // 3. blur SSAO texture to remove noise
    // ------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderSSAOBlur->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, outTex);
    ShapeMgr::DrawQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return blurTex;
}

void SSAOProcessor::Resize(int w, int h)
{
    Unload();
    Regenerate(w,h);

}

void SSAOProcessor::Unload()
{
    glDeleteFramebuffers(1, &ssaoBuffer);
    glDeleteFramebuffers(1, &blurFBO);

    glDeleteTextures(1, &outTex);
    glDeleteTextures(1, &blurTex);
}

void SSAOProcessor::Regenerate(int w, int h)
{
    glGenFramebuffers(1, &ssaoBuffer);  glGenFramebuffers(1, &blurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBuffer);
    // SSAO color buffer
    glGenTextures(1, &outTex);
    glBindTexture(GL_TEXTURE_2D, outTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outTex, 0);

    unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments);

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;
    // and blur stage
    glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
    glGenTextures(1, &blurTex);
    glBindTexture(GL_TEXTURE_2D, blurTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTex, 0);

    glDrawBuffers(1, attachments);

    unsigned int rboDepth1;
    glGenRenderbuffers(1, &rboDepth1);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth1);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth1);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint SSAOProcessor::GetSSAOTex()
{
    return outTex;
}

BloomProcessor::BloomFBO::BloomFBO(int w, int h, int chainLength)
{
    this->chainLength = chainLength;

    Regenerate(w, h);
}

void BloomProcessor::BloomFBO::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void BloomProcessor::BloomFBO::Unload()
{
    for (int i = 0; i < MipChain.size(); i++) {
        glDeleteTextures(1, &MipChain[i]->texture);
        MipChain[i]->texture = 0;
    }
    MipChain.clear();
    glDeleteFramebuffers(1, &FBO);
    FBO = 0;
}

void BloomProcessor::BloomFBO::Regenerate(int w, int h)
{
    Unload();

    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glm::vec2 mipSize((float)w, (float)h);
    glm::ivec2 mipIntSize((int)w, (int)h);
    // Safety check
    if (w > (int)INT_MAX || h > (int)INT_MAX) {
        std::cerr << "Window size conversion overflow - cannot build bloom FBO!\n";
        return;
    }

    for (int i = 0; i < chainLength; i++)
    {
        BloomMipMap* mip = new BloomMipMap;

        mipSize *= 0.5f;
        mipIntSize /= 2;
        mip->size = mipSize;
        mip->intSize = mipIntSize;

        glGenTextures(1, &mip->texture);
        glBindTexture(GL_TEXTURE_2D, mip->texture);
        // we are downscaling an HDR color buffer, so we need a float texture format
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
            (int)mipSize.x, (int)mipSize.y,
            0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        std::cout << "Created bloom mip " << mipIntSize.x << 'x' << mipIntSize.y << std::endl;
        MipChain.push_back(mip);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, MipChain[0]->texture, 0);

    // setup attachments
    unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments);

    // check completion status
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("gbuffer FBO error, status: 0x\%x\n", status);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

BloomProcessor::BloomProcessor(int w, int h)
{
    srcViewportSize = glm::ivec2(w, h);
    srcViewportSizeFlt = glm::vec2((float)w, (float)h);

    // Framebuffer
    const unsigned int num_bloom_mips = 5; // Experiment with this value

    FBO = new BloomFBO(w, h, num_bloom_mips);

    // Shaders
    downsampleShader = new Shader("Assets/Editor/shaders/12/");
    upsampleShader = new Shader("Assets/Editor/shaders/13/");

    // Downsample
    downsampleShader->use();
    downsampleShader->setInt("srcTexture", 0);

    // Upsample
    upsampleShader->use();
    upsampleShader->setInt("srcTexture", 0);
}

void BloomProcessor::Unload()
{
    FBO->Unload();
}

GLuint BloomProcessor::RenderBloom(GLuint source, float filterRadius)
{
    FBO->Bind();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderDownsamples(source);
    RenderUpsamples(filterRadius);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, RenderMgr::GetSceneWinSize().x, RenderMgr::GetSceneWinSize().y);

    return FBO->GetMipChain()[0]->texture;
}

void BloomProcessor::RenderDownsamples(GLuint source)
{
    const std::vector<BloomMipMap*>& mipChain = FBO->GetMipChain();

    glDisable(GL_BLEND);

    downsampleShader->use();
    downsampleShader->setInt("srcTexture", 0);
    downsampleShader->setVec2("srcResolution", srcViewportSizeFlt);

    // Bind srcTexture (HDR color buffer) as initial texture input
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, source);

    // Progressively downsample through the mip chain
    for (int i = 0; i < mipChain.size(); i++)
    {
        const BloomMipMap* mip = mipChain[i];
        glViewport(0, 0, mip->size.x, mip->size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, mip->texture, 0);

        ShapeMgr::DrawQuad();

        // Set current mip resolution as srcResolution for next iteration
        downsampleShader->setVec2("srcResolution", mip->size);
        // Set current mip as texture input for next iteration
        glBindTexture(GL_TEXTURE_2D, mip->texture);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void BloomProcessor::RenderUpsamples(float filterRadius)
{
    const std::vector<BloomMipMap*>& mipChain = FBO->GetMipChain();

    upsampleShader->use();
    upsampleShader->setFloat("filterRadius", filterRadius);
    upsampleShader->setInt("srcTexture", 0);

    // Enable additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    for (int i = mipChain.size() - 1; i > 0; i--)
    {
        const BloomMipMap* mip = mipChain[i];
        const BloomMipMap* nextMip = mipChain[i - 1];

        // Bind viewport and texture from where to read
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mip->texture);

        // Set framebuffer render target (we write to this texture)
        glViewport(0, 0, nextMip->size.x, nextMip->size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, nextMip->texture, 0);

        ShapeMgr::DrawQuad();
    }

    // Disable additive blending
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Restore if this was default

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

GLuint BloomProcessor::GetTexture()
{
    return FBO->GetMipChain()[0]->texture;
}

void BloomProcessor::Resize(int w, int h)
{
    FBO->Resize(w, h);

    srcViewportSize = glm::ivec2(w, h);
    srcViewportSizeFlt = glm::vec2((float)w, (float)h);
}

GraphicsPostProcessor::GraphicsPostProcessor(int w, int h)
{
	//ssao_processor = new SSAOProcessor(w, h);
    bloom_processor = new BloomProcessor(w, h);
}



void GraphicsPostProcessor::Resize(int w, int h)
{

    //ssao_processor->Resize(w, h);
    bloom_processor->Resize(w, h);
}
