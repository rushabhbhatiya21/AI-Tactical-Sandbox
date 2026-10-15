#include "vertexBuffer.h"
#include "types/vertex.h"

VertexBuffer::VertexBuffer()
{
	glGenBuffers(1, &id);
}

VertexBuffer::~VertexBuffer()
{
	if (id != 0)
	{
		glDeleteBuffers(1, &id);
	}
}

void VertexBuffer::bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, id);
}

void VertexBuffer::upload(const std::vector<Vertex>& vertices)
{
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}
