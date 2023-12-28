// Your First C++ Program

#include "aurora/aurora.hpp"
#include "engine/assets/processor.hpp"
#include "ui/imgui-layer.hpp"
#include "utils/timer.hpp"

int main() {

    InitEngine();

    ImguiLayer* layer = new ImguiLayer;

    layer->InitLayer();

    InitScriptEngine();

    Project::Create("C:/Users/chris/Downloads/testproj/");
        
    while (!RenderMgr::CheckCloseWindow()) {
        if (Project::ProjectLoaded()) {
            if (Scene::GetScene()->runtimePlaying) {
                Scene::GetScene()->OnRuntimeUpdate();
            } else
            {
                Scene::GetScene()->OnUpdate();
            }
        }
        RenderMgr::UpdateGraphicsDevice();
        layer->UpdateLayer();
        RenderMgr::SwapBuffers();
    }

    RenderMgr::DestroyGraphicsDevice();
    

    return 0;
}
