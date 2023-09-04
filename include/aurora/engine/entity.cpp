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

bool Entity::selectedEntity;
int Entity::selected_id;

std::map<std::string, std::shared_ptr<Component>(*)()> ComponentRegistry::cmp_map = std::map<std::string, std::shared_ptr<Component>(*)()>();

void Transform::CopyTransforms(mat4 matrix, bool isGlobal)
{
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(matrix, scale, rotation, translation, skew, perspective);

    glm::vec3 absPos = vec3{0};
    for (Transform* p = this->parent; p != nullptr; p = p->parent)
        absPos += p->position;

    glm::vec3 absScale = vec3{ 1 };
    for (Transform* p = this->parent; p != nullptr; p = p->parent)
        absScale = { absScale.x * p->scale.x, absScale.y * p->scale.y, absScale.z * p->scale.z };

    vec3 absRot = vec3{ 0 };
    for (Transform* p = this->parent; p != nullptr; p = p->parent)
        absRot += p->GetEulerAngles();

    vec3 prePos = this->position;
    vec3 preScl = this->scale;

    if (isGlobal) {

        this->position = translation - absPos;

        this->rotation = quat(eulerAngles(rotation)-absRot);
        this->scale = scale / absScale;

        if (parent)
        {
            this->position = translation - absPos;
        }
    } else
    {
        this->position = translation;

        this->rotation = rotation;
        this->scale = scale;
    }
}

vec3 Transform::GetEulerAngles()
{
    return glm::eulerAngles(rotation);
}

vec3 Transform::GetAbsolutePosition()
{
    glm::vec3 absPos = this->position;
    for (Transform* p = this->parent; p != nullptr; p = p->parent)
        absPos += p->position;
    return absPos;
}

vec3 Transform::GetAbsoluteScale()
{
    glm::vec3 absScale = { this->scale.x, this->scale.y, this->scale.z };
    for (Transform* p = this->parent; p != nullptr; p = p->parent)
        absScale = { absScale.x * p->scale.x, absScale.y * p->scale.y, absScale.z * p->scale.z };
    return absScale;
}

vec3 Transform::GetAbsoluteRotation()
{
    vec3 absRot = this->GetEulerAngles();
    for (Transform* p = this->parent; p != nullptr; p = p->parent)
        absRot += p->GetEulerAngles();
    return absRot;
}

quat Transform::GetAbsoluteRotationQuat()
{
    return quat(GetAbsoluteRotation());
}


mat4 Transform::GetMatrix()
{
    mat4 matrix = mat4(1.0);

    matrix = translate(matrix, position);
    matrix *= toMat4(rotation);
    matrix = glm::scale(matrix, scale);

    return matrix;
}

mat4 Transform::GetGlobalMatrix()
{
    mat4 matrix = mat4(1.0);

    matrix = translate(matrix, GetAbsolutePosition());
    matrix *= toMat4(GetAbsoluteRotationQuat());
    matrix = glm::scale(matrix, GetAbsoluteScale());

    return matrix;
}

void Transform::Reset()
{
    position = { 0,0,0 };
    rotation = quat({0,0,0});
    scale = { 1,1,1 };
}

void Transform::Update()
{
    for (Transform* entity : children)
    {
        if (entity->entity != nullptr) {
            if (entity->entity->enabled) {
                entity->entity->Update();
            }
        }
    }
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
    if (transform->parent) {
        int i = 0;

        for (Transform* child : transform->parent->children)
        {
            if (child->entity->id == id)
            {
                break;
            }
            i++;
        }

        transform->parent->children.erase(transform->parent->children.begin() + i);
    }

    for (Transform* value : transform->children)
    {
        value->entity->Delete();
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

void Entity::SetParent(Entity* e)
{

    if (transform->parent) {
        int i = 0;

        for (Transform* child : transform->parent->children)
        {
            if (child->entity->id == id)
            {
                break;
            }
            i++;
        }

        transform->parent->children.erase(transform->parent->children.begin() + i);
    }
    for (Transform* entity : transform->children)
    {
	    if (entity->entity->id == e->id)
	    {
            return;
	    }
    }

    e->transform->children.push_back(this->transform);

    transform->parent = e->transform;
}

void Entity::DrawTree(Entity* entity)
{

    const bool is_selected = (selected_id == entity->id);

    std::string cicon = ICON_FA_CHECK;

    if (!entity->enabled)
        cicon = ICON_FA_XMARK;

    if (ImGui::Button((cicon + "###" + to_string(entity->id)).c_str()))
    {
        entity->enabled = !entity->enabled;
    }


    ImGui::SameLine();

    string label = entity->name + "" + to_string(entity->id);

	if (ImGui::TreeNode(label.c_str()))
	{
        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if (is_selected) {
            ImGui::SetItemDefaultFocus();
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            selected_id = entity->id;
            selectedEntity = true;
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::Text(entity->name.c_str());
            ImGui::SetDragDropPayload("ENT_MOVE", &entity->id, sizeof(int));
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENT_MOVE"))
            {
                int payload_n = *(const int*)payload->Data;

                Scene::GetScene()->entity_mgr->entities[payload_n]->SetParent(entity);

            }
            ImGui::EndDragDropTarget();
        }

        ImGui::Indent();
		for (Transform* child : entity->transform->children)
		{
			DrawTree(child->entity);
		}
        ImGui::Unindent();

		ImGui::TreePop();
	} else
	{
        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if (is_selected) {
            ImGui::SetItemDefaultFocus();
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            selected_id = entity->id;
            selectedEntity = true;
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::Text(entity->name.c_str());
            ImGui::SetDragDropPayload("ENT_MOVE", &entity->id, sizeof(int));
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENT_MOVE"))
            {
                int payload_n = *(const int*)payload->Data;

                Scene::GetScene()->entity_mgr->entities[payload_n]->SetParent(entity);

            }
            ImGui::EndDragDropTarget();
        }
	}

}

void Entity::Update()
{

    transform->entity = this;

    transform->Update();

    for (auto const& component : components)
    {
        component.get()->Update();
    }
}

Scene::EntityMgr::EntityMgr()
{
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

    delete entity;
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
    scene->physics_factory = new PhysicsFactory;

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