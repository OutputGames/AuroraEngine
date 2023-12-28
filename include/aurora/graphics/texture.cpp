#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "lighting.hpp"
#include "shape.hpp"
#include "rendering/camera.hpp"
#include "rendering/render.hpp"
#include "rendering/shader.hpp"
#include "stb/stb_image.h"

Texture Texture::Load(std::string path, bool flip)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    // load and generate the texture

    stbi_set_flip_vertically_on_load(flip);  

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {

        GLenum format;

        switch (nrChannels)
        {
        case 1:
            format = GL_R;
            break;
        case 2:
            format = GL_RG;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            break;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        Logger::Log("Failed to load texture at " + path, Logger::LOG_ERROR, "TEXTURE");
    }
    stbi_image_free(data);

    Texture tex;

    tex.ID = texture;
    tex.width = width;
    tex.height = height;
    tex.path = path;
    tex.isCubemap = false;

    return tex;
}

void Texture::Unload()
{
    glDeleteTextures(1, &ID);
}


CubemapTexture CubemapTexture::Load(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            Logger::Log("Failed to load cubemap face at " + faces[i], Logger::LOG_ERROR, "TEXTURE");
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    CubemapTexture texture = {};

    texture.ID = textureID;
    texture.height = height;
    texture.width = width;
    texture.type = "texture_cubemap";

    return texture;
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO2 = 0;
unsigned int cubeVBO2 = 0;
void renderCube2()
{
    // initialize (if necessary)
    if (cubeVAO2 == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
             // bottom face
             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
              1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             // top face
             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
              1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
              1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
              1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO2);
        glGenBuffers(1, &cubeVBO2);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO2);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO2);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}


