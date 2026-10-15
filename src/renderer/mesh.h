#pragma once

#include "vertexArray.h"
#include "vertexBuffer.h"
#include "indexBuffer.h"

#include <vector>
#include "types/vertex.h"
#include "types/index.h"

class Mesh
{
public:
	Mesh(
		const std::vector<Vertex>& vertices,
		const std::vector<Index>& indices
	);

	// disable copy construction and assignment
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	void bind() const;

	unsigned int getIndexCount() const;

private:
	VertexArray vertexArray;
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;

	unsigned int indexCount = 0;
};