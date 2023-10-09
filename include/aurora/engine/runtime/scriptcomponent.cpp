#include "scriptcomponent.hpp"

#include "monort.hpp"

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

    // Fields

    MonoRuntime::MonoScriptInstance* instance = MonoRuntime::GetEntityScriptInstance(entity->id);

    if (instance)
    {
        const auto& fields = instance->GetClass()->GetFields();

        for (const auto& [name, field] : fields)
        {
	        if (field.type == MonoRuntime::ScriptFieldType::Float)
	        {
                float data = instance->GetFieldValue<float>(name);
                ImGui::DragFloat(name.c_str(), &data, 0.01f);
                instance->SetFieldValue<float>(name, data);
	        }
        }
    }

    if (!scriptExists)
        ImGui::PopStyleColor();



}

void ScriptComponent::Init()
{
    name = "Aurora.TestComponent";
}