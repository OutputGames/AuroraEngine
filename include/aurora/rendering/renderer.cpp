#include "renderer.hpp"

#include "render.hpp"

CLASS_DEFINITION(Component, MeshRenderer)
CLASS_DEFINITION(Component, ModelRenderer)


void MeshRenderer::Init()
{
}

void MeshRenderer::Update()
{
	RenderData* render_data = new RenderData;
	render_data->mesh = mesh;
	render_data->matrix = entity->transform->GetMatrix();
	RenderMgr::renderObjs.push_back(render_data);
}

void MeshRenderer::Unload()
{
}

string MeshRenderer::GetIcon()
{
	return icon;
}

void ModelRenderer::Init()
{
}

void ModelRenderer::Update()
{
	for (Mesh* mesh : model->meshes)
	{
		RenderData* render_data = new RenderData;
		render_data->mesh = mesh;
		render_data->matrix = entity->transform->GetMatrix();
		RenderMgr::renderObjs.push_back(render_data);
	}
}

void ModelRenderer::Unload()
{
	delete model;
}

string ModelRenderer::GetIcon()
{
	return icon;
}
