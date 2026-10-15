#pragma once
#include <glad/glad.h>

#include "types/index.h"
#include <vector>

class IndexBuffer
{
public:
	IndexBuffer();
	~IndexBuffer();

	IndexBuffer(const IndexBuffer&) = delete;
	IndexBuffer operator=(const IndexBuffer&) = delete;

	void bind() const;
	void upload(const std::vector<Index> indices);
private:
	GLuint id = 0;
};