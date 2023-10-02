#include "renderer.hpp"

#include "render.hpp"
#include "engine/imgui_ext.hpp"

CLASS_DEFINITION(Component, MeshRenderer)
CLASS_DEFINITION(Component, ModelRenderer)

using namespace nlohmann;


void MeshRenderer::Init()
{
	entity->material->LoadShader(Shader::CheckIfExists("editor/shaders/0"));
}

void MeshRenderer::Update()
{
	RenderData* render_data = new RenderData;
	render_data->mesh = mesh;
	render_data->matrix = entity->transform->GetGlobalMatrix();

	render_data->castShadow = true;
	render_data->cullBack = true;
	render_data->material = entity->material;

	entity->material->textures = mesh->data->textures;

	RenderMgr::renderObjs.push_back(render_data);
}

void MeshRenderer::Unload()
{
}

std::string MeshRenderer::PrintToJSON()
{
	json j;

	j["mesh_path"] = mesh->data->path;
	j["mesh_index"] = mesh->data->index;

	return j.dump();
}

void MeshRenderer::LoadFromJSON(nlohmann::json data)
{
	mesh = Mesh::Load(data["mesh_path"], data["mesh_index"]);
}

void MeshRenderer::EngineRender()
{
	ImGui::FileDialog();
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

		json unifs;

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

			model = Model::LoadModel(filePathName);

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
