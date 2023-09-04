#include "imgui_ext.hpp"

bool ImGui::FileDialog(string* path, string formats, string& filePathName, string& filePath)
{
    ImGui::InputText("Path", path);

    ImGui::SameLine();

    if (ImGui::Button("..."))
    {
        ImGuiFileDialog::Instance()->OpenDialog("engine_cmp_dlg", "Open a File", formats.c_str(), ".");
    }


    bool ret = false;
    // display
    if (ImGuiFileDialog::Instance()->Display("engine_cmp_dlg"))
    {

        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

            ret = true;

            // action

        }
        // close

        ImGuiFileDialog::Instance()->Close();
    }

    return ret;
}
