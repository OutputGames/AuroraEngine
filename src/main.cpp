// Your First C++ Program

#include "aurora/aurora.hpp"

int main() {

    InitEngine();

    

    while (!RenderMgr::CheckCloseWindow()) {
        if (Project::ProjectLoaded()) {
            //Scene::GetScene()->physics_factory->Update();
            Scene::GetScene()->entity_mgr->Update();
        }
        RenderMgr::UpdateGraphicsDevice();
    }

    RenderMgr::DestroyGraphicsDevice();
    

    return 0;
}
