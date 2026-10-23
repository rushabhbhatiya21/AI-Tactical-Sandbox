#include "arenaLayout.h"

#include <glm/trigonometric.hpp>
#include <cmath>

glm::vec3 ArenaLayout::findMarker(ArenaMarkerKind kind) const
{
	for (const ArenaMarker& marker : markers)
		if (marker.kind == kind)
			return marker.position;

	return glm::vec3(0.0f);
}

namespace
{
	constexpr float arenaHalfSize = 30.0f;
	constexpr float perimeterHeight = 4.0f;
	constexpr float wallHeight = 3.0f;
	constexpr float wallThickness = 0.5f;
	constexpr float lowCoverHeight = 1.0f;
	constexpr float highCoverHeight = 2.2f;
	constexpr float coverThickness = 0.6f;
	constexpr float platformHeight = 3.0f;

	// Small helpers so the layout below reads as a list of placements, not box maths.
	class ArenaBuilder
	{
	public:
		explicit ArenaBuilder(ArenaLayout& layout) : layout(layout) {}

		// A box resting on the floor (y = 0) with its footprint centred at (x, z).
		void block(ArenaPieceKind kind, float x, float z, float sizeX, float height, float sizeZ, float baseY = 0.0f)
		{
			layout.pieces.push_back({ kind, glm::vec3(x, baseY + height * 0.5f, z), glm::vec3(sizeX, height, sizeZ) });
		}

		// A straight wall segment running along X from x0 to x1 at depth z.
		void wallAlongX(ArenaPieceKind kind, float x0, float x1, float z, float height, float thickness)
		{
			block(kind, (x0 + x1) * 0.5f, z, std::abs(x1 - x0), height, thickness);
		}

		// A straight wall segment running along Z from z0 to z1 at x.
		void wallAlongZ(ArenaPieceKind kind, float z0, float z1, float x, float height, float thickness)
		{
			block(kind, x, (z0 + z1) * 0.5f, thickness, height, std::abs(z1 - z0));
		}

		// A ramp whose top surface starts on the floor at `foot` and climbs `rise` metres
		// at `angleDegrees`, heading in `yawDegrees` (0 = +X, 90 = -Z, 180 = -X, 270 = +Z).
		void ramp(glm::vec3 foot, float yawDegrees, float angleDegrees, float rise, float width, float thickness = 0.3f)
		{
			const float yaw = glm::radians(yawDegrees);
			const float angle = glm::radians(angleDegrees);

			// Matches Ry: local +X maps to (cos yaw, 0, -sin yaw)
			const glm::vec3 dir(std::cos(yaw), 0.0f, -std::sin(yaw));
			const glm::vec3 up(0.0f, 1.0f, 0.0f);

			const float run = rise / std::tan(angle);
			const float length = std::sqrt(run * run + rise * rise);

			// Centre of the top surface, then push down by half the thickness along the surface normal
			const glm::vec3 topCentre = foot + dir * (run * 0.5f) + up * (rise * 0.5f);
			const glm::vec3 normal = up * std::cos(angle) - dir * std::sin(angle);
			const glm::vec3 center = topCentre - normal * (thickness * 0.5f);

			// Transform applies Rz (tilt along local X) first, then Ry (heading)
			layout.pieces.push_back({ ArenaPieceKind::Ramp, center, glm::vec3(length, thickness, width),
				glm::vec3(0.0f, yawDegrees, angleDegrees) });
		}

		// Solid stairs from the floor at `foot`, climbing in `yawDegrees` (same convention as ramp).
		void stairs(glm::vec3 foot, float yawDegrees, int steps, float stepRise, float stepRun, float width)
		{
			const float yaw = glm::radians(yawDegrees);
			const glm::vec3 dir(std::cos(yaw), 0.0f, -std::sin(yaw));
			const bool alongX = std::abs(dir.x) > 0.5f;

			for (int i = 0; i < steps; ++i)
			{
				const glm::vec3 stepCentre = foot + dir * (stepRun * (i + 0.5f));
				const float height = stepRise * (i + 1);
				if (alongX)
					block(ArenaPieceKind::Stairs, stepCentre.x, stepCentre.z, stepRun, height, width);
				else
					block(ArenaPieceKind::Stairs, stepCentre.x, stepCentre.z, width, height, stepRun);
			}
		}

		void marker(ArenaMarkerKind kind, float x, float y, float z, int group = 0)
		{
			layout.markers.push_back({ kind, glm::vec3(x, y, z), group });
		}

	private:
		ArenaLayout& layout;
	};
}

