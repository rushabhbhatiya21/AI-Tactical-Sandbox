#include "mesh.h"

Mesh::Mesh(
    const std::vector<Vertex>& vertices,
    const std::vector<Index>& indices
) : indexCount(static_cast<unsigned int>(indices.size()))
{
    vertexArray.bind();
    vertexBuffer.upload(vertices);
    indexBuffer.upload(indices);
    vertexArray.configure(vertexBuffer);
}

void Mesh::bind() const
{
    vertexArray.bind();
}

unsigned int Mesh::getIndexCount() const
{
    return indexCount;
}