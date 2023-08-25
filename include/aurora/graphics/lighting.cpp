#include "lighting.hpp"

#include "billboard.hpp"
#include "imgui/imgui.h"
#include "rendering/camera.hpp"


CLASS_DEFINITION(Component, Light)

vector<Light*> LightingMgr::lights;

void Light::Init()
{
	if (LightingMgr::lights.size() < 10) {
		id = LightingMgr::lights.size();
		LightingMgr::lights.push_back(this);
	}

	if (!entity->GetComponent<Billboard>()) {
		Billboard* billboard = entity->AttachComponent<Billboard>();

		billboard->texture = Texture::Load("resources/textures/lightbulb.png", false);

		billboard->Init();
	}
}

void Light::Update()
{
	enabled = entity->enabled;

	if (entity->GetComponent<Billboard>())
	{
		entity->GetComponent<Billboard>()->color = color;
	}
}

void Light::Unload()
{
	LightingMgr::RemoveLight(this);
}

void Light::EngineRender()
{
	float lcol[3] = {color.r, color.g, color.b};

	ImGui::ColorPicker3("Light Color", lcol);

	color = { lcol[0], lcol[1], lcol[2] };

	ImGui::SliderFloat("Light Power", &power, 0, 1000);
}

string Light::GetIcon()
{
	return icon;
}

void LightingMgr::EditMaterial(Material* material)
{
	int ctr = 0;
	for (int i = 0; i < 10; i++)
	{
		string light_un = "lights[" + std::to_string(ctr) + "]";
		if (i < lights.size()) {
			Light* light = lights[i];
			material->GetUniform(light_un + ".position")->v3 = light->entity->transform->position;
			material->GetUniform(light_un + ".color")->v3 = light->color;
			material->GetUniform(light_un + ".enabled")->b = light->enabled;
			material->GetUniform(light_un + ".power")->f = light->power;
		} else
		{
			material->GetUniform(light_un + ".enabled")->b = false;
		}
		ctr++;
	}
}

void LightingMgr::RemoveLight(Light* light)
{
	int lid = light->id;

	for (Light* value : lights)
	{
		if (value->id > lid)
		{
			value->id -= 1;
		}
	}

	lights.erase(lights.begin() + lid);

	//cout << "removed light at " << lid << std::endl;

}
