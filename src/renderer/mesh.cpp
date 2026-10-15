#include "mesh.h"

Mesh::Mesh(
    const std::vector<Vertex>& vertices,
    const std::vector<Index>& indices
) : indexCount(static_cast<unsigned int>(indices.size()))
{
    vertexBuffer.upload(vertices);
    indexBuffer.upload(indices);
    vertexArray.bind();
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