#ifndef PROJECT_HPP
#define PROJECT_HPP

#include "entity.hpp"
#include "utils/utils.hpp"

struct AURORA_API Project
{
    std::string name;
    void Save();
    static Project* Load(std::string path);
    static Project* Create(std::string path, std::string name="");

    static Project* GetProject();

    void LoadScene(std::string name);
    void LoadScenePath(std::string p);

    static bool ProjectLoaded();

private:
    friend Scene;

    std::string save_path;

    std::vector<Scene*> scenes;
    Scene* loaded_scene;
};


#endif