#if !defined(PHYSICS_HPP)
#define PHYSICS_HPP

#include "engine/entity.hpp"

struct PhysicsFactory
{

	void Init();

	void Update();

	PhysicsFactory();

private:

	friend class RigidBody3D;

};

class RigidBody3D : public Component
{
	CLASS_DECLARATION(RigidBody3D)

public:
	RigidBody3D(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	RigidBody3D() = default;

	void Init() override;

	void Update() override;

private:

};

#endif // PHYSICS_HPP
