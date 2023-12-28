#include "imgui-layer.hpp"

#include "core/core-engine.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "rendering/camera.hpp"
#include "rendering/render.hpp"

#include "graphics/animation.hpp"
#include "engine/entity.hpp"
#include "engine/log.hpp"
#include "engine/project.hpp"
#include "engine/assets/processor.hpp"
#include "graphics/lighting.hpp"
#include "graphics/model.hpp"
#include "utils/filesystem.hpp"
#include "utils/input.hpp"
#include "engine/componentregistry.hpp"
#include "engine/imgui_ext.hpp"

using namespace Engine;

void AlignForWidth(float width, float alignment = 0.5f)
{
    ImGuiStyle& style = ImGui::GetStyle();
    float avail = ImGui::GetContentRegionAvail().x;
    float off = (avail - width) * alignment;
    if (off > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
}

struct ImVec3 { float x, y, z; ImVec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) { x = _x; y = _y; z = _z; } };

ImVec4 convertto(ImVec3 v)
{
    return { v.x, v.y, v.z, 1.00f };
}

void imgui_easy_theming(ImVec3 color_for_text, ImVec3 color_for_head, ImVec3 color_for_area, ImVec3 color_for_body, ImVec3 color_for_pops)
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Text] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.58f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.95f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.58f);
    style.Colors[ImGuiCol_Border] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.00f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.75f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.47f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.21f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.80f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.50f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.50f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.86f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.76f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.86f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_Tab] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.43f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(color_for_pops.x, color_for_pops.y, color_for_pops.z, 0.92f);
}


void ColorEditVec3(ImVec3& c, string label)
{
    float co[3] = { c.x, c.y,c.z };

    ImGui::ColorEdit3(label.c_str(), co);

    c = { co[0], co[1], co[2] };
}

ImVec3 GetVec3(ImVec4 v)
{
    return { v.x, v.y, v.z };
}

void ImguiLayer::InitLayer()
{


    if (!imgui_setup) {


        ImGui::SetCurrentContext(RenderMgr::GetIMGUIContext());

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.

        imgui_setup = true;
    }

    GLFWwindow* window = GetWindow();
    const float radius = 25.0F;
    float camX = sin(45) * radius;
    float camZ = cos(45) * radius;
    glm::vec3 camPos = glm::vec3(camX, 25 / 2, camZ);



    editorCamera = EditorCamera::GetEditorCam();

    editorCamera->position = camPos;

    imguiTextEditor = new TextEditor;

    currentGizmoOperation = ImGuizmo::UNIVERSAL;
    currentGizmoMode = ImGuizmo::WORLD;
}



void ImguiLayer::UpdateLayer()
{


    editorCamera->useDirection = true;

    editorCamera->Update();

    NewFrame();
    Dockspace();
    MainMenuBar();

    if (Project::ProjectLoaded())
    {
        SceneView();
        GameView();
        Inspector();
        Explorer();
        Log();
        DebugMenu();
        Styler();
        AssetBrowser();
    }

    FileDialogues();

    EndFrame();

}

void ImguiLayer::DestroyLayer()
{

}

void ImguiLayer::NewFrame()
{

    Scene* currentScene = Scene::GetScene();

    ImGui::AutoNewFrame();

    if (Engine::InputMgr::IsKeyPressed(ImGuiKey_Delete))
    {
        if (selectedEntity) {
            auto const& entity = currentScene->entity_mgr->entities[selectedEntityId];
            entity->Delete();
            selectedEntity = false;
        }
        else if (isFileSelected)
        {
            filesystem::remove(selectedPath);
            isFileSelected = false;
            Project::GetProject()->processor->CheckForEdits(false);
        }
    }

    if (Engine::InputMgr::IsKeyPressed((ImGuiKey_Escape)))
    {
        if (selectedEntity) {
            selectedEntity = false;
        }
        else if (isFileSelected)
        {
            isFileSelected = false;
        }
    }

    ImGuizmo::BeginFrame();
}

void assetCreate(string path, bool i, Asset::AssetType type, ImguiLayer* layer)
{
    filesystem::path p(path);

    if (i)
    {
        filesystem::create_directory(p);
    }
    else
    {
        AssetProcessor::CreateDefaultAsset(path, type);
    }

    layer->isFileSelected = true;
    layer->selectedPath = p;


    Project::GetProject()->processor->CheckForEdits(false);
}

void assetCreateMenu(ImguiLayer* layer)
{
    if (layer->assetDir != "") {
        if (ImGui::Selectable("Folder")) { assetCreate(layer->assetDir.string() + "/New Folder", true, Asset::FileAsset, layer); }

        ImGui::Separator();

        if (ImGui::Selectable("C# Script")) { assetCreate(layer->assetDir.string() + "/NewScript.cs", false, Asset::ScriptAsset, layer); }

        if (ImGui::BeginMenu("Shader")) {
            if (ImGui::Selectable("Standard Shader")) { assetCreate(layer->assetDir.string() + "/NewShader.shader", false, Asset::ShaderAsset, layer); }

            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::Selectable("Scene")) { assetCreate(layer->assetDir.string() + "/NewScene.auscene", false, Asset::FileAsset, layer); }
        if (ImGui::Selectable("Prefab")) { assetCreate(layer->assetDir.string() + "/NewPrefab.prefab", false, Asset::PrefabAsset, layer); }

        ImGui::Separator();

        if (ImGui::Selectable("Material")) { assetCreate(layer->assetDir.string() + "/NewMaterial.mat", false, Asset::MaterialAsset, layer); }

    }
}

