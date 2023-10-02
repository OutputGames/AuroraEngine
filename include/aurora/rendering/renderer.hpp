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

    void Init() override;
    void Update() override;
    void Unload() override;
    std::string PrintToJSON() override;
    void LoadFromJSON(nlohmann::json data) override;
    void EngineRender() override;

	std::string icon = ICON_FA_CUBE;

    std::string GetIcon() override;

    Mesh* mesh;
};

class ModelRenderer : public Component 
{
    CLASS_DECLARATION(ModelRenderer)
public:
    ModelRenderer(std::string&& initialValue)
        : Component(move(initialValue)) {
    }

    ModelRenderer() = default;

    void Init()  override;
    void Update() override;
    void Unload() override;
    std::string PrintToJSON() override;
    void LoadFromJSON(nlohmann::json data) override;
    void EngineRender() override;

	std::string icon = ICON_FA_CUBES;

    std::string GetIcon() override;

    Model* model;
};


#endif