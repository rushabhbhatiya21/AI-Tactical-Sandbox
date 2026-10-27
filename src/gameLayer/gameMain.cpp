#include "gameMain.h"
#include "input/inputState.h"

#include <mesh.h>
#include <camera.h>
#include <shader.h>
#include <renderer.h>
#include <builders/cubeMeshBuilder.h>

#include <physicsWorldTests.h>

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
	physicsWorld.init();

	// Every arena piece is the unit cube scaled to its size, so the collider's half-extents are half of it
	arenaLayout = buildTestArena();
	for (const ArenaPiece& piece : arenaLayout.pieces)
	{
		GameObject& gameObject = world.createGameObject();
		gameObject.transform.position = piece.center;
		gameObject.transform.scale = piece.size;
		gameObject.transform.rotation = piece.rotationDegrees;
		gameObject.renderComponent = std::make_unique<RenderComponent>(*cubeMesh, materialFor(piece.kind));

		physicsWorld.addStaticBox(piece.center, piece.size * 0.5f, piece.rotationDegrees);
	}

#pragma region player

	GameObject& player = world.createGameObject();
	// Spawn markers are at feet level; the transform is the centre of the box
	player.transform.scale = glm::vec3(0.6f, 1.8f, 0.6f);
	player.transform.position = arenaLayout.findMarker(ArenaMarkerKind::PlayerSpawn) + glm::vec3(0.0f, player.transform.scale.y * 0.5f, 0.0f);
	player.renderComponent = std::make_unique<RenderComponent>(*cubeMesh, playerMaterial);

	playerController = std::make_unique<PlayerController>(player, physicsWorld);

#pragma endregion

#pragma region EnemyAIAgent

	GameObject& aIAgent = world.createGameObject();
	aIAgent.transform.scale = glm::vec3(0.6f, 1.8f, 0.6f);
	aIAgent.transform.position = arenaLayout.findMarker(ArenaMarkerKind::EnemySpawn) + glm::vec3(0.0f, aIAgent.transform.scale.y * 0.5f, 0.0f);
	aIAgent.renderComponent = std::make_unique<RenderComponent>(*cubeMesh, enemyMaterial);

	aiController = std::make_unique<AIController>(aIAgent, physicsWorld);

#pragma endregion

	thirdPersonCamera = std::make_unique<ThirdPersonCamera>(player);

	runLineOfSightTests(physicsWorld);
}

void Game::updateGame(float deltaTime, const InputState& inputState)
{
	world.update(deltaTime);
	physicsWorld.update(deltaTime);
	aiController->update(deltaTime);

	if (inputState.toggleDebugCamera)
		debugCameraEnabled = !debugCameraEnabled;

	if (!debugCameraEnabled)
	{
		playerController->update(deltaTime, inputState, thirdPersonCamera->getYaw());
		thirdPersonCamera->update(deltaTime);
	}
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
	const ICamera& activeCamera = debugCameraEnabled ? static_cast<const ICamera&>(camera) : *thirdPersonCamera;
	renderer->beginFrame();
	shader->use();

	// High above the arena so the whole 60 x 60 m area is lit from a similar angle
	shader->setVec3("light.position", glm::vec3(15.0f, 60.0f, 25.0f));
	shader->setVec3("light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	shader->setVec3("light.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
	shader->setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
	shader->setVec3("viewPos", activeCamera.getPosition());

	for (const auto& gameObject : gameObjects)
	{
		if (!gameObject || !gameObject->renderComponent) continue;

		shader->setVec3("material.diffuse", gameObject->renderComponent->material.diffuse);
		shader->setVec3("material.specular", gameObject->renderComponent->material.specular);
		shader->setFloat("material.shininess", gameObject->renderComponent->material.shininess);
		renderer->draw(gameObject->renderComponent->mesh, gameObject->transform, activeCamera, *shader);
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
	else
		thirdPersonCamera->processMouseMovement(xoffset, yoffset);
}

void Game::processMouseScroll(float yoffset)
{
	if (debugCameraEnabled)
		camera.processMouseScroll(yoffset);
}

const Material& Game::materialFor(ArenaPieceKind kind) const
{
	switch (kind)
	{
	case ArenaPieceKind::Floor:     return groundMaterial;
	case ArenaPieceKind::Wall:      return wallMaterial;
	case ArenaPieceKind::LowCover:  return lowCoverMaterial;
	case ArenaPieceKind::HighCover: return highCoverMaterial;
	case ArenaPieceKind::Platform:  return platformMaterial;
	case ArenaPieceKind::Ramp:
	case ArenaPieceKind::Stairs:    return rampMaterial;
	}
	return wallMaterial;
}

void Game::closeGame()
{
	playerController.reset();
	thirdPersonCamera.reset();
	world.clear();
	physicsWorld.shutdown();
	cubeMesh.reset();
	shader.reset();
	renderer.reset();
}
