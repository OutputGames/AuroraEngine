// Your First C++ Program

#include "aurora/aurora.hpp"
#include "engine/assets/processor.hpp"

int main() {

    InitEngine();

    

    while (!RenderMgr::CheckCloseWindow()) {
        if (Project::ProjectLoaded()) {
            Project::GetProject()->processor->CheckForEdits();
            if (Scene::GetScene()->runtimePlaying) {
                Scene::GetScene()->OnRuntimeUpdate();
            } else
            {
                Scene::GetScene()->OnUpdate();
            }
        }
        RenderMgr::UpdateGraphicsDevice();
    }

    RenderMgr::DestroyGraphicsDevice();
    

    return 0;
}
