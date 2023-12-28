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


    enum AssetType
    {
        FileAsset = 0,
        TextAsset,
        ScriptAsset,
        ModelAsset,
        ShaderAsset,
        TextureAsset,
        PrefabAsset,
        AnimationAsset,
        MaterialAsset,
    };

private:
    friend class AssetProcessor;

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

struct MaterialAsset : Asset
{
    shared_ptr<Material> GetMaterial()
    {
        return make_shared<Material>(*this->material);
    }

    Material* material;
};

struct AssetProcessor
{
	void SetupImporters();
    void Init(string path);
    AURORA_API void CheckForEdits(bool IsThreaded);

    template <typename T>
     AURORA_API T* GetAsset(string path);

    vector<Asset*> GetAssetsOfType(Asset::AssetType type);

    AURORA_API static void CreateDefaultAsset(string path, Asset::AssetType type);

private:

    AURORA_API bool IsFileCached(string path);
    unordered_map<string,Asset*> cachedFiles;
    string project_path;
    unordered_map<Asset::AssetType, function<Asset* (AssetProcessor*, string)>> assetImportFuncs;

    template <typename Importer>
    Asset* ImportAsset(string path);

    void ImportNewAsset(string path);
    void ReloadAsset(string path);
};

template <typename T>
T* AssetProcessor::GetAsset(string path)
{
    string filePath = filesystem::path(path).string();
	std::replace(filePath.begin(), filePath.end(), '/', '\\'); // replace all 'x' to 'y'

    if (IsFileCached(filePath))
    {
        return static_cast<T*>(cachedFiles[filePath]);
    }
    cout << filesystem::path(path).string() << " is not cached." << endl;
    return nullptr;
}

template <typename Importer>
Asset* AssetProcessor::ImportAsset(string path)
{
    std::replace(path.begin(), path.end(), '/', '\\');
    Importer* importer = new Importer;
    Asset::AssetType type = Asset::GetTypeFromExtension(path);
    importer->importPath = path;
    importer->SetupProcessors();
    importer->Preprocess(type);
    Asset* a = importer->ImportAsset();
    importer->Postprocess(type,a);
    return a;
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

    void Preprocess(Asset::AssetType type);
    void Postprocess(Asset::AssetType type, Asset* asset);

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
    Shader* DefaultShader=new Shader("Assets/Editor/shaders/0/");

    ModelAsset* ImportAsset() override;
};

struct ScriptImporter : AssetImporter
{

    ScriptAsset* ImportAsset() override;
};

struct AnimationImporter : AssetImporter
{
    AnimationAsset* ImportAsset() override;
};

struct MaterialImporter : AssetImporter
{
    MaterialAsset* ImportAsset() override;
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

        model_importer->GlobalScale = 10;
    }

    virtual void OnPostprocessMaterial(MaterialAsset* asset) {}
    virtual void OnPreprocessMaterial() {}

    static vector<AssetPostprocessor*> processors;

private:

};

#endif