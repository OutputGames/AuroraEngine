#ifndef PROJECT_HPP
#define PROJECT_HPP

#include "entity.hpp"
#include "utils/utils.hpp"

struct AssetProcessor;

struct AURORA_API Project
{
    std::string name;
    void Save();
    static Project* Load(std::string path);
    static Project* Create(std::string path, std::string name="");

    void Init();

    static Project* GetProject();

    void LoadScene(std::string name);
    void LoadScenePath(std::string p);

    static bool ProjectLoaded();

    AssetProcessor* processor;

    string GetAssetPath() { return assetPath; }
    string GetPath() { return assetPath + "./"; };

private:
    friend Scene;

    Project(string path);

    std::string save_path;
    string assetPath;

    std::vector<Scene*> scenes;
    Scene* loaded_scene;
};


#endif