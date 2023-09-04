#include "billboard.hpp"

#include "engine/imgui_ext.hpp"
#include "imgui/imgui.h"
#include "rendering/camera.hpp"
#include "rendering/render.hpp"
#include "rendering/renderer.hpp"

CLASS_DEFINITION(Component, Billboard)

using namespace nlohmann;

void Billboard::Init()
{

    /*
	static const std::vector<GLfloat> vbd = {
	 -0.5f, -0.5f, 0.0f,
	  0.5f, -0.5f, 0.0f,
	 -0.5f,  0.5f, 0.0f,
	  0.5f,  0.5f, 0.0f,
	};

    Mesh::MeshData* mesh_data = new Mesh::MeshData;

	for (int v = 0; v < vbd.size(); v+=3)
	{
		Vertex vertex = Vertex();

		vertex.Position = { vbd[v], vbd[v + 1], vbd[v + 2] };

		mesh_data->vertices.push_back(vertex);
	}

    int k = 0;
    for (int j = 0; j < (vbd.size() / 3); j++)
    {
        for (int i = 0; i < 3; i++)
        {
            mesh_data->indices.push_back(k + i);
        }
        //face.Indices.Add(mesh.IndexBuffer.Data[0][k + 1]);
        //face.Indices.Add(mesh.IndexBuffer.Data[0][k + 2]);


        k = k + 3;
    }

    mesh_data->name = "BillboardMesh";

    mesh_data->textures.push_back(texture);

    mesh = Mesh::Upload(mesh_data);

    Shader* shader = new Shader("resources/shaders/2/");

    mesh->material->shader = shader;
    mesh->material->ProcessUniforms();
    */


    
    Model* model = Model::LoadModel("editor/models/plane.fbx");

    Shader * shader = new Shader("editor/shaders/2/");

    model->SetShader(shader);

    mesh = model->meshes[0];
    
}

void Billboard::Update()
{
    texture.type = "texture_diffuse";

    if (mesh->material->textures.size() <= 0)
    {
        mesh->material->textures.push_back(texture);
    }
    else {
        mesh->material->textures[0] = texture;
    }

    mesh->material->GetUniform("BillboardPos")->v3 = entity->transform->position;
    mesh->material->GetUniform("BillboardSize")->v2 = { entity->transform->scale };
    mesh->material->GetUniform("color")->v3 = color;

    RenderData* data = new RenderData;
    data->mesh = mesh;

    data->castShadow = false;
    data->cullBack = true;

    RenderMgr::renderObjs.push_back(data);

    //mat4 mat = entity->transform->GetMatrix();

    //mat = glm::lookAt(entity->transform->position, cam->position, cam->up);

    //entity->transform->CopyTransforms(mat);
}

void Billboard::Unload()
{
}

void Billboard::EngineRender()
{

    std::string s = texture.path;

    string filePathName="",filePath="";

    bool opened = ImGui::FileDialog(&s, ".png,.jpg", filePathName, filePath);

    if (opened)
    {
        texture.Unload();

        texture = Texture::Load(filePathName);

        texture.isCubemap = false;
    }
}

std::string Billboard::PrintToJSON()
{

    json j;

    j["texture_path"] = texture.path;

    for (int i = 0; i < 3; ++i)
    {
        j["color"][i] = color[i];
    }

    return j.dump();
}

void Billboard::LoadFromJSON(nlohmann::json data)
{
	for (int i = 0; i < 3; ++i)
	{
        color[i] = data["color"][i];
	}

    texture = Texture::Load(data["texture_path"]);
}

std::string Billboard::GetIcon()
{
    return icon;
}
