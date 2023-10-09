
#ifndef COMP_REGISTER
#define COMP_REGISTER

#include "utils/utils.hpp"
#include "entity.hpp"
#include "graphics/billboard.hpp"
#include "graphics/lighting.hpp"
#include "physics/physics.hpp"
#include "runtime/scriptcomponent.hpp"

struct AURORA_API ComponentRegistry
{
    template<typename T> static std::shared_ptr<Component> createInstance() { return make_shared< T >(); }

    //typedef std::map<std::string, Component* (*)()> map_type;
    static std::map<std::string, std::shared_ptr<Component>(*)()> cmp_map;


    template <typename... C>
    struct ComponentRegister
    {

    };

    using RegisteredComponents = ComponentRegister<Billboard, PointLight, MeshRenderer, Skybox, ScriptComponent, RigidBody3D>;
    
};

template <typename... Comp>
void RegisterComponent();

template <typename... Comp>
void RegisterComponent(ComponentRegistry::ComponentRegister<Comp...>);

#endif