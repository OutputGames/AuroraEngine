#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "render.hpp"
#include "graphics/animation.hpp"
#include "utils/utils.hpp"

struct CameraBase
{
	vec3 position = vec3{ 0 }, direction = vec3{ 0 }, lookat=vec3{0};
	AURORA_API mat4 GetViewMatrix();
	AURORA_API mat4 GetProjectionMatrix(float aspect);
	float FOV = 70, near = 0.01f, far = 10000.0f;
	bool useDirection=false;
	vec3 up={0,1,0}, right={1,0,0};

	RenderTexture* framebuffer;

	AURORA_API void Update(bool IsGameView=false);

	AURORA_API static CameraBase* CreateCamera();

	AURORA_API void CopyView(mat4 view);

private: 

	CameraBase() = default;
};

inline mat4 CameraBase::GetProjectionMatrix(float aspect)
{
	mat4 projection = glm::perspective(glm::radians(FOV), aspect, near, far);

	return projection;
}

class Camera : public Component
{
	CLASS_DECLARATION(Camera)

public:
	Camera(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	Camera() = default;

	void LateUpdate() override;
	void Init() override;

	mat4 GetViewMatrix() { return m_Cam->GetViewMatrix(); };
	mat4 GetProjMatrix() { return  m_Cam->GetProjectionMatrix(RenderMgr::GetAspect()); }

	float FieldOfView;

	void EngineRender() override;

	 AURORA_API static Camera* GetMain();

	RenderTexture* GetRenderTexture() { return m_Cam->framebuffer; };

private:
	CameraBase* m_Cam;

	bool isMain;

};


#endif
