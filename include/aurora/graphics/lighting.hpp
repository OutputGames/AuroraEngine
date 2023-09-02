#ifndef LIGHTING_HPP
#define LIGHTING_HPP

#include "model.hpp"
#include "engine/entity.hpp"



class Light : public Component 
{
    CLASS_DECLARATION(Light)
public:
    Light(std::string&& initialValue)
        : Component(move(initialValue)) {
    }

    Light() = default;

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

    void CalcShadowMap();

private:
    friend LightingMgr;

    unsigned int shadowMap;
    unsigned int depthMapFBO;

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
	std::vector<Light*> lights;
    Skybox* sky=nullptr;
	void EditMaterial(Material* material);
	void RemoveLight(Light* light);
};

#endif