CubemapTexture CubemapTexture::LoadFromPath(string p)
{

    Shader equirectangularToCubemapShader("Assets/Editor/shaders/6/");

    // pbr: setup framebuffer
    // ----------------------
    unsigned int captureFBO;
    unsigned int captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // pbr: load the HDR environment map
    // ---------------------------------
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float* data = stbi_loadf(p.c_str(), &width, &height, &nrComponents, 0);
    unsigned int hdrTexture;
    if (data)
    {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        Logger::Log("Failed to load texture at " + p, Logger::LOG_ERROR, "TEXTURE");
    }

    // pbr: setup cubemap to render to and attach to framebuffer
    // ---------------------------------------------------------
    unsigned int envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
    // ----------------------------------------------------------------------------------------------
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // pbr: convert HDR equirectangular environment map to cubemap equivalent
    // ----------------------------------------------------------------------
    equirectangularToCubemapShader.use();
    equirectangularToCubemapShader.setInt("equirectangularMap", 0);
    equirectangularToCubemapShader.setMat4("projection", captureProjection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        equirectangularToCubemapShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube2();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    CubemapTexture texture = {};

    texture.ID = envCubemap;
    texture.height = height;
    texture.width = width;
    texture.type = "texture_cubemap";
    texture.path = p;
    texture.isCubemap = true;

    return texture;
}

RenderTexture* RenderTexture::Load(int w, int h, CameraBase* camera)
{

    RenderTexture* rt = new RenderTexture;

    rt->width = w;
    rt->height = h;

    rt->isCubemap = false;

    rt->Regenerate(w, h);
    
    DeferredLightingData* dld = new DeferredLightingData(w, h);

    rt->lightingData = dld;
    rt->camera = camera;

    rt->post_processing = new GraphicsPostProcessor(w, h);

    return rt;
}

void RenderTexture::Resize(int w, int h)
{

    width = w;
    height = h;

    Regenerate(w, h);

    lightingData->Resize({ w,h });
    post_processing->Resize(w, h);
}

void RenderTexture::Unload()
{
    glDeleteFramebuffers(1, &fbo);
}

void RenderTexture::Render(bool igv)
{
    //lightingData = EditorCamera::GetEditorCam()->framebuffer.lightingData;

    glBindFramebuffer(GL_FRAMEBUFFER, lightingData->gBuffer);

    float aspect = lightingData->size.x / lightingData->size.y;

    mat4 view;
    mat4 projection;

    view = camera->GetViewMatrix();
    projection = camera->GetProjectionMatrix(aspect);


    glViewport(0, 0, lightingData->size.x, lightingData->size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto render_obj : RenderMgr::renderObjs)
    {
	    if (!render_obj->deferred)
            continue;

        if (render_obj->useDepthMask == false) {
            glDepthMask(GL_FALSE);
            glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        }

        if (render_obj->cullBack == false)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CW);
        }
        else
        {
            //glEnable(GL_CULL_FACE);
            //glCullFace(GL_BACK);
            //glFrontFace(GL_CW);
        }

        render_obj->material->Update();

        render_obj->material->shader->setMat4("model", render_obj->matrix);
        render_obj->material->shader->setMat4("view", view);
        render_obj->material->shader->setMat4("projection", projection);
        render_obj->material->shader->setVec3("viewPos", camera->position);

        render_obj->mesh->Draw(render_obj->instanced, render_obj->instances);

        if (render_obj->useDepthMask == false) {
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS); // set depth function back to default
        }
        glDisable(GL_CULL_FACE);

    }

    //GLuint ssaoTex = post_processing->ssao_processor->GenerateSSAOTex(lightingData, projection);

    if (true) {

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        static Shader* shaderLightingPass;

        if (shaderLightingPass == nullptr)
        {
            shaderLightingPass = new Shader("Assets/Editor/shaders/9/");
            shaderLightingPass->use();

            int texCtr = 8;

            shaderLightingPass->setInt("gPosition", 0);
            shaderLightingPass->setInt("gNormal", 1);
            shaderLightingPass->setInt("gAlbedo", 2);
            shaderLightingPass->setInt("gCombined", 3);
            shaderLightingPass->setInt("gTransmission", 4);

            shaderLightingPass->setInt("brdf", 5);
            shaderLightingPass->setInt("irradiance", 6);
            shaderLightingPass->setInt("prefilter", 7);
            shaderLightingPass->setInt("cubemap", 8);
        }

        glm::vec2 screenSize = lightingData->size;

        // 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
    // -----------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderLightingPass->use();

        int texCtr = 8;



        shaderLightingPass->setMat4("view", view);
        shaderLightingPass->setVec2("screenParams", screenSize);
        shaderLightingPass->setMat4("projection", projection);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, lightingData->gPos);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, lightingData->gNrm);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, lightingData->gAlb);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, lightingData->gSha);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, lightingData->gTrm);
    	glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, LightingMgr::GetBRDFTexture());

        LightingMgr* lighting = Scene::GetScene()->light_mgr;

        if (lighting->sky != nullptr) {
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_CUBE_MAP, lighting->sky->irradianceMap);
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_CUBE_MAP, lighting->sky->prefilterMap);
            glActiveTexture(GL_TEXTURE8);
            glBindTexture(GL_TEXTURE_CUBE_MAP, lighting->sky->cubemap_texture.ID);
        }

        int ctr = 0;
        for (int i = 0; i < 10; i++)
        {
            std::string light_un = "lights[" + to_string(ctr) + "]";
            if (i < lighting->lights.size()) {
                PointLight* light = lighting->lights[i];

                shaderLightingPass->setVec3(light_un + ".position", light->entity->transform->position);
                shaderLightingPass->setVec3(light_un + ".direction", light->entity->transform->GetEulerAngles());
                shaderLightingPass->setVec3(light_un + ".color", light->color);
                shaderLightingPass->setBool(light_un + ".enabled", light->enabled);
                shaderLightingPass->setFloat(light_un + ".power", light->power);

                /*
                shaderLightingPass->setFloat("far_plane", light->far_plane);
                shaderLightingPass->setFloat("near_plane", light->near_plane);
                shaderLightingPass->setInt("cascadeCount", GetCascadeLevels(light->far_plane).size());
                for (size_t i2 = 0; i2 < GetCascadeLevels(light->far_plane).size(); ++i2)
                {
                    shaderLightingPass->setFloat("cascadePlaneDistances[" + std::to_string(i2) + "]", GetCascadeLevels(light->far_plane)[i2]);
                }
                */

                shaderLightingPass->setMat4(light_un + ".lightSpaceMatrix", light->spaceMatrix);



                if (light->shadowMap != 0)
                {
                    texCtr++;
                    shaderLightingPass->setInt("shadowMap", texCtr);

                    GLenum tex = GL_TEXTURE0 + texCtr;

                    glActiveTexture(tex);
                    glBindTexture(GL_TEXTURE_2D, light->shadowMap);

                }

            }
            else
            {
                shaderLightingPass->setBool(light_un + ".enabled", false);
                //material->entity->material->uniforms[(light_un + ".enabled")].b = false;
            }
            ctr++;
        }

        shaderLightingPass->setVec3("viewPos", camera->position);
        // finally render quad
        ShapeMgr::DrawQuad();

        // 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
        // ----------------------------------------------------------------------------------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, lightingData->gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo); // write to default framebuffer
        // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
        // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
        // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
        glBlitFramebuffer(0, 0, screenSize.x, screenSize.y, 0, 0, screenSize.x, screenSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        //GLuint bloomID = post_processing->bloom_processor->RenderBloom(ID, 0.005f);

        RealID = ID;

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        

        if (lighting->settings->renderNonDeferredObjects)
        {
            for (RenderData* render_obj : RenderMgr::renderObjs)
            {

                if (render_obj->deferred)
                    continue;
                if (igv) {
                    if (render_obj->editorOnly)
                        continue;
                }

                if (render_obj->useDepthMask == false) {
                    glDepthMask(GL_FALSE);
                    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
                }

                if (render_obj->cullBack == false)
                {
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_BACK);
                    glFrontFace(GL_CW);
                }
                else
                {
                    //glEnable(GL_CULL_FACE);
                    //glCullFace(GL_BACK);
                    //glFrontFace(GL_CW);
                }

                render_obj->material->Update();

                render_obj->material->shader->setMat4("model", render_obj->matrix);
                render_obj->material->shader->setMat4("view", view);
                render_obj->material->shader->setMat4("projection", projection);
                render_obj->material->shader->setVec3("viewPos", camera->position);

                render_obj->mesh->Draw(render_obj->instanced, render_obj->instances);

                if (render_obj->useDepthMask == false) {
                    glDepthMask(GL_TRUE);
                    glDepthFunc(GL_LESS); // set depth function back to default
                }
                glDisable(GL_CULL_FACE);
            }
        }

    }



    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

GLuint RenderTexture::GetID()
{
    return RealID;
}

void RenderTexture::Regenerate(int w, int h)
{
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &ID);

    GLuint id, texture;

    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    // create a color attachment texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments);

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "LOG_ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    fbo = id;
    ID = texture;

    width = w;
    height = h;
}
