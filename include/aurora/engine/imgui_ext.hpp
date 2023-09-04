#ifndef IMGUI_EXT

#define IMGUI_EXT

#include "utils/utils.hpp"

namespace ImGui {
    bool FileDialog(string* path, string formats, string& filePathName, string& filePath);
}

#endif