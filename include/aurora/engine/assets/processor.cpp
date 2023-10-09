#include "processor.hpp"

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
	return FileAsset;
}

string Prefab::ToString()
{
	json e = ToJson();

	return e.dump();
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

	json m;

	json mu;

	for (pair<const string, Material::UniformData> uniform : entity.material->uniforms)
	{
		json u;

		Material::UniformData* uniformD = &uniform.second;

		u["type"] = uniformD->type;
		u["b"] = uniformD->b;
		u["i"] = uniformD->i;
		u["f"] = uniformD->f;
		u["v2"] = { uniformD->v2.x, uniformD->v2.y };
		u["v3"] = { uniformD->v3.x, uniformD->v3.y, uniformD->v3.z };
		u["v4"] = { uniformD->v4.x, uniformD->v4.y, uniformD->v4.z, uniformD->v4.w };


		mu[uniform.first] = u;

	}

	m["uniforms"] = mu;
	m["shader"] = entity.material->shader->shaderDirectory;

	e["material"] = m;

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
	assetImportFuncs[Asset::AnimationAsset] = [](AssetProcessor* processor, string path) {return processor->ImportAsset<AssetImporter>(path); };

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
	for (Asset* cached_file : cachedFiles)
	{
		if (filesystem::exists(cached_file->path)) {
			std::replace(cached_file->path.begin(), cached_file->path.end(), '/', '\\');
			filesystem::file_time_type lwt = filesystem::last_write_time(cached_file->path);
			if (cached_file->lastWriteTime != lwt)
			{
				cout << cached_file->path << " was modified, running " << Asset::AssetTypeToString(cached_file->type) << " processor" << endl;
				cached_file->lastWriteTime = lwt;
			}
		} else
		{
			cachedFiles.erase(cachedFiles.begin() + ctr);
		}
		ctr++;
	}
}

bool AssetProcessor::IsFileCached(string path)
{
	std::replace(path.begin(), path.end(), '/', '\\');
	for (Asset* cached_file : cachedFiles)
	{
		if (cached_file->path == path)
		{
			return true;
			break;
		}
	}
	return false;
}

void AssetProcessor::ImportNewAsset(string path)
{
	std::replace(path.begin(), path.end(), '/', '\\');
	Asset::AssetType type = Asset::GetTypeFromExtension(path);
	Asset* asset = assetImportFuncs[type](this, path);
	asset->path = path;
	asset->lastWriteTime = filesystem::last_write_time(path);
	cachedFiles.push_back(asset);
}

void AssetImporter::SetupProcessors()
{
	for (AssetPostprocessor* processor : AssetPostprocessor::processors)
	{
		processor->importer = this;
		processor->assetPath = importPath;
	}
}

ShaderAsset* ShaderImporter::ImportAsset()
{

	cout << "importing shader " << importPath << endl;

	Shader* shader = new Shader(importPath, IsGeometryShader, LogShader);

	ShaderAsset* asset = new ShaderAsset;

	asset->shader = shader;

	return asset;
}

Prefab* PrefabImporter::ImportAsset()
{

	for (AssetPostprocessor* processor : AssetPostprocessor::processors)
	{
		processor->OnPreprocessPrefab();
	}

	//cout << "importing prefab " << importPath << endl;

	string data = Filesystem::ReadFileString(importPath);

	Prefab* prefab = new Prefab(Entity::Load(data));

	for (AssetPostprocessor* processor : AssetPostprocessor::processors)
	{
		processor->OnPostprocessPrefab(prefab);
	}

	return prefab;
}

ModelAsset* ModelImporter::ImportAsset()
{

	for (AssetPostprocessor* processor : AssetPostprocessor::processors)
	{
		processor->OnPreprocessModel();
	}

	cout << "importing model " << importPath << endl;

	ModelAsset* asset = new ModelAsset;

	Model::ModelImportSettings* settings = new Model::ModelImportSettings;
	settings->globalScale = GlobalScale;

	asset->model = Model::LoadModel(importPath, settings);

	Prefab* prefab = new Prefab(asset->model->LoadEntityPrefab(importPath, asset->model));

	asset->prefab = prefab;

	for (AssetPostprocessor* processor : AssetPostprocessor::processors)
	{
		processor->OnPostprocessModel(asset);
	}
	return asset;
}
