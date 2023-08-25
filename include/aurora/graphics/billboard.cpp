#include "billboard.hpp"

#include "rendering/camera.hpp"
#include "rendering/render.hpp"
#include "rendering/renderer.hpp"

CLASS_DEFINITION(Component, Billboard)

void Billboard::Init()
{

    /*
	static const vector<GLfloat> vbd = {
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


    
    Model* model = Model::LoadModel("resources/models/plane.fbx");

    Shader * shader = new Shader("resources/shaders/2/");

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
}

string Billboard::GetIcon()
{
    return icon;
}
