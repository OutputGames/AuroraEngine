#ifndef INPUT_HPP
#define INPUT_HPP

#include "utils.hpp"

namespace Engine
{
    AURORA_API struct InputMgr
    {
        static bool IsKeyUp(int key);
        static bool IsKeyDown(int key);
        static bool IsKeyPressed(int key);
        static void Update();

        static void Init();
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

        // glfw: whenever the mouse scroll wheel scrolls, this callback is called
        // ----------------------------------------------------------------------
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    };
    
} // namespace Engine





#endif