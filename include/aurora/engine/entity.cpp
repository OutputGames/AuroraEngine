#include "entity.hpp"

#include "egs.hpp"
#include "json.hpp"
#include "log.hpp"
#include "project.hpp"
#include "assets/processor.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"
#include "imgui/imgui.h"
#include "graphics/lighting.hpp"
#include "physics/physics.hpp"
#include "rendering/render.hpp"
#include "runtime/monort.hpp"
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
    vec3 rot;

    rot = eulerAngles(rotation);

    return rot;
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

void Transform::Delete()
{
    if (parent) {
        for (shared_ptr<Transform> transform : parent->children)
        {
	        if (transform->index > index)
	        {
                transform->index -= 1;
	        }
        }

        //cout << "removed " << entity->name << " as child to " << parent->entity->name << endl;

        parent = nullptr;
    } else
    {
        //cout << entity->name << ": deleting with no parent" << endl;
    }

    for (int i = 0; i < children.size(); ++i)
    {
        shared_ptr<Transform> t = children[i];

        //cout << "removing child " << t->entity->name << " id: " << i << " from " << entity->name << endl;

        t->parent = this;
        t->entity->Delete();
        t->parent = nullptr;
    }

    children.clear();
}

void Transform::Update()
{
    for (shared_ptr<Transform> entity : children)
    {
        if (entity->entity != nullptr) {
            //entity->entity->transform = entity.get();
            entity->parent = this;
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
    if (transform) {
        transform->Delete();
    } else
    {
        cout << name << " has no transform?" << endl;
        assert((transform == nullptr));
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

        for (shared_ptr<Transform> child : transform->parent->children)
        {
            if (child->entity->id == id)
            {
                break;
            }
            i++;
        }

        transform->parent->children.erase(transform->parent->children.begin() + i);
    }
    for (shared_ptr<Transform> entity : transform->children)
    {
	    if (entity->entity->id == e->id)
	    {
            return;
	    }
    }

    this->transform->index = e->transform->children.size();

    e->transform->children.push_back(make_shared<Transform>(*this->transform));

    transform->parent = e->transform;
}

void Entity::DrawTree(Entity* entity)
{
    if (entity == nullptr)
        return;

    const bool is_selected = (selected_id == entity->id);

    ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow;

    ImGuiTreeNodeFlags node_flags = base_flags;

    if (is_selected)
        node_flags |= ImGuiTreeNodeFlags_Selected;

    std::string cicon = ICON_FA_CHECK;

    if (!entity->enabled)
        cicon = ICON_FA_XMARK;

    if (ImGui::Button((cicon + "###" + to_string(entity->id)).c_str()))
    {
        entity->enabled = !entity->enabled;
    }


    ImGui::SameLine();

    string label = entity->name + "###" + to_string(entity->id);



	if (ImGui::TreeNodeEx(label.c_str(), node_flags))
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
		for (shared_ptr<Transform> child : entity->transform->children)
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

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
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

void Entity::SetShader(Shader* shader)
{
    material->LoadShader(shader);
}

Prefab* Entity::Export()
{
    Prefab* prb = new Prefab(this);

    return prb;
}

Entity* Entity::Load(string data)
{
    Entity* entity = new Entity;

    entity->LoadData(data);

    return entity;
}

void Entity::LoadData(string data)
{
    Entity* entity = this;

    json ej = json::parse(data);

    Entity* e = entity;

    e->name = ej["name"];

    json transform = ej["transform"];

    e->transform->position = { transform["position"][0], transform["position"][1], transform["position"][2] };
    e->transform->rotation = quat({ transform["rotation"][0], transform["rotation"][1], transform["rotation"][2] });

    e->transform->scale = { transform["scale"][0], transform["scale"][1], transform["scale"][2] };

    json children = transform["children"];

    for (auto child : children)
    {
        string childData = child.dump(JSON_INDENT_AMOUNT);

        Entity* childEntity = Load(childData);

        childEntity->SetParent(entity);
    }

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

    e->material->LoadFromData(ej["material"].dump(JSON_INDENT_AMOUNT));
}

void Entity::Update()
{

    if (material)
    {
        material->entity = this;
    }

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

    for (auto const& entity : entities)
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

    InsertEntity(ent);

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

    uint32_t id = entity->id;

    for (auto const& nentity : entities)
    {
        if (nentity->id > id)
        {
            nentity->id -= 1;
        }
    }

    entities.erase(entities.begin() + id);
}

Entity* Scene::EntityMgr::DuplicateEntity(Entity* entity, Entity* parent)
{
    Entity* new_entity = CreateEntity(entity->name);

    new_entity->transform->position = entity->transform->position;
    new_entity->transform->scale = entity->transform->scale;

    new_entity->transform->rotation = entity->transform->rotation;

    if (parent) {
        new_entity->SetParent(parent);
    }

    int ctr = 0;
    for (auto const& child : entity->transform->children)
    {
        auto const& new_child = make_shared<Transform>(*child);
        new_child->entity = DuplicateEntity(child->entity, new_entity);
    }

    for (auto const& component : entity->components)
    {
        std::shared_ptr<Component> new_component = component.get()->clone();
        new_component->entity = new_entity;
        new_entity->components.emplace_back(new_component);
    }
    new_entity->Init();

    new_entity->material->shader = entity->material->shader;
    new_entity->material->ProcessUniforms();

    for (pair<const string, Material::UniformData> uniform : new_entity->material->uniforms)
    {
        Material::UniformData* uniformPtr = &new_entity->material->uniforms[uniform.first];
        Material::UniformData* preUniform = &entity->material->uniforms[uniform.first];

        uniformPtr->type = preUniform->type;
        uniformPtr->b = preUniform->b;
        uniformPtr->f = preUniform->f;
        uniformPtr->i = preUniform->i;
        uniformPtr->m2 = preUniform->m2;
        uniformPtr->m3 = preUniform->m3;
        uniformPtr->m4 = preUniform->m4;
        uniformPtr->v2 = preUniform->v2;
        uniformPtr->v3 = preUniform->v3;
        uniformPtr->v4 = preUniform->v4;

        new_entity->material->uniforms[uniform.first] = *uniformPtr;
    }

    return new_entity;

}

void Scene::EntityMgr::InsertEntity(Entity* entity)
{
    shared_ptr<Entity> ptr(entity);

    entities.push_back(ptr);
}

void Scene::EntityMgr::InsertPrefab(Prefab* prefab)
{
    DuplicateEntity(prefab->GetEntity());
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

void Scene::OnRuntimeStart()
{
    OnUnload();

    saveState = EditorGameSystem::SaveCurrentState();

    runtimePlaying = true;
    {
        MonoRuntime::OnRuntimeStart(this);

        auto ents = entity_mgr->GetEntitiesWithComponent<ScriptComponent>();

        for (auto ent : ents)
        {
            MonoRuntime::OnCreateComponent(ent);
        }

    }
}

void Scene::OnRuntimeUpdate()
{

    physics_factory->Update();

    entity_mgr->Update();

    auto ents = entity_mgr->GetEntitiesWithComponent<ScriptComponent>();

    float dt = RenderMgr::GetDeltaTime();

    for (auto ent : ents)
    {
        MonoRuntime::OnUpdateComponent(ent, dt);
    }

    for (auto const& entity : entity_mgr->entities)
    {
        //MonoRuntime::OnUpdateEntity(entity, dt);
    }

}

void Scene::OnRuntimeUnload()
{
    runtimePlaying = false;
    if (saveState) {
        LoadScene(saveState->data);
    }
    saveState = nullptr;
    OnStart();
}

void Scene::OnStart()
{
}

void Scene::OnUnload()
{
}

string Scene::ToString()
{
    json j;
    j["name"] = name;

    json entities;

    for (auto const& entity : entity_mgr->entities)
    {

        entities[entity->name] = entity->Export()->ToJson();
    }

    j["entities"] = entities;
    j["path"] = path;

    return j.dump(JSON_INDENT_AMOUNT);
}

void Scene::LoadScene(string sc)
{
    light_mgr->Unload();

    entity_mgr->entities.clear();

    Shader::UnloadAllShaders();

    json scene = json::parse(sc);

    Scene* s = this;

    for (auto ej : scene["entities"])
    {
        Entity* e = s->entity_mgr->CreateEntity(ej["name"]);

        e->LoadData(ej.dump());

    }
}

void Scene::OnUpdate()
{

    entity_mgr->Update();
}

std::string Scene::SaveScene()
{


    std::string projFilePath = "Assets/Scenes/" + name + ".auscene";
    std::ofstream projFile(projFilePath, std::ofstream::out | std::ofstream::trunc);

    string data = ToString();

    projFile << data;

    projFile.close();

    return data;
}

Scene* Scene::GetScene()
{
    if (Project::GetProject()) {
        return Project::GetProject()->loaded_scene;
    } else
    {
        return nullptr;
    }
}

Scene* Scene::LoadScene(std::string sc, bool in)
{
    json scene = json::parse(sc);

    Scene* s = new Scene;
    s->name = scene["name"];

    s->light_mgr->sky = nullptr;

    s->path = scene["path"];

    if (in) {
        Project::GetProject()->scenes.push_back(s);
    } else
    {
        int itr = 0;
        bool loaded = false;
        for (Scene* value : Project::GetProject()->scenes)
        {

            if (value->name == s->name)
            {
                Project::GetProject()->scenes[itr] = s;
                loaded = true;
                break;
            }

            itr++;
        }
        if (!loaded)
        {
            Project::GetProject()->scenes.push_back(s);
        }
    }
    Project::GetProject()->LoadScene(s->name);

    for (auto ej : scene["entities"])
    {
        Entity* e = s->entity_mgr->CreateEntity(ej["name"]);

        e->LoadData(ej.dump());

    }

    return s;
}

Scene* Scene::LoadScenePath(string path)
{
    string data = Filesystem::ReadFileString(path);

    json scene = json::parse(data);

    Scene* s = new Scene;
    s->name = scene["name"];

    s->light_mgr->sky = nullptr;

    s->path = scene["path"];

    for (auto ej : scene["entities"])
    {
        Entity* e = s->entity_mgr->CreateEntity(ej["name"]);

        e->LoadData(ej.dump());

    }

    return s;
}

Scene* Scene::CreateScene(std::string s)
{
    if (!std::filesystem::is_directory("Assets/Scenes/") || !std::filesystem::exists("Assets/Scenes/")) { // Check if src folder exists
        std::filesystem::create_directory("Assets/Scenes/"); // create src folder
    }

    Scene* scene = new Scene;
    scene->name = s;
    //scene->physics_factory = new PhysicsFactory;
    {
        Entity* entity = scene->entity_mgr->CreateEntity("Skybox");

        Skybox* sb = entity->AttachComponent<Skybox>();

        sb->LoadTexture("Assets/Editor/textures/newport_loft.hdr");

        entity->Init();

        scene->light_mgr->sky = sb;
    }

    {
        Entity* entity = scene->entity_mgr->CreateEntity("Light");

        PointLight* light = entity->AttachComponent<PointLight>();

        light->color = vec3(1, 1, 1);
        light->power = 1000;
        light->enabled = true;

        entity->Init();

        entity->transform->position = { 10,10,10 };

        scene->light_mgr->lights.push_back(light);
    }

    std::string projFilePath =  "Assets/Scenes/" + s + ".auscene";

    std::ofstream projFile(projFilePath);

    nlohmann::json projJSON;

    projJSON["name"] = s;

    projFile << projJSON.dump(JSON_INDENT_AMOUNT);

    projFile.close();

    scene->path = projFilePath;

    return scene;
}

Scene::Scene()
{
    entity_mgr = new EntityMgr;
    physics_factory = new PhysicsFactory;
    light_mgr = new LightingMgr;
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

template <class T>
vector<Entity*> Scene::EntityMgr::GetEntitiesWithComponent()
{
    vector<Entity*> cmpEntities;

    for (auto const& entity : entities)
    {
        if (entity->GetComponent<T>()) {
            cmpEntities.push_back(entity.get());
        }
    }

    return cmpEntities;
}
