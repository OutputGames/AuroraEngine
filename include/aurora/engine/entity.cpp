#include "entity.hpp"

#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"
#include "imgui/imgui.h"

std::vector<Entity*> EntityMgr::entities;

const std::size_t Component::Type = std::hash<const char*>()(TO_STRING(Component));

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

void Entity::RenderComponents()
{
    for (auto const& component : components)
    {
        string componentName = component->GetName();
        string componentIcon = component->GetIcon();
        string headername = componentIcon + " "+componentName;
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
    EntityMgr::RemoveEntity(this);
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

void EntityMgr::Update()
{
    for (Entity* entity : entities)
    {
        if (entity->enabled) {
            entity->Update();
        }
    }
}

Entity *EntityMgr::CreateEntity()
{
    Entity* ent = new Entity;

    ent->id = entities.size();

    entities.push_back(ent);

    return ent;

}

Entity *EntityMgr::CreateEntity(std::string name)
{
    Entity* ent = CreateEntity();
    ent->name = name;
    return ent;
}

void EntityMgr::RemoveEntity(Entity* entity)
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

Entity* EntityMgr::DuplicateEntity(Entity* entity)
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

string Component::GetIcon()
{
    return this->icon;
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