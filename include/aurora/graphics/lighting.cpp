#include "lighting.hpp"

#include "billboard.hpp"
#include "shape.hpp"
#include "engine/imgui_ext.hpp"
#include "glm/detail/_noise.hpp"
#include "glm/detail/_noise.hpp"
#include "glm/detail/_noise.hpp"
#include "glm/detail/_noise.hpp"
#include "imgui/imgui.h"
#include "rendering/camera.hpp"
#include "stb/stb_image.h"


CLASS_DEFINITION(Component, PointLight)
CLASS_DEFINITION(Component, Skybox)

using namespace nlohmann;

Mesh* skyboxMesh;

void PointLight::Init()
{
	if (Scene::GetScene()) {
		if (Scene::GetScene()->light_mgr->lights.size() < 10) {
			id = Scene::GetScene()->light_mgr->lights.size();
			Scene::GetScene()->light_mgr->lights.push_back(this);
		}
	}

	if (!entity->GetComponent<Billboard>()) {
		Billboard* billboard = entity->AttachComponent<Billboard>();

		billboard->texture = Texture::Load("Assets/Editor/icons/svgs/solid/lightbulb.png", false);

		billboard->Init();
	} else
	{
		Billboard* billboard = entity->GetComponent<Billboard>();

		billboard->texture = Texture::Load("Assets/Editor/icons/svgs/solid/lightbulb.png", false);
	}

}

void PointLight::Update()
{
	enabled = entity->enabled;

	if (entity->GetComponent<Billboard>())
	{
		entity->GetComponent<Billboard>()->color = color;
		entity->GetComponent<Billboard>()->showInEditorOnly = true;
	}
}

void PointLight::Unload()
{
	Scene::GetScene()->light_mgr->RemoveLight(this);
}

void PointLight::EngineRender()
{
	float lcol[3] = {color.r, color.g, color.b};

	ImGui::ColorPicker3("Light Color", lcol);

	color = { lcol[0], lcol[1], lcol[2] };

	ImGui::SliderFloat("Light Power", &power, 0, 1000);

	ImGui::DragFloat("Near Plane", &near_plane, 0.1f, 0.0001f, far_plane-0.001f);
	ImGui::DragFloat("Far Plane", &far_plane, 0.1f, near_plane+0.001f, 1000.0f);
}

std::string PointLight::GetIcon()
{
	return icon;
}

std::string PointLight::PrintToJSON()
{

	json j;

	for (int i = 0; i < 3; ++i)
	{
		j["color"][i] = color[i];
	}

	j["power"] = power;
	j["id"] = id;

	return j.dump(JSON_INDENT_AMOUNT);
}

void PointLight::LoadFromJSON(nlohmann::json data)
{
	for (int i = 0; i < 3; ++i)
	{
		color[i] = data["color"][i];
	}

	power = data["power"];
	id = data["id"];
}

Shader* simpleDepthShader = nullptr;

std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview)
{
	const auto inv = glm::inverse(projview);

	std::vector<glm::vec4> frustumCorners;
	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}

	return frustumCorners;
}

const unsigned int SHADOWMAP_RESOLUTION = 1024;


std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
	return getFrustumCornersWorldSpace(proj * view);
}

glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane, mat4 view, vec3 lightDir)
{
	const auto proj = glm::perspective(
		glm::radians(70.0f), (float)SHADOWMAP_RESOLUTION / (float)SHADOWMAP_RESOLUTION, nearPlane,
		farPlane);
	const auto corners = getFrustumCornersWorldSpace(proj, view);

	glm::vec3 center = vec3{0};
	for (const auto& v : corners)
	{
		center += glm::vec3(v);
	}
	center /= corners.size();

	const auto lightView = glm::lookAt(center+lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();
	for (const auto& v : corners)
	{
		const auto trf = lightView * v;
		minX = std::min(minX, trf.x);
		maxX = std::max(maxX, trf.x);
		minY = std::min(minY, trf.y);
		maxY = std::max(maxY, trf.y);
		minZ = std::min(minZ, trf.z);
		maxZ = std::max(maxZ, trf.z);
	}

	// Tune this parameter according to the scene
	constexpr float zMult = 10.0f;
	if (minZ < 0)
	{
		minZ *= zMult;
	}
	else
	{
		minZ /= zMult;
	}
	if (maxZ < 0)
	{
		maxZ /= zMult;
	}
	else
	{
		maxZ *= zMult;
	}

	const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
	return lightProjection * lightView;
}

vector<float> GetCascadeLevels(float cameraFarPlane)
{
	std::vector<float> shadowCascadeLevels{ cameraFarPlane / 50.0f, cameraFarPlane / 25.0f, cameraFarPlane / 10.0f, cameraFarPlane / 2.0f };
	return shadowCascadeLevels;
}

std::vector<glm::mat4> getLightSpaceMatrices(float cameraFarPlane, float cameraNearPlane, mat4 v, vec3 d)
{
	vector<float> shadowCascadeLevels = GetCascadeLevels(cameraFarPlane);
	std::vector<glm::mat4> ret;
	for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
	{
		if (i == 0)
		{
			ret.push_back(getLightSpaceMatrix(cameraNearPlane, shadowCascadeLevels[i],v,d));
		}
		else if (i < shadowCascadeLevels.size())
		{
			ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i],v,d));
		}
		else
		{
			ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], cameraFarPlane,v,d));
		}
	}
	return ret;
}

