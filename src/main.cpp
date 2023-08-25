// Your First C++ Program

#include "aurora/aurora.hpp"
#include "engine/entity.hpp"
#include "graphics/lighting.hpp"
#include "rendering/renderer.hpp"
#include "utils/filesystem.hpp"

int main() {


    std::cout << "Started Aurora at " << Engine::Filesystem::GetCurrentDir() << std::endl;

    RenderMgr::InitGraphicsDevice();

    {
        Entity* entity = EntityMgr::CreateEntity("TestEntity");

        ModelRenderer* renderer = entity->AttachComponent<ModelRenderer>();
        renderer->model = Model::LoadModel("resources/models/test0.fbx");
        Shader* shader = new Shader("resources/shaders/0/");
        renderer->model->SetShader(shader);

        entity->transform->scale = vec3(0.1);
    }


    {
        Entity* entity = EntityMgr::CreateEntity("TestEntity2");

        ModelRenderer* renderer = entity->AttachComponent<ModelRenderer>();
        renderer->model = Model::LoadModel("resources/models/cube.fbx");
        Shader* shader = new Shader("resources/shaders/0/");
        renderer->model->SetShader(shader);
    }

    {
        Entity* entity = EntityMgr::CreateEntity("Light");

        Light* light = entity->AttachComponent<Light>();

        light->color = vec3(1, 1, 1);
        light->power = 1000;
        light->enabled = true;

        entity->Init();

    }

    cout << "Made Entity";

    while (!RenderMgr::CheckCloseWindow()) {
        EntityMgr::Update();
        RenderMgr::UpdateGraphicsDevice();
    }

    RenderMgr::DestroyGraphicsDevice();

    return 0;
}
