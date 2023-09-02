#include "entity.hpp"

#include "json.hpp"
#include "log.hpp"
#include "project.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"
#include "imgui/imgui.h"
#include "graphics/lighting.hpp"
#include "physics/physics.hpp"
#include "utils/filesystem.hpp"

using namespace nlohmann;

const std::size_t Component::Type = std::hash<const char*>()(TO_STRING(Component));

std::map<std::string, std::shared_ptr<Component>(*)()> ComponentRegistry::cmp_map = std::map<std::string, std::shared_ptr<Component>(*)()>();

void Transform::CopyTransforms(mat4 matrix)
{
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(matrix, scale, rotation, translation, skew, perspective);

    this->position = translation;
    this->rotation = rotation;
    this->scale = scale;
}

vec3 Transform::GetEulerAngles()
{
    return glm::eulerAngles(rotation);
}


mat4 Transform::GetMatrix()
{
    mat4 matrix = mat4(1.0);

    matrix = translate(matrix, position);
    matrix *= toMat4(rotation);
    matrix = glm::scale(matrix, scale);

    return matrix;
}

void Transform::Reset()
{
    position = { 0,0,0 };
    rotation = quat({0,0,0});
    scale = { 1,1,1 };
}

bool Entity::RemoveComponent(std::string n)
{
    int id = 0;
    for (const auto & component : components)
    {
        
	    if (component->GetName() == n)
	    {
            break;
	    }
        id++;
    }


    components.erase(components.begin() + id);
    return true;
}

void Entity::AddComponent(std::string name)
{
    auto const& c = ComponentRegistry::cmp_map[name]();
    c->entity = this;
    c->Init();
    components.push_back(c);
}

void Entity::RenderComponents()
{
    for (auto const& component : components)
    {
        std::string componentName = component->GetName();
        std::string componentIcon = component->GetIcon();
        std::string headername = componentIcon + " "+componentName;
        if (ImGui::Button(ICON_FA_TRASH_CAN))
        {
            RemoveComponent(componentName);
            continue;
        }
        ImGui::SameLine();
        if (ImGui::CollapsingHeader((headername.c_str()))) {
            component.get()->EngineRender();
        }
    }
}

void Entity::Delete()
{
    for (auto const& component : components)
    {
        component.get()->Unload();
    }
   Scene::GetScene()->entity_mgr->RemoveEntity(this);
}

void Entity::Init()
{
    for (auto const& component : components)
    {
        component->Init();
    }
}

void Entity::Update()
{
    for (auto const& component : components)
    {
        component.get()->Update();
    }
}

void Scene::EntityMgr::Update()
{
    for (Entity* entity : entities)
    {
        if (entity->enabled) {
            entity->Update();
        }
    }
}

Entity *Scene::EntityMgr::CreateEntity()
{
    Entity* ent = new Entity;

    ent->id = entities.size();

    entities.push_back(ent);

    return ent;

}

Entity *Scene::EntityMgr::CreateEntity(std::string name)
{
    Entity* ent = CreateEntity();
    ent->name = name;
    return ent;
}

void Scene::EntityMgr::RemoveEntity(Entity* entity)
{
    int id = entity->id;

    for (Entity* entity : entities)
    {
        if (entity->id > id)
        {
            entity->id -= 1;
        }
    }

    entities.erase(entities.begin() + id);
}

Entity* Scene::EntityMgr::DuplicateEntity(Entity* entity)
{
    Entity* new_entity = CreateEntity(entity->name);

    new_entity->transform = new Transform(*entity->transform);

    for (auto const& component : entity->components)
    {
        std::shared_ptr<Component> new_component = component.get()->clone();
        new_component->entity = new_entity;
        new_entity->components.emplace_back(new_component);
    }
    new_entity->Init();

    return new_entity;

}

void Component::Init()
{
}

void Component::Update()
{
}

void Component::Unload()
{
}

void Component::SetEnabled(bool enb)
{
}

void Component::EngineRender()
{

}