void ImguiLayer::Dockspace()
{
    bool p_open = true;
    static bool opt_fullscreen = true; // Is the Dockspace full-screen?
    static bool opt_padding = false; // Is there padding (a blank space) between the window edge and the Dockspace?
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None; // Config flags for the Dockspace

    // In this example, we're embedding the Dockspace into an invisible parent window to make it more configurable.
    // We set ImGuiWindowFlags_NoDocking to make sure the parent isn't dockable into because this is handled by the Dockspace.
    //
    // ImGuiWindowFlags_MenuBar is to show a menu bar with config options. This isn't necessary to the functionality of a
    // Dockspace, but it is here to provide a way to change the configuration flags interactively.
    // You can remove the MenuBar flag if you don't want it in your app, but also remember to remove the code which actually
    // renders the menu bar, found at the end of this function.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    // Is the example in Fullscreen mode?
    if (opt_fullscreen)
    {
        // If so, get the main viewport:
        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        // Set the parent window's position, size, and viewport to match that of the main viewport. This is so the parent window
        // completely covers the main viewport, giving it a "full-screen" feel.
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        // Set the parent window's styles to match that of the main viewport:
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // No corner rounding on the window
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f); // No border around the window

        // Manipulate the window flags to make it inaccessible to the user (no titlebar, resize/move, or navigation)
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        // The example is not in Fullscreen mode (the parent window can be dragged around and resized), disable the
        // ImGuiDockNodeFlags_PassthruCentralNode flag.
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so the parent window should not have its own background:
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // If the padding option is disabled, set the parent window's padding size to 0 to effectively hide said padding.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::Begin("DockSpace", &p_open, window_flags);

    // Remove the padding configuration - we pushed it, now we pop it:
    if (!opt_padding)
        ImGui::PopStyleVar();

    // Pop the two style rules set in Fullscreen mode - the corner rounding and the border size.
    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Check if Docking is enabled:
    ImGuiIO& io = ImGui::GetIO();

    // If it is, draw the Dockspace with the DockSpace() function.
    // The GetID() function is to give a unique identifier to the Dockspace - here, it's "MyDockSpace".
    ImGuiID dockspace_id = ImGui::GetID("EngineDS");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
}

void ImguiLayer::MainMenuBar()
{

    static std::filesystem::path assetdir = "";

    static bool fdopen = false;
    static ImVec2 mousePos;

    if (Project::ProjectLoaded() && assetdir == "")
    {
        assetdir = Project::GetProject()->GetAssetPath();
    }

    if (ImGui::BeginMenuBar())
    {

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", "Ctrl+N"))
            {
                std::cout << "made new project" << std::endl;
            }

            if (ImGui::MenuItem("Open", "Ctrl+O"))
            {
                std::cout << "opened new project" << std::endl;
            }

            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                std::cout << "saved new project" << std::endl;
            }

            ImGui::EndMenu();
        }

        if (InputMgr::IsKeyDown(ImGuiKey_LeftCtrl))
        {
            if (InputMgr::IsKeyPressed(ImGuiKey_N))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Create new project", nullptr, ".");
                fileDataType = 0;
            }

            if (InputMgr::IsKeyPressed(ImGuiKey_O))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Open a Project", ".auproject", ".");
                fileDataType = 1;
            }

            if (InputMgr::IsKeyPressed(ImGuiKey_S))
            {
                Project::GetProject()->Save();
            }
        }

        if (ImGui::BeginMenu("Options"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            //ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            //ImGui::MenuItem("Padding", NULL, &opt_padding);
            //ImGui::Separator();

            // Display a menu item for each Dockspace flag, clicking on one will toggle its assigned flag.
            //if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
            //if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
            //if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
            //if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
            //if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
            //ImGui::Separator();

            ImGui::EndMenu();
        }

        if (Project::ProjectLoaded()) {

            Scene* currentScene = Scene::GetScene();

            if (ImGui::BeginMenu("Add"))
            {


                if (ImGui::Selectable(ICON_FA_CUBE " Empty Entity"))
                {
                    Entity* entity = currentScene->entity_mgr->CreateEntity("New Entity");
                }


                if (ImGui::Selectable(ICON_FA_CUBE " Cube"))
                {
                    Entity* entity = currentScene->entity_mgr->CreateEntity("Cube");

                    MeshRenderer* renderer = entity->AttachComponent<MeshRenderer>();
                    renderer->mesh = Mesh::Load("Assets/Editor/models/cube.fbx", 0);
                    Shader* shader = new Shader("Assets/Editor/shaders/0/");
                    entity->SetShader(shader);

                    //entity->AttachComponent<BoxCollider>();
                    entity->AttachComponent<RigidBody3D>();

                    entity->Init();
                }

                if (ImGui::Selectable(ICON_FA_CIRCLE " Sphere"))
                {
                    Entity* entity = currentScene->entity_mgr->CreateEntity("Sphere");

                    MeshRenderer* renderer = entity->AttachComponent<MeshRenderer>();
                    renderer->mesh = Mesh::Load("Assets/Editor/models/sphere.fbx", 0);
                    Shader* shader = new Shader("Assets/Editor/shaders/0/");
                    entity->SetShader(shader);

                    //entity->AttachComponent<SphereCollider>();
                    entity->AttachComponent<RigidBody3D>();

                    entity->Init();
                }

                if (ImGui::Selectable("Cone"))
                {
                    Entity* entity = currentScene->entity_mgr->CreateEntity("Cone");

                    MeshRenderer* renderer = entity->AttachComponent<MeshRenderer>();
                    renderer->mesh = Mesh::Load("Assets/Editor/models/cone.fbx", 0);
                    Shader* shader = new Shader("Assets/Editor/shaders/0/");
                    entity->SetShader(shader);
                }

                if (ImGui::Selectable("Cylinder"))
                {
                    Entity* entity = currentScene->entity_mgr->CreateEntity("Cylinder");

                    MeshRenderer* renderer = entity->AttachComponent<MeshRenderer>();
                    renderer->mesh = Mesh::Load("Assets/Editor/models/cylinder.fbx", 0);
                    Shader* shader = new Shader("Assets/Editor/shaders/0/");
                    entity->SetShader(shader);
                }

                ImGui::Separator();

                if (ImGui::Selectable(ICON_FA_LIGHTBULB " Point Light"))
                {
                    Entity* entity = currentScene->entity_mgr->CreateEntity("Light");

                    PointLight* light = entity->AttachComponent<PointLight>();

                    light->color = vec3(1, 1, 1);
                    light->power = 1000;
                    light->enabled = true;

                    entity->Init();
                }

                ImGui::Separator();

                if (ImGui::Selectable(ICON_FA_GLOBE " Skybox"))
                {
                    Entity* entity = currentScene->entity_mgr->CreateEntity("Skybox");

                    Skybox* sb = entity->AttachComponent<Skybox>();

                    sb->LoadTexture("Assets/Editor/textures/newport_loft.hdr");

                    entity->Init();
                }

                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Create"))
            {
                assetCreateMenu(this);
                ImGui::EndMenu();
            }
        }

        ImGui::EndMenuBar();
    }

    ImGui::End();
}

