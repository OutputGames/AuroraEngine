#include "input.hpp"

#include "rendering/camera.hpp"
#include "rendering/render.hpp"

float lastX;
float lastY;
bool firstMouse = true;

vec2 MousePos;
vec2 MouseDelta;

struct InputData
{
    vector<int> lastKeysPressed;
	std::vector<int> keysPressed;

	bool lastkeysPressedContains(int k)
	{
		for (int i = 0; i < lastKeysPressed.size(); ++i)
		{
			if (lastKeysPressed[i] == k)
			{
				return true;
				break;
			}
		}
		return false;
	}

	bool keysPressedContains(int k)
	{
		for (int i = 0; i < keysPressed.size(); ++i)
		{
			if (keysPressed[i] == k)
			{
				return true;
				break;
			}
		}
		return false;
	}
};

InputData* prevInputData;

bool Engine::InputMgr::IsKeyUp(int key)
{
	return !ImGui::IsKeyDown(static_cast<ImGuiKey>(key));
}

bool Engine::InputMgr::IsKeyDown(int key)
{
	return ImGui::IsKeyDown(static_cast<ImGuiKey>(key));
}

bool Engine::InputMgr::IsKeyPressed(int key)
{
	return ImGui::IsKeyPressed(static_cast<ImGuiKey>(key));
}

bool Engine::InputMgr::IsMouseButtonPressed(int button)
{
	return ImGui::IsMouseClicked(button);
}

bool Engine::InputMgr::IsMouseButtonDown(int button)
{
	return ImGui::IsMouseDown(button);
}

bool Engine::InputMgr::IsMouseButtonUp(int button)
{
	return !ImGui::IsMouseDown(button);
}


vec2 Engine::InputMgr::GetMousePos()
{
	return MousePos;
}

vec2 Engine::InputMgr::GetMouseDelta()
{
	return MouseDelta;
}

void Engine::InputMgr::Update()
{

	double mx, my;

	glfwGetCursorPos(GetWindow(), &mx, &my);

	vec2 initialMousePos = MousePos;

	MousePos = { mx,my };

	MouseDelta = MousePos - initialMousePos;

	prevInputData->lastKeysPressed = prevInputData->keysPressed;
	prevInputData->keysPressed.clear();
}

void Engine::InputMgr::Init()
{
	prevInputData = new InputData;
	lastX = RenderMgr::GetWindowSize().x;
	lastY = RenderMgr::GetWindowSize().y;
}

void Engine::InputMgr::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		prevInputData->keysPressed.push_back(key);
	}
}

void Engine::InputMgr::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;
}

void Engine::InputMgr::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
}
