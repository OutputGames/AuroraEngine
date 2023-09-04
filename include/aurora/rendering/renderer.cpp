#include "renderer.hpp"

#include "render.hpp"

CLASS_DEFINITION(Component, MeshRenderer)
CLASS_DEFINITION(Component, ModelRenderer)

using namespace nlohmann;


void MeshRenderer::Init()
{
}

void MeshRenderer::Update()
{
	RenderData* render_data = new RenderData;
	render_data->mesh = mesh;
	render_data->matrix = entity->transform->GetGlobalMatrix();

	render_data->castShadow = true;
	render_data->cullBack = true;

	RenderMgr::renderObjs.push_back(render_data);
}

void MeshRenderer::Unload()
{
}

std::string MeshRenderer::GetIcon()
{
	return icon;
}

void ModelRenderer::Init()
{
	model = new Model;
}

void ModelRenderer::Update()
{
	if (model) {
		for (Mesh* mesh : model->meshes)
		{
			RenderData* render_data = new RenderData;
			render_data->mesh = mesh;
			render_data->matrix = entity->transform->GetGlobalMatrix();

			if (entity->name == "Sphere") {

				//render_data->matrix = translate(render_data->matrix, { 0,sin(glfwGetTime()), 0 });
			}

			render_data->castShadow = true;
			render_data->cullBack = true;
			RenderMgr::renderObjs.push_back(render_data);
		}
	}
}

void ModelRenderer::Unload()
{
	delete model;
}

std::string ModelRenderer::PrintToJSON()
{
	json j;

	j["model_path"] = model->path;

	json m;

	for (Mesh* value : model->meshes)
	{
		json ms;

		ms["name"] = value->data->name;

		json mat;

		Material* material = value->material;

		mat["shaderPath"] = material->shader->shaderDirectory;

		json unifs;

		for (std::pair<std::string, Material::UniformData*> uniform : material->uniforms)
		{
			json u;

			u["name"] = uniform.first;
			u["type"] = uniform.second->type;

			u["b"] = uniform.second->b;
			u["i"] = uniform.second->i;
			u["f"] = uniform.second->f;
			for (int i = 0; i < 2; ++i)
			{
				u["v2"][i] = uniform.second->v2[i];
			}

			for (int i = 0; i < 3; ++i)
			{
				u["v3"][i] = uniform.second->v3[i];
			}

			for (int i = 0; i < 4; ++i)
			{
				u["v4"][i] = uniform.second->v4[i];
			}


			unifs[uniform.first] = u;
		}

		mat["uniforms"] = unifs;

		ms["material"] = mat;

		m["meshes"][value->data->name] = ms;
	}

	j["model"] = m;

	return j.dump();
}

void ModelRenderer::LoadFromJSON(nlohmann::json data)
{
	model = Model::LoadModel(data["model_path"]);

	int i = 0;

	for (auto m : data["model"]["meshes"])
	{
		Mesh* mesh = model->meshes[i];
		mesh->data->name = m["name"];

		json mat = m["material"];

		json unifs = mat["uniforms"];

		Shader* s = new Shader(mat["shaderPath"]);

		mesh->material->LoadShader(s);

		for (auto unif : unifs)
		{
			std::string uname = unif["name"];
			Material::UniformData* dat = mesh->material->uniforms.at(uname);

			dat->b = unif["b"];
			dat->i = unif["i"];
			dat->f = unif["f"];
			dat->v2 = { unif["v2"][0],unif["v2"][1]};
			dat->v3 = { unif["v3"][0],unif["v3"][1], unif["v3"][2] };
			dat->v4 = { unif["v4"][0],unif["v4"][1],unif["v4"][2],unif["v4"][3] };

		}

		i++;
	}
}

void ModelRenderer::EngineRender()
{
	std::string s = model->path;

	ImGui::InputText("Image Path", &s);

	ImGui::SameLine();

	if (ImGui::Button("..."))
	{
		ImGuiFileDialog::Instance()->OpenDialog("engine_cmp_dlg", "Open a File", ".fbx", ".");
	}

	// display
	if (ImGuiFileDialog::Instance()->Display("engine_cmp_dlg"))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

			//std::cout << filePath << filePathName << std::endl;

			Material* m = nullptr;

			if (model && model->meshes.size() > 0) {
				m = model->meshes[0]->material;
			}

			model = Model::LoadModel(filePathName);

			model->SetShader(new Shader("editor/shaders/0/"));

			if (model && m != nullptr) {
				model->meshes[0]->material = m;
			}

			// action
		}

		// close
		ImGuiFileDialog::Instance()->Close();
	}
}

std::string ModelRenderer::GetIcon()
{
	return icon;
}
