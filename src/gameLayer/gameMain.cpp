#include "gameMain.h"
#include "input/inputState.h"

#include <mesh.h>
#include <camera.h>
#include <shader.h>
#include <renderer.h>

Game::Game() = default;
Game::~Game() = default;

void Game::initGame()
{
	triangleMesh = std::make_unique<Mesh>(vertices, indices);
	shader = std::make_unique<Shader>(RESOURCES_PATH "shaders/basic.vs", RESOURCES_PATH "shaders/basic.fs");
	renderer = std::make_unique<Renderer>();

	GameObject& player = world.createGameObject();
	player.transform.position = glm::vec3(-1.0f, 0.0f, 0.0f);

	GameObject& wall = world.createGameObject();
	wall.transform.position = glm::vec3(1.0f, 0.0f, 0.0f);

	player.renderComponent = std::make_unique<RenderComponent>(*triangleMesh, greenMaterial);
	wall.renderComponent = std::make_unique<RenderComponent>(*triangleMesh, redMaterial);
}

void Game::updateGame(float deltaTime, const InputState& inputState)
{
	world.update(deltaTime);
	const std::vector<std::unique_ptr<GameObject>>& gameObjects = world.getGameObjects();

	renderer->beginFrame();
	shader->use();
	for (const auto& gameObject : gameObjects)
	{
		if (!gameObject || !gameObject->renderComponent) continue;

		shader->setVec4("objectColor", gameObject->renderComponent->material.color);
		renderer->draw(gameObject->renderComponent->mesh, gameObject->transform, camera, *shader);
	}

	renderer->endFrame();
}

void Game::framesizeCallback(int w, int h)
{
	if (w <= 0 || h <= 0) return;
	camera.setAspectRatio((float)w / h);
}

void Game::closeGame()
{
	world.clear();
	triangleMesh.reset();
	shader.reset();
	renderer.reset();
}
