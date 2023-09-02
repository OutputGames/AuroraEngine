#include "render.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "graphics/animation.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "engine/entity.hpp"
#include "engine/log.hpp"
#include "engine/project.hpp"
#include "graphics/lighting.hpp"
#include "graphics/model.hpp"
#include "utils/filesystem.hpp"

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

GLFWwindow* window;

bool mouse_cond, selectedEntity, selectedFile;

std::filesystem::path selected_file;
std::string filetype, filedata;

TextureColorBuffer* bfr;

TextEditor* text_editor;

ImVec2 oldGS;

uint32_t selected_id;

bool fidopen=false;
int fdtype;

mat4 view, projection;

std::vector<RenderData*> RenderMgr::renderObjs;

static std::map<std::string, Texture> textureCache = {
};

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

GLFWwindow* GetWindow()
{
    return window;
}

void RenderMgr::InitGraphicsDevice()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "AuroraEngine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        exit(-1);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    //imgui_io = io;

    io.Fonts->AddFontDefault();
    float baseFontSize = 16; // 13.0f is the size of the default font. Change to the font size you use.
    float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    io.Fonts->AddFontFromFileTTF((std::string("resources/fonts/")+std::string(FONT_ICON_FILE_NAME_FAS)).c_str(), iconFontSize, &icons_config, icons_ranges);
    // use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        //style.WindowRounding = 0.0f;
        //style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bfr = new TextureColorBuffer();

    oldGS = { flt SCR_WIDTH, flt SCR_HEIGHT };

    camera = new Camera();

    const float radius = 25.0F;
    float camX = sin(45) * radius;
    float camZ = cos(45) * radius;
    glm::vec3 camPos = glm::vec3(camX, 25 / 2, camZ);

    camera->position = camPos;

    view = camera->GetViewMatrix();

    text_editor = new TextEditor;

    Logger::Log("Initialized graphics device. ", Logger::INFO, "GFX");
}

void RenderMgr::UpdateGraphicsDevice()
{
            // input
    // -----
    processInput(window);

    //camera->Update();

    camera->up = vec3(view[1]);
    camera->right = vec3(view[0]);
    camera->position = vec3(view[3]);

    mat4 tempMat = inverse(view);

    camera->position = tempMat[3];

    if (Project::ProjectLoaded())
    {
        //Scene::GetScene()->entity_mgr->entities[0]->transform->position = camera->position;
    }


    bfr->Bind();

    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int width = oldGS.x, height = oldGS.y;
            // camera/view transformation
    const float radius = 25.0F;
    float camX = sin(45) * radius;
    float camZ = cos(45) * radius;
    glm::vec3 camPos = glm::vec3(camX, 25/2, camZ);

    //camera->position = camPos;

    camera->lookat = vec3(0, 0, 0);
    camera->useDirection = false;

    float aspect = flt width / flt height;

    projection = camera->GetProjectionMatrix(aspect);


    if (Project::ProjectLoaded()) {
        for (int i = 0; i < Scene::GetScene()->light_mgr->lights.size(); ++i)
        {
            Light* l = Scene::GetScene()->light_mgr->lights[i];
            l->CalcShadowMap();
        }
    }

    bfr->Bind();

    glViewport(0, 0, oldGS.x, oldGS.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (RenderData* render_obj : renderObjs)
    {
        if (render_obj->useDepthMask == false) {
            glDepthMask(GL_FALSE);
            glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        }

        if (render_obj->cullBack == false)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CW);
        }

        if (render_obj->mesh->material->shader) {
            Shader* rshader = render_obj->mesh->material->shader;
            rshader->reload();
            rshader->use();
        }
        if (render_obj->mesh->material->GetUniform("model"))
			render_obj->mesh->material->GetUniform("model")->m4 = render_obj->matrix;
        if (render_obj->mesh->material->GetUniform("view"))
			render_obj->mesh->material->GetUniform("view")->m4 = view;
        if (render_obj->mesh->material->GetUniform("projection")) 
			render_obj->mesh->material->GetUniform("projection")->m4 = projection;

        //render_obj->mesh->material->GetUniform("albedo")->v3 = albedo;
        //render_obj->mesh->material->GetUniform("roughness")->f = roughness;
        if (render_obj->mesh->material->GetUniform("ao")) {
            render_obj->mesh->material->GetUniform("ao")->f = 1.0;
        }
        //render_obj->mesh->material->GetUniform("metallic")->f = metallic;
        if (render_obj->mesh->material->GetUniform("viewPos")) {
            render_obj->mesh->material->GetUniform("viewPos")->v3 = camera->position;
        }

        if (render_obj->mesh->material->GetUniform("cameraRight")) {
            render_obj->mesh->material->GetUniform("cameraRight")->v3 = camera->right;
        }

        if (render_obj->mesh->material->GetUniform("cameraUp")) {
            render_obj->mesh->material->GetUniform("cameraUp")->v3 = camera->up;
        }

        if (render_obj->mesh->material->GetUniform("lights[0].position")) {
            Scene::GetScene()->light_mgr->EditMaterial(render_obj->mesh->material);
            bfr->Bind();
        }

        render_obj->mesh->Draw();

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS); // set depth function back to default
        glDisable(GL_CULL_FACE);
    } 

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    renderObjs.clear();

    RenderEngineSpace();

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void RenderMgr::DestroyGraphicsDevice()
{
        glfwTerminate();
}