void ImguiLayer::SceneView()
{
    Scene* currentScene = Scene::GetScene();
    //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin(ICON_FA_GLOBE " Scene Window");
    {

        // Using a Child allow to fill all the space of the window.
        // It also alows customization
        ImGui::BeginChild("SceneRender");
        // Get the size of the child (i.e. the whole draw size of the windows).
        ImVec2 wsize = ImGui::GetWindowSize();

        ImVec2 vview = ImGui::GetContentRegionAvail();

        ImVec2 wpos = ImGui::GetWindowPos();

        if (editorCamera->framebuffer->width != wsize.x && editorCamera->framebuffer->height != wsize.y) {
            editorCamera->framebuffer->Resize(vview.x, vview.y);
            sceneSize = wsize;
        }

        // Because I use the texture from OpenGL, I need to invert the V from the UV.
        ImGui::Image((ImTextureID)editorCamera->framebuffer->GetID(), wsize, ImVec2(0, 1), ImVec2(1, 0));


        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_MOVE"))
            {
                const wchar_t* payload_n = (const wchar_t*)payload->Data;

                wstring ws(payload_n);

                string payload_s(ws.begin(), ws.end());

                filesystem::path path(payload_s);

                if (fileType == "ModelFile" || fileType == "MeshFile")
                {
                    ModelAsset* asset = Project::GetProject()->processor->GetAsset<ModelAsset>(path.string());

                    if (asset && asset->prefab)
                    {
                        Scene::GetScene()->entity_mgr->InsertPrefab(asset->prefab);
                    }

                    //MeshRenderer* renderer = entity->AttachComponent<MeshRenderer>();
                    //renderer->mesh = Mesh::Load(path.string(), 0);
                }
                if (fileType == "PrefabFile")
                {
                    Prefab* prefab = Project::GetProject()->processor->GetAsset<Prefab>(path.string());

                    if (prefab) {
                        Scene::GetScene()->entity_mgr->InsertPrefab(prefab);
                    }
                }

                Logger::Log(path.relative_path().string() + ", " + fileType, Logger::DBG, "DBG");

            }
            ImGui::EndDragDropTarget();
        }

        ImGui::SetItemAllowOverlap();

        ImGui::SetCursorPos(ImGui::GetWindowContentRegionMin());

        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 window_center = ImVec2(window_pos.x + window_size.x * 0.5f, window_pos.y + window_size.y * 0.5f);
        ImVec2 window_bound = { window_pos.x + window_size.x, window_pos.y + window_size.y };
        ImVec2 mouse_pos = ImGui::GetMousePos();

        ImVec2 mousePos = mouse_pos;

        mousePos = ImClamp(mousePos, wpos, window_bound);

        double xpos = 0, ypos = 0;

        glfwGetCursorPos(GetWindow(), &xpos, &ypos);

        float min = -0.075;
        float max = -min;

        // glfwGetTime is called only once, the first time this function is called
        static double lastTime = glfwGetTime();

        // Compute time difference between current and last frame
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);


        mousePos.x = min + ((mousePos.x - window_pos.x) / (window_bound.x - window_pos.x)) * (max - min);
        mousePos.y = min + ((mousePos.y - window_pos.y) / (window_bound.y - window_pos.y)) * (max - min);

        string s = "Mouse Pos: " + std::to_string(mouse_pos.x) + ", " + to_string(mouse_pos.y);
        string s2 = "Mouse Pos: " + std::to_string(xpos) + ", " + to_string(ypos);

        static vector<float> deltaTimeCache;

        static float ms;

        deltaTimeCache.push_back(deltaTime);

        int k = 0;

        int n = 2;

        if (glfwGetTime() != 0)
        {
            k = (int)glfwGetTime() % n;
        }

        int k2 = 1;

        if (k != 0)
        {
            k2 = (int)glfwGetTime() % k;
        }

        if (k2 == 0)
        {

            float avgdt = 0;

            for (float delta_time_cache : deltaTimeCache)
            {
                avgdt += delta_time_cache;
            }

            avgdt = avgdt / deltaTimeCache.size();

            ms = round(1000.0 / avgdt);

            ms *= 0.001;

            ms = round(ms);

            deltaTimeCache.clear();
        }

        string f = "FrameTime: " + std::to_string((int)ms);
        ImGui::TextColored({ 1,0,0,1 }, f.c_str());


        string pIcon = ICON_FA_PLAY;

        if (currentScene->runtimePlaying)
        {
            pIcon = ICON_FA_STOP;
        }

        if (ImGui::Button(pIcon.c_str()))
        {

            if (!currentScene->runtimePlaying)
                currentScene->OnRuntimeStart();
            else
                currentScene->OnRuntimeUnload();
        }



        //ImGui::TextColored(ImVec4(1, 0, 0, 1), s.c_str());
        //ImGui::TextColored(ImVec4(1, 0, 0, 1), s2.c_str());

        // Initial horizontal angle : toward -Z
        static float horizontalAngle = 3.14f;
        // Initial vertical angle : none
        static float verticalAngle = 0.0f;

        float speed = 30.0f; // 3 units / second
        float mouseSpeed = 1;


        // Initial position : on +Z
        static glm::vec3 position = glm::vec3(0, 0, 5);

        //position = camera->position;

        if (ImGui::IsWindowFocused()) {
            vec2 mouseDelta = InputMgr::GetMouseDelta();

            if (InputMgr::IsMouseButtonDown(ImGuiMouseButton_Middle))
                editorCamera->Pan(mouseDelta);
            else if (InputMgr::IsMouseButtonDown(ImGuiMouseButton_Left))
                editorCamera->Rotate(mouseDelta);
            else if (InputMgr::IsMouseButtonDown(ImGuiMouseButton_Right))
                editorCamera->Zoom(mouseDelta.y);

        }

        // Reset mouse position for next frame
        //glfwSetCursorPos(window, window_pos.x / 2, window_pos.y / 2);

        ImGui::SetItemAllowOverlap();

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::BeginFrame();
        ImGuizmo::Enable(true);

        ImGuizmo::SetDrawlist();

        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, wsize.x, wsize.y);


        mat4 view = editorCamera->GetViewMatrix();
        mat4 projection = editorCamera->GetProjectionMatrix(RenderMgr::GetAspect());

        if (selectedEntity) {

            glm::mat4 matrix = currentScene->entity_mgr->entities[selectedEntityId]->transform->GetGlobalMatrix();

            mat4 gridMat = glm::mat4(1.0);

            //ImGuizmo::DrawGrid(&view[0][0], &projection[0][0], &gridMat[0][0], 100);
            ImGuizmo::Manipulate(&view[0][0], &projection[0][0], currentGizmoOperation, currentGizmoMode, &matrix[0][0]);
            //ImGuizmo::DrawCubes(&view[0][0], &projection[0][0], &matrix[0][0], 1);


            currentScene->entity_mgr->entities[selectedEntityId]->transform->CopyTransforms(matrix, true);
        }

        float viewManipulateRight = ImGui::GetWindowPos().x + wsize.x;
        float viewManipulateTop = ImGui::GetWindowPos().y;

        ImGuizmo::ViewManipulate(&view[0][0], 100.0f, ImVec2(viewManipulateRight - 128, viewManipulateTop), ImVec2(128, 128), 0x10101010);

        if (view != editorCamera->GetViewMatrix())
            editorCamera->CopyView(view);


        /*



        */

        //ImGui::GetForegroundDrawList()->AddCircle(wpos, 10.0f, IM_COL32(0, 255, 0, 200), 0, 10);

        //ImGui::GetForegroundDrawList()->AddCircle(mouse_pos, 10.0f, IM_COL32(0, 255, 0, 200), 0, 10);

        //ImGui::GetForegroundDrawList()->AddCircle(window_bound, 10.0f, IM_COL32(0, 255, 0, 200), 0, 10);

        ImGui::EndChild();

        lastTime = currentTime;
    }
    ImGui::End();
    //ImGui::PopStyleVar();
}

