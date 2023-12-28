#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "engine/entity.hpp"
#include "graphics/model.hpp"

class MeshRenderer : public Component 
{
    CLASS_DECLARATION(MeshRenderer)
public:
    MeshRenderer(std::string&& initialValue)
        : Component(move(initialValue)) {
    }

    MeshRenderer() = default;

    AURORA_API void Init() override;
    AURORA_API void Update() override;
    AURORA_API void Unload() override;
    AURORA_API std::string PrintToJSON() override;
    AURORA_API void LoadFromJSON(nlohmann::json data) override;
    AURORA_API void EngineRender() override;

	std::string icon = ICON_FA_CUBE;

    AURORA_API std::string GetIcon() override;

    Mesh* mesh;
private:
    int meshIndex=0;
};

class ModelRenderer : public Component 
{
    CLASS_DECLARATION(ModelRenderer)
public:
    ModelRenderer(std::string&& initialValue)
        : Component(move(initialValue)) {
    }

    ModelRenderer() = default;

    AURORA_API void Init()  override;
    AURORA_API void Update() override;
    AURORA_API void Unload() override;
    AURORA_API std::string PrintToJSON() override;
    AURORA_API void LoadFromJSON(nlohmann::json data) override;
    AURORA_API void EngineRender() override;

	std::string icon = ICON_FA_CUBES;

    AURORA_API std::string GetIcon() override;

    Model* model;
};


#endif