#include "project.hpp"

#include <future>

#include "json.hpp"
#include "assets/processor.hpp"
#include "dirent/dirent.h"
#include "utils/filesystem.hpp"

#include "graphics/lighting.hpp"
#include "utils/timer.hpp"

using namespace nlohmann;

static Project* current_project;

void Project::Save()
{
	if (current_project) {
		json j;
		j["Name"] = name;

		for (Scene* scene : scenes)
		{
			scene->SaveScene();
		}

		j["LoadedScene"] = loaded_scene->path;

		std::string projFilePath = name + ".auproject";

		std::ofstream projFile(projFilePath, std::ofstream::out | std::ofstream::trunc);

		projFile << j.dump(JSON_INDENT_AMOUNT);

		projFile.close();

		std::cout << "Saved project sucessfully." << std::endl;
	}
}

Project* Project::Load(std::string path)
{
	Project* p = new Project(path);
	Shader::UnloadAllShaders();

	std::filesystem::path projPath = std::filesystem::path(path);

	p->assetPath = projPath.string() + "/Assets/";


	Filesystem::CopyRecursive("resources/", path + "/Assets/Editor/");

	std::filesystem::current_path(projPath);

	std::string name = projPath.filename().string();

	std::ifstream f(path + "/" + name + ".auproject") ;
	json j = json::parse(f);

	
	current_project = p;

	p->name = j["Name"];

	p->LoadScenePath(j["LoadedScene"]);

	p->Init();

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

	std::filesystem::path projPath = std::filesystem::path(path);

	std::filesystem::create_directory(projPath.string() + "/Assets/");

	Project* proj = new Project(path);

	proj->assetPath = projPath.string() + "/Assets/";

	if (name == "")
	{
		name = projPath.filename().string();
	}

	std::string projFilePath = projPath.string()+"/" + name + ".auproject";

	std::ofstream projFile(projFilePath);

	nlohmann::json projJSON;

	projJSON["Name"] = name;

	projJSON["LoadedScene"] = "Scenes/Main.auscene";

	projFile << projJSON.dump(JSON_INDENT_AMOUNT);

	projFile.close();

	proj->name = name;

	proj->scenes.push_back(Scene::CreateScene("Main"));

	proj->LoadScene("Main");

	current_project = proj;

	proj->Init();

	return proj;
}

void ProcessEdits()
{
	Project::GetProject()->processor->CheckForEdits(true);
}


void Project::Init()
{
	processor = new AssetProcessor;
	processor->Init(assetPath);
	auto handle = std::async(launch::async, ProcessEdits);

	cout << "created asset processor thread" << endl;

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
	Scene* s = Scene::LoadScenePath(p);

	scenes.push_back(s);



	loaded_scene = s;
}

bool Project::ProjectLoaded()
{
	return current_project != nullptr;
}

Project::Project(string path)
{

	Filesystem::CopyRecursive("resources/", path + "/Assets/Editor/");

	std::filesystem::current_path(path);

	loaded_scene = nullptr;
	save_path = path;
}
