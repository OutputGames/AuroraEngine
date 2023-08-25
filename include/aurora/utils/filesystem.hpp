#ifndef FS_HPP
#define FS_HPP

#include "utils.hpp"

namespace Engine {
    struct Filesystem {
        static std::string ReadFileString(std::string path);
        static bool WriteFileString(string path, string data);
        static void DeleteFile(string path);
        static std::string GetCurrentDir();
    };
};

#endif