#include "vertexArray.h"
#include "vertexBuffer.h"
#include "types/vertex.h"
#include <cstddef>

VertexArray::VertexArray()
{
	glGenVertexArrays(1, &id);
}

VertexArray::~VertexArray()
{
	if (id != 0)
	{
		glDeleteVertexArrays(1, &id);
	}
}

void VertexArray::bind() const
{
	glBindVertexArray(id);
}

void VertexArray::configure(const VertexBuffer& vertexBuffer)
{
	glBindVertexArray(id);
	vertexBuffer.bind();

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, uv)));
	glEnableVertexAttribArray(2);
}