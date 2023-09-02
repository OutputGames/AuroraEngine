#include "physics.hpp"

CLASS_DEFINITION(Component, Rigidbody)
CLASS_DEFINITION(Component, Collider)
CLASS_DEFINITION(Collider, BoxCollider)
CLASS_DEFINITION(Collider, SphereCollider)

float deltaTime, lastTime;

void Rigidbody::Init()
{
	_body = new PhysicsFactory::PhysicsBody(entity->transform);
	_body->user = this;
	//_body->_ibody->setType(static_cast<reactphysics3d::BodyType>(bodyType));
}

void Rigidbody::Update()
{
	//_body->_ibody->setType(static_cast<reactphysics3d::BodyType>(bodyType));
	int oldBt = _body->bodyType;

	_body->bodyType = static_cast<PhysicsFactory::PhysicsBody::BodyType>(bodyType);

	/*
	if (oldBt == PhysicsFactory::PhysicsBody::STATIC)
	{
		_body->position = entity->transform->position;
		btVector3 localInertia(0, 0, 0);
		if (bodyType != PhysicsFactory::PhysicsBody::STATIC) {
			if (_body->_ibody->getCollisionShape()) {
				_body->_ibody->getCollisionShape()->calculateLocalInertia(_body->mass, localInertia);

			}
		}
		_body->_ibody->setInvInertiaDiagLocal(localInertia);
	}
	*/
	
	entity->transform->position = _body->position;
}

void Rigidbody::Unload()
{

}

void Rigidbody::EngineRender()
{

	const char* items[] = { "STATIC", "KINEMATIC", "DYNAMIC" };

	if (ImGui::BeginCombo("##combo", items[bodyType]))
	{
		for (int n = 0; n < IM_ARRAYSIZE(items); n++)
		{
			bool is_selected = (n == bodyType); // You can store your selection however you want, outside or inside your objects
			if (ImGui::Selectable(items[n], is_selected))
				bodyType = n;
			if (is_selected)
				ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
		}
		ImGui::EndCombo();
	}
}

std::string Rigidbody::GetIcon()
{
	return ICON_FA_CHILD;
}

void Collider::Init(btCollisionShape* shape)
{
	Scene* s = Scene::GetScene();

	btScalar mass(1.f);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btCollisionShape* colShape = shape;

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(PhysicsFactory::ConvertTransform(entity->transform));
	_ibody = new btRigidBody(mass, myMotionState, colShape, localInertia);

	_ibody->setUserPointer(entity);

	s->physics_factory->dynamicsWorld->addRigidBody(_ibody);
}

void Collider::Update()
{
	if (isStatic)
	{
		_ibody->setCollisionFlags(btRigidBody::CF_STATIC_OBJECT);
	} else
	{
		_ibody->setCollisionFlags(btRigidBody::CF_DYNAMIC_OBJECT);
	}
}

void Collider::EngineRender()
{
	ImGui::Checkbox("Is Static", &isStatic);
}

void BoxCollider::Init()
{
	_shape = new btBoxShape({50,50,50});
	Collider::Init(_shape);
}

void BoxCollider::Update()
{
	//_shape->setHalfExtents(PhysicsFactory::ConvertVector3(entity->transform->scale / 2.0f));
	_shape = static_cast<btBoxShape*>(_ibody->getCollisionShape());
	//_shape->setLocalScaling(PhysicsFactory::ConvertVector3(entity->transform->scale));
	Collider::Update();
}

void SphereCollider::Init()
{
	_shape = new btSphereShape(9.0);
	Collider::Init(_shape);
}

void SphereCollider::Update()
{
	//_shape->setHalfExtents(PhysicsFactory::ConvertVector3(entity->transform->scale / 2.0f));
	_shape = static_cast<btSphereShape*>(_ibody->getCollisionShape());
	//_shape->setLocalScaling(PhysicsFactory::ConvertVector3(entity->transform->scale));
	Collider::Update();
}

PhysicsFactory::PhysicsBody::PhysicsBody(Transform* t)
{

	position = t->position;
	velocity = vec3{ 0 };
	force = vec3{ 0 };

	Scene* s = Scene::GetScene();

	btScalar mass(1.f);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btCollisionShape* colShape = new btSphereShape(btScalar(1.));

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(ConvertTransform(t));
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
	_ibody = new btRigidBody(rbInfo);

	s->physics_factory->dynamicsWorld->addRigidBody(_ibody);

	Scene::GetScene()->physics_factory->bodies.push_back(this);
}

PhysicsFactory::PhysicsFactory()
{
	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setGravity(PhysicsFactory::ConvertVector3(m_gravity));
}

vec3 PhysicsFactory::ConvertVector3(btVector3 v)
{
	return { v.x(), v.y(), v.z() };
}

vec4 PhysicsFactory::ConvertVector4(btVector4 v)
{
	return { v.x(), v.y(), v.z(), v.w() };
}

btVector3 PhysicsFactory::ConvertVector3(vec3 v)
{
	return { v.x, v.y ,v.z };
}

btVector4 PhysicsFactory::ConvertVector4(vec4 v)
{
	return { v.x,v.y,v.z,v.w };
}

btTransform PhysicsFactory::ConvertTransform(Transform* t)
{
	btTransform transform;

	transform.setIdentity();

	transform.setOrigin(ConvertVector3(t->position));

	return transform;
}

void PhysicsFactory::Update()
{
	float time = glfwGetTime();

	deltaTime = time - lastTime;

	lastTime = time;

	dynamicsWorld->stepSimulation(deltaTime, 10);

	for (PhysicsBody* body : bodies)
	{
		if (body->bodyType != PhysicsBody::STATIC) {
			btRigidBody* rbody = body->_ibody;
			btTransform trans;
			if (rbody && rbody->getMotionState())
			{
				rbody->getMotionState()->getWorldTransform(trans);
			}
			else
			{
				trans = body->_ibody->getWorldTransform();
			}
			body->position = ConvertVector3(body->_ibody->getWorldTransform().getOrigin());
		} else
		{
			body->_ibody->setWorldTransform(ConvertTransform(body->user->entity->transform));
		}
	}


	//print positions of all objects
	for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
		btRigidBody* body = btRigidBody::upcast(obj);
		btTransform trans;
		if (body && body->getMotionState())
		{
			body->getMotionState()->getWorldTransform(trans);
		}
		else
		{
			trans = obj->getWorldTransform();
		}
		Entity* user = static_cast<Entity*>(body->getUserPointer());

		user->transform->position = ConvertVector3(trans.getOrigin());
	}
}
