#include "renderer.h"
#include "mesh.h"
#include "camera.h"
#include "shader.h"
#include "types/transform.h"

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

void Renderer::draw(const Mesh& mesh, const Transform& transform, const Camera& camera, const Shader& shader)
{
	shader.use();

	glm::mat4 model = transform.getModelMatrix();
	shader.setMat4("model", model);
	glm::mat4 view = camera.getViewMatrix();
	shader.setMat4("view", view);
	glm::mat4 projection = camera.getProjectionMatrix();
	shader.setMat4("projection", projection);

	mesh.bind();
	glDrawElements(GL_TRIANGLES, mesh.getIndexCount(), GL_UNSIGNED_SHORT, NULL);
}
