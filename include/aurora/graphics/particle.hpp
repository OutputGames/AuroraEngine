#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "utils/utils.hpp"
#include "engine/entity.hpp"

struct ParticleSettings
{
	float lifetime=10;
	float radius = 1;
};

struct Particle
{
	vec3 position;
	float time;
	vec3 velocity, acceleration;
};

class ParticleEmitter : public Component
{
	CLASS_DECLARATION(ParticleEmitter)

public:
	ParticleEmitter(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	ParticleEmitter() = default;

	void Init() override;
	void EngineRender() override;
	void Update() override;

	void Emit(int amount);

	Mesh* mesh;
	int meshIndex;
	int maxAmount;
	float frameDelay = 10;

	ParticleSettings* settings;

private:

	int particleAmt = 0;

	float tmr = 0;
	bool tmrOn = false;

	vector<Particle*> particles;
};


#endif