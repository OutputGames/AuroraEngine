#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "processor.hpp"
#include "processor.hpp"
#include "processor.hpp"
#include "engine/entity.hpp"
#include "utils/utils.hpp"
class Animation;

#define importer_friend friend struct AssetImporter;

struct Asset
{
    string path;
    filesystem::file_time_type lastWriteTime;

private:
    friend class AssetProcessor;

    enum AssetType
    {
        FileAsset = 0,
        TextAsset,
        ScriptAsset,
        ModelAsset,
        ShaderAsset,
        TextureAsset,
        PrefabAsset,
        AnimationAsset
    };

    static string AssetTypeToString(AssetType type);
    static AssetType GetTypeFromExtension(string path);
    AssetType type;
};

struct Prefab : Asset
{
	Entity* GetEntity()
	{

        return &entity;
	}

    Prefab(Entity* entity)
	{
        this->entity = *entity;
	}


    string ToString();
    json ToJson();

private:

    friend class PrefabImporter;

    Entity entity;;
};

struct TextAsset : Asset
{
	string GetData()
	{
        return data;
	}

    string data;
};

struct ScriptAsset : Asset
{
	
};

struct ModelAsset : Asset
{
    Model* model;
    Prefab* prefab;
};

struct ShaderAsset : Asset
{

    Shader* GetShader()
    {
        return shader;
    }

    Shader* shader;
};

struct AnimationAsset : Asset
{

    Animation* GetAnimation();

    Animation* animation;
};

struct AssetProcessor
{
	void SetupImporters();
    void Init(string path);
    AURORA_API void CheckForEdits();

    template <typename T>

    T* GetAsset(string path);

private:

    bool IsFileCached(string path);
    vector<Asset*> cachedFiles;
    string project_path;
    unordered_map<Asset::AssetType, function<Asset* (AssetProcessor*, string)>> assetImportFuncs;

    template <typename Importer>
    Asset* ImportAsset(string path);

    void ImportNewAsset(string path);
};

template <typename T>
T* AssetProcessor::GetAsset(string path)
{
    string filePath = filesystem::path(path).string();
	std::replace(filePath.begin(), filePath.end(), '/', '\\'); // replace all 'x' to 'y'

    if (IsFileCached(filePath))
    {
        for (Asset* cached_file : cachedFiles)
        {
            if (cached_file->path == filePath)
            {
                return static_cast<T*>(cached_file);
            }
        }
    }
    cout << filesystem::path(path).string() << " is not cached." << endl;
    return nullptr;
}

template <typename Importer>
Asset* AssetProcessor::ImportAsset(string path)
{
    std::replace(path.begin(), path.end(), '/', '\\');
    Importer* importer = new Importer;
    importer->importPath = path;
    importer->SetupProcessors();
    return importer->ImportAsset();
}

struct AssetImporter
{
    string importPath;

    virtual ~AssetImporter() = default;

private:

    friend AssetProcessor;
    friend class AssetPostprocessor;

    virtual Asset* ImportAsset()
    {
        loadedAsset = new Asset;
        return loadedAsset;
    }

	void SetupProcessors();

    Asset* loadedAsset;
};

struct ShaderImporter : AssetImporter
{
    bool IsGeometryShader=false;
    bool LogShader=true;

    ShaderAsset* ImportAsset() override;
};

struct PrefabImporter : AssetImporter
{
    Prefab* ImportAsset() override;
};

struct ModelImporter : AssetImporter
{
    float GlobalScale=1;

    ModelAsset* ImportAsset() override;
};

struct AssetPostprocessor
{
    AssetImporter* importer;
    string assetPath;

    virtual void OnPostprocessPrefab(Prefab* prefab) {}
    virtual void OnPreprocessPrefab() {}

    virtual void OnPostprocessModel(ModelAsset* asset) {}
    virtual void OnPreprocessModel()
    {
        ModelImporter* model_importer = static_cast<ModelImporter*>(importer);

        model_importer->GlobalScale = 1;
    }


    static vector<AssetPostprocessor*> processors;

private:

};

#endif