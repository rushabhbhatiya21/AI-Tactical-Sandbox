#pragma once
#include <gameObject.h>

namespace JPH
{
	class CharacterVirtual;
}

struct SoundEvent;
class PhysicsWorld;

enum class AIState
{
	Unaware,
	Engage,
	Investigate,
	Search
};

enum class VisualPerceptionState
{
	Visible,
	Remembered,
	Unknown
};

enum class AuditoryPerceptionState
{
	Heard,
	Remembered,
	Unknown
};

// Snapshot of what the agent currently knows, for the debug panel
struct AIDebugInfo
{
	bool playerInRange = false;
	bool playerWithinFov = false;
	bool canSeePlayer = false;
	float distanceToPlayer = 0.0f;
	float detectionRange = 0.0f;
	glm::vec3 lastKnownLocation = glm::vec3();
	float timeSinceLastSeenPlayer = 0.0f;
	bool hasRecentMemory = false;
	VisualPerceptionState visualPerceptionState = VisualPerceptionState::Unknown;
	glm::vec3 lastKnownSoundLocation = glm::vec3();
	bool hasHeardSound = false;
	bool hasRecentSoundMemory = false;
	AuditoryPerceptionState auditoryPerceptionState = AuditoryPerceptionState::Unknown;
	AIState currentState = AIState::Unaware;
};

class AIController
{
public:
	AIController(GameObject& aiAgent, GameObject& player, PhysicsWorld& world, JPH::CharacterVsCharacterCollisionSimple& characterCollision);
	~AIController();

	AIController(const AIController&) = delete;
	AIController& operator=(const AIController&) = delete;

	void update(float dt);
	JPH::CharacterVirtual* getCharacter() const;

	bool canSeePlayer() const;
	AIDebugInfo getDebugInfo() const;
	VisualPerceptionState getVisualPerceptionState() const;
	AuditoryPerceptionState getAuditoryPerceptionState() const;

	void processSoundEvent(const SoundEvent& sound);

private:
	bool isPlayerInDetectionRange() const;
	bool isPlayerWithinFOV() const;
	void updateVisualPerception(float dt);
	bool hasRecentMemory() const;

	bool canHearSound(const SoundEvent& sound) const;
	void updateSoundPerception(float dt);
	bool hasRecentSoundMemory() const;

	void updateDecision();
	void updateMovementDirection(float dt);
	void enterState(AIState newState);

	void updateInvestigateBehavior(float dt);

private:
	GameObject& aiAgent;
	GameObject& player;
	PhysicsWorld& physicsWorld;
	JPH::CharacterVsCharacterCollisionSimple& characterCollision;

	JPH::CharacterVirtual* character = nullptr;

	float halfHeight = 0.0f;
	float movementSpeed = 2.0f;
	glm::vec3 movementDirection = glm::vec3(0.0f, 0.0f, 1.0f);

	// visual perception
	glm::vec3 lastKnownLocation = glm::vec3();
	bool hasAcquiredValidPosition = false;
	float timeSinceLastSeenPlayer = 0.0f;
	float memoryDuration = 5.0f;

	// hearing perception
	glm::vec3 lastKnownSoundLocation = glm::vec3();
	bool hasHeardSound = false;
	bool heardSoundThisFrame = false;
	float timeSinceLastHeardSound = 0.0f;
	float soundMemoryDuration = 5.0f;

	// investigate
	bool hasReachedSoundLocation = false;
	float investigateFacingYaw = 0.0f;
	float investigateScanDirection = 1.0f;
	float investigateScanAngle = 0.0f;
	float investigateAngleChangeTime = 0.0f;

	AIState currentState = AIState::Unaware;
	AIState previousState = AIState::Unaware;

	float searchTimer = 0.0f;
	float investigateTimer = 0.0f;

	static constexpr float SEARCH_TIME = 10.0f;
	static constexpr float INVESTIGATE_TIME = 10.0f;
	static constexpr float DETECTION_RANGE = 20.0f;
	static constexpr float ARRIVAL_DISTANCE = 0.75f;
	static constexpr float INVESTIGATE_SCAN_ANGLE = 45.0f;
	static constexpr float INVESTIGATE_SCAN_SPEED = 45.0f;
	static constexpr float INVESTIGATE_ANGLE_CHANGE_WAIT_TIME = 2.0f;
};