#include "particle.hpp"

#include "engine/project.hpp"
#include "engine/assets/processor.hpp"
#include "rendering/render.hpp"
#include "utils/math.hpp"

CLASS_DEFINITION(Component, ParticleEmitter)

void ParticleEmitter::Init()
{
	entity->material->LoadShader(new Shader("Assets/Editor/shaders/Instanced0/"));
	settings = new ParticleSettings;
}

void ParticleEmitter::EngineRender()
{
	vector<Asset*> assets = Project::GetProject()->processor->GetAssetsOfType(Asset::ModelAsset);

	string modelName = "";

	if (mesh) {

		modelName = filesystem::path(mesh->data->parent->path).stem().string();
	}

	if (ImGui::BeginCombo("Model", modelName.c_str())) {

		for (Asset* asset : assets)
		{
			ModelAsset* modelAsset = (ModelAsset*)asset;

			if (ImGui::Selectable(filesystem::path(modelAsset->path).string().c_str()))
			{
				meshIndex = 0;
				mesh = modelAsset->model->meshes[meshIndex];
			}
		}

		ImGui::EndCombo();
	}


	if (mesh) {
		if (ImGui::SliderInt("Mesh Index", &meshIndex, 0, mesh->data->parent->meshes.size() - 1))
		{
			mesh = mesh->data->parent->meshes[meshIndex];
		}
	}


	ImGui::NewLine();

	ImGui::Text("Particle Settings");

	ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 1);

	ImGui::DragFloat("Lifetime", &settings->lifetime);
	ImGui::InputInt("Max Amount", &maxAmount);
	ImGui::DragFloat("Frame Delay", &frameDelay);
	ImGui::DragFloat("Radius", &settings->radius);

	ImGui::NewLine();

	ImGui::Text("Particle Info");

	ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 1);

	string s = "Particle amount: " + to_string(particleAmt);

	ImGui::Text(s.c_str());

}

void ParticleEmitter::Update()
{
	particleAmt = 0;
	int ctr = 0;
	float deltaTime = RenderMgr::GetDeltaTime();
	for (Particle* particle : particles)
	{
		if (particle) {

			particleAmt++;

			particle->velocity += particle->acceleration * deltaTime;
			
			particle->position += particle->velocity * deltaTime;

			mat4 matrix = entity->transform->GetGlobalMatrix();

			matrix = translate(matrix, particle->position);

			if (!entity->material->GetUniform("offsets[" + to_string(ctr) + "]")) {

				Material::UniformData* data = new Material::UniformData;

				data->m4 = matrix;
				data->type = ShaderFactory::Mat4;

				entity->material->SetUniform("offsets[" + to_string(ctr) + "]", *data);
			}
			else
			{
				entity->material->uniforms["offsets[" + to_string(ctr) + "]"].m4 = matrix;
			}

			if (particle->time >= settings->lifetime)
			{
				if (particles.size() > 0)
				{
					particles[ctr] = nullptr;
					continue;
				}
			}

			particle->time += deltaTime;

		}
		ctr++;
	}

	if (tmrOn)
	{
		tmr+=deltaTime;
		if (tmr >= frameDelay*deltaTime)
		{	
			tmrOn = false;
			tmr = 0;
		}
	}

	if (particleAmt < maxAmount && !tmrOn) {
		Emit(1);
		tmrOn = true;
	}

	if (mesh) {
		RenderData* render_data = new RenderData;
		render_data->mesh = mesh;
		render_data->matrix = entity->transform->GetGlobalMatrix();

		render_data->castShadow = true;
		render_data->cullBack = true;
		render_data->material = entity->material;
		render_data->instanced = true;
		render_data->instances = particleAmt;

		entity->material->textures = mesh->data->textures;

		RenderMgr::renderObjs.push_back(render_data);
	}
}

void ParticleEmitter::Emit(int amount)
{
	for (int i = 0; i < amount; ++i)
	{
		float random1 = Math::GetRandomValue(-100, 100);
		Particle* particle = new Particle;
		particle->acceleration = { 0,-9.8,0.05 };
		particle->velocity = { sin(random1) * settings->radius, 0,cos(random1) * settings->radius};
		particle->position = vec3{ 0 };
		particle->time = 0;
		int ctr = 0;

		for (Particle* value : particles)
		{
			if (value == nullptr)
			{
				particles[ctr] = particle;
				return;
			}
			ctr++;
		}

		particles.push_back(particle);

	}
}
