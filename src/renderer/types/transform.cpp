#include "transform.h"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Transform::getModelMatrix() const
{
	glm::mat4 result = glm::mat4(1.0f);
	result = glm::translate(result, position);
	result = glm::rotate(result, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	result = glm::rotate(result, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	result = glm::rotate(result, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	result = glm::scale(result, scale);

	return result;
}
