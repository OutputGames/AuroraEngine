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

    if (!scriptExists)
        ImGui::PopStyleColor();



}

void ScriptComponent::Init()
{
    name = "Aurora.TestComponent";
}