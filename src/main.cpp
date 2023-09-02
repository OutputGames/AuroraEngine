// Your First C++ Program

#include "aurora/aurora.hpp"
#include "engine/entity.hpp"
#include "engine/log.hpp"
#include "engine/project.hpp"
#include "graphics/billboard.hpp"
#include "graphics/lighting.hpp"
#include "physics/physics.hpp"
#include "rendering/renderer.hpp"
#include "utils/filesystem.hpp"

template <class Comp>
void RegisterComponent()
{
    string rawname = typeid(Comp).name();

    string sub = "class ";

    std::string::size_type i = rawname.find(sub);

    if (i != std::string::npos)
        rawname.erase(i, sub.length());


    ComponentRegistry::cmp_map[rawname] = &ComponentRegistry::createInstance<Comp>;

    Logger::Log("Registered component: "+rawname, Logger::DBG, "BOOT");
}

int main() {


    Logger::Log("Started Aurora at " + Engine::Filesystem::GetCurrentDir(), Logger::DBG, "BOOT");
    Logger::Log("LoggerTest", Logger::INFO, "TEST");
    Logger::Log("LoggerTest", Logger::DBG, "TEST");
    Logger::Log("LoggerTest", Logger::WARN, "TEST");
    Logger::Log("LoggerTest", Logger::LOG_ERROR, "TEST");


    //std::cout << "Started Aurora at " << Engine::Filesystem::GetCurrentDir() << std::endl;

    RegisterComponent<Billboard>();
    RegisterComponent<Light>();
    RegisterComponent<MeshRenderer>();
    RegisterComponent<ModelRenderer>();
    RegisterComponent<Skybox>();
    //RegisterComponent<Rigidbody>();
    RegisterComponent<BoxCollider>();
    RegisterComponent<SphereCollider>();

    RenderMgr::InitGraphicsDevice();

    //Project* p = Project::Create("C:/Users/chris/Downloads/TestProject");

    /*

    {
        Entity* entity = Scene::GetScene()->entity_mgr->CreateEntity("TestEntity");

        ModelRenderer* renderer = entity->AttachComponent<ModelRenderer>();
        renderer->model = Model::LoadModel("resources/models/test0.fbx");
        Shader* shader = new Shader("resources/shaders/0/");
        renderer->model->SetShader(shader);

        entity->transform->scale = vec3(0.1);
    }


    {
        Entity* entity = Scene::GetScene()->entity_mgr->CreateEntity("TestEntity2");

        ModelRenderer* renderer = entity->AttachComponent<ModelRenderer>();
        renderer->model = Model::LoadModel("resources/models/cube.fbx");
        Shader* shader = new Shader("resources/shaders/0/");
        renderer->model->SetShader(shader);
    }

    {
        Entity* entity = Scene::GetScene()->entity_mgr->CreateEntity("Light");

        Light* light = entity->AttachComponent<Light>();

        light->color = vec3(1, 1, 1);
        light->power = 1000;
        light->enabled = true;

        entity->Init();

    }

    */

    /*
    {
        Entity* entity = Scene::GetScene()->entity_mgr->CreateEntity("Skybox");

        Skybox* sb = entity->AttachComponent<Skybox>();

        sb->cubemap_texture = CubemapTexture::LoadFromPath("resources/textures/cubemap/test/");

        entity->Init();

    }
    */

    while (!RenderMgr::CheckCloseWindow()) {
        if (Project::ProjectLoaded()) {
            Scene::GetScene()->physics_factory->Update();
            Scene::GetScene()->entity_mgr->Update();
        }
        RenderMgr::UpdateGraphicsDevice();
    }

    RenderMgr::DestroyGraphicsDevice();

    return 0;
}
