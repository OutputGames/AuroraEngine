#ifndef BILLBOARD_HPP
#define BILLBOARD_HPP

#include "model.hpp"
#include "engine/entity.hpp"
#include "rendering/renderer.hpp"

struct CameraBase;

class Billboard : public Component {
    CLASS_DECLARATION(Billboard)
public:
    Billboard(std::string&& initialValue)
        : Component(move(initialValue)) {
    }

    Billboard() = default;

    void Init() override;
    void Update() override;
    void Unload() override;
    void EngineRender() override;

    std::string PrintToJSON() override;
    void LoadFromJSON(nlohmann::json data) override;

    Texture texture;

    std::string icon = ICON_FA_IMAGE;

    std::string GetIcon() override;

    vec3 color;

    bool showInEditorOnly = false;

private:

    Mesh* mesh;
};

#endif