
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
#include "mono/metadata/attrdefs.h"
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

    enum class ScriptFieldType
    {
        None = 0,
        Float, Vector2, Vector3, Vector4,
        Int, Bool, UInt, Double, Short, Byte
    };

    struct ScriptField
    {
        ScriptFieldType type;
        string name;
        MonoClassField* class_field;
    };

    struct MonoScript
    {
    public:
        MonoScript() = default;
        MonoScript(string classNamespace, string name, MonoAssemblyAurora* assembly);

        MonoObject* Instantiate();
        MonoMethod* GetMethod(string name, int parameterCount);
        MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

        const map<string, ScriptField>& GetFields() const { return fields; };

        string cNamespace, cName;

        MonoClass* monoClass = nullptr;

        map<string, ScriptField> fields;
    };

    struct MonoScriptInstance
    {
    public:
        MonoScriptInstance(MonoScript* script, Entity* entity);

        void InvokeOnCreate();
        void InvokeOnUpdate(float dt);

        MonoScript* GetClass();

        template<typename T>
        T GetFieldValue(const string name)
        {
            bool success = GetFieldValueInternal(name, fieldValueBuffer);

            if (!success)
                return T();

            return *(T*)fieldValueBuffer;
        }

        template<typename T>
        void SetFieldValue(const string name, const T value)
        {
            SetFieldValueInternal(name, &value);
        }

        MonoObject* m_instance = nullptr;
    private:
        MonoScript* m_script;

        bool GetFieldValueInternal(const string name, void* buffer);
        void SetFieldValueInternal(const string name, const void* value);

        MonoMethod* m_initMethod = nullptr;
        MonoMethod* m_updateMethod = nullptr;
        MonoMethod* m_constructor = nullptr;

        inline static char fieldValueBuffer[8];
    };

    AURORA_API static void Initialize();
    static MonoAssemblyAurora* OpenAssembly(string path);
    static void CompileScript(string path);
    static void OnRuntimeStart(Scene* scene);
    static void OnRuntimeUnload();
    static void CompileProject(string path, string outPath);
    static bool ClassExists(string className);
    static MonoScriptInstance* GetEntityScriptInstance(uint32_t entityId);

    static void InitProject();

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

        unordered_map<string, ScriptFieldType> scriptFieldTypeMap = {
        	{"System.Single", ScriptFieldType::Float},
        	{"System.Double", ScriptFieldType::Double}
        };
    };

    static ScriptFieldType MonoTypeToScriptFieldType(MonoType* type);
    static string FieldTypeToString(ScriptFieldType type);

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
    static MonoString* Material_GetValue(int id, MonoString* type, MonoString* name);
    static void Material_SetValue(int id, MonoString* type, MonoString* name, MonoString* data);
    static float Time_GetTime();
    static float Time_GetSineTime();

    static void InitFunctions();
    template <typename... C>
    static void RegisterComponent();

    template <typename... C>
    static void RegisterComponent(ComponentRegistry::ComponentRegister<C...>);
    static void InitComponents();
};

#endif // MONORUNTIME
