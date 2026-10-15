#pragma once
#include <glad/glad.h>
#include <vector>

struct Vertex;

class VertexBuffer
{
public:
	VertexBuffer();
	~VertexBuffer();

	VertexBuffer(const VertexBuffer&) = delete;
	VertexBuffer& operator=(const VertexBuffer&) = delete;

	void bind() const;
	void upload(const std::vector<Vertex>& vertices);

private:
	GLuint id = 0;
};