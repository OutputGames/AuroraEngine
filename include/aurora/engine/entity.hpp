#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "imgui/icons/IconsFontAwesome6.h"
#include "utils/utils.hpp"

#define TO_STRING( x ) #x

//****************
#define CLASS_DECLARATION( classname )                                                      \
public:                                                                                     \
    static const std::size_t Type;                                                          \
    virtual bool IsClassType( const std::size_t classType ) const override;                 \
    virtual std::shared_ptr<Component> clone() const override \
	{\
    return std::make_shared<classname>(*this);\
    }

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
}

struct Transform {
    vec3 position;
    quat rotation;
    vec3 scale=vec3(1.0);
    void CopyTransforms(mat4 matrix);
    mat4 GetMatrix();
    void Reset();
};

class Component;

struct Entity
{
    Transform* transform;

    std::uint32_t id;

    std::vector<std::shared_ptr<Component>> components;

    string name;

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

    template< class ComponentType >
    std::vector< ComponentType* > GetComponents();

    template< class ComponentType >
    int RemoveComponents();

    void RenderComponents();
    void Delete();

    void Init();
private:
    friend class EntityMgr;

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
    components.emplace_back(std::make_shared< CompType >(std::forward< Args >(params)...));
    for (auto&& component : components) {
        if (component->IsClassType(CompType::Type)) {
            CompType* comp = static_cast<CompType*>(component.get());
            comp->entity = this;
            return comp;
        }
    }

    return static_cast<CompType*>(nullptr);
}

struct EntityMgr
{
    static std::vector<Entity*> entities;
    static void Update();
    static Entity* CreateEntity();
    static Entity* CreateEntity(std::string name);
    static void RemoveEntity(Entity* entity);
    static Entity* DuplicateEntity(Entity* entity);
};

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

        string rawname = typeid(*this).name();

        string sub = "class ";

        std::string::size_type i = rawname.find(sub);

        if (i != std::string::npos)
            rawname.erase(i, sub.length());

        return rawname;
    }

public:
    std::string                             value = "uninitialized";
	string icon=ICON_FA_GEARS;

    virtual void Init();
    virtual void Update();
    virtual void Unload();
    virtual void SetEnabled(bool enb);
    virtual void EngineRender();
    virtual string GetIcon();

public:
    Entity* entity;
    bool enabled = true;
};

#endif