ArenaLayout buildTestArena()
{
	ArenaLayout layout;
	ArenaBuilder b(layout);

	using K = ArenaPieceKind;
	const float s = arenaHalfSize;

#pragma region floor & perimeter

	// Floor top at y = 0
	b.block(K::Floor, 0.0f, 0.0f, s * 2.0f, 0.5f, s * 2.0f, -0.5f);

	b.wallAlongX(K::Wall, -s - 1.0f, s + 1.0f, -s - 0.5f, perimeterHeight, 1.0f); // north
	b.wallAlongX(K::Wall, -s - 1.0f, s + 1.0f,  s + 0.5f, perimeterHeight, 1.0f); // south
	b.wallAlongZ(K::Wall, -s, s,  s + 0.5f, perimeterHeight, 1.0f);              // east
	b.wallAlongZ(K::Wall, -s, s, -s - 0.5f, perimeterHeight, 1.0f);              // west

#pragma endregion

#pragma region south - player spawn

	b.marker(ArenaMarkerKind::PlayerSpawn, 0.0f, 0.0f, 24.0f);

	b.block(K::LowCover, -4.0f, 18.0f, 3.0f, lowCoverHeight, coverThickness);
	b.block(K::LowCover,  4.0f, 18.0f, 3.0f, lowCoverHeight, coverThickness);

#pragma endregion

#pragma region centre - cover lanes (x -12..14, z -10..12)

	// Three staggered rows so there's always a next piece of cover to move to
	b.block(K::LowCover,  -9.0f, 8.0f, 3.0f, lowCoverHeight,  coverThickness);
	b.block(K::HighCover, -3.0f, 8.0f, 2.0f, highCoverHeight, coverThickness);
	b.block(K::LowCover,   3.0f, 8.0f, 3.0f, lowCoverHeight,  coverThickness);
	b.block(K::HighCover,  9.0f, 8.0f, 2.0f, highCoverHeight, coverThickness);

	b.block(K::HighCover, -6.0f, 2.0f, 2.0f, highCoverHeight, coverThickness);
	b.block(K::LowCover,   0.0f, 2.0f, 4.0f, lowCoverHeight,  coverThickness);
	b.block(K::HighCover,  6.0f, 2.0f, 2.0f, highCoverHeight, coverThickness);
	b.block(K::LowCover,  12.0f, 2.0f, 3.0f, lowCoverHeight,  coverThickness);

	b.block(K::LowCover,  -9.0f, -4.0f, 3.0f, lowCoverHeight,  coverThickness);
	b.block(K::LowCover,  -3.0f, -4.0f, 3.0f, lowCoverHeight,  coverThickness);
	b.block(K::HighCover,  3.0f, -4.0f, 2.0f, highCoverHeight, coverThickness);
	b.block(K::LowCover,   9.0f, -4.0f, 3.0f, lowCoverHeight,  coverThickness);

	// Side-on cover, useful when flanked
	b.block(K::HighCover, -12.0f, 5.0f, coverThickness, highCoverHeight, 2.5f);
	b.block(K::LowCover,   14.0f, -1.0f, coverThickness, lowCoverHeight, 3.0f);

	// Patrol route around the cover lanes (group 0, in order)
	b.marker(ArenaMarkerKind::PatrolPoint, -13.5f, 0.0f, -9.0f, 0);
	b.marker(ArenaMarkerKind::PatrolPoint,  15.0f, 0.0f, -9.0f, 0);
	b.marker(ArenaMarkerKind::PatrolPoint,  15.0f, 0.0f, 12.0f, 0);
	b.marker(ArenaMarkerKind::PatrolPoint, -13.5f, 0.0f, 12.0f, 0);

#pragma endregion

#pragma region north - open field (x -14..14, z -30..-14)

	b.block(K::HighCover, -8.0f, -22.0f, 1.0f, 2.5f, 1.0f);
	b.block(K::HighCover,  0.0f, -26.0f, 1.0f, 2.5f, 1.0f);
	b.block(K::HighCover,  8.0f, -20.0f, 1.0f, 2.5f, 1.0f);
	b.block(K::LowCover,   3.0f, -16.0f, 2.0f, lowCoverHeight, coverThickness);

	b.marker(ArenaMarkerKind::EnemySpawn, 0.0f, 0.0f, -24.0f);

#pragma endregion

#pragma region west - raised platform (x -30..-20, z -10..10, 3 m high)

	b.block(K::Platform, -25.0f, 0.0f, 10.0f, platformHeight, 20.0f);

	// Ramps up the east face, climbing west (yaw 180). Max walkable slope is 45 degrees.
	const float platformEast = -20.0f;
	auto eastFoot = [&](float angleDegrees, float z)
	{
		return glm::vec3(platformEast + platformHeight / std::tan(glm::radians(angleDegrees)), 0.0f, z);
	};
	b.ramp(eastFoot(30.0f, -6.0f), 180.0f, 30.0f, platformHeight, 3.0f);
	b.ramp(eastFoot(44.0f,  0.0f), 180.0f, 44.0f, platformHeight, 3.0f);   // just walkable
	b.ramp(eastFoot(50.0f,  6.0f), 180.0f, 50.0f, platformHeight, 3.0f);   // too steep

	// Long gentle ramp up the north face, climbing south (yaw 270)
	const float platformNorth = -10.0f;
	const float gentleRun = platformHeight / std::tan(glm::radians(15.0f));
	b.ramp(glm::vec3(-25.0f, 0.0f, platformNorth - gentleRun), 270.0f, 15.0f, platformHeight, 3.0f);

	// Stairs up the south face, climbing north (yaw 90): 12 x 0.25 m steps
	const int stepCount = 12;
	const float stepRun = 0.35f;
	b.stairs(glm::vec3(-25.0f, 0.0f, 10.0f + stepCount * stepRun), 90.0f, stepCount, platformHeight / stepCount, stepRun, 3.0f);

	// Parapets to take cover behind on top
	b.block(K::LowCover, -21.0f, -4.0f, coverThickness, lowCoverHeight, 3.0f, platformHeight);
	b.block(K::LowCover, -21.0f,  3.0f, coverThickness, lowCoverHeight, 3.0f, platformHeight);

	b.marker(ArenaMarkerKind::EnemySpawn, -26.0f, platformHeight, 0.0f);

#pragma endregion

#pragma region north-east - zig-zag corridors (x 16..30, z -30..-4)

	// Outer walls: entrance at the south-west corner (z -8..-4), exit at the north-west corner (z -30..-27)
	b.wallAlongZ(K::Wall, -27.0f, -8.0f, 16.0f, wallHeight, wallThickness);
	b.wallAlongX(K::Wall, 16.0f, s, -4.0f, wallHeight, wallThickness);

	// Interior walls alternate their gap between the east and west ends
	b.wallAlongX(K::Wall, 16.0f, 27.0f,  -9.0f, wallHeight, wallThickness);
	b.wallAlongX(K::Wall, 19.0f,     s, -14.0f, wallHeight, wallThickness);
	b.wallAlongX(K::Wall, 16.0f, 27.0f, -19.0f, wallHeight, wallThickness);
	b.wallAlongX(K::Wall, 19.0f,     s, -24.0f, wallHeight, wallThickness);

	// A crate in one corridor to break the straight line
	b.block(K::LowCover, 22.0f, -16.5f, 1.2f, lowCoverHeight, 1.2f);

	b.marker(ArenaMarkerKind::EnemySpawn, 24.0f, 0.0f, -27.0f);

#pragma endregion

#pragma region south-east - flanking loop (x 16..30, z -4..30)

	// Central block with a passage on each side so squads can split and meet behind it
	b.block(K::Wall, 23.0f, 13.0f, 6.0f, 4.0f, 10.0f);

	// Cover in front of and behind the block, plus side-on cover against the outer edge of each passage,
	// all kept clear of the patrol loop below
	b.block(K::LowCover,  23.0f,  4.0f, 3.0f, lowCoverHeight,  coverThickness);
	b.block(K::HighCover, 23.0f, 21.5f, 3.0f, highCoverHeight, coverThickness);
	b.block(K::LowCover,  16.7f, 13.0f, coverThickness, lowCoverHeight, 2.5f);
	b.block(K::LowCover,  29.3f, 13.0f, coverThickness, lowCoverHeight, 2.5f);

	b.marker(ArenaMarkerKind::EnemySpawn, 23.0f, 0.0f, 25.0f);

	// Patrol loop around the block (group 1)
	b.marker(ArenaMarkerKind::PatrolPoint, 18.0f, 0.0f,  1.0f, 1);
	b.marker(ArenaMarkerKind::PatrolPoint, 28.0f, 0.0f,  1.0f, 1);
	b.marker(ArenaMarkerKind::PatrolPoint, 28.0f, 0.0f, 25.0f, 1);
	b.marker(ArenaMarkerKind::PatrolPoint, 18.0f, 0.0f, 25.0f, 1);

#pragma endregion

#pragma region south-west - debug corner (x -30..-16, z 16..30)

	// One of each cover type
	b.block(K::LowCover,  -27.0f, 18.0f, 2.0f, lowCoverHeight,  coverThickness);
	b.block(K::HighCover, -23.0f, 18.0f, 2.0f, highCoverHeight, coverThickness);

	// Step-height tests: the character steps up 0.4 m, so expect pass / borderline / blocked
	b.block(K::Stairs, -28.0f, 23.0f, 2.0f, 0.25f, 2.0f);
	b.block(K::Stairs, -24.5f, 23.0f, 2.0f, 0.40f, 2.0f);
	b.block(K::Stairs, -21.0f, 23.0f, 2.0f, 0.50f, 2.0f);

	// Gap test: two pillars 0.8 m apart (the player is 0.6 m wide)
	b.block(K::HighCover, -26.4f, 27.5f, 1.0f, 2.5f, 1.0f);
	b.block(K::HighCover, -24.6f, 27.5f, 1.0f, 2.5f, 1.0f);

#pragma endregion

	return layout;
}
