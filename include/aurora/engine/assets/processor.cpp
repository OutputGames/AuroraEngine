#include "processor.hpp"

#include "graphics/animation.hpp"
#include "utils/filesystem.hpp"

using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

vector<AssetPostprocessor*> AssetPostprocessor::processors;

string Asset::AssetTypeToString(AssetType type)
{
	switch (type)
	{
	case FileAsset:
		break;
	case TextAsset:
		return "TextFile";
	case ScriptAsset:
		return "Script";
	case ModelAsset:
		return "Model";
	case ShaderAsset:
		return "Shader";
	case TextureAsset:
		return "Texture";
	case PrefabAsset:
		return "Prefab";
	case MaterialAsset:
		return "Material";
	default:
		break;
	}
	return "File";
}

Asset::AssetType Asset::GetTypeFromExtension(string path)
{
	filesystem::path fspath(path);

	string ext = fspath.extension().string();

	if (ext == ".png" || ext == ".jpg")
	{
		return TextureAsset;
	}
	if (ext == ".cs")
	{
		return ScriptAsset;
	}
	if (ext == ".txt")
	{
		return TextAsset;
	}
	if (ext == ".fbx")
	{
		return ModelAsset;
	}
	if (ext == ".prefab")
	{
		return PrefabAsset;
	}
	if (ext == ".smd")
	{
		return AnimationAsset;
	}
	if (ext == ".mat")
	{
		return MaterialAsset;
	}
	if (ext == ".shader") { return ShaderAsset; }
	return FileAsset;
}

string Prefab::ToString()
{
	json e = ToJson();

	return e.dump(JSON_INDENT_AMOUNT);
}

json Prefab::ToJson()
{
	json e;
	e["name"] = entity.name;

	Transform* t = entity.transform;

	e["transform"]["position"] = { t->position.x, t->position.y,t->position.z };
	for (int i = 0; i < 3; ++i)
	{
		e["transform"]["rotation"][i] = t->rotation[i];
	}
	e["transform"]["scale"] = { t->scale.x, t->scale.y,t->scale.z };

	json ch;

	int ctr = 0;

	for (shared_ptr<Transform> child : entity.transform->children)
	{
		Prefab* c_prefab = new Prefab(child->entity);

		ch[ctr] = c_prefab->ToJson();
		ctr++;
	}
	
	e["transform"]["children"] = ch;

	e["id"] = entity.id;

	e["enabled"] = entity.enabled;

	json c;

	for (auto const& component : entity.components)
	{
		c[component->GetName()] = json::parse(component.get()->PrintToJSON());
	}

	e["components"] = c;

	e["material"] = entity.material->Export();

	return e;
}

void AssetProcessor::SetupImporters()
{

	// Importers

	assetImportFuncs[Asset::FileAsset] = [](AssetProcessor* processor, string path) {return processor->ImportAsset<AssetImporter>(path); };
	assetImportFuncs[Asset::TextAsset] = [](AssetProcessor* processor, string path) {return processor->ImportAsset<AssetImporter>(path); };
	assetImportFuncs[Asset::ScriptAsset] = [](AssetProcessor* processor, string path) {return processor->ImportAsset<AssetImporter>(path); };
	assetImportFuncs[Asset::ModelAsset] = [](AssetProcessor* processor, string path) {return processor->ImportAsset<ModelImporter>(path); };
	assetImportFuncs[Asset::ShaderAsset] = [](AssetProcessor* processor, string path) {return processor->ImportAsset<ShaderImporter>(path); };
	assetImportFuncs[Asset::TextureAsset] = [](AssetProcessor* processor, string path) {return processor->ImportAsset<AssetImporter>(path); };
	assetImportFuncs[Asset::PrefabAsset] = [](AssetProcessor* processor, string path) {return processor->ImportAsset<PrefabImporter>(path); };
	assetImportFuncs[Asset::AnimationAsset] = [](AssetProcessor* processor, string path) {return processor->ImportAsset<AnimationImporter>(path); };
	assetImportFuncs[Asset::MaterialAsset] = [](AssetProcessor* processor, string path) {return processor->ImportAsset<MaterialImporter>(path); };

	// Post processors

	AssetPostprocessor::processors.push_back(new AssetPostprocessor);

}

void AssetProcessor::Init(string path)
{

	SetupImporters();

	for (const auto& dirEntry : recursive_directory_iterator(path)) {
		filesystem::path dirPath(dirEntry);
		if (!dirEntry.is_directory())
		{
			ImportNewAsset(dirPath.string());
		}
	}
	project_path = path;
}

void AssetProcessor::CheckForEdits()
{
	for (const auto& dirEntry : recursive_directory_iterator(project_path)) {
		filesystem::path dirPath(dirEntry);
		if (!dirEntry.is_directory())
		{
			if (!IsFileCached(dirPath.string()))
			{
				ImportNewAsset(dirPath.string());
			}
		}
	}
	int ctr = 0;
	for (const auto& [cachedPath, cached_file] : cachedFiles)
	{
		if (cached_file) {
			if (filesystem::exists(cached_file->path)) {
				std::replace(cached_file->path.begin(), cached_file->path.end(), '/', '\\');
				filesystem::file_time_type lwt = filesystem::last_write_time(cached_file->path);
				if (cached_file->lastWriteTime != lwt)
				{
					cout << cached_file->path << " was modified, running " << Asset::AssetTypeToString(cached_file->type) << " processor" << endl;
					ReloadAsset(cachedPath);
				}
			}
			else
			{
				cachedFiles.erase(cachedPath);
			}
		}
		ctr++;
	}
}

