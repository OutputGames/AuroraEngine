#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "render.hpp"
#include "graphics/animation.hpp"
#include "utils/utils.hpp"

struct Camera
{
	vec3 position, direction, lookat;
	mat4 GetViewMatrix();
	mat4 GetProjectionMatrix(float aspect);
	float FOV = 70, near = 0.01f, far = 10000.0f;
	bool useDirection;
	vec3 up={0,1,0}, right={1,0,0};
};

inline mat4 Camera::GetViewMatrix()
{
	vec3 center = position + direction;

	if (!useDirection)
	{
		center = lookat;
	}

	mat4 view = glm::lookAt(position, center, up);

	return view;
}

inline mat4 Camera::GetProjectionMatrix(float aspect)
{
	mat4 projection = glm::perspective(glm::radians(FOV), aspect, near, far);

	return projection;
}

static Camera* camera;

#endif
