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
	friend class StaticBody3D;

};

class RigidBody3D : public Component
{
	CLASS_DECLARATION(RigidBody3D)

public:
	RigidBody3D(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	RigidBody3D() = default;

	AURORA_API void Init() override;

	AURORA_API void Update() override;

private:
};

class StaticBody3D : public Component
{
	CLASS_DECLARATION(StaticBody3D)

public:
	StaticBody3D(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	StaticBody3D() = default;

	AURORA_API void Init() override;

	AURORA_API void Update() override;

private:
};

#endif // PHYSICS_HPP