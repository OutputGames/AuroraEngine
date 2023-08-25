#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "engine/entity.hpp"
#include "graphics/model.hpp"

class MeshRenderer : public Component 
{
    CLASS_DECLARATION(MeshRenderer)
public:
    MeshRenderer(std::string&& initialValue)
        : Component(std::move(initialValue)) {
    }

    MeshRenderer() = default;

    void Init() override;
    void Update() override;
    void Unload() override;

	string icon = ICON_FA_CUBE;

    string GetIcon() override;

    Mesh* mesh;
};

class ModelRenderer : public Component 
{
    CLASS_DECLARATION(ModelRenderer)
public:
    ModelRenderer(std::string&& initialValue)
        : Component(std::move(initialValue)) {
    }

    ModelRenderer() = default;

    void Init()  override;
    void Update() override;
    void Unload() override;

	string icon = ICON_FA_CUBES;

    string GetIcon() override;

    Model* model;
};


#endif