void PointLight::CalcShadowMap(int prebuffer)
{
	//near_plane = 0.01f;
	//far_plane = 500.0f;

	//vector<float> shadowCascadeLevels = GetCascadeLevels(far_plane);
	if (shadowMap == 0)
	{
		glGenFramebuffers(1, &depthMapFBO);
		// create depth texture
		unsigned int depthMap;
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, prebuffer);

		shadowMap = depthMap;
	}

	if (simpleDepthShader == nullptr)
	{
		simpleDepthShader = new Shader("Assets/Editor/shaders/7");
	}


	glm::mat4 lightProjection, lightView;
	lightProjection = glm::perspective(glm::radians(70.0f), (GLfloat)SHADOWMAP_RESOLUTION / (GLfloat)SHADOWMAP_RESOLUTION, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
	//lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);

	lightView = glm::lookAt(entity->transform->position, vec3{ 0 }, glm::vec3(0.0, 1.0, 0.0));

	spaceMatrix = lightProjection * lightView;

	glViewport(0, 0, SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	//simpleDepthShader->reload();
	simpleDepthShader->use();
	simpleDepthShader->setMat4("lightSpaceMatrix", spaceMatrix);
	simpleDepthShader->setFloat("near_plane", near_plane);
	simpleDepthShader->setFloat("far_plane", far_plane);

	glCullFace(GL_FRONT);

	for (RenderData* render_obj : RenderMgr::renderObjs)
	{
		if (render_obj->castShadow) {

			simpleDepthShader->setMat4("model", render_obj->matrix);

			render_obj->mesh->Draw(render_obj->instanced, render_obj->instances);
		}
	}

	glCullFace(GL_BACK);

	glBindFramebuffer(GL_FRAMEBUFFER, prebuffer);
	glViewport(0, 0, RenderMgr::GetSceneWinSize().x, RenderMgr::GetSceneWinSize().y);
}

void Skybox::Init()
{
	if (skyboxMesh == nullptr)
	{
		skyboxMesh = Mesh::Load("Assets/Editor/models/cube.fbx", 0);
		entity->material->LoadShader(new Shader("Assets/Editor/shaders/1/"));
	}

	if (Scene::GetScene()) {
		Scene::GetScene()->light_mgr->sky = this;
	}
}

void Skybox::Update()
{
	
	RenderData* render_data = new RenderData;
	
	render_data->mesh = skyboxMesh;
	render_data->material = entity->material;

	if (entity->material->textures.size() <= 0)
	{
		entity->material->textures.push_back(cubemap_texture);
	}
	render_data->material->textures[0] = cubemap_texture;

	mat4 mat(1.0);

	mat = scale(mat, vec3{ 1000.0f });

	render_data->matrix = mat;

	render_data->useDepthMask = false;
	render_data->deferred = false;
	

	RenderMgr::renderObjs.push_back(render_data);
	
}

void Skybox::EngineRender()
{
	string filePath, filePathName;
	bool opened = false;

	if (opened)
	{
		cubemap_texture.Unload();

		glDeleteTextures(1, &prefilterMap);
		glDeleteTextures(1, &irradianceMap);

		LoadTexture(filePathName);

	}
}

unsigned int captureFBO;
unsigned int captureRBO;

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			 // bottom face
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 // top face
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void Skybox::LoadTexture(std::string path)
{

	if (captureFBO == 0)
	{
		glGenFramebuffers(1, &captureFBO);
		glGenRenderbuffers(1, &captureRBO);
	}

	Shader equirectangularToCubemapShader("Assets/Editor/shaders/6/", false, false);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	// pbr: load the HDR environment std::map
	// ---------------------------------
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
	unsigned int hdrTexture;
	if (data)
	{
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load HDR image." << std::endl;
	}

	// pbr: setup cubemap to render to and attach to framebuffer
	// ---------------------------------------------------------
	unsigned int envCubemap;
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
	// ----------------------------------------------------------------------------------------------
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// pbr: convert HDR equirectangular environment std::map to cubemap equivalent
	// ----------------------------------------------------------------------
	equirectangularToCubemapShader.use();
	equirectangularToCubemapShader.setInt("equirectangularMap", 0);
	equirectangularToCubemapShader.setMat4("projection", captureProjection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		equirectangularToCubemapShader.setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	CubemapTexture texture = {};

	texture.ID = envCubemap;
	texture.height = height;
	texture.width = width;
	texture.type = "texture_cubemap";
	texture.path = path;
	texture.isCubemap = true;

	cubemap_texture = texture;

	Shader irradianceShader("Assets/Editor/shaders/4/", false, false);
	Shader prefilterShader("Assets/Editor/shaders/5/", false, false);

	// pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	// pbr: solve diffuse integral by convolution to create an irradiance (cube)std::map.
	// -----------------------------------------------------------------------------
	irradianceShader.use();
	irradianceShader.setInt("environmentMap", 0);
	irradianceShader.setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture.ID);

	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		irradianceShader.setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
	// --------------------------------------------------------------------------------
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)std::map.
	// ----------------------------------------------------------------------------------------------------
	prefilterShader.use();
	prefilterShader.setInt("environmentMap", 0);
	prefilterShader.setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture.ID);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = static_cast<unsigned int>(128 * pow(0.5, mip));
		unsigned int mipHeight = static_cast<unsigned int>(128 * pow(0.5, mip));
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilterShader.setFloat("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilterShader.setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderCube();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::string Skybox::PrintToJSON()
{

	json j;

	j["texture_path"] = cubemap_texture.path;

	return j.dump(JSON_INDENT_AMOUNT);
}

void Skybox::LoadFromJSON(json data)
{
	if (data.contains("texture_path")) {
		LoadTexture(data["texture_path"]);
	}
}

DeferredLightingData::DeferredLightingData(int w, int h)
{
	Generate(w, h);
}

void DeferredLightingData::Resize(vec2 size)
{
	this->size = size;

	glDeleteFramebuffers(1, &gBuffer);

	glDeleteTextures(1, &gPos);
	glDeleteTextures(1, &gNrm);
	glDeleteTextures(1, &gAlb);
	glDeleteTextures(1, &gSha);

	gBuffer = 0;

	Generate(size.x, size.y);
}

void DeferredLightingData::Generate(int w, int h)
{
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	glm::vec2 screenSize = { w, h };

	size = screenSize;

	// position color buffer
	glGenTextures(1, &gPos);
	glBindTexture(GL_TEXTURE_2D, gPos);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenSize.x, screenSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPos, 0);
	// normal color buffer
	glGenTextures(1, &gNrm);
	glBindTexture(GL_TEXTURE_2D, gNrm);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenSize.x, screenSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNrm, 0);
	// color + specular color buffer
	glGenTextures(1, &gAlb);
	glBindTexture(GL_TEXTURE_2D, gAlb);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenSize.x, screenSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlb, 0);
	glGenTextures(1, &gSha);
	glBindTexture(GL_TEXTURE_2D, gSha);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenSize.x, screenSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gSha, 0);

	glGenTextures(1, &gTrm);
	glBindTexture(GL_TEXTURE_2D, gTrm);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenSize.x, screenSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gTrm, 0);

	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, attachments);
	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenSize.x, screenSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LightingMgr::EditMaterial(Material* material)
{
	if (captureFBO == 0)
	{
		glGenFramebuffers(1, &captureFBO);
		glGenRenderbuffers(1, &captureRBO);
	}

	bool hasBRDF=false, hasIrradiance=false, hasPrefilter=false, hasShadowMap=false;
	for (int i = 0; i < material->textures.size(); ++i)
	{

		Texture t = material->textures[i];


		if (t.type == "texture_brdf");
		{
			hasBRDF = true;
		}
		if (t.type == "texture_rad")
		{
			if (sky)
			{
				t.ID = sky->irradianceMap;
			}
			hasIrradiance = true;
		}
		if (t.type == "texture_pref")
		{
			if (sky)
			{
				t.ID = sky->prefilterMap;
			}
			hasPrefilter = true;
		}
		if (t.type == "texture_shad")
		{
			hasShadowMap = true;
		}
	}

	if (!hasBRDF)
	{
		Texture t = Texture{};

		t.ID = GetBRDFTexture();
		t.type = "texture_brdf";

		material->textures.push_back(t); 
	}


	if (sky) {
		try
		{
			if (!hasIrradiance)
			{
				if (sky->irradianceMap == 0)
				{
				}
				else {
					Texture t = { };

					t.ID = sky->irradianceMap;
					t.type = "texture_rad";
					t.isCubemap = true;

					material->textures.push_back(t);
				}
			}

			if (!hasPrefilter)
			{
				if (sky->prefilterMap == 0)
				{

				}
				else {
					Texture t = { };

					t.ID = sky->prefilterMap;
					t.type = "texture_pref";
					t.isCubemap = true;

					material->textures.push_back(t);
				}
			}
		}
		catch (...)
		{
		}
	}

	int ctr = 0;
	for (int i = 0; i < 10; i++)
	{
		std::string light_un = "lights[" + to_string(ctr) + "]";
		if (i < lights.size()) {
			PointLight* light = lights[i];
			material->entity->material->uniforms[(light_un + ".position")].v3 = light->entity->transform->position;
			material->entity->material->uniforms[(light_un + ".direction")].v3 = light->entity->transform->GetEulerAngles();
			material->entity->material->uniforms[(light_un + ".color")].v3 = light->color;
			material->entity->material->uniforms[(light_un + ".enabled")].b = light->enabled;
			material->entity->material->uniforms[(light_un + ".power")].f = light->power;

			material->entity->material->uniforms[(light_un + ".lightSpaceMatrix")].m4 = light->spaceMatrices[0];

			if (!hasShadowMap && light->shadowMap != 0)
			{
				Texture t = { };

				t.ID = light->shadowMap;
				t.type = "texture_shad";
				t.isCubemap = false;

				material->textures.push_back(t);
			}
		} else
		{
			material->entity->material->uniforms[(light_un + ".enabled")].b = false;
		}
		ctr++;
	}
}

