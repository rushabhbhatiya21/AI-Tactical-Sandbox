#include "transform.h"
#include <glm/gtc/matrix_transform.hpp>

Transform::Transform()
{
}

Transform::~Transform()
{
}

glm::mat4 Transform::getModelMatrix() const
{
	glm::mat4 identity = glm::mat4(1, 1, 1, 1);
}