bool RenderMgr::CheckCloseWindow()
{
    return glfwWindowShouldClose(window);
}

void RenderMgr::RenderScene()
{
}

void AlignForWidth(float width, float alignment = 0.5f)
{
    ImGuiStyle& style = ImGui::GetStyle();
    float avail = ImGui::GetContentRegionAvail().x;
    float off = (avail - width) * alignment;
    if (off > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
}

void RenderMgr::RenderEngineSpace()
{

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuizmo::BeginFrame();

    bool show_demo_window = true;

    bool p_open = true;
    // Variables to configure the Dockspace example.
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

        // If you copied this demo function into your own code and removed ImGuiWindowFlags_MenuBar at the top of the function,
    // you should remove the below if-statement as well.
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

            if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
            {
	            if (ImGui::IsKeyPressed(ImGuiKey_N))
	            {
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Create new project", nullptr, ".");
                    fdtype = 0;
	            }

                if (ImGui::IsKeyPressed(ImGuiKey_O))
                {
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Open a Project", ".auproject", ".");
                    fdtype = 1;
                }

                if (ImGui::IsKeyPressed(ImGuiKey_S))
                {
                    Project::GetProject()->Save();
                }
            }

            if (ImGui::BeginMenu("Options"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
                ImGui::MenuItem("Padding", NULL, &opt_padding);
                ImGui::Separator();

                // Display a menu item for each Dockspace flag, clicking on one will toggle its assigned flag.
                if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
                if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
                if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
                if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
                if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
                ImGui::Separator();

                ImGui::EndMenu();
            }

            if (Project::ProjectLoaded()) {

                Scene* currentScene = Scene::GetScene();

                if (ImGui::BeginMenu("Add"))
                {

                    if (ImGui::Selectable(ICON_FA_CUBE " Cube"))
                    {
                        Entity* entity = currentScene->entity_mgr->CreateEntity("Cube");

                        ModelRenderer* renderer = entity->AttachComponent<ModelRenderer>();
                        renderer->model = Model::LoadModel("editor/models/cube.fbx");
                        Shader* shader = new Shader("editor/shaders/0/");
                        renderer->model->SetShader(shader);
                    }

                    if (ImGui::Selectable(ICON_FA_CIRCLE " Sphere"))
                    {
                        Entity* entity = currentScene->entity_mgr->CreateEntity("Sphere");

                        ModelRenderer* renderer = entity->AttachComponent<ModelRenderer>();
                        renderer->model = Model::LoadModel("editor/models/sphere.fbx");
                        Shader* shader = new Shader("editor/shaders/0/");
                        renderer->model->SetShader(shader);
                    }

                    if (ImGui::Selectable("Cone"))
                    {
                        Entity* entity = currentScene->entity_mgr->CreateEntity("Cone");

                        ModelRenderer* renderer = entity->AttachComponent<ModelRenderer>();
                        renderer->model = Model::LoadModel("editor/models/cone.fbx");
                        Shader* shader = new Shader("editor/shaders/0/");
                        renderer->model->SetShader(shader);
                    }

                    if (ImGui::Selectable("Cylinder"))
                    {
                        Entity* entity = currentScene->entity_mgr->CreateEntity("Cylinder");

                        ModelRenderer* renderer = entity->AttachComponent<ModelRenderer>();
                        renderer->model = Model::LoadModel("editor/models/cylinder.fbx");
                        Shader* shader = new Shader("editor/shaders/0/");
                        renderer->model->SetShader(shader);
                    }

                    ImGui::Separator();

                    if (ImGui::Selectable(ICON_FA_LIGHTBULB " Point Light"))
                    {
                        Entity* entity = currentScene->entity_mgr->CreateEntity("Light");

                        Light* light = entity->AttachComponent<Light>();

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

                        sb->LoadTexture("editor/textures/newport_loft.hdr");

                        entity->Init();

                        bfr->Resize({ oldGS.x, oldGS.y });
                    }

                    glBindFramebuffer(GL_FRAMEBUFFER, 0);

                    ImGui::EndMenu();
                }
            }

            ImGui::EndMenuBar();
        }

        ImGui::End();

        if (Project::ProjectLoaded()) {

            Scene* currentScene = Scene::GetScene();

        static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::UNIVERSAL);
        static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

        ImGui::Begin(ICON_FA_GAMEPAD " Scene Window");
        {
            // Using a Child allow to fill all the space of the window.
            // It also alows customization
            ImGui::BeginChild("SceneRender");
            // Get the size of the child (i.e. the whole draw size of the windows).
            ImVec2 wsize = ImGui::GetWindowSize();

            ImVec2 wpos = ImGui::GetWindowPos();

            if (wsize.x != oldGS.x && wsize.y != oldGS.y) {
                bfr->Resize({ wsize.x, wsize.y });
            }

            oldGS = wsize;



            mouse_cond = ImGui::IsWindowFocused();

            // Because I use the texture from OpenGL, I need to invert the V from the UV.
            ImGui::Image((ImTextureID)bfr->texture, wsize, ImVec2(0, 1), ImVec2(1, 0));

            ImGuizmo::SetOrthographic(false);
            ImGuizmo::BeginFrame();
            ImGuizmo::Enable(true);

            ImGuizmo::SetDrawlist();

            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, wsize.x, wsize.y);

            if (selectedEntity) {

                glm::mat4 matrix = currentScene->entity_mgr->entities[selected_id]->transform->GetMatrix();

                mat4 gridMat = glm::mat4(1.0);

                //ImGuizmo::DrawGrid(&view[0][0], &projection[0][0], &gridMat[0][0], 100);
                ImGuizmo::Manipulate(&view[0][0], &projection[0][0], mCurrentGizmoOperation, mCurrentGizmoMode, &matrix[0][0]);
                //ImGuizmo::DrawCubes(&view[0][0], &projection[0][0], &matrix[0][0], 1);


                currentScene->entity_mgr->entities[selected_id]->transform->CopyTransforms(matrix);
            }

            float viewManipulateRight = ImGui::GetWindowPos().x + wsize.x;
            float viewManipulateTop = ImGui::GetWindowPos().y;

            ImGuizmo::ViewManipulate(&view[0][0], 100.0f, ImVec2(viewManipulateRight - 128, viewManipulateTop), ImVec2(128, 128), 0x10101010);

            ImVec2 window_pos = ImGui::GetWindowPos();
            ImVec2 window_size = ImGui::GetWindowSize();
            ImVec2 window_center = ImVec2(window_pos.x + window_size.x * 0.5f, window_pos.y + window_size.y * 0.5f);
            ImVec2 window_bound = { window_pos.x + window_size.x, window_pos.y + window_size.y };
            ImVec2 mouse_pos = ImGui::GetMousePos();

            mouse_pos = ImClamp(mouse_pos, wpos, window_bound);

            /*

            // Initial horizontal angle : toward -Z
            static float horizontalAngle = 3.14f;
            // Initial vertical angle : none
            static float verticalAngle = 0.0f;

            float speed = 3.0f; // 3 units / second
            float mouseSpeed = 0.005f;


            horizontalAngle += mouseSpeed * float(window_center.x / 2 - mouse_pos.x);
            verticalAngle += mouseSpeed * float(window_center.y / 2 - mouse_pos.y);

            // Direction : Spherical coordinates to Cartesian coordinates conversion
            glm::vec3 direction(
                cos(verticalAngle)* sin(horizontalAngle),
                sin(verticalAngle),
                cos(verticalAngle)* cos(horizontalAngle)
            );

            // Right std::vector
            glm::vec3 right = glm::vec3(
                sin(horizontalAngle - 3.14f / 2.0f),
                0,
                cos(horizontalAngle - 3.14f / 2.0f)
            );

            // Up std::vector
            glm::vec3 up = glm::cross(right, direction);

            // Initial position : on +Z
            static glm::vec3 position = glm::vec3(0, 0, 5);

            view = glm::lookAt(position, position + direction, up);

            // Reset mouse position for next frame
            glfwSetCursorPos(window, window_pos.x/2, window_pos.y/2);

            */

            ImGui::GetForegroundDrawList()->AddCircle(wpos, 10.0f, IM_COL32(0, 255, 0, 200), 0, 10);

            ImGui::GetForegroundDrawList()->AddCircle(mouse_pos, 10.0f, IM_COL32(0, 255, 0, 200), 0, 10);

            ImGui::GetForegroundDrawList()->AddCircle(window_bound, 10.0f, IM_COL32(0, 255, 0, 200), 0, 10);

            ImGui::EndChild();
        }
        ImGui::End();

        ImGui::Begin(ICON_FA_LIST " Properties");
        {
            if (selectedEntity) {
                Entity* entity = currentScene->entity_mgr->entities[selected_id];

                ImGui::Checkbox("Enabled", &entity->enabled);

                ImGui::SameLine();

                ImGui::InputText("Name", &entity->name);

                ImGui::SameLine();

                ImGui::Text(("ID: " + to_string(entity->id)).c_str());

                if (ImGui::CollapsingHeader(ICON_FA_UP_DOWN_LEFT_RIGHT " Transform")) {

                    if (ImGui::IsKeyPressed((ImGuiKey)90))
                        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
                    if (ImGui::IsKeyPressed((ImGuiKey)69))
                        mCurrentGizmoOperation = ImGuizmo::ROTATE;
                    if (ImGui::IsKeyPressed((ImGuiKey)82)) // r Key
                        mCurrentGizmoOperation = ImGuizmo::SCALE;
                    if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
                        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
                    ImGui::SameLine();
                    if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
                        mCurrentGizmoOperation = ImGuizmo::ROTATE;
                    ImGui::SameLine();
                    if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
                        mCurrentGizmoOperation = ImGuizmo::SCALE;

                    glm::mat4 matrix = currentScene->entity_mgr->entities[selected_id]->transform->GetMatrix();
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

                    currentScene->entity_mgr->entities[selected_id]->transform->CopyTransforms(matrix);

                    if (mCurrentGizmoOperation != ImGuizmo::SCALE)
                    {
                        if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
                            mCurrentGizmoMode = ImGuizmo::LOCAL;
                        ImGui::SameLine();
                        if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
                            mCurrentGizmoMode = ImGuizmo::WORLD;
                    }

                    if (ImGui::Button("Reset"))
                    {
                        currentScene->entity_mgr->entities[selected_id]->transform->Reset();
                    }
                }

                entity->RenderComponents();

                if (currentScene->entity_mgr->entities[selected_id]->GetComponent<ModelRenderer>()) {

                    if (ImGui::CollapsingHeader(ICON_FA_PAINTBRUSH " Material")) {
                        Mesh* m = currentScene->entity_mgr->entities[selected_id]->GetComponent<ModelRenderer>()->model->meshes[0];
                        float albed[3] = { m->material->GetUniform("albedo")->v3.x, m->material->GetUniform("albedo")->v3.y, m->material->GetUniform("albedo")->v3.z };

                        ImGui::ColorPicker3("Albedo", albed);

                        m->material->GetUniform("albedo")->v3 = { albed[0], albed[1], albed[2] };

                        ImGui::SliderFloat("Roughness", &m->material->GetUniform("roughness")->f, 0, 1);
                        ImGui::SliderFloat("Metallic", &m->material->GetUniform("metallic")->f, 0, 1);
                    }

                }

                ImGui::Separator();

                AlignForWidth(ImGui::CalcTextSize("Add Component").x + ImGui::GetStyle().ItemSpacing.x);

                static bool acopen = false;

                ImVec2 cursorPos = ImGui::GetCursorScreenPos();

                ImVec2 cursorSize = ImGui::CalcTextSize("Add Component");

                cursorPos.y += cursorSize.y* 2;

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
                                break;
                            }
                        }
                    }

                    ImGui::EndChild();

                    ImGui::End();

                    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                    {
                        acopen = false;
                    }
                }

                if (ImGui::IsKeyPressed((ImGuiKey_Delete)))
                {
                    entity->Delete();
                    selectedEntity = false;
                }

                if (ImGui::IsKeyPressed((ImGuiKey_Escape)))
                {
                    selectedEntity = false;
                }

                if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_D))
                {
                    Entity* clone = currentScene->entity_mgr->DuplicateEntity(entity);

                    clone->name = "Copy of " + clone->name;
                    selected_id = clone->id;
                }
            }
            else if (selectedFile)
            {
                std::string filename = selected_file.filename().string();
                ImGui::InputText("Name", &filename);

                ImGui::SameLine();

                if (ImGui::Button(ICON_FA_TRASH_CAN "###file"))
                {
                    Engine::Filesystem::DeleteFile(selected_file.string());
                    selectedFile = false;
                }

                std::string ext = selected_file.extension().string();

                static float padding = 16.0f;
                static float thumbnailSize = 64;
                float cellSize = thumbnailSize * padding;

                float panelWidth = ImGui::GetContentRegionAvail().x;
                int columnCount = (int)(panelWidth / cellSize);

                if (columnCount < 1)
                {
                    columnCount = 1;
                }

                if (filetype == "TextFile" || filetype == "CodeFile")
                {
                    if (filetype == "CodeFile")
                    {
                        TextEditor::LanguageDefinition lang;

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

                        text_editor->SetLanguageDefinition(lang);


                    }
                    else
                    {
                    }

                    auto cpos = text_editor->GetCursorPosition();

                    ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, text_editor->GetTotalLines(),
                        text_editor->IsOverwrite() ? "Ovr" : "Ins",
                        text_editor->CanUndo() ? "*" : " ",
                        text_editor->GetLanguageDefinition().mName.c_str(), filename.c_str());

                    text_editor->Render("TextEditor");

                    Engine::Filesystem::WriteFileString(selected_file.string(), text_editor->GetText());


                }
                else if (filetype == "ImageFile")
                {
                    if (!textureCache.count(selected_file.string())) {

                        Texture ntex = Texture::Load(selected_file.string(), false);

                        textureCache.insert(std::pair<std::string, Texture>(selected_file.string(), ntex));
                    }

                    Texture tex = textureCache.at(selected_file.string());

                    ImGui::Image((ImTextureID)tex.ID, { thumbnailSize, thumbnailSize });

                    ImGui::SliderFloat("##ThumbnailSize", &thumbnailSize, 16, tex.width, "");

                    padding = thumbnailSize / 32;

                }
            }
        }
        ImGui::End();

        ImGui::Begin(ICON_FA_MAGNIFYING_GLASS " Explorer");

        for (Entity* entity : currentScene->entity_mgr->entities)
        {
            const bool is_selected = (selected_id == entity->id);

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
                selected_id = entity->id;
                selectedEntity = true;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::End();

        ImGui::Begin(ICON_FA_FOLDER_OPEN " Asset Browser");

        static std::filesystem::path assetdir = Engine::Filesystem::GetCurrentDir();

        if (ImGui::Button(ICON_FA_TURN_UP "###ab"))
        {
            if (assetdir.string() != Engine::Filesystem::GetCurrentDir()) {
                assetdir = assetdir.parent_path();
            }
        }

        ImGui::SameLine();

        ImGui::PushItemWidth(ImGui::CalcTextSize(assetdir.string().c_str()).x + 25);
        ImGui::InputText("###currentdir", &assetdir.string());
        ImGui::PopItemWidth();

        ImVec2 windowSize = ImGui::GetWindowSize();

        ImGui::BeginChild("MainFiles", { windowSize.x, (windowSize.y * 0.75f) });

        static float padding = 16.0f;
        static float thumbnailSize = 64;
        float cellSize = thumbnailSize * padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);

        if (columnCount < 1)
        {
            columnCount = 1;
        }

        ImGui::Columns(columnCount, 0, false);

        static std::map<std::string, Texture> fileIcons = {
            {"Folder", Texture::Load("editor/icons/svgs/solid/folder.png", false)},
            {"File", Texture::Load("editor/icons/svgs/solid/file.png", false)},
            {"TextFile", Texture::Load("editor/icons/svgs/solid/file-lines.png", false)},
            {"CodeFile", Texture::Load("editor/icons/svgs/solid/file-code.png", false)},
            {"ConfigFile", Texture::Load("editor/icons/svgs/solid/gears.png", false)},
            {"Executable", Texture::Load("editor/icons/svgs/solid/computer.png", false)},
            {"Library", Texture::Load("editor/icons/svgs/solid/book.png", false)},
            {"ModelFile", Texture::Load("editor/icons/svgs/solid/cubes.png", false)},
            {"MeshFile", Texture::Load("editor/icons/svgs/solid/cube.png", false)},
            {"SceneFile", Texture::Load("editor/icons/svgs/solid/earth-americas.png", false)},
                        {"ProjectFile", Texture::Load("editor/icons/svgs/solid/diagram-project.png", false)},
        };

        for (auto& directory : std::filesystem::directory_iterator(assetdir))
        {
            const auto& path = directory.path();
            //auto relativePath = std::filesystem::relative(path, )

            std::string filename = path.filename().string();

            //ImGui::Columns(10);

            float s = 64;

            std::string bicon = ICON_FA_FOLDER;
            std::string type = "File";

            ImVec4 iconColor = ImVec4(1, 1, 1, 1);

            bool ploaded = false;

            unsigned int tex = 0;

            if (!directory.is_directory())
            {
                bicon = ICON_FA_FILE;
                std::string ext = path.extension().string();
                if (ext == ".png" || ext == ".jpg")
                {
                    std::string adir = assetdir.string();

                    //replace(adir.begin(), adir.end(), '\\', '/');

                    std::string pth = directory.path().string();

                    //replace(pth.begin(), pth.end(), '\\', '/');
                    if (!textureCache.count(pth)) {

                        Texture ntex = Texture::Load(pth, false);

                        textureCache.insert(std::pair<std::string, Texture>(pth, ntex));
                    }
                    tex = textureCache.at(pth).ID;
                    ploaded = true;
                    type = "ImageFile";
                }
                else if (ext == ".cpp" || ext == ".hpp" || ext == ".h" || ext == ".c" || ext == ".glsl" || ext == ".lua" || ext == ".dll" || ext == ".pdb")
                {
                    type = "CodeFile";
                }
                else if (ext == ".bat" || ext == ".cmd" || ext == ".ini")
                {
                    type = "ConfigFile";
                }
                else if (ext == ".txt")
                {
                    type = "TextFile";
                }
                else if (ext == ".exe" || ext == ".app")
                {
                    type = "Executable";
                }
                else if (ext == ".lib")
                {
                    type = "Library";
                }
                else if (ext == ".fbx" || ext == ".dae")
                {
                    type = "ModelFile";
                }
                else if (ext == ".obj")
                {
                    type = "MeshFile";
                } else if (ext == ".auscene")
                {
                    type = "SceneFile";
                } else if (ext == ".auproject")
                {
                    type = "ProjectFile";
                }
            }
            else
            {
                type = "Folder";
                iconColor = { 1,0.85,0.7,1.0 };
            }

            bool clicked = false;

            if (tex == 0) {
                clicked = ImGui::ImageButton((ImTextureID)fileIcons.find(type)->second.ID, { thumbnailSize,thumbnailSize }, { 0,0 }, { 1,1 }, -1, { 0,0,0,0 }, iconColor);
            }
            else
            {
                clicked = ImGui::ImageButton((ImTextureID)tex, { thumbnailSize,thumbnailSize }, { 0,0 }, { 1,1 }, -1, { 0,0,0,0 }, iconColor);
            }

            if (ImGui::IsItemHovered())
            {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    if (directory.is_directory()) {
                        assetdir /= path.filename();
                    } else
                    {

                        if (type == "ProjectFile")
                        {
                            Project::Load(path.parent_path().string());
                        } else if (type == "SceneFile")
                        {
                            Project::GetProject()->LoadScenePath(path.string());
                        }
                    }
                }
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    selected_file = directory.path();
                    if (!directory.is_directory()) {
                        filedata = Engine::Filesystem::ReadFileString(selected_file.string());
                        text_editor->SetText(filedata);
                    }
                    filetype = type;
                    selectedFile = true;
                    selectedEntity = false;
                }

            }

            ImGui::TextWrapped(filename.c_str());

            ImGui::NextColumn();
        }

        ImGui::Columns(1);

        ImGui::EndChild();

        ImGui::SliderFloat("##ThumbnailSize", &thumbnailSize, 16, 64, "");
        //ImGui::SameLine();
        //ImGui::SliderFloat("Padding", &padding, 0, 32);

        padding = thumbnailSize / 32;

        ImGui::End();

        ImGui::Begin(ICON_FA_MESSAGE " Log");

        for (std::pair<std::pair<int, std::string>, Logger::Level> logged_entry : Logger::loggedEntries)
        {

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
        }

        ImGui::End();

        ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::Begin("Texture Viewer");

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

        ImGui::End();

    }

    ImGui::SetNextWindowSize({ 600,400 });

    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse))
    {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            switch (fdtype)
            {
            case 0:
                Project::Create(filePath);
                selectedEntity = false;
                selectedFile = false;
                break;
            case 1:
                Project::Load(filePath);
                bfr->Resize({ oldGS.x, oldGS.y });
            }
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
}

glm::vec2 RenderMgr::GetWindowSize()
{
    return { SCR_WIDTH,SCR_HEIGHT };
}

glm::vec2 RenderMgr::GetSceneWinSize()
{
    return { oldGS.x, oldGS.y };
}

bool RenderMgr::CheckMouseInputs()
{
    return mouse_cond;
}

FrameBuffer::FrameBuffer()
{
    glGenFramebuffers(1, &id);
    Bind();

}

void FrameBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}

TextureColorBuffer::TextureColorBuffer()
{
    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    // create a color attachment texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "LOG_ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TextureColorBuffer::Resize(glm::vec2 size)
{
    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

    glViewport(0, 0, size.x, size.y);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
