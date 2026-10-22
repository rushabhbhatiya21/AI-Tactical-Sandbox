#pragma once
#include "world.h"
#include "camera.h"
#include "material.h"
#include "playerController.h"

#include "types/vertex.h"
#include "types/index.h"

#include <memory>
#include <vector>

class Mesh;
class Shader;
class Renderer;
class InputState;

class Game
{
public:
	Game();
	~Game();

	void initGame();
	void updateGame(float dt, const InputState& inputState);
	void framesizeCallback(int w, int h);
	void processMouseMovement(float xoffset, float yoffset);
	void processMouseScroll(float yoffset);
	void closeGame();

private:
	World world;
	Camera camera{ glm::vec3(18.0f, 16.0f, 12.0f), glm::vec3(0.0f, 1.0f, 0.0f), -146.3f, -36.5f };

	std::unique_ptr<Mesh> cubeMesh;
	std::unique_ptr<Shader> shader;
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<PlayerController> playerController;

	bool debugCameraEnabled = false;

	// Arena ground - muted olive-gray
	Material groundMaterial{ glm::vec3(0.30f, 0.32f, 0.29f), glm::vec3(0.08f), 8.0f };

	// Arena walls — muted concrete
	Material wallMaterial{ glm::vec3(0.38f, 0.42f, 0.46f), glm::vec3(0.15f), 16.0f };

	// Obstacle 1 — blue-gray metal
	Material obstacle1Material{ glm::vec3(0.22f, 0.35f, 0.48f), glm::vec3(0.45f, 0.45f, 0.45f), 48.0f };

	// Obstacle 2 — weathered orange
	Material obstacle2Material{ glm::vec3(0.65f, 0.30f, 0.12f), glm::vec3(0.20f, 0.20f, 0.20f), 20.0f };

	// Player — muted tactical teal
	Material playerMaterial{ glm::vec3(0.12f, 0.48f, 0.42f), glm::vec3(0.35f, 0.35f, 0.35f), 32.0f };
};