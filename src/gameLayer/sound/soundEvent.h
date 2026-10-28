#pragma once
#include <glm/glm.hpp>

enum class SoundEventType
{
	Gunshot,
	Footstep,
	Impact
};

struct SoundEvent
{
	glm::vec3 origin = glm::vec3();
	float radius = 0.0f;
	SoundEventType eventType;
};