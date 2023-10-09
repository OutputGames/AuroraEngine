#if !defined(SCRIPT_CMP)
#define SCRIPT_CMP
#include "engine/entity.hpp"

class ScriptComponent : public Component
{
	CLASS_DECLARATION(ScriptComponent)

public:
	ScriptComponent(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	ScriptComponent() = default;
    void EngineRender() override;
    void Init() override;

    string name;


};

#endif // SCRIPT_CMP
