#include "filesystem.hpp"

std::string Filesystem::ReadFileString(std::string path)
{
      const std::ifstream input_stream(path, std::ios_base::binary);

    if (input_stream.fail()) {
        throw std::runtime_error("Failed to open file located at "+path);
    }

    std::stringstream buffer;
    buffer << input_stream.rdbuf();

    return buffer.str();  
}

bool Filesystem::WriteFileString(string path, string data)
{
    std::ofstream out(path);
    out << data;
    out.close();
    return true;
}

void Filesystem::DeleteFile(string path)
{
    filesystem::remove(path);
}

std::string Filesystem::GetCurrentDir()
{
    return std::filesystem::current_path().generic_string();
}
