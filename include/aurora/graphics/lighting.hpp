#ifndef LIGHTING_HPP
#define LIGHTING_HPP

#include "model.hpp"
#include "engine/entity.hpp"

class Light : public Component 
{
    CLASS_DECLARATION(Light)
public:
    Light(std::string&& initialValue)
        : Component(std::move(initialValue)) {
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

	string icon = ICON_FA_LIGHTBULB;

    string GetIcon() override;
};

struct LightingMgr {
    static vector<Light*> lights;
    static void EditMaterial(Material* material);
    static void RemoveLight(Light* light);
};

#endif