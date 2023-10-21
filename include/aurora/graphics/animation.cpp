#include "animation.hpp"

#include "engine/project.hpp"
#include "engine/assets/processor.hpp"
#include "rendering/render.hpp"
#include "rendering/renderer.hpp"

CLASS_DEFINITION(Component, Animator)

void Animator::Update()
{
	float dt = RenderMgr::GetDeltaTime();

	if (m_CurrentAnimation) {
		UpdateAnimation(dt);

		auto transforms = GetFinalBoneMatrices();

		int ctr = 0;
		for (mat4 transform : transforms)
		{
			Material::UniformData data = { ShaderFactory::Mat4 };

			data.m4 = transform;

			entity->material->SetUniform("finalBonesMatrices[" + std::to_string(ctr) + "]", data);
			ctr++;
		}
	}
}

void Animator::EngineRender()
{
	vector<Asset*> assets = Project::GetProject()->processor->GetAssetsOfType(Asset::AnimationAsset);

	string modelName = "";

	if (m_CurrentAnimation) {

		modelName = filesystem::path(m_CurrentAnimation->path).stem().string();
	}

	if (ImGui::BeginCombo("Animation", modelName.c_str())) {

		for (Asset* asset : assets)
		{
			AnimationAsset* modelAsset = (AnimationAsset*)asset;

			if (ImGui::Selectable(filesystem::path(modelAsset->path).string().c_str()))
			{
				if (entity->GetComponent<MeshRenderer>()) {
					Animation* newAnimation = new Animation(modelAsset->path, entity->GetComponent<MeshRenderer>()->mesh->data->parent);
					PlayAnimation(newAnimation);
				}
			}
		}

		ImGui::EndCombo();
	}
}