void ImguiLayer::GameView()
{
    Scene* currentScene = Scene::GetScene();
    if (ImGui::Begin(ICON_FA_GAMEPAD "Game Window"))
    {

        ImGui::BeginChild("GameRender");

        // Get the size of the child (i.e. the whole draw size of the windows).
        ImVec2 wsize = ImGui::GetWindowSize();

        ImVec2 vview = ImGui::GetContentRegionAvail();

        Camera* main = Camera::GetMain();

        if (main) {

            RenderTexture* tex = main->GetRenderTexture();

            if (tex->width != wsize.x && tex->height != wsize.y) {
                main->GetRenderTexture()->Resize(vview.x, vview.y);
            }
            // Because I use the texture from OpenGL, I need to invert the V from the UV.
            ImGui::Image((ImTextureID)main->GetRenderTexture()->ID, wsize, ImVec2(0, 1), ImVec2(1, 0));
        }
        else
        {
            ImGui::Text("NO MAIN CAMERA");
        }

        ImGui::EndChild();

    }
    ImGui::End();
}

void ImguiLayer::Inspector()
{
    Scene* currentScene = Scene::GetScene();
    ImGui::Begin(ICON_FA_LIST " Properties");
    {
        if (selectedEntity) {
            auto const& entity = currentScene->entity_mgr->entities[selectedEntityId];

            ImGui::Checkbox("Enabled", &entity->enabled);

            ImGui::SameLine();

            ImGui::InputText("Name", &entity->name);

            ImGui::SameLine();

            ImGui::Text(("ID: " + to_string(entity->id)).c_str());

            if (ImGui::CollapsingHeader(ICON_FA_UP_DOWN_LEFT_RIGHT " Transform")) {

                if (InputMgr::IsKeyPressed((ImGuiKey)90))
                    currentGizmoOperation = ImGuizmo::TRANSLATE;
                if (InputMgr::IsKeyPressed((ImGuiKey)69))
                    currentGizmoOperation = ImGuizmo::ROTATE;
                if (InputMgr::IsKeyPressed((ImGuiKey)82)) // r Key
                    currentGizmoOperation = ImGuizmo::SCALE;
                if (ImGui::RadioButton("Translate", currentGizmoOperation == ImGuizmo::TRANSLATE))
                    currentGizmoOperation = ImGuizmo::TRANSLATE;
                ImGui::SameLine();
                if (ImGui::RadioButton("Rotate", currentGizmoOperation == ImGuizmo::ROTATE))
                    currentGizmoOperation = ImGuizmo::ROTATE;
                ImGui::SameLine();
                if (ImGui::RadioButton("Scale", currentGizmoOperation == ImGuizmo::SCALE))
                    currentGizmoOperation = ImGuizmo::SCALE;

                glm::mat4 matrix = currentScene->entity_mgr->entities[selectedEntityId]->transform->GetMatrix();
                float matrixTranslation[3], matrixRotation[3], matrixScale[3];
                ImGuizmo::DecomposeMatrixToComponents(&matrix[0][0], matrixTranslation, matrixRotation, matrixScale);
                ImGui::DragFloat3("Tr", matrixTranslation);
                ImGui::DragFloat3("Rt", matrixRotation);
                ImGui::DragFloat3("Sc", matrixScale);

                for (int i = 0; i < 3; ++i)
                {
                    //matrixRotation[i] *= RAD2DEG;
                }

                ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, &matrix[0][0]);

                /*
                matrix = translate(matrix, { matrixTranslation[0], matrixTranslation[1], matrixTranslation[2] });
                matrix = rotate(matrix, matrixRotation[0], { 1,0,0 });
                matrix = rotate(matrix, matrixRotation[1], { 0,1,0 });
                matrix = rotate(matrix, matrixRotation[2], { 0,0,1 });
                matrix = scale(matrix, { matrixScale[0], matrixScale[1], matrixScale[2] });
                */

                currentScene->entity_mgr->entities[selectedEntityId]->transform->CopyTransforms(matrix);

                if (currentGizmoOperation != ImGuizmo::SCALE)
                {
                    if (ImGui::RadioButton("Local", currentGizmoMode == ImGuizmo::LOCAL))
                        currentGizmoMode = ImGuizmo::LOCAL;
                    ImGui::SameLine();
                    if (ImGui::RadioButton("World", currentGizmoMode == ImGuizmo::WORLD))
                        currentGizmoMode = ImGuizmo::WORLD;
                }

                if (ImGui::Button("Reset"))
                {
                    currentScene->entity_mgr->entities[selectedEntityId]->transform->Reset();
                }
            }

            entity->RenderComponents();

            if (true) {


                if (ImGui::CollapsingHeader(ICON_FA_PAINTBRUSH " Material")) {

                    if (entity->GetComponent<MeshRenderer>() && entity->GetComponent<MeshRenderer>()->mesh) {

                        Mesh* m = currentScene->entity_mgr->entities[selectedEntityId]->GetComponent<MeshRenderer>()->mesh;

                    }


                    Shader* os = entity->material->shader;
                    Shader* s = entity->material->shader;

                    string st = "none";

                    if (s)
                    {
                        st = s->name;
                    }

                    if (ImGui::BeginCombo("Shader", st.c_str())) {

                        for (pair<const string, Shader*> loaded_shader : Shader::GetLoadedShaders())
                        {
                            if (ImGui::Selectable(loaded_shader.first.c_str()))
                            {
                                s = loaded_shader.second;
                            }
                        }

                        ImGui::EndCombo();
                    }

                    ImGui::Separator();

                    if (s)
                    {
                        if (s != os) {
                            entity->material->LoadShader(s);
                        }
                    }

                    /*

                    if (entity->material->uniforms.size() > 0) {

                        if (entity->material->uniforms.count("albedo")) {

                            float albed[3] = {
                                entity->material->entity->material->uniforms[("albedo")].v3.x,
                                entity->material->entity->material->uniforms[("albedo")].v3.y,
                                entity->material->entity->material->uniforms[("albedo")].v3.z
                            };

                            ImGui::ColorPicker3("Albedo", albed);

                            entity->material->entity->material->uniforms[("albedo")].v3 = { albed[0], albed[1], albed[2] };


                            ImGui::SliderFloat("Roughness", &entity->material->entity->material->uniforms[("roughness")].f, 0, 1);
                            ImGui::SliderFloat("Metallic", &entity->material->entity->material->uniforms[("metallic")].f, 0, 1);

                        }
                    }

                    */

                    /*

                    for (auto [propertyName, property] : entity->material->properties)
                    {
                        switch (property->GetType())
                        {
                        case ShaderFactory::Float:
                        {
                            ImGui::DragFloat(propertyName.c_str(), &property->defaultValueF, 0.01);
                            break;
                        }
                        case ShaderFactory::Int:
                        {
                            ImGui::DragInt(propertyName.c_str(), &property->defaultValueI, 1);
                            break;
                        }
                        case ShaderFactory::Bool:
                        {
                            ImGui::Checkbox(propertyName.c_str(), &property->defaultValueB);
                            break;
                        }
                        case ShaderFactory::Vector2:
                        {
                            float v[2];

                            for (int i = 0; i < 2; ++i)
                            {
                                v[i] = property->defaultValue2[i];
                            }

                            ImGui::DragFloat2(propertyName.c_str(), v);

                            for (int i = 0; i < 2; ++i)
                            {
                                property->defaultValue2[i] = v[i];
                            }
                            break;
                        }
                        case ShaderFactory::Vector3:
                        {
                            float v[3];

                            for (int i = 0; i < 3; ++i)
                            {
                                v[i] = property->defaultValue3[i];
                            }

                            ImGui::DragFloat3(propertyName.c_str(), v);

                            for (int i = 0; i < 3; ++i)
                            {
                                property->defaultValue3[i] = v[i];
                            }
                            break;
                        }
                        case ShaderFactory::Vector4:
                        {
                            float v[4];

                            for (int i = 0; i < 4; ++i)
                            {
                                v[i] = property->defaultValue4[i];
                            }

                            ImGui::DragFloat4(propertyName.c_str(), v);

                            for (int i = 0; i < 4; ++i)
                            {
                                property->defaultValue4[i] = v[i];
                            }
                            break;
                        }
                        case ShaderFactory::Color:
                            float v[4];

                            for (int i = 0; i < 4; ++i)
                            {
                                v[i] = property->defaultValue4[i];
                            }

                            ImGui::ColorEdit4(propertyName.c_str(), v);

                            for (int i = 0; i < 4; ++i)
                            {
                                property->defaultValue4[i] = v[i];
                            }
                            break;
                        default:
                            break;
                        }
                    }

*/



                    for (pair<const string, Material::UniformData> uniform : entity->material->uniforms)
                    {
                        switch (uniform.second.type)
                        {
                        case ShaderFactory::Bool:
                            ImGui::Checkbox(uniform.first.c_str(), &uniform.second.b);
                            break;
                        case ShaderFactory::Int:
                            ImGui::DragInt(uniform.first.c_str(), &uniform.second.i);
                            break;
                        case ShaderFactory::Float:
                            ImGui::DragFloat(uniform.first.c_str(), &uniform.second.f, 0.01);
                            break;
                        case ShaderFactory::Vector2:
                        {
                            float v[2];

                            for (int i = 0; i < 2; ++i)
                            {
                                v[i] = uniform.second.v2[i];
                            }

                            ImGui::DragFloat2(uniform.first.c_str(), v);

                            for (int i = 0; i < 2; ++i)
                            {
                                uniform.second.v2[i] = v[i];
                            }
                        }
                        break;
                        case ShaderFactory::Vector3:
                        {
                            float v[3];

                            for (int i = 0; i < 3; ++i)
                            {
                                v[i] = uniform.second.v3[i];
                            }

                            ImGui::DragFloat3(uniform.first.c_str(), v);

                            for (int i = 0; i < 3; ++i)
                            {
                                uniform.second.v3[i] = v[i];
                            }
                        }
                        break;
                        case ShaderFactory::Vector4:
                        {
                            float v[4];

                            for (int i = 0; i < 4; ++i)
                            {
                                v[i] = uniform.second.v4[i];
                            }

                            ImGui::DragFloat4(uniform.first.c_str(), v);

                            for (int i = 0; i < 4; ++i)
                            {
                                uniform.second.v4[i] = v[i];
                            }
                        }
                        break;
                        case ShaderFactory::Color:
                            float v[4];

                            for (int i = 0; i < 4; ++i)
                            {
                                v[i] = uniform.second.v4[i];
                            }

                            ImGui::ColorEdit4(uniform.first.c_str(), v);

                            for (int i = 0; i < 4; ++i)
                            {
                                uniform.second.v4[i] = v[i];
                            }
                            break;
                        default:
                            break;
                        }

                        entity->material->uniforms[uniform.first] = uniform.second;
                    }
                }

            }

            ImGui::Separator();

            AlignForWidth(ImGui::CalcTextSize("Add Component").x + ImGui::GetStyle().ItemSpacing.x);

            static bool acopen = false;

            ImVec2 cursorPos = ImGui::GetCursorScreenPos();

            ImVec2 cursorSize = ImGui::CalcTextSize("Add Component");

            cursorPos.y += cursorSize.y * 2;

            if (ImGui::Button("Add Component"))
            {
                acopen = true;
            }

            if (acopen)
            {
                ImGui::SetNextWindowPos(cursorPos);


                ImGui::Begin("Add Component");

                static std::string filter = "";

                ImGui::InputText(ICON_FA_MAGNIFYING_GLASS, &filter);

                ImGui::BeginChild("acid");

                for (std::pair<const std::string, std::shared_ptr<Component>(*)()> cmp_map : ComponentRegistry::cmp_map)
                {
                    if (cmp_map.first.find(filter) != std::string::npos) {
                        bool selected = false;

                        ImGui::Selectable(cmp_map.first.c_str(), &selected);

                        if (selected)
                        {
                            entity->AddComponent(cmp_map.first);
                            acopen = false;
                            break;
                        }
                    }
                }

                ImGui::EndChild();

                ImGui::End();

                if (InputMgr::IsKeyPressed(ImGuiKey_Escape))
                {
                    acopen = false;
                }
            }
            if (InputMgr::IsKeyDown(ImGuiKey_LeftCtrl) && InputMgr::IsKeyPressed(ImGuiKey_D))
            {
                auto const& clone = currentScene->entity_mgr->DuplicateEntity(entity.get());

                clone->name = "Copy of " + clone->name;
                selectedEntityId = clone->id;
            }
        }
        else if (isFileSelected)
        {
            std::string filename = selectedPath.filename().string();
            ImGui::InputText("Name", &filename);

            ImGui::SameLine();

            if (ImGui::Button(ICON_FA_TRASH_CAN "###file"))
            {
                Filesystem::DeleteFile(selectedPath.string());
                isFileSelected = false;
            }

            std::string ext = selectedPath.extension().string();

            static float padding = 16.0f;
            static float thumbnailSize = 64;
            float cellSize = thumbnailSize * padding;

            float panelWidth = ImGui::GetContentRegionAvail().x;
            int columnCount = (int)(panelWidth / cellSize);

            if (columnCount < 1)
            {
                columnCount = 1;
            }

            if (fileType == "TextFile" || fileType == "CodeFile")
            {
                if (fileType == "CodeFile")
                {
                    TextEditor::LanguageDefinition lang;

                    lang = TextEditor::LanguageDefinition::C();

                    if (ext == ".glsl")
                    {
                        lang = TextEditor::LanguageDefinition::GLSL();
                    }
                    else if (ext == "lua")
                    {
                        lang = TextEditor::LanguageDefinition::Lua();
                    }
                    else if (ext == ".cpp" || ext == ".hpp" || ext == ".h" || ext == ".c")
                    {
                        lang = TextEditor::LanguageDefinition::CPlusPlus();
                    }

                    imguiTextEditor->SetLanguageDefinition(lang);


                }
                else
                {
                }

                auto cpos = imguiTextEditor->GetCursorPosition();

                ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, imguiTextEditor->GetTotalLines(),
                    imguiTextEditor->IsOverwrite() ? "Ovr" : "Ins",
                    imguiTextEditor->CanUndo() ? "*" : " ",
                    imguiTextEditor->GetLanguageDefinition().mName.c_str(), filename.c_str());

                imguiTextEditor->Render("TextEditor");

                if (InputMgr::IsKeyDown(ImGuiKey_LeftCtrl) && InputMgr::IsKeyPressed(ImGuiKey_S))
                    Filesystem::WriteFileString(selectedPath.string(), imguiTextEditor->GetText());


            }
            else if (fileType == "ImageFile")
            {
                if (!textureCache.count(selectedPath.string())) {
                    
                    Texture ntex = Texture::Load(selectedPath.string(), false);

                    textureCache.insert(std::pair<std::string, Texture>(selectedPath.string(), ntex));
                }

                Texture tex = textureCache.at(selectedPath.string());

                ImGui::Image((ImTextureID)tex.ID, { thumbnailSize, thumbnailSize });

                ImGui::SliderFloat("##ThumbnailSize", &thumbnailSize, 16, tex.width, "");

                padding = thumbnailSize / 32;

            }
            else if (fileType == "ModelFile")
            {
                /*
                ImTextureID t = (ImTextureID)loadedModels[selected_file.string()]->GetIcon();

                ImGui::Image(t, { thumbnailSize, thumbnailSize }, {1,1}, {0,0});

                ImGui::SliderFloat("##ThumbnailSize", &thumbnailSize, 16, 500, "");

                padding = thumbnailSize / 32;
                */
            }
        }
    }
    ImGui::End();

}

