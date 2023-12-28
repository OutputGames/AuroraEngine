#include "rendering/render.hpp"

#include "engine/componentregistry.hpp"
#include "engine/entity.hpp"
#include "engine/log.hpp"
#include "engine/project.hpp"
#include "graphics/billboard.hpp"
#include "graphics/lighting.hpp"
#include "rendering/renderer.hpp"
#include "utils/filesystem.hpp"

AURORA_API void InitEngine();
AURORA_API void InitScriptEngine();

template<typename T>

AURORA_API T* AttachComponent();