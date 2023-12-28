#include "core-engine.hpp"


EditorCamera* EditorMain;

void EngineCore::InitEngine()
{
}


EditorCamera* EditorCamera::GetEditorCam()
{
    if (EditorMain == nullptr)
        EditorMain = static_cast<EditorCamera*>(CreateCamera());

    return EditorMain;
}

void EditorCamera::Pan(vec2 delta)
{
    auto [xSpeed, ySpeed] = PanSpeed();
    direction += -right * delta.x * xSpeed * FOV;
    direction += right * delta.y * ySpeed * FOV;
}

void EditorCamera::Rotate(vec2 delta)
{
    float yawSign = up.y < 0 ? -1.0f : 1.0f;
    direction.y += yawSign * delta.x * RotationSpeed();
    direction.z += delta.y * RotationSpeed();
}

void EditorCamera::Zoom(float delta)
{
    FOV -= delta * ZoomSpeed();
}

float EditorCamera::ZoomSpeed()
{
    float distance = FOV * 0.2f;
    distance = std::max(distance, 0.0f);
    float speed = distance * distance;
    speed = std::min(speed, 100.0f); // max speed = 100
    return speed;
}

float EditorCamera::RotationSpeed()
{
    return 0.8f;
}

pair<float, float> EditorCamera::PanSpeed()
{
    float x = std::min(framebuffer->width / 1000.0f, 2.4f); // max = 2.4f
    float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

    float y = std::min(framebuffer->height / 1000.0f, 2.4f); // max = 2.4f
    float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

    return { xFactor, yFactor };
}
