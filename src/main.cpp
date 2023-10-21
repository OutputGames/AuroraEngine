// Your First C++ Program

#include "aurora/aurora.hpp"
#include "engine/assets/processor.hpp"
#include "utils/timer.hpp"

int main() {

    InitEngine();

    

    //Project::Create("C:/Users/chris/Downloads/testproj");
        
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
    }

    RenderMgr::DestroyGraphicsDevice();
    

    return 0;
}
