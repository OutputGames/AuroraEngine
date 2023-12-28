#ifndef INPUT_HPP
#define INPUT_HPP

#include "utils.hpp"

namespace Engine
{
    AURORA_API struct InputMgr
    {
        AURORA_API static bool IsKeyUp(int key);
        AURORA_API static bool IsKeyDown(int key);
        AURORA_API static bool IsKeyPressed(int key);
        AURORA_API static bool IsMouseButtonPressed(int button);
        AURORA_API static bool IsMouseButtonDown(int button);
        AURORA_API static bool IsMouseButtonUp(int button);
        static void Update();

        static void Init();
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

        // glfw: whenever the mouse scroll wheel scrolls, this callback is called
        // ----------------------------------------------------------------------
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    	AURORA_API static vec2 GetMousePos();
    	AURORA_API static vec2 GetMouseDelta();
    };

} // namespace Engine





#endif