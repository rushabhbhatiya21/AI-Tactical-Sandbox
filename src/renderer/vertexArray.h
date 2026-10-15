#pragma once
#include <glad/glad.h>
#include <vector>

class VertexBuffer;

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	VertexArray(const VertexArray&) = delete;
	VertexArray operator=(const VertexArray&) = delete;

	void bind() const;
	void configure(const VertexBuffer& vertexBuffer);

private:
	GLuint id = 0;
};