#include "gameMain.h"
#include "gameMain.h"
#include "input/inputState.h"

#include <mesh.h>
#include <camera.h>
#include <shader.h>
#include <renderer.h>
#include <builders/cubeMeshBuilder.h>

Game::Game() = default;
Game::~Game() = default;

void Game::initGame()
{
	std::vector<Vertex> vertices;
	std::vector<Index> indices;
	buildCube(vertices, indices);
	cubeMesh = std::make_unique<Mesh>(vertices, indices);

	shader = std::make_unique<Shader>(RESOURCES_PATH "shaders/basic_lighting.vs", RESOURCES_PATH "shaders/basic_lighting.fs");
	renderer = std::make_unique<Renderer>();

#pragma region ground & walls

	GameObject& ground = world.createGameObject();
	ground.transform.scale = glm::vec3(20.0f, 0.2f, 20.0f);
	ground.transform.position.y = 0.1f;
	ground.renderComponent = std::make_unique<RenderComponent>(*cubeMesh, groundMaterial);

	GameObject& northWall = world.createGameObject();
	northWall.transform.scale = glm::vec3(20.0f, 3.0f, 1.0f);
	northWall.transform.position = glm::vec3(0.0f, 1.5f, -10.0f);
	northWall.renderComponent = std::make_unique<RenderComponent>(*cubeMesh, wallMaterial);

	GameObject& southWall = world.createGameObject();
	southWall.transform.scale = glm::vec3(20.0f, 3.0f, 1.0f);
	southWall.transform.position = glm::vec3(0.0f, 1.5f, 10.0f);
	southWall.renderComponent = std::make_unique<RenderComponent>(*cubeMesh, wallMaterial);

	GameObject& easthWall = world.createGameObject();
	easthWall.transform.scale = glm::vec3(1.0f, 3.0f, 20.0f);
	easthWall.transform.position = glm::vec3(10.0f, 1.5f, 0.0f);
	easthWall.renderComponent = std::make_unique<RenderComponent>(*cubeMesh, wallMaterial);

	GameObject& westhWall = world.createGameObject();
	westhWall.transform.scale = glm::vec3(1.0f, 3.0f, 20.0f);
	westhWall.transform.position = glm::vec3(-10.0f, 1.5f, 0.0f);
	westhWall.renderComponent = std::make_unique<RenderComponent>(*cubeMesh, wallMaterial);

#pragma endregion

#pragma region obstacles

	GameObject& obstacleA = world.createGameObject();
	obstacleA.transform.position = glm::vec3(-4.0f, 1.2f, -2.0f);
	obstacleA.transform.scale = glm::vec3(2.0f, 2.0f, 1.0f);
	obstacleA.renderComponent = std::make_unique<RenderComponent>(*cubeMesh, obstacle1Material);

	GameObject& obstacleB = world.createGameObject();
	obstacleB.transform.position = glm::vec3(3.0f, 0.8f, -3.0f);
	obstacleB.transform.scale = glm::vec3(3.0f, 1.2f, 1.5f);
	obstacleB.renderComponent = std::make_unique<RenderComponent>(*cubeMesh, obstacle2Material);

#pragma endregion

#pragma region player

	GameObject& player = world.createGameObject();
	player.transform.position = glm::vec3(0.0f, 1.0f, 4.0f);
	player.transform.scale = glm::vec3(0.6f, 1.8f, 0.6f);
	player.renderComponent = std::make_unique<RenderComponent>(*cubeMesh, playerMaterial);

	playerController = std::make_unique<PlayerController>(player);

#pragma endregion

}

void Game::updateGame(float deltaTime, const InputState& inputState)
{
	world.update(deltaTime);

	if (inputState.toggleDebugCamera)
		debugCameraEnabled = !debugCameraEnabled;

	if (!debugCameraEnabled)
		playerController->update(deltaTime, inputState);
	else
	{
		if (inputState.moveForward)
			camera.processKeyboard(FORWARD, deltaTime);
		if (inputState.moveBackward)
			camera.processKeyboard(BACKWARD, deltaTime);
		if (inputState.moveRight)
			camera.processKeyboard(RIGHT, deltaTime);
		if (inputState.moveLeft)
			camera.processKeyboard(LEFT, deltaTime);
		if (inputState.moveUp)
			camera.processKeyboard(UP, deltaTime);
		if (inputState.moveDown)
			camera.processKeyboard(DOWN, deltaTime);
	}

	const std::vector<std::unique_ptr<GameObject>>& gameObjects = world.getGameObjects();
	renderer->beginFrame();
	shader->use();

	shader->setVec3("light.position", glm::vec3(5.0f, 8.0f, 5.0f));
	shader->setVec3("light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	shader->setVec3("light.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
	shader->setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
	shader->setVec3("viewPos", camera.Position);

	for (const auto& gameObject : gameObjects)
	{
		if (!gameObject || !gameObject->renderComponent) continue;

		shader->setVec3("material.diffuse", gameObject->renderComponent->material.diffuse);
		shader->setVec3("material.specular", gameObject->renderComponent->material.specular);
		shader->setFloat("material.shininess", gameObject->renderComponent->material.shininess);
		renderer->draw(gameObject->renderComponent->mesh, gameObject->transform, camera, *shader);
	}

	renderer->endFrame();
}

void Game::framesizeCallback(int w, int h)
{
	if (w <= 0 || h <= 0) return;
	camera.setAspectRatio((float)w / h);
}

void Game::processMouseMovement(float xoffset, float yoffset)
{
	if (debugCameraEnabled)
		camera.processMouseMovement(xoffset, yoffset);
}

void Game::processMouseScroll(float yoffset)
{
	if (debugCameraEnabled)
		camera.processMouseScroll(yoffset);
}

void Game::closeGame()
{
	world.clear();
	cubeMesh.reset();
	shader.reset();
	renderer.reset();
}
