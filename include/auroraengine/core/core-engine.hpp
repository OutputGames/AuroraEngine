#ifndef CORE_ENGINE
#define CORE_ENGINE
#include "rendering/camera.hpp"

struct EngineCore
{
    static void InitEngine();
};

struct EditorCamera : CameraBase
{
	static EditorCamera* GetEditorCam();


	void Pan(vec2 delta);

	void Rotate(vec2 delta);

	void Zoom(float delta);

	float ZoomSpeed();
	float RotationSpeed();
	pair<float, float> PanSpeed();


};



#endif