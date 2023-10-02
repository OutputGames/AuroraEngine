#include "monort.hpp"

#include "engine/project.hpp"
#include "utils/filesystem.hpp"

MonoRuntime::runtime_variables* MonoRuntime::m_data;

CLASS_DEFINITION(Component, ScriptComponent)

void ScriptComponent::EngineRender()
{

    bool scriptExists = MonoRuntime::ClassExists(name);

    const auto& componentClasses = MonoRuntime::GetEntityClasses();

    if (!scriptExists)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.3f, 1.0f));

    if (ImGui::InputText("Class", &name))
    {

    }

    if (!scriptExists)
        ImGui::PopStyleColor();



}

void ScriptComponent::Init()
{
    name = "Aurora.MeshRenderer";
}

void MonoRuntime::MonoAssemblyAurora::InvokeStaticMethod(string descStr)
{

    if (this == nullptr)
        return;

    //Build a method description object
    MonoMethodDesc* TypeMethodDesc;
    char* TypeMethodDescStr = (char*)descStr.c_str();
    TypeMethodDesc = mono_method_desc_new(TypeMethodDescStr, NULL);
    if (!TypeMethodDesc)
    {
        Logger::Log("FAILED TO CREATE METHOD DESCRIPTION", Logger::LOG_ERROR, "MONORUNTIME");
        return;
    }

    //Search the method in the image

    MonoImage* image = mono_assembly_get_image(assembly);

    MonoMethod* method;
    method = mono_method_desc_search_in_image(TypeMethodDesc, image);
    if (!method)
    {
        Logger::Log("FAILED TO FIND DESCRIPTION IN IMAGE", Logger::LOG_ERROR, "MONORUNTIME");
        return;
    }

    //run the method
    mono_runtime_invoke(method, nullptr, nullptr, nullptr);
}

void MonoRuntime::MonoAssemblyAurora::Reload()
{

    int dataSize;

    char* data = Filesystem::ReadFileBytes(path, dataSize);

    MonoImage* image;
    MonoImageOpenStatus status;
    image = mono_image_open_from_data_full(data, dataSize, 1, &status, 0);
    if (!image)
    {
        Logger::Log("FAILED TO GET IMAGE " + path, Logger::LOG_ERROR, "MONORUNTIME");
        return;
    }
}

void MonoRuntime::MonoAssemblyAurora::PrintTypes()
{
    m_data->componentClasses.clear();

    MonoImage* image = mono_assembly_get_image(assembly);
    const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
    int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

    for (int32_t i = 0; i < numTypes; i++)
    {
        uint32_t cols[MONO_TYPEDEF_SIZE];
        mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

        const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
        const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

        string fullName;
        if (strlen(nameSpace) != 0)
            fullName = string(nameSpace) + "." + string(name);
        else
            fullName = name;

        MonoClass* mono_class = mono_class_from_name(image, nameSpace, name);
        MonoClass* comp_class = mono_class_from_name(image, "Aurora", "Entity");

        bool isComponent = mono_class_is_subclass_of(mono_class, comp_class, false);

        if (isComponent && fullName != "Aurora.Entity")
        {
            MonoRuntime::m_data->componentClasses[fullName] = new MonoScript(nameSpace, name, this);
        }

        printf("%s.%s\n", nameSpace, name);
    }
}

void MonoRuntime::MonoAssemblyAurora::AddInternalCall(string method, const void* func)
{
    mono_add_internal_call(method.c_str(), func);
}

MonoRuntime::MonoScript::MonoScript(string classNamespace, string name, MonoAssemblyAurora* assembly)
{
    MonoImage* image = mono_assembly_get_image(assembly->assembly);
    MonoClass* klass = mono_class_from_name(image, classNamespace.c_str(), name.c_str());

    if (klass == nullptr)
    {
        Logger::Log("FAILED TO GET CLASS  " + classNamespace + ":" + name, Logger::LOG_ERROR, "MONORUNTIME");
        // Log error here
        return;
    }

    monoClass = klass;
    cName = name;
    cNamespace = classNamespace;

}

