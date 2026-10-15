#include "indexBuffer.h"

IndexBuffer::IndexBuffer()
{
	glGenBuffers(1, &id);
}

IndexBuffer::~IndexBuffer()
{
	if (id != 0)
	{
		glDeleteBuffers(1, &id);
	}
}

void IndexBuffer::bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

void IndexBuffer::upload(const std::vector<Index> indices)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Index), indices.data(), GL_STATIC_DRAW);
}
