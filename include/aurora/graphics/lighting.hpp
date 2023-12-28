#ifndef LIGHTING_HPP
#define LIGHTING_HPP

#include "model.hpp"
#include "engine/entity.hpp"



class PointLight : public Component 
{
    CLASS_DECLARATION(PointLight)
public:
    PointLight(std::string&& initialValue)
        : Component(move(initialValue)) {
    }

    PointLight() = default;

    AURORA_API void Init() override;
    AURORA_API void Update() override;
    AURORA_API void Unload() override;
    AURORA_API void EngineRender() override;


    vec3 color;
    bool enabled = true;
    float power = 1;

    int id;

	std::string icon = ICON_FA_LIGHTBULB;

    AURORA_API std::string GetIcon() override;
    AURORA_API std::string PrintToJSON() override;
    AURORA_API void LoadFromJSON(json data) override;

    void CalcShadowMap(int prebuffer);

    unsigned int shadowMap;
    unsigned int depthMapFBO;
    //unsigned int matricesUBO;

    float near_plane = 1.0f, far_plane = 75.0f;

    vector<mat4> spaceMatrices;
    mat4 spaceMatrix;
};

class Skybox : public Component
{
    CLASS_DECLARATION(Skybox)
public:
    Skybox(std::string&& initialValue)
        : Component(move(initialValue)) {
    }

    Skybox() = default;

    AURORA_API void Init() override;
    AURORA_API void Update() override;
    AURORA_API void EngineRender() override;

    AURORA_API void LoadTexture(std::string path);

    AURORA_API std::string PrintToJSON() override;
    AURORA_API void LoadFromJSON(json data) override;

    CubemapTexture cubemap_texture;

    unsigned int irradianceMap;
    unsigned int prefilterMap;
};

struct DeferredLightingData
{
    GLuint gBuffer;
    GLuint gPos, gNrm, gAlb, gSha, gTrm;

    DeferredLightingData(int w, int h);

    void Resize(vec2 size);

    void Generate(int w, int h);

    vec2 size;

};

struct LightingSettings
{
    bool renderNonDeferredObjects = true;
};

struct LightingMgr {
	std::vector<PointLight*> lights;
    Skybox* sky=nullptr;
	void EditMaterial(Material* material);
	void RemoveLight(PointLight* light);
    void Unload();

    static GLuint GetBRDFTexture();

    LightingSettings* settings=new LightingSettings;

private:
    unsigned int gBuffer=0;
    unsigned int gPosition, gNormal, gAlbedoSpec, gCombined;
};

#endif