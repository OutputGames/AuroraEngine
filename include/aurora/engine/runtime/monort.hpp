
#include "engine/componentregistry.hpp"
#if !defined(MONORUNTIME)

#include "utils/utils.hpp"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include "mono/metadata/object.h"
#include "mono/metadata/mono-debug.h"
#include "mono/metadata/threads.h"
#include "mono/metadata/reflection.h"
#include "engine/entity.hpp"

#define MONORUNTIME



struct MonoRuntime
{

    struct MonoAssemblyAurora
    {
        MonoAssembly* assembly;
        string path;
        void InvokeStaticMethod(string descStr);
        void Reload();
        void PrintTypes();
        void AddInternalCall(string method, const void* func);
    };

    struct MonoScript
    {
    public:
        MonoScript() = default;
        MonoScript(string classNamespace, string name, MonoAssemblyAurora* assembly);

        MonoObject* Instantiate();
        MonoMethod* GetMethod(string name, int parameterCount);
        MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);
        string cNamespace, cName;

        MonoClass* monoClass = nullptr;
    };

    struct MonoScriptInstance
    {
    public:
        MonoScriptInstance(MonoScript* script, Entity* entity);

        void InvokeOnCreate();
        void InvokeOnUpdate(float dt);

        MonoObject* m_instance = nullptr;
    private:
        MonoScript* m_script;

        MonoMethod* m_initMethod = nullptr;
        MonoMethod* m_updateMethod = nullptr;
        MonoMethod* m_constructor = nullptr;
    };

    static void Initialize();
    static MonoAssemblyAurora* OpenAssembly(string path);
    static void CompileScript(string path);
    static void OnRuntimeStart(Scene* scene);
    static void OnRuntimeUnload();
    static void CompileProject(string path, string outPath);
    static bool ClassExists(string className);


    static void OnCreateComponent(Entity* entity);
    static void OnUpdateComponent(Entity* entity, float dt);
    static void OnCreateEntity(Entity* entity);
    static void OnUpdateEntity(Entity* entity, float dt);

    struct runtime_variables
    {

		unordered_map<string, MonoScript*> componentClasses;
		unordered_map<uint32_t, MonoScriptInstance*> componentInstances;
		unordered_map<uint32_t, MonoScriptInstance*> entityInstances;
        unordered_map < MonoType*, function<bool(Entity*)>> hasComponentFuncs;
        MonoScript* entityClass;

		MonoDomain* rootDomain;

		MonoDomain* appDomain;

		MonoAssemblyAurora* coreAssembly;
    };

    static std::string MonoStringToUTF8(MonoString* monoString);
    static MonoString* UTF8ToMonoString(string str);
    static  MonoImage* GetCoreAssemblyImage();

    static runtime_variables* m_data;

    static bool CheckMonoError(MonoError& error);

    static unordered_map<string, MonoScript*> GetEntityClasses();
};

struct InternalCalls
{
    static MonoClass* Entity_CreateEntity(MonoString* name);
    static MonoClass* Entity_GetEntity(int id);
    static void Transform_GetPosition(int id, glm::vec3* outTranslation);
    static void Transform_SetPosition(int id, glm::vec3* outTranslation);
    static void Entity_HasComponent(int id, MonoReflectionType* type);

    static void Material_GetTextures(int id, vector<Texture>* textures);
    static void Material_SetUniforms(int id, map<MonoString*, MonoObject*>* uniforms);
    static void Material_GetUniforms(int id,MonoObject* dict);

    static void InitFunctions();
    template <typename... C>
    static void RegisterComponent();

    template <typename... C>
    static void RegisterComponent(ComponentRegistry::ComponentRegister<C...>);
    static void InitComponents();
};

#endif // MONORUNTIME
