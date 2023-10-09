#include "egs.hpp"

#include "entity.hpp"

void EditorGameSystem::LoadSaveState(SaveState *state)
{
    Scene::LoadScene(state->data, false);
}

SaveState *EditorGameSystem::SaveCurrentState()
{
    SaveState* ss = new SaveState;

    ss->data = Scene::GetScene()->ToString();

    return ss;
}
