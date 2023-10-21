#include "physics.hpp"

CLASS_DEFINITION(Component, RigidBody3D)
CLASS_DEFINITION(Component, StaticBody3D)

void PhysicsFactory::Init()
{
}

void PhysicsFactory::Update()
{
}

PhysicsFactory::PhysicsFactory()
{
	Init();
}

void RigidBody3D::Init()
{

}

void RigidBody3D::Update()
{

}
