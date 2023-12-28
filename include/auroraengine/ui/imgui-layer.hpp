#ifndef IMGUI_LAYER
#define IMGUI_LAYER

#include "graphics/texture.hpp"
#include "utils/utils.hpp"

struct Entity;
struct EditorCamera;

struct ImguiLayer {
	AURORA_API  void InitLayer();
	AURORA_API  void UpdateLayer();

	AURORA_API void DestroyLayer();


	bool imgui_setup = false;

	 TextEditor* imguiTextEditor;
	 ImVec2 sceneSize;
	 EditorCamera* editorCamera;

	 bool isFileSelected=false;
	 filesystem::path selectedPath, assetDir;
	 string fileType, fileData;
	 int fileDataType=0;


	 bool selectedEntity=false;
	 int selectedEntityId=0;

	 ImGuizmo::OPERATION currentGizmoOperation;
	 ImGuizmo::MODE currentGizmoMode;

	 std::map<std::string, Texture> textureCache;

	// Actual ImGUI Functions
	 void NewFrame();
	 void Dockspace();
	 void MainMenuBar();
	 void SceneView();
	 void GameView();
	 void Inspector();
	 void Explorer();
	 void Log();
	 void DebugMenu();
	 void Styler();
	 void AssetBrowser();
	 void FileDialogues();
	 void EndFrame();

	 void DrawEntityTree(Entity* entity);
};

#endif