void LightingMgr::RemoveLight(PointLight* light)
{
	int lid = light->id;

	for (PointLight* value : lights)
	{
		if (value->id > lid)
		{
			value->id -= 1;
		}
	}

	lights.erase(lights.begin() + lid);

	//std::cout << "removed light at " << lid << std::endl;

}

void LightingMgr::Unload()
{
	lights.clear();
	sky = nullptr;

	glDeleteFramebuffers(1, &gBuffer);

	glDeleteTextures(1, &gPosition);
	glDeleteTextures(1, &gNormal);
	glDeleteTextures(1, &gAlbedoSpec);
	glDeleteTextures(1, &gCombined);

	gBuffer = 0;
}

GLuint LightingMgr::GetBRDFTexture()
{

	static unsigned int brdfLUTTexture;

	if (brdfLUTTexture == 0)
	{
		if (captureFBO == 0)
		{
			glGenFramebuffers(1, &captureFBO);
			glGenRenderbuffers(1, &captureRBO);
		}

		//glDeleteTextures(1, &brdfLUTTexture);


		if (brdfLUTTexture == 0) {
			glGenTextures(1, &brdfLUTTexture);
		}

		Shader brdfShader("Assets/Editor/shaders/3/", false, false);

		brdfShader.reload();

		// pre-allocate enough memory for the LUT texture.
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
		// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

		glViewport(0, 0, 512, 512);
		brdfShader.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ShapeMgr::DrawQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	return brdfLUTTexture;
}
