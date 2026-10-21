#include "gameMain.h"
#include <vector>
#include <memory>

#include <types/vertex.h>
#include <types/index.h>
#include <types/transform.h>
#include <mesh.h>
#include <camera.h>
#include <shader.h>
#include <renderer.h>

std::vector<Vertex> vertices = {
	Vertex{ { -0.5f, -0.5f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	Vertex{ { 0.5f,  -0.5f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	Vertex{ { 0.0f,   0.5f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } }
};

std::vector<Vertex> vertices2 = {
	Vertex{ { -0.5f, -0.5f, 2.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	Vertex{ { 0.5f,  -0.5f, 2.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	Vertex{ { 0.0f,   0.5f, 2.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } }
};
std::vector<Index> indices = { 0, 1, 2 };

std::unique_ptr<Mesh> triangleMesh;
std::unique_ptr<Mesh> triangleMesh2;
Transform defaultTransform;
Camera camera;
std::unique_ptr<Shader> shader;
std::unique_ptr<Renderer> renderer;

void initGame()
{
	triangleMesh2 = std::make_unique<Mesh>(vertices2, indices);
	triangleMesh = std::make_unique<Mesh>(vertices, indices);
	shader = std::make_unique<Shader>(RESOURCES_PATH "shaders/basic.vs", RESOURCES_PATH "shaders/basic.fs");
	renderer = std::make_unique<Renderer>();
}

void updateGame()
{
	renderer->beginFrame();
	renderer->draw(*triangleMesh2, defaultTransform, camera, *shader);
	renderer->draw(*triangleMesh, defaultTransform, camera, *shader);
	renderer->endFrame();
}

void framesizeCallback(int w, int h)
{
	if (h <= 0) return;
	camera.setAspectRatio((float)w / h);
}

void closeGame()
{
}
