#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "utils/utils.hpp"
#include "engine/entity.hpp"
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

class Rigidbody;

struct PhysicsFactory
{

    struct PhysicsBody
    {
        PhysicsBody(Transform* t);

        vec3 position, velocity, force;

        float mass=0.1;

        enum BodyType
        {
            STATIC = 0,
            KINEMATIC,
            DYNAMIC,
        };

        BodyType bodyType = static_cast<BodyType>(2);

        btRigidBody* _ibody;

        Rigidbody* user;

    };

    struct CollisionBody
    {
        CollisionBody(Transform* t);
    };

    PhysicsFactory();

    static vec3 ConvertVector3(btVector3 v);
    static vec4 ConvertVector4(btVector4 v);
    static btVector3 ConvertVector3(vec3 v);
    static btVector4 ConvertVector4(vec4 v);
    static btTransform ConvertTransform(Transform* t);

    btDiscreteDynamicsWorld* dynamicsWorld;

    vec3 m_gravity = vec3(0, -9.80665, 0);

    void Update();

private:

    friend Scene;

    vector<PhysicsBody*> bodies;
};

class Rigidbody : public Component 
{
    CLASS_DECLARATION(Rigidbody)
public:
    Rigidbody(std::string&& initialValue)
        : Component(move(initialValue)) {
    }

    Rigidbody() = default;

    void Init() override;
    void Update() override;
    void Unload() override;
    void EngineRender() override;

	std::string icon = ICON_FA_CUBE;

    std::string GetIcon() override;

    int bodyType;

private:
    friend class Collider;
    PhysicsFactory::PhysicsBody* _body;
};


class Collider : public Component
{
	CLASS_DECLARATION(Collider)

public:
	Collider(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	Collider() = default;

void Init(btCollisionShape* shape);
void Update() override;

bool isStatic=true;

void EngineRender() override;

    btRigidBody* _ibody;
};

class BoxCollider : public Collider
{
    CLASS_DECLARATION(BoxCollider)

public:
    BoxCollider(std::string&& initialValue) : Collider(move(initialValue))
    {
    }

    BoxCollider() = default;

    void Init() override;
    void Update() override;

private:
    btBoxShape* _shape;
};

class SphereCollider : public Collider
{
    CLASS_DECLARATION(SphereCollider)

public:
    SphereCollider(std::string&& initialValue) : Collider(move(initialValue))
    {
    }

    SphereCollider() = default;

    void Init() override;
    void Update() override;

private:
    btSphereShape* _shape;
};


#endif