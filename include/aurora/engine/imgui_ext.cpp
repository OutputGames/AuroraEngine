#include "imgui_ext.hpp"

bool ImGui::FileDialog()
{
	
	if (ImGui::Button("..."))
	{
		
	}
	ImGui::SameLine();
	ImGui::Text("C:/filepath");
	return true;
}