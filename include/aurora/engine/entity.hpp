#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <map>
#include <memory>

#include "json.hpp"
#include "imgui/icons/IconsFontAwesome6.h"
#include "utils/utils.hpp"

#define TO_STRING( x ) #x

struct PhysicsFactory;
struct LightingMgr;
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

struct Transform {
    vec3 position;
    quat rotation;
    vec3 scale=vec3(1.0);
    void CopyTransforms(mat4 matrix);

    vec3 GetEulerAngles();

    mat4 GetMatrix();
    void Reset();
};

struct Entity
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

    void RenderComponents();
    void Delete();

    void Init();
private:
    friend class Scene;

    void Update();

    Entity()
    {
        enabled = true;
        transform = new Transform();
    };
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

struct Scene
{
    struct EntityMgr
    {
    	std::vector<Entity*> entities;
    	void Update();
    	Entity* CreateEntity();
    	Entity* CreateEntity(std::string name);
    	void RemoveEntity(Entity* entity);
    	Entity* DuplicateEntity(Entity* entity);
    };

    EntityMgr* entity_mgr;
    LightingMgr* light_mgr;
    PhysicsFactory* physics_factory;

    void Update();
    std::string SaveScene();
    static Scene* GetScene();
    static Scene* LoadScene(std::string s, bool isNew);
    static Scene* CreateScene(std::string s);

    void SetPath(std::string p);

    std::string name;

private:
    friend class Project;
    std::string path;
};

struct ComponentRegistry
{
    template<typename T> static std::shared_ptr<Component> createInstance() { return make_shared< T >(); }

    //typedef std::map<std::string, Component* (*)()> map_type;
    static std::map<std::string, std::shared_ptr<Component>(*)()> cmp_map;
};

#endif