#ifndef FS_HPP
#define FS_HPP

#include "utils.hpp"

struct Filesystem {
    static std::string ReadFileString(std::string path);
    static bool WriteFileString(string path, string data);
    static void DeleteFile(string path);
    static std::string GetCurrentDir();
    static void CopyRecursive(const filesystem::path& src, const filesystem::path& target) noexcept
    {
        try
        {
            std::filesystem::copy(src, target, std::filesystem::copy_options::overwrite_existing | filesystem::copy_options::recursive);
        }
        catch (std::exception& e)
        {
            std::cout << e.what();
        }
    }
};

#endif