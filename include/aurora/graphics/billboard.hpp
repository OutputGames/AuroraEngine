#ifndef BILLBOARD_HPP
#define BILLBOARD_HPP

#include "model.hpp"
#include "engine/entity.hpp"
#include "rendering/renderer.hpp"

struct Camera;

class Billboard : public Component {
    CLASS_DECLARATION(Billboard)
public:
    Billboard(std::string&& initialValue)
        : Component(std::move(initialValue)) {
    }

    Billboard() = default;

    void Init() override;
    void Update() override;
    void Unload() override;
    void EngineRender() override;

    Texture texture;

    Camera* cam;

    string icon = ICON_FA_IMAGE;

    string GetIcon() override;

    vec3 color;

private:
    Mesh* mesh;
};

#endif