MonoObject* MonoRuntime::MonoScript::Instantiate()
{
    // Allocate an instance of our class
    MonoObject* classInstance = mono_object_new(m_data->appDomain, monoClass);

    if (classInstance == nullptr)
    {
        Logger::Log("FAILED TO CREATE INSTANCE ", Logger::LOG_ERROR, "MONORUNTIME");
        return nullptr;
        // Log error here and abort
    }

    // Call the parameterless (default) constructor
    mono_runtime_object_init(classInstance);

    return classInstance;
}

MonoMethod* MonoRuntime::MonoScript::GetMethod(string name, int parameterCount)
{
    // Get a reference to the method in the class
    MonoMethod* method = mono_class_get_method_from_name(monoClass, name.c_str(), parameterCount);

    if (method == nullptr)
    {
        // No method called "PrintFloatVar" with 0 parameters in the class, log error or something

        Logger::Log("NO METHOD CALLED '" + name + "' WITH " + std::to_string(parameterCount) + " PARAMETERS IN CLASS", Logger::LOG_ERROR, "MONORUNTIME");
        return nullptr;
    }

    return method;
}

MonoObject* MonoRuntime::MonoScript::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
{
    // Call the C# method on the objectInstance instance, and get any potential exceptions
    MonoObject* exception = nullptr;
    MonoObject* obj = mono_runtime_invoke(method, instance, params, &exception);

    if (exception)
    {
        mono_print_unhandled_exception(exception);

        Logger::Log("EXCEPTION:  "  , Logger::LOG_ERROR, "MONORUNTIME");
        return nullptr;
    }

    return obj;
}

MonoRuntime::MonoScriptInstance::MonoScriptInstance(MonoScript* script, Entity* entity)
{
    m_script = script;

    m_instance = script->Instantiate();
    m_initMethod = script->GetMethod("OnInit", 0);
    m_updateMethod = script->GetMethod("OnUpdate", 1);

    if (mono_class_is_subclass_of(script->monoClass, m_data->entityClass->monoClass, false))
    {
        m_constructor = m_data->entityClass->GetMethod(".ctor", 1);
    } else
    {
        m_constructor = script->GetMethod(".ctor", 1);
    }

    {
        void* payload = &entity->id;
        script->InvokeMethod(m_instance, m_constructor, &payload);
    }
}

void MonoRuntime::MonoScriptInstance::InvokeOnCreate()
{
    m_script->InvokeMethod(m_instance, m_initMethod, nullptr);
}

void MonoRuntime::MonoScriptInstance::InvokeOnUpdate(float dt)
{

    void* payload = &dt;

    m_script->InvokeMethod(m_instance, m_updateMethod, &payload);
}

void MonoRuntime::Initialize()
{
    //#pragma region Init mono runtime
    mono_set_dirs(MONO_HOME "/lib",
        MONO_HOME "/etc");

    m_data = new runtime_variables;

    //Init a domain
    m_data->rootDomain = mono_jit_init("AuroraJITRuntime");
    if (!m_data->rootDomain)
    {
        Logger::Log("FAILED TO CREATE JIT DOMAIN", Logger::LOG_ERROR, "MONORUNTIME");
        return;
    }

    m_data->appDomain = mono_domain_create_appdomain("AuroraScriptRuntime", nullptr);
    mono_domain_set(m_data->appDomain, true);

    //CompileProject("C:/Users/chris/Downloads/AuroraEngine/scriptcore/generated/Aurora-ScriptCore.csproj", "");

    m_data->coreAssembly = OpenAssembly("C:/Users/chris/Downloads/AuroraEngine/scriptcore/bin/Debug/Aurora-ScriptCore.dll");

    InternalCalls::InitFunctions();

    m_data->entityClass = new MonoScript("Aurora", "Entity", m_data->coreAssembly);
}

MonoRuntime::MonoAssemblyAurora* MonoRuntime::OpenAssembly(string path)
{

    if (filesystem::exists(path)) {

        int fileSize = 0;
        char* fileData = Filesystem::ReadFileBytes(path, fileSize);

        // NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

        if (status != MONO_IMAGE_OK)
        {
            const char* errorMessage = mono_image_strerror(status);
            // Log some error message using the errorMessage data
            return nullptr;
        }

        std::filesystem::path pdbPath = path;
        pdbPath.replace_extension(".pdb");

        if (std::filesystem::exists(pdbPath))
        {

            int fileSize = 0;
            char* pdbFileData = Filesystem::ReadFileBytes(pdbPath.string(), fileSize);
            mono_byte* pdb = (mono_byte*)pdbFileData;
            mono_debug_open_image_from_memory(image, pdb, fileSize);
        }

        MonoAssembly* assembly = mono_assembly_load_from_full(image, path.c_str(), &status, 0);
        mono_image_close(image);

        // Don't forget to free the file data
        delete[] fileData;

        MonoAssemblyAurora* a_assembly = new MonoAssemblyAurora;

        a_assembly->assembly = assembly;
        a_assembly->path = path;

        a_assembly->PrintTypes();

        return a_assembly;
    } else
    {
        return  nullptr;
    }
}

void MonoRuntime::CompileScript(string path)
{
    filesystem::create_directory("generated/");

    filesystem::path fpath(path);

    if (!filesystem::exists(path))
        return;

    std::remove(("generated/" + fpath.stem().string() + ".dll").c_str());

    std::string command = "\"" MONO_HOME "/bin/mcs\" " + fpath.relative_path().string() + " -target:library -out:generated/"+fpath.stem().string() + ".dll -v";

    //Compile the script
    system(command.c_str());
}

void MonoRuntime::OnRuntimeStart(Scene* scene)
{
    m_data->entityInstances.clear();

    for (Entity* entity : scene->entity_mgr->entities)
    {
        OnCreateEntity(entity);
    }
}

void MonoRuntime::OnRuntimeUnload()
{

}

void MonoRuntime::CompileProject(string path, string outPath)
{

}

bool MonoRuntime::ClassExists(string className)
{
    return m_data->componentClasses.find(className) != m_data->componentClasses.end();
}

void MonoRuntime::OnCreateComponent(Entity* entity)
{
    const auto& sc = entity->GetComponent<ScriptComponent>();

    if (MonoRuntime::ClassExists(sc->name))
    {
        MonoScriptInstance* instance = new MonoScriptInstance(m_data->componentClasses[sc->name], entity);
        m_data->componentInstances[entity->id] = instance;

        MonoClass* i_class = mono_object_get_class(instance->m_instance);


        MonoProperty* nameProperty = mono_class_get_property_from_name(i_class, "Name");
        MonoString* mname = MonoRuntime::UTF8ToMonoString(entity->name);
        mono_property_set_value(nameProperty, instance->m_instance, (void**)&mname, nullptr);

        instance->InvokeOnCreate();
    }
}

void MonoRuntime::OnUpdateComponent(Entity* entity, float dt)
{
    uint32_t id = entity->id;
    assert(m_data->componentInstances.find(id) != m_data->componentInstances.end());

    MonoScriptInstance* instance = m_data->componentInstances[id];

    instance->InvokeOnUpdate(dt);

}

void MonoRuntime::OnCreateEntity(Entity* entity)
{
    MonoScriptInstance* instance = new MonoScriptInstance(m_data->entityClass, entity);
    m_data->entityInstances[entity->id] = instance;

    MonoClass* i_class = mono_object_get_class(instance->m_instance);

    MonoClassField* idField = mono_class_get_field_from_name(i_class, "Id");
    MonoProperty* nameProperty = mono_class_get_property_from_name(i_class, "Name");
    mono_field_set_value(instance->m_instance, idField, &entity->id);

    MonoString* mname = MonoRuntime::UTF8ToMonoString(entity->name);
    mono_property_set_value(nameProperty, instance->m_instance, (void**)&mname, nullptr);

    instance->InvokeOnCreate();
}

