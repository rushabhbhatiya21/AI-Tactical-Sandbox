#include "renderer.h"
#include "mesh.h"

Renderer::Renderer()
{

}

Renderer::~Renderer()
{

}

void Renderer::beginFrame()
{

}

void Renderer::endFrame()
{

}

void Renderer::draw(const Mesh& mesh)
{
	mesh.bind();
	glDrawElements(GL_TRIANGLES, mesh.getIndexCount(), GL_UNSIGNED_SHORT, NULL);
}
