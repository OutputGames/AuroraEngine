#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "utils/utils.hpp"

struct Texture
{
    unsigned int ID;

    std::string type, path;

    int height, width;

    static Texture Load(std::string path, bool flip=false);

    void Unload();

};

struct CubemapTexture : Texture
{

    static CubemapTexture Load(vector<std::string> faces);

};

#endif