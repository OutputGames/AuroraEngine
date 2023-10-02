// Your First C++ Program

#include "aurora/aurora.hpp"

int main() {

    InitEngine();

    

    while (!RenderMgr::CheckCloseWindow()) {
        if (Project::ProjectLoaded()) {
            //Scene::GetScene()->physics_factory->Update();
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
