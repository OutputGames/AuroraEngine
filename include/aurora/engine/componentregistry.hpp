
#ifndef COMP_REGISTER
#define COMP_REGISTER

#include "utils/utils.hpp"
#include "entity.hpp"
#include "graphics/billboard.hpp"
#include "graphics/lighting.hpp"
#include "graphics/particle.hpp"
#include "physics/physics.hpp"
#include "runtime/scriptcomponent.hpp"
#include <graphics/animation.hpp>
#include <rendering/camera.hpp>

struct AURORA_API ComponentRegistry
{
    template<typename T> static std::shared_ptr<Component> createInstance() { return make_shared< T >(); }

    //typedef std::map<std::string, Component* (*)()> map_type;
    inline static std::map<std::string, std::shared_ptr<Component>(*)()> cmp_map= std::map<std::string, std::shared_ptr<Component>(*)()>();;


    template <typename... C>
    struct ComponentRegister
    {

    };

    using RegisteredComponents = ComponentRegister<Billboard, PointLight, MeshRenderer, Skybox, ScriptComponent, ParticleEmitter, Animator, Camera>;
    
};

template <typename... Comp>
void RegisterComponent();

template <typename... Comp>
void RegisterComponent(ComponentRegistry::ComponentRegister<Comp...>);

#endif