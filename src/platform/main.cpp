#include <iostream>
#include <algorithm>
#include <assert.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "gameMain.h"
#include "input/inputState.h"

struct KeyEdgeState
{
	bool debugCameraWasPressed = false;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, InputState& inputState, KeyEdgeState& edges);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// mouse
float lastX = (float)SCR_WIDTH / 2.f;
float lastY = (float)SCR_HEIGHT / 2.f;
bool firstMouse = true;

// time
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

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

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

#pragma endregion


#pragma region shutdown

	game.closeGame();
	glfwTerminate();
	std::cout << "[INFO] Application shutting down" << std::endl;

#pragma endregion

	return 0;
}

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