void ImguiLayer::Explorer()
{
    Scene* currentScene = Scene::GetScene();
    ImGui::Begin(ICON_FA_MAGNIFYING_GLASS " Explorer");

    for (auto const& entity : currentScene->entity_mgr->entities)
    {
        if (entity->transform->parent == nullptr) {
            /*
            const bool is_selected = (Entity::selected_id == entity->id);

            std::string cicon = ICON_FA_CHECK;

            if (!entity->enabled)
                cicon = ICON_FA_XMARK;

            if (ImGui::Button((cicon + "###" + to_string(entity->id)).c_str()))
            {
                entity->enabled = !entity->enabled;
            }


            ImGui::SameLine();

            string label = entity->name + "" + to_string(entity->id);


            if (ImGui::Selectable(label.c_str(), is_selected)) {
                Entity::selected_id = entity->id;
                selectedEntity = true;
            }


*                */

            DrawEntityTree(entity.get());
        }
    }

    ImGui::End();
}

void ImguiLayer::Log()
{

    ImGui::Begin(ICON_FA_MESSAGE " Log");

    map<string, Logger::Level> loggedents;

    for (std::pair<std::pair<int, std::string>, Logger::Level> logged_entry : Logger::loggedEntries)
    {
        if (!loggedents.count(logged_entry.first.second)) {
            ImVec4 col = { 1,1,1,1 };

            switch (logged_entry.second)
            {
            case Logger::INFO:
                break;
            case Logger::DBG:
                col = { 0,.5,1,1 };
                break;
            case Logger::WARN:
                col = { 1,1,0,1 };
                break;
            case Logger::LOG_ERROR:
                col = { 1,.25,.25,1 };
                break;
            }

            ImGui::TextColored(col, logged_entry.first.second.c_str());

            ImGui::NextColumn();

            loggedents.insert({ logged_entry.first.second, logged_entry.second });
        }
    }

    ImGui::End();
}

