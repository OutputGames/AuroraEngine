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

    void Init() override;
    void Update() override;
    void Unload() override;
    void EngineRender() override;


    vec3 color;
    bool enabled = true;
    float power = 1;

    int id;

	std::string icon = ICON_FA_LIGHTBULB;

    std::string GetIcon() override;
    std::string PrintToJSON() override;
    void LoadFromJSON(json data) override;

    void CalcShadowMap(int prebuffer);

private:
    friend LightingMgr;

    unsigned int shadowMap;
    unsigned int depthMapFBO;
    unsigned int matricesUBO;

    float near_plane = 0.01f, far_plane = 10000.0f;

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

    void Init() override;
    void Update() override;
    void EngineRender() override;

    void LoadTexture(std::string path);

    std::string PrintToJSON() override;
    void LoadFromJSON(json data) override;
private:
    friend LightingMgr;

    CubemapTexture cubemap_texture;

    unsigned int irradianceMap;
    unsigned int prefilterMap;
};

struct LightingMgr {
	std::vector<PointLight*> lights;
    Skybox* sky=nullptr;
	void EditMaterial(Material* material);
	void RemoveLight(PointLight* light);
    unsigned int GetGeometryBuffer();
    void UpdateGeometryBuffer(vec3 viewPos, unsigned prebuffer, mat4 v = mat4(0.0), vec3 vp = vec3(0.0), mat4 p=mat4(0.0));
    void ResizeGeometryBuffer(vec2 size);
    void Unload();
private:
    unsigned int gBuffer=0;
    unsigned int gPosition, gNormal, gAlbedoSpec, gCombined;
};

#endif