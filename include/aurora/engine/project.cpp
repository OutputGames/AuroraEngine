#include "project.hpp"

#include "json.hpp"
#include "dirent/dirent.h"
#include "utils/filesystem.hpp"

#include "graphics/lighting.hpp"

using namespace nlohmann;

static Project* current_project;

void Project::Save()
{
	json j;
	j["name"] = name;

	json jscenes;

	for (Scene* scene : scenes)
	{
		jscenes[scene->name] = json::parse(scene->SaveScene());
	}

	j["scenes"] = jscenes;

	j["loadedScene"] = loaded_scene->name;

	std::string projFilePath = name + ".auproject";

	std::ofstream projFile(projFilePath, std::ofstream::out | std::ofstream::trunc);

	projFile << j.dump();

	projFile.close();

	std::cout << "Saved project sucessfully." << std::endl;
}

Project* Project::Load(std::string path)
{
	Project* p = new Project;

	std::filesystem::path projPath = std::filesystem::path(path);


	Filesystem::CopyRecursive("resources/", path + "/editor/");

	std::filesystem::current_path(projPath);

	std::string name = projPath.filename().string();

	std::ifstream f(path + "/" + name + ".auproject") ;
	json j = json::parse(f);

	
	current_project = p;

	p->name = j["name"];
	for (json scene : j["scenes"])
	{
		Scene* s = Scene::LoadScene(scene.dump(), true);
	}

	p->LoadScene(j["loadedScene"]);

	return p;
}

Project* Project::Create(std::string path, std::string name)
{
	if (!std::filesystem::is_directory(path) || !std::filesystem::exists(path)) { // Check if src folder exists
		std::filesystem::create_directory(path); // create src folder
	}
	else
	{
		//return nullptr;
	}

	Project* proj = new Project;

	proj->save_path = path;

	std::filesystem::path projPath = std::filesystem::path(path);


	Filesystem::CopyRecursive("resources/", path + "/editor/");

	std::filesystem::current_path(projPath);

	if (name == "")
	{
		name = projPath.filename().string();
	}

	std::string projFilePath = projPath.string()+"/" + name + ".auproject";

	std::ofstream projFile(projFilePath);

	nlohmann::json projJSON;

	projJSON["name"] = name;

	json scenesJSON;

	{
		json scene;

		scenesJSON["Main"] = scene;
	}

	projJSON["scenes"] = scenesJSON;

	projFile << projJSON.dump();

	projFile.close();

	std::filesystem::create_directory(projPath.string() + "/resources/");

	proj->name = name;

	proj->scenes.push_back(Scene::CreateScene("Main"));

	proj->LoadScene("Main");

	current_project = proj;

	return proj;
}

Project* Project::GetProject()
{
	return current_project;
}

void Project::LoadScene(std::string name)
{
	Scene* s = nullptr;

	for (Scene* value : scenes)
	{
		if (value->name == name)
		{
			s = value;
		}
	}

	loaded_scene = s;
}

void Project::LoadScenePath(std::string p)
{
	Scene* s = nullptr;
	std::filesystem::path pth(p);


	int idx = 0;
	for (Scene* value : scenes)
	{
		if (value->name == pth.stem().string())
		{
			s = value;
			break;
		}
		idx++;
	}

	//delete scenes[idx];
	scenes[idx] = Scene::LoadScene(Filesystem::ReadFileString(p), false);

	loaded_scene = s;
}

bool Project::ProjectLoaded()
{
	return current_project != nullptr;
}