vector<Asset*> AssetProcessor::GetAssetsOfType(Asset::AssetType type)
{
	vector<Asset*> assets;
	for (const auto& [cachedPath ,cached_file] : cachedFiles)
	{
		if (cached_file->type == type)
		{
			assets.push_back(cached_file);
		}
	}

	return assets;
}

void AssetProcessor::CreateDefaultAsset(string path, Asset::AssetType type)
{
	switch (type)
	{
	default:
		Filesystem::CreateFile(path);
		break;
	case Asset::ShaderAsset:
	{
		string data = Shader::CreateShader();

		Filesystem::WriteFileString(path, data);
		break;
	}
	case Asset::MaterialAsset:
	{
		string data = (new Material())->Export();

		Filesystem::WriteFileString(path, data);
	}
	}
}

bool AssetProcessor::IsFileCached(string path)
{
	std::replace(path.begin(), path.end(), '/', '\\');

	return (cachedFiles.find(path) != cachedFiles.end());
}

void AssetProcessor::ImportNewAsset(string path)
{
	std::replace(path.begin(), path.end(), '/', '\\');
	Asset::AssetType type = Asset::GetTypeFromExtension(path);
	Asset* asset = assetImportFuncs[type](this, path);
	asset->path = path;
	asset->lastWriteTime = filesystem::last_write_time(path);
	asset->type = type;
	cachedFiles.insert({ asset->path,asset });
}

void AssetProcessor::ReloadAsset(string path)
{
	std::replace(path.begin(), path.end(), '/', '\\');
	Asset::AssetType type = Asset::GetTypeFromExtension(path);
	Asset* asset = assetImportFuncs[type](this, path);
	asset->path = path;
	asset->lastWriteTime = filesystem::last_write_time(path);
	asset->type = type;
	cachedFiles[asset->path] = asset;
}

void AssetImporter::SetupProcessors()
{
	for (AssetPostprocessor* processor : AssetPostprocessor::processors)
	{
		processor->importer = this;
		processor->assetPath = importPath;
	}
}

void AssetImporter::Preprocess(Asset::AssetType type)
{
	switch (type)
	{
	case Asset::PrefabAsset:
		for (AssetPostprocessor* processor : AssetPostprocessor::processors)
		{
			processor->OnPreprocessPrefab();
		}
		break;
	case Asset::ModelAsset:
		for (AssetPostprocessor* processor : AssetPostprocessor::processors)
		{
			processor->OnPreprocessModel();
		}
		break;
	case Asset::MaterialAsset:
		for (AssetPostprocessor* processor : AssetPostprocessor::processors)
		{
			processor->OnPreprocessMaterial();
		}
		break;
	default:
		break;
	}
}

void AssetImporter::Postprocess(Asset::AssetType type, Asset* asset)
{
	switch (type)
	{
	case Asset::PrefabAsset:
		for (AssetPostprocessor* processor : AssetPostprocessor::processors)
		{
			processor->OnPostprocessPrefab(static_cast<Prefab*>(asset));
		}
		break;
	case Asset::ModelAsset:
		for (AssetPostprocessor* processor : AssetPostprocessor::processors)
		{
			processor->OnPostprocessModel(static_cast<ModelAsset*>(asset));
		}
		break;
	case Asset::MaterialAsset:
		for (AssetPostprocessor* processor : AssetPostprocessor::processors)
		{
			processor->OnPostprocessMaterial(static_cast<MaterialAsset*>(asset));
		}
		break;
	default:
		break;
	}
}

ShaderAsset* ShaderImporter::ImportAsset()
{

	cout << "importing shader " << importPath << endl;

	Shader* shader = new Shader(importPath.c_str(), true);

	ShaderAsset* asset = new ShaderAsset;

	asset->shader = nullptr;

	return asset;
}

Prefab* PrefabImporter::ImportAsset()
{

	//cout << "importing prefab " << importPath << endl;

	string data = Filesystem::ReadFileString(importPath);

	Prefab* prefab = new Prefab(Entity::Load(data));

	return prefab;
}

ModelAsset* ModelImporter::ImportAsset()
{

	//cout << "importing model " << importPath << endl;

	ModelAsset* asset = new ModelAsset;

	Model::ModelImportSettings* settings = new Model::ModelImportSettings;
	settings->globalScale = GlobalScale;

	asset->model = Model::LoadModel(importPath, settings);

	Prefab* prefab = new Prefab(asset->model->LoadEntityPrefab(importPath, asset->model, DefaultShader));

	asset->prefab = prefab;

	asset->prefab->GetEntity()->transform->scale = { GlobalScale,GlobalScale,GlobalScale };

	return asset;
}

ScriptAsset* ScriptImporter::ImportAsset()
{
	ScriptAsset* script = new ScriptAsset;



	return script;
}

AnimationAsset* AnimationImporter::ImportAsset()
{
	AnimationAsset* asset = new AnimationAsset;

	asset->animation = new Animation(importPath);

	return asset;
}

MaterialAsset* MaterialImporter::ImportAsset()
{
	MaterialAsset* asset = new MaterialAsset;

	Material* material = new Material;

	material->LoadFromData(Filesystem::ReadFileString(importPath));

	asset->material = material;

	return asset;

}
