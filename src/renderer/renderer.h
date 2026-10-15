#pragma once

class Mesh;

class Renderer
{
public:
	Renderer();
	~Renderer();

private:
	void beginFrame();
	void endFrame();
	void draw(const Mesh& mesh);

private:
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int ibo = 0;
};