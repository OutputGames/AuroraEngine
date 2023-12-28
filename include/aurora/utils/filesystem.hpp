#ifndef FS_HPP
#define FS_HPP

#include "utils.hpp"

struct Filesystem {
    AURORA_API static std::string ReadFileString(std::string path);
    AURORA_API static bool WriteFileString(string path, string data);
    AURORA_API static char* ReadFileBytes(string path, int& dataSize);
    AURORA_API static void DeleteFile(string path);
    AURORA_API static std::string GetCurrentDir();
    AURORA_API static void CopyRecursive(const filesystem::path& src, const filesystem::path& target) noexcept
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
    AURORA_API static void CreateFile(string path)
    {
        ofstream file(path);

        file.close();
    }
    AURORA_API static string ReplaceOccurences(std::string str, const std::string& from, const std::string& to);
};

#endif