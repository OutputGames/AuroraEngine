#ifndef UTILS_HPP
#define UTILS_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include <numeric>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "json.hpp"

#include "imgui/ImGuiFileDialog.h"
#include "imgui/ImGuizmo.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"
#include "imgui/TextEditor.h"
#include "imgui/icons/IconsFontAwesome6.h"

#include "engine/log.hpp"

using namespace glm;
using namespace std;
using namespace nlohmann;

#define flt (float)

#ifndef PI
#define PI 3.14159265358979323846f
#endif

#ifndef EPSILON
#define EPSILON 0.000001f
#endif

#ifndef DEG2RAD
#define DEG2RAD (PI/180.0f)
#endif

#ifndef RAD2DEG
#define RAD2DEG (180.0f/PI)
#endif

#ifndef AURORA_DLLBUILD
#define AURORA_API __declspec(dllexport)
#else
#define AURORA_API __declspec(dllimport)
#endif

#define JSON_INDENT_AMOUNT 4


GLFWwindow* GetWindow();

#endif