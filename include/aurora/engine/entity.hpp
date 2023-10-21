#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <map>
#include <memory>

#include "json.hpp"
#include "graphics/model.hpp"
#include "imgui/icons/IconsFontAwesome6.h"
#include "utils/utils.hpp"

#define TO_STRING( x ) #x

struct Prefab;
struct SaveState;
struct AURORA_API PhysicsFactory;
struct AURORA_API LightingMgr;
class Component;


//****************
#define CLASS_DECLARATION( classname )                                                      \
public:                                                                                     \
    static const std::size_t Type;                                                          \
    virtual bool IsClassType( const std::size_t classType ) const override;                 \
    virtual std::shared_ptr<Component> clone() const override                               \
	{                                                                                       \
    return std::make_shared<classname>(*this);                                              \
    }                                                                                       \

//****************
// CLASS_DEFINITION
// 
// This macro must be included in the class definition to properly initialize 
// variables used in type checking. Take special care to ensure that the 
// proper parentclass is indicated or the run-time type information will be
// incorrect. Only works on single-inheritance RTTI.
//****************
#define CLASS_DEFINITION( parentclass, childclass )                                         \
const std::size_t childclass::Type = std::hash< const char* >()( TO_STRING( childclass ) ); \
bool childclass::IsClassType( const std::size_t classType ) const {                         \
        if ( classType == childclass::Type )                                                \
            return true;                                                                    \
        return parentclass::IsClassType( classType );                                       \
}                                                                                           \


#define QUICKCLASS(parentclass, classname) \
class classname : public parentclass\
{\
    CLASS_DECLARATION(classname)\
public:\
    classname(std::string&& initialValue)\
        : parentclass(move(initialValue)) {\
    }\
    classname() = default; \

struct AURORA_API Transform {
    vec3 position;
    quat rotation;
    vec3 scale=vec3(1.0);
    void CopyTransforms(mat4 matrix, bool isGlobal=false);

    vec3 GetEulerAngles();

    vec3 GetAbsolutePosition();
    vec3 GetAbsoluteScale();
    vec3 GetAbsoluteRotation();
    quat GetAbsoluteRotationQuat();

    mat4 GetMatrix();
    mat4 GetGlobalMatrix();
    void Reset();
    void Delete();

    void Update();

    class Entity* entity;

    Transform* parent=nullptr;


    std::vector<shared_ptr<Transform>> children;

private:
    friend Entity;

    int index;
};

struct AURORA_API Entity
{
    Transform* transform;

    uint32_t id;

    std::vector<std::shared_ptr<Component>> components;

    std::string name;

    bool enabled;

    template <class CompType, typename... Args>
    CompType* AttachComponent(Args&&... params);

    template< class ComponentType >
    inline ComponentType* GetComponent() {
        for (auto&& component : components) {
            if (component->IsClassType(ComponentType::Type))
                return static_cast<ComponentType*>(component.get());
        }

        return static_cast<ComponentType*>(nullptr);
    }

    template< class ComponentType >
    bool RemoveComponent();

    bool RemoveComponent(std::string n);

    template< class ComponentType >
    std::vector< ComponentType* > GetComponents();

    template< class ComponentType >
    int RemoveComponents();

    void AddComponent(std::string name);

    template< class ComponentType >
    bool HasComponent()
    {
        return GetComponent<ComponentType>() != nullptr;
    }

    void RenderComponents();
    void Delete();

    void Init();

    void SetParent(Entity* e);

    static void DrawTree(Entity* n);

    static bool selectedEntity;
    static int selected_id;

    Material* material;

    void SetShader(Shader* shader);

    Prefab* Export();

    static Entity* Load(string data);
    void LoadData(string data);

    ~Entity()
    {
        material = nullptr;
        components.clear();
        transform = nullptr;
        id = 0;
        name = "";
    }

    Entity()
    {
        enabled = true;
        transform = new Transform();
        transform->entity = this;
        material = new Material;
        material->entity = this;
    };

private:
    friend class Scene;
    friend Transform;

    bool treeopen=false;

    void Update();
};

template <class CompType, typename ... Args>
CompType* Entity::AttachComponent(Args&&... params)
{
    if (!GetComponent<CompType>()) {
        components.emplace_back(make_shared< CompType >(forward< Args >(params)...));
        for (auto&& component : components) {
            if (component->IsClassType(CompType::Type)) {
                CompType* comp = static_cast<CompType*>(component.get());
                comp->entity = this;
                return comp;
            }
        }
    }

    return static_cast<CompType*>(nullptr);
}

class Component {
public:
    static const std::size_t                    Type;
    virtual bool                                IsClassType(const std::size_t classType) const {
        return classType == Type;
    }
    virtual std::shared_ptr<Component> clone() const
    {
        return std::make_shared<Component>(*this);
    }
public:

    virtual                                ~Component() = default;
    Component(std::string&& initialValue)
        : value(initialValue) {
    }

    Component() = default;

    virtual void imgui_properties() {};

    inline virtual std::string GetName() {

        std::string rawname = typeid(*this).name();

        std::string sub = "class ";

        std::string::size_type i = rawname.find(sub);

        if (i != std::string::npos)
            rawname.erase(i, sub.length());

        return rawname;
    }

public:
    std::string                             value = "uninitialized";
	std::string icon=ICON_FA_GEARS;

    virtual void Init();
    virtual void Update();
    virtual void Unload();
    virtual void SetEnabled(bool enb);
    virtual void EngineRender();
    virtual std::string PrintToJSON();
    virtual void LoadFromJSON(nlohmann::json data);
    virtual std::string GetIcon();

public:
    Entity* entity;
    bool enabled = true;
};

struct AURORA_API Scene
{
    struct AURORA_API EntityMgr
    {
        EntityMgr();

    	void Update();
    	Entity* CreateEntity();
    	Entity* CreateEntity(std::string name);
    	void RemoveEntity(Entity* entity);
    	Entity* DuplicateEntity(Entity* entity, Entity* parent=nullptr);
        void InsertEntity(Entity* entity);

        void InsertPrefab(Prefab* prefab);

        template <class T>
        vector<Entity*> GetEntitiesWithComponent();



        vector<shared_ptr<Entity>> entities;
    };

    EntityMgr* entity_mgr;
    LightingMgr* light_mgr;
    PhysicsFactory* physics_factory;

    void OnRuntimeStart();
    void OnRuntimeUpdate();
    void OnRuntimeUnload();

    void OnStart();
    void OnUpdate();
    void OnUnload();
    string ToString();
    void LoadScene(string s);
    std::string SaveScene();
    static Scene* GetScene();
    static Scene* LoadScene(std::string s, bool isNew);
    static Scene* LoadScenePath(string path);
    static Scene* CreateScene(std::string s);

    Scene();

    void SetPath(std::string p);

    std::string name;
    bool paused=false, runtimePlaying=false;

private:
    friend class Project;
    std::string path;
    SaveState* saveState = nullptr;;
};

#endif