void ImguiLayer::DebugMenu()
{
    ImGui::Begin("Debug Menu");

    ImGui::Text("Texture Debugger");

    static int selectedTex = 0;

    if (ImGui::Button("<"))
    {
        selectedTex--;
    }

    ImGui::SameLine();

    ImGui::Text(to_string(selectedTex).c_str());

    ImGui::SameLine();

    if (ImGui::Button(">"))
    {
        selectedTex++;
    }

    selectedTex = ImClamp(selectedTex, 0, 100);

    ImGui::Image((ImTextureID)selectedTex, { 256,256 });

    ImGui::Separator();


    ImGui::End();
}

void ImguiLayer::Styler()
{
    ImGui::Begin("Style Editor");

    ImGuiStyle s = ImGui::GetStyle();

    static ImVec3 textCol = GetVec3(s.Colors[ImGuiCol_Text]), headCol = GetVec3(s.Colors[ImGuiCol_Button]), areaCol = GetVec3(s.Colors[ImGuiCol_FrameBg]), bodyCol = GetVec3(s.Colors[ImGuiCol_WindowBg]), popsCol = GetVec3(s.Colors[ImGuiCol_PopupBg]);

    ColorEditVec3(textCol, "Text Color");
    ColorEditVec3(headCol, "Head Color");
    ColorEditVec3(areaCol, "Area Color");
    ColorEditVec3(bodyCol, "Body Color");
    ColorEditVec3(popsCol, "Popup Color");

    float d = 15;

    headCol.x = bodyCol.x - flt(d / 255.0f);
    headCol.y = bodyCol.y - flt(d / 255.0f);
    headCol.z = bodyCol.z - flt(d / 255.0f);

    d = 15;

    areaCol.x = headCol.x - flt(d / 255.0f);
    areaCol.y = headCol.y - flt(d / 255.0f);
    areaCol.z = headCol.z - flt(d / 255.0f);

    popsCol = bodyCol;

    imgui_easy_theming(textCol, headCol, areaCol, bodyCol, areaCol);

    if (ImGui::Button("Show popup"))
    {
        ImGui::OpenPopup("TestPopup");
    }

    ImVec2 wsiz = { RenderMgr::GetWindowSize().x, RenderMgr::GetWindowSize().y };

    static float div = 3.5;

    ImVec2 winSize = { RenderMgr::GetWindowSize().x / div,RenderMgr::GetWindowSize().y / div };

    ImGui::SetNextWindowSize(winSize);
    //ImGui::SetNextWindowPos({ wsiz.x * 0.5f, wsiz.y * 0.5f });

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("TestPopup", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::DragFloat("size", &div, 0.01f, 0.25, 10);

        //div = std::clamp(div, 1.5f, 5.0f);

        if (InputMgr::IsKeyDown(ImGuiKey_Escape))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}

void ImguiLayer::AssetBrowser()
{
    if (ImGui::Begin(ICON_FA_FOLDER_OPEN "Asset Browser"))
    {
	    
    }
    ImGui::End();

}

void ImguiLayer::FileDialogues()
{
    ImGui::SetNextWindowSize({ 600,400 });
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse))
    {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            switch (fileDataType)
            {
            case 0:
                Project::Create(filePath);
                selectedEntity = false;
                isFileSelected = false;
                break;
            case 1:
                Project::Load(filePath);
            }
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }

}