void MonoRuntime::OnUpdateEntity(Entity* entity, float dt)
{
    uint32_t id = entity->id;
    assert(m_data->entityInstances.find(id) != m_data->entityInstances.end());

    MonoScriptInstance* instance = m_data->entityInstances[id];

    instance->InvokeOnUpdate(dt);
}

bool MonoRuntime::CheckMonoError(MonoError& error)
{
	bool hasError = !mono_error_ok(&error);
	if (hasError)
	{
		unsigned short errorCode = mono_error_get_error_code(&error);
		const char* errorMessage = mono_error_get_message(&error);
		printf("Mono Error!\n");
		printf("\tError Code: %hu\n", errorCode);
		printf("\tError Message: %s\n", errorMessage);
		mono_error_cleanup(&error);
	}
	return hasError;
}

unordered_map<string, MonoRuntime::MonoScript*> MonoRuntime::GetEntityClasses()
{
    return m_data->componentClasses;
}

std::string MonoRuntime::MonoStringToUTF8(MonoString* monoString)
{
	if (monoString == nullptr || mono_string_length(monoString) == 0)
		return "";

	MonoError error;
	char* utf8 = mono_string_to_utf8_checked(monoString, &error);
	if (CheckMonoError(error))
		return "";
	std::string result(utf8);
	mono_free(utf8);
	return result;
}

MonoString* MonoRuntime::UTF8ToMonoString(string str)
{
    return mono_string_new(m_data->appDomain, str.c_str());
}

MonoClass* InternalCalls::Entity_CreateEntity(MonoString* name)
{
    MonoRuntime::MonoScript* script = new MonoRuntime::MonoScript("Aurora", "Entity", MonoRuntime::m_data->coreAssembly);

    MonoObject* klass = script->Instantiate();
    MonoClass* i_class = mono_object_get_class(klass);

    if (Scene::GetScene())
    {
        Entity* entity = Scene::GetScene()->entity_mgr->CreateEntity(MonoRuntime::MonoStringToUTF8(name));
        MonoClassField* idField = mono_class_get_field_from_name(i_class, "Id");
        MonoProperty* nameProperty = mono_class_get_property_from_name(i_class, "Name");
        mono_field_set_value(klass, idField, &entity->id);

        MonoString* mname = MonoRuntime::UTF8ToMonoString(entity->name);
        mono_property_set_value(nameProperty, klass, (void**)&mname, nullptr);

        //script->InvokeMethod(klass, script->GetMethod("Init", 0));

        
    }

    return i_class;
}

MonoClass* InternalCalls::Entity_GetEntity(int id)
{

    MonoRuntime::MonoScriptInstance* instance = MonoRuntime::m_data->entityInstances[(uint32_t)id];

    MonoClass* i_class = mono_object_get_class(instance->m_instance);

    return i_class;
}

void InternalCalls::Transform_GetPosition(int id, glm::vec3* outTranslation)
{
    Entity* entity = Scene::GetScene()->entity_mgr->entities[id];

    *outTranslation = entity->transform->position;
}

void InternalCalls::Transform_SetPosition(int id, glm::vec3* outTranslation)
{
    Entity* entity = Scene::GetScene()->entity_mgr->entities[id];

    entity->transform->position = *outTranslation;
}

void InternalCalls::InitFunctions()
{
	MonoRuntime::m_data->coreAssembly->AddInternalCall("Aurora.InternalCalls::CreateEntity", &Entity_CreateEntity);
    MonoRuntime::m_data->coreAssembly->AddInternalCall("Aurora.InternalCalls::GetEntity", &Entity_GetEntity);
    MonoRuntime::m_data->coreAssembly->AddInternalCall("Aurora.InternalCalls::TransformGetPosition", &Transform_GetPosition);
    MonoRuntime::m_data->coreAssembly->AddInternalCall("Aurora.InternalCalls::TransformSetPosition", &Transform_SetPosition);

}