#pragma once
#include "world.h"
#include "camera.h"
#include "material.h"

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
	void closeGame();

private:
	World world;
	Camera camera;
	std::unique_ptr<Mesh> triangleMesh;
	std::unique_ptr<Shader> shader;
	std::unique_ptr<Renderer> renderer;

	Material redMaterial{ glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) };
	Material greenMaterial{ glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) };

	std::vector<Vertex> vertices = {
	Vertex{ { -0.5f, -0.5f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	Vertex{ { 0.5f,  -0.5f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	Vertex{ { 0.0f,   0.5f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } }
	};

	std::vector<Index> indices = { 0, 1, 2 };
};