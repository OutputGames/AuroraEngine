#include "aurora.hpp"

#include "engine/runtime/monort.hpp"

template <typename ... C>
void RegisterComponent()
{
    ([]()
    {
	    string rawname = typeid(C).name();

	    string sub = "class ";

	    std::string::size_type i = rawname.find(sub);

	    if (i != std::string::npos)
	        rawname.erase(i, sub.length());


	    ComponentRegistry::cmp_map[rawname] = &ComponentRegistry::createInstance<C>;

	    Logger::Log("Registered component: " + rawname, Logger::DBG, "BOOT");
    }(), ...);
}

template <typename ... C>
void RegisterComponent(ComponentRegistry::ComponentRegister<C...>)
{
    RegisterComponent<C ...>();
}


void InitEngine()
{
    Logger::Log("Started Aurora at " + Filesystem::GetCurrentDir(), Logger::DBG, "BOOT");


    //std::cout << "Started Aurora at " << Engine::Filesystem::GetCurrentDir() << std::endl;

    RegisterComponent(ComponentRegistry::RegisteredComponents());

    RenderMgr::InitGraphicsDevice();
    MonoRuntime::Initialize();

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
}
