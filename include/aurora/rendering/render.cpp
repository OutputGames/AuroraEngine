#include "render.hpp"

#include "graphics/animation.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "engine/entity.hpp"
#include "engine/log.hpp"
#include "engine/project.hpp"
#include "engine/assets/processor.hpp"
#include "engine/runtime/monort.hpp"
#include "graphics/lighting.hpp"
#include "graphics/model.hpp"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "utils/filesystem.hpp"
#include "utils/input.hpp"

//unsigned int SCR_WIDTH = 800;
//unsigned int SCR_HEIGHT = 600;

GLFWwindow* window;

bool mouse_cond, selectedFile;

//unordered_map<string, Shader*> Shader::loadedShaders;

std::filesystem::path selected_file;
std::string filetype, filedata;

//TextEditor* text_editor;

ImVec2 oldSS, oldGS;

bool renderUndeferred = true;
bool checkedDelta = false;

//uint32_t Entity::selected_id;

bool fidopen=false;
int fdtype;
//mat4 view, projection;

ImGuiContext* ImguiCtx=nullptr;

std::vector<RenderData*> RenderMgr::renderObjs;


//EditorCamera* camera;



// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------

GLFWwindow* GetWindow()
{
    return window;
}

void RenderMgr::InitGraphicsDevice(string windowName, unsigned SCR_WIDTH, unsigned SCR_HEIGHT)
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, windowName.c_str(), NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        exit(-1);
    }


    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImguiCtx = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows


    // Setup Dear ImGui style
    ImGui::StyleColorsDark();


    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(GetWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 410");

    io.Fonts->AddFontDefault();
    float baseFontSize = 16; // 13.0f is the size of the default font. Change to the font size you use.
    float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    io.Fonts->AddFontFromFileTTF((std::string("resources/fonts/") + std::string(FONT_ICON_FILE_NAME_FAS)).c_str(), iconFontSize, &icons_config, icons_ranges);
    // use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid


    oldSS = { flt SCR_WIDTH, flt SCR_HEIGHT };

    Engine::InputMgr::Init();

    Logger::Log("Initialized graphics device. ", Logger::INFO, "GFX");
}

void RenderMgr::UpdateGraphicsDevice()
{
            // input
    // -----
    checkedDelta = false;

    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (ImGui::IsKeyPressed(ImGuiKey_R))
    {
        Shader::ReloadAllShaders();
    }


    if (Project::ProjectLoaded()) {
        Scene* s = Scene::GetScene();

        for (int i = 0; i < s->light_mgr->lights.size(); ++i)
        {
            PointLight* l = s->light_mgr->lights[i];
            l->CalcShadowMap(0);
        }

    }
    else {
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    RenderEngineSpace();

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



void RenderMgr::RenderEngineSpace()
{

    // Start the Dear ImGui frame



    // Variables to configure the Dockspace example.





        // If you copied this demo function into your own code and removed ImGuiWindowFlags_MenuBar at the top of the function,
    // you should remove the below if-statement as well.

}

glm::vec2 RenderMgr::GetWindowSize()
{


    int SCR_WIDTH, SCR_HEIGHT;

    glfwGetWindowSize(window, &SCR_WIDTH, &SCR_HEIGHT);

    return { SCR_WIDTH,SCR_HEIGHT };
}

glm::vec2 RenderMgr::GetSceneWinSize()
{
    return { oldSS.x, oldSS.y };
}

bool RenderMgr::CheckMouseInputs()
{
    return mouse_cond;
}

float RenderMgr::GetDeltaTime()
{
    static double lastTime = glfwGetTime();

    // Compute time difference between current and last frame
    double currentTime = glfwGetTime();
    static float deltaTime = 0;
    if (!checkedDelta) {
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;
        checkedDelta = true;
    }
    return deltaTime;
}

float RenderMgr::GetAspect()
{
    float aspect = flt GetSceneWinSize().x / flt GetSceneWinSize().y;
    return aspect;
}

void RenderMgr::SwapBuffers()
{
    renderObjs.clear();
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
}

ImGuiContext* RenderMgr::GetIMGUIContext()
{
    return ImguiCtx;
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

    int SCR_WIDTH, SCR_HEIGHT;

    glfwGetWindowSize(window, &SCR_WIDTH, &SCR_HEIGHT);

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

void TextureColorBuffer::Unload()
{
    glDeleteRenderbuffers(1, &renderBuffer);
    glDeleteFramebuffers(1, &id);
}
