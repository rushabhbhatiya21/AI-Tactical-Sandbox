#include <iostream>
#include <algorithm>
#include <assert.h>
#include <cfloat>
#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "gameMain.h"
#include "input/inputState.h"

struct KeyEdgeState
{
	bool debugCameraWasPressed = false;
	bool cursorToggleWasPressed = false;
	bool jumpWasPressed = false;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, InputState& inputState, KeyEdgeState& edges);
void drawFpsOverlay(float fps, bool& aiDebugVisible);
void drawAiDebugPanel(const Game& game, bool& enabled);
void setCursorFree(GLFWwindow* window, bool free);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// mouse
float lastX = (float)SCR_WIDTH / 2.f;
float lastY = (float)SCR_HEIGHT / 2.f;
bool firstMouse = true;
bool cursorFree = false;   // F2: release the cursor so debug UI can be clicked (pauses mouse aiming)

// time
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// fps: averaged over a short window so the number is readable
float fpsTimer = 0.0f;
int fpsFrames = 0;
float fps = 0.0f;

// debug ui
bool aiDebugVisible = true;   // whether the AI Debug panel is drawn at all (toggled from the fps overlay)
bool aiDebugEnabled = true;   // whether the panel queries the AI (toggled inside the panel)

int main()
{

#pragma region init glfw

	std::cout << "[INFO] Application started" << std::endl;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

#pragma endregion


#pragma region window creation

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "AI Sandbox", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	std::cout << "[INFO] Window created" << std::endl;

#pragma endregion


#pragma region init glad

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	std::cout << "[INFO] OpenGL initialized" << std::endl;

#pragma endregion


#pragma region configure global opengl state

	glEnable(GL_DEPTH_TEST);

#pragma endregion


#pragma region init imgui

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr; // don't write imgui.ini next to the exe
	// The cursor starts captured for mouse aiming, so ImGui ignores the mouse until F2 frees it
	io.ConfigFlags |= ImGuiConfigFlags_NoMouse | ImGuiConfigFlags_NoMouseCursorChange;

	// install_callbacks = true chains to the GLFW callbacks we registered above
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	std::cout << "[INFO] ImGui initialized" << std::endl;

#pragma endregion


#pragma region init game

	Game game;
	glfwSetWindowUserPointer(window, &game);
	game.initGame();

#pragma endregion

#pragma region render loop

	KeyEdgeState edges;

	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		fpsTimer += deltaTime;
		++fpsFrames;
		if (fpsTimer >= 0.5f)
		{
			fps = fpsFrames / fpsTimer;
			fpsTimer = 0.0f;
			fpsFrames = 0;
		}

		// input
		// -----
		InputState inputState;
		processInput(window, inputState, edges);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.f); // clear color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// update game here
		game.updateGame(deltaTime, inputState);

		// debug ui, drawn on top of the scene
		// -----------------------------------
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		drawFpsOverlay(fps, aiDebugVisible);

		if (aiDebugVisible)
			drawAiDebugPanel(game, aiDebugEnabled);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

#pragma endregion


#pragma region shutdown

	game.closeGame();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	std::cout << "[INFO] Application shutting down" << std::endl;

#pragma endregion

	return 0;
}

#pragma region debug ui

void drawFpsOverlay(float fps, bool& aiDebugVisible)
{
	// No NoInputs flag: the show/hide button must be clickable (only possible while F2 frees the cursor)
	const ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::Begin("##fpsOverlay", nullptr, flags);
	ImGui::Text("%.0f FPS", fps);
	ImGui::Text("%.2f ms", fps > 0.0f ? 1000.0f / fps : 0.0f);

	// Transparent with a white outline; a faint white fill gives hover / press feedback
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.12f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.25f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.85f));
	ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	if (ImGui::SmallButton(aiDebugVisible ? "Hide AI Debug" : "Show AI Debug"))
		aiDebugVisible = !aiDebugVisible;
	ImGui::PopStyleColor(5);
	ImGui::PopStyleVar(2);

	ImGui::End();
}

namespace
{
	const ImVec4 debugRed(0.86f, 0.20f, 0.18f, 1.0f);
	const ImVec4 debugGreen(0.32f, 0.80f, 0.42f, 1.0f);
	const ImVec4 debugMuted(0.60f, 0.61f, 0.65f, 1.0f);

	// "label        (dot) VALUE" with the dot and value coloured by state
	void drawStatusRow(const char* label, bool good, const char* goodText, const char* badText)
	{
		ImGui::TextColored(debugMuted, "%s", label);
		ImGui::SameLine(120.0f);

		const ImVec4 color = good ? debugGreen : debugRed;
		const float lineHeight = ImGui::GetTextLineHeight();
		const ImVec2 cursor = ImGui::GetCursorScreenPos();
		ImGui::GetWindowDrawList()->AddCircleFilled(
			ImVec2(cursor.x + lineHeight * 0.5f, cursor.y + lineHeight * 0.5f),
			lineHeight * 0.3f, ImGui::GetColorU32(color));
		ImGui::Dummy(ImVec2(lineHeight, lineHeight));

		ImGui::SameLine();
		ImGui::TextColored(color, "%s", good ? goodText : badText);
	}
}

void drawAiDebugPanel(const Game& game, bool& enabled)
{
	const ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	// Pinned to the top-right corner, fixed width, height fits the content
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(
		ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 10.0f, viewport->WorkPos.y + 10.0f),
		ImGuiCond_Always, ImVec2(1.0f, 0.0f));
	ImGui::SetNextWindowSizeConstraints(ImVec2(260.0f, 0.0f), ImVec2(260.0f, FLT_MAX));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 10.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 6.0f));
	ImGui::PushStyleColor(ImGuiCol_Border, debugRed);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.10f, 0.88f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.42f, 0.08f, 0.08f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.55f, 0.10f, 0.10f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.86f, 0.20f, 0.18f, 0.35f));

	ImGui::Begin("AI Debug", nullptr, flags);

	// Full-width on/off toggle: green when on, grey when off
	const ImVec4 buttonColor = enabled ? ImVec4(0.18f, 0.52f, 0.27f, 1.0f) : ImVec4(0.28f, 0.28f, 0.31f, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonColor.x + 0.08f, buttonColor.y + 0.08f, buttonColor.z + 0.08f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonColor.x - 0.05f, buttonColor.y - 0.05f, buttonColor.z - 0.05f, 1.0f));
	if (ImGui::Button(enabled ? "ENABLED" : "DISABLED", ImVec2(-FLT_MIN, 0.0f)))
		enabled = !enabled;
	ImGui::PopStyleColor(3);

	ImGui::Separator();

	if (enabled)
	{
		// Only query the AI while enabled, so turning the panel off also skips the line-of-sight raycast
		const AIDebugInfo info = game.getAIDebugInfo();

		drawStatusRow("Line of sight", info.canSeePlayer, "VISIBLE", "HIDDEN");
		drawStatusRow("In range", info.playerInRange, "YES", "NO");

		ImGui::Spacing();
		ImGui::TextColored(debugMuted, "Distance to player");

		const float fraction = info.detectionRange > 0.0f ? std::min(info.distanceToPlayer / info.detectionRange, 1.0f) : 0.0f;
		char overlay[32];
		snprintf(overlay, sizeof(overlay), "%.1f / %.0f m", info.distanceToPlayer, info.detectionRange);

		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, info.playerInRange ? debugGreen : debugRed);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.16f, 0.16f, 0.19f, 1.0f));
		ImGui::ProgressBar(fraction, ImVec2(-FLT_MIN, 0.0f), overlay);
		ImGui::PopStyleColor(2);
	}
	else
	{
		ImGui::TextColored(debugMuted, "Paused - no AI queries");
	}

	ImGui::Spacing();
	ImGui::TextColored(debugMuted, cursorFree ? "F2: back to aiming" : "F2: free cursor to click");

	ImGui::End();

	ImGui::PopStyleColor(5);
	ImGui::PopStyleVar(5);
}

#pragma endregion

#pragma region callbacks

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	auto* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
	if (!game) return;
	game->framesizeCallback(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	auto* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
	if (!game) return;
	if (cursorFree) return; // the cursor is driving the debug ui, not the camera
	if (firstMouse) // initially set to true
	{
		lastX = static_cast<float>(xpos);
		lastY = static_cast<float>(ypos);
		firstMouse = false;
	}

	float xoffset = static_cast<float>(xpos) - lastX;
	float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates range from bottom to top

	lastX = static_cast<float>(xpos);
	lastY = static_cast<float>(ypos);

	game->processMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	auto* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
	if (!game) return;
	game->processMouseScroll(static_cast<float>(yoffset));
}

#pragma region endregion


#pragma region input

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, InputState& inputState, KeyEdgeState& edges)
{
	inputState.toggleDebugCamera = false;

	bool debugCameraPressed = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;

	if (debugCameraPressed && !edges.debugCameraWasPressed)
	{
		inputState.toggleDebugCamera = true;
	}
	edges.debugCameraWasPressed = debugCameraPressed;

	bool cursorTogglePressed = glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS;
	if (cursorTogglePressed && !edges.cursorToggleWasPressed)
		setCursorFree(window, !cursorFree);
	edges.cursorToggleWasPressed = cursorTogglePressed;

	// Edge-triggered so holding space doesn't bunny-hop on every landing
	bool jumpPressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
	inputState.jump = jumpPressed && !edges.jumpWasPressed;
	edges.jumpWasPressed = jumpPressed;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		inputState.moveForward = true;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		inputState.moveBackward = true;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		inputState.moveLeft = true;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		inputState.moveRight = true;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		inputState.moveUp = true;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		inputState.moveDown = true;
}

#pragma endregion

// Switch between captured cursor (mouse aiming) and a free cursor that can click ImGui windows
void setCursorFree(GLFWwindow* window, bool free)
{
	cursorFree = free;
	glfwSetInputMode(window, GLFW_CURSOR, free ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

	ImGuiIO& io = ImGui::GetIO();
	if (free)
		io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
	else
		io.ConfigFlags |= ImGuiConfigFlags_NoMouse;

	// The cursor position jumps when the mode changes; re-anchor so the camera doesn't snap
	firstMouse = true;
}