std::string Component::PrintToJSON()
{
    return "{}";
}

void Component::LoadFromJSON(nlohmann::json data)
{
    Logger::Log("Tried to load a component, loaded from base class.", Logger::WARN, "CMP");
}

std::string Component::GetIcon()
{
    return this->icon;
}

void Scene::Update()
{
    entity_mgr->Update();
}

std::string Scene::SaveScene()
{
    json j;
    j["name"] = name;

    json entities;

    for (Entity* entity : entity_mgr->entities)
    {
        json e;
        e["name"] = entity->name;

        Transform* t = entity->transform;

        e["transform"]["position"] = {t->position.x, t->position.y,t->position.z };
        for (int i = 0; i < 3; ++i)
        {
            e["transform"]["rotation"][i] = t->rotation[i];
        }
        e["transform"]["scale"] = { t->scale.x, t->scale.y,t->scale.z };

        e["id"] = entity->id;

        e["enabled"] = entity->enabled;

        json c;

        for (auto const& component : entity->components)
        {
            c[component->GetName()] = json::parse(component.get()->PrintToJSON());
        }

        e["components"] = c;

        entities[entity->name] = e;
    }

    j["entities"] = entities;
    j["path"] = path;

    std::string projFilePath = "resources/scenes/" + name + ".auscene";
    std::ofstream projFile(projFilePath, std::ofstream::out | std::ofstream::trunc);

    projFile << j.dump();

    projFile.close();

    return j.dump();
}

Scene* Scene::GetScene()
{
    return Project::GetProject()->loaded_scene;
}

Scene* Scene::LoadScene(std::string sc, bool in)
{
    json scene = json::parse(sc);

    Scene* s = new Scene;
    s->name = scene["name"];
    s->entity_mgr = new Scene::EntityMgr;
    s->light_mgr = new LightingMgr;
    s->physics_factory = new PhysicsFactory;

    s->light_mgr->sky = nullptr;

    s->path = scene["path"];

    if (in) {
        Project::GetProject()->scenes.push_back(s);
    }
    Project::GetProject()->LoadScene(s->name);

    for (auto ej : scene["entities"])
    {
        Entity* e = s->entity_mgr->CreateEntity(ej["name"]);

        json transform = ej["transform"];

        e->transform->position = { transform["position"][0], transform["position"][1], transform["position"][2] };
        e->transform->rotation = quat({ transform["rotation"][0], transform["rotation"][1], transform["rotation"][2] });

        e->transform->scale = { transform["scale"][0], transform["scale"][1], transform["scale"][2] };

        json c = ej["components"];

        auto obj = c.get<json::object_t>();

        for (auto cs : obj)
        {
            e->AddComponent(cs.first);
        }

        for (auto component : e->components)
        {
            component->LoadFromJSON(c[component->GetName()]);
        }

        e->enabled = ej["enabled"];
    }

    return s;
}

Scene* Scene::CreateScene(std::string s)
{
    if (!std::filesystem::is_directory("resources/scenes/") || !std::filesystem::exists("resources/scenes/")) { // Check if src folder exists
        std::filesystem::create_directory("resources/scenes/"); // create src folder
    }

    Scene* scene = new Scene;
    scene->name = s;
    scene->entity_mgr = new EntityMgr;
    scene->light_mgr = new LightingMgr;

    scene->light_mgr->sky = nullptr;

    std::string projFilePath =  "resources/scenes/" + s + ".auscene";

    std::ofstream projFile(projFilePath);

    nlohmann::json projJSON;

    projJSON["name"] = s;

    projFile << projJSON.dump();

    projFile.close();

    scene->path = projFilePath;

    return scene;
}

void Scene::SetPath(std::string p)
{
    path = p;
}


template <class ComponentType>
bool Entity::RemoveComponent()
{
    return true;
}

template <class ComponentType>
std::vector<ComponentType*> Entity::GetComponents()
{
    return 0;
}

template <class ComponentType>
int Entity::RemoveComponents()
{
    return 0;
}