void ImguiLayer::EndFrame()
{
    ImGui::RenderFrame();
}


void ImguiLayer::DrawEntityTree(Entity* entity)
{
    if (entity == nullptr)
        return;

    const bool is_selected = (selectedEntityId == entity->id);

    ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow;

    ImGuiTreeNodeFlags node_flags = base_flags;

    if (is_selected)
        node_flags |= ImGuiTreeNodeFlags_Selected;

    std::string cicon = ICON_FA_CHECK;

    if (!entity->enabled)
        cicon = ICON_FA_XMARK;

    if (ImGui::Button((cicon + "###" + to_string(entity->id)).c_str()))
    {
        entity->enabled = !entity->enabled;
    }


    ImGui::SameLine();

    string label = entity->name + "###" + to_string(entity->id);



    if (ImGui::TreeNodeEx(label.c_str(), node_flags))
    {
        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if (is_selected) {
            ImGui::SetItemDefaultFocus();
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            selectedEntityId = entity->id;
            selectedEntity = true;
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::Text(entity->name.c_str());
            ImGui::SetDragDropPayload("ENT_MOVE", &entity->id, sizeof(int));
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENT_MOVE"))
            {
                int payload_n = *(const int*)payload->Data;

                Scene::GetScene()->entity_mgr->entities[payload_n]->SetParent(entity);

            }
            ImGui::EndDragDropTarget();
        }

        ImGui::Indent();
        for (shared_ptr<Transform> child : entity->transform->children)
        {
            DrawEntityTree(child->entity);
        }
        ImGui::Unindent();

        ImGui::TreePop();
    }
    else
    {
        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if (is_selected) {
            ImGui::SetItemDefaultFocus();
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            selectedEntityId = entity->id;
            selectedEntity = true;
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
        {
            ImGui::Text(entity->name.c_str());
            ImGui::SetDragDropPayload("ENT_MOVE", &entity->id, sizeof(int));
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENT_MOVE"))
            {
                int payload_n = *(const int*)payload->Data;

                Scene::GetScene()->entity_mgr->entities[payload_n]->SetParent(entity);

            }
            ImGui::EndDragDropTarget();
        }
    }

}
