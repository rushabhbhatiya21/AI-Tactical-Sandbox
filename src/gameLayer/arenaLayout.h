#pragma once
#include <glm/glm.hpp>
#include <vector>

// What a piece of arena geometry is. Rendering picks a material from it,
// and later the AI uses it to tell cover from walls from walkable surfaces.
enum class ArenaPieceKind
{
	Floor,
	Wall,        // tall, blocks movement and line of sight
	LowCover,    // ~1 m, crouch behind / shoot over
	HighCover,   // ~2.2 m, full cover, blocks line of sight
	Platform,    // raised walkable surface
	Ramp,        // walkable (or deliberately too steep) slope
	Stairs,
};

// Every piece is a box: the unit cube scaled to `size`, rotated by `rotationDegrees`
// (same Euler convention as Transform), centred at `center`.
struct ArenaPiece
{
	ArenaPieceKind kind;
	glm::vec3 center;
	glm::vec3 size;
	glm::vec3 rotationDegrees = glm::vec3(0.0f);
};

enum class ArenaMarkerKind
{
	PlayerSpawn,
	EnemySpawn,
	PatrolPoint,
};

// A point of interest with no geometry. Positions are at floor level (feet).
// `group` ties markers together, e.g. the waypoints of one patrol route in order.
struct ArenaMarker
{
	ArenaMarkerKind kind;
	glm::vec3 position;
	int group = 0;
};

struct ArenaLayout
{
	std::vector<ArenaPiece> pieces;
	std::vector<ArenaMarker> markers;

	// Returns the first marker of this kind, or the origin if there is none.
	glm::vec3 findMarker(ArenaMarkerKind kind) const;
};

// 60 x 60 m test arena, floor top at y = 0, split into zones that each exercise one thing:
//   north   - open field with sparse pillars (detection range, long sightlines)
//   centre  - lanes of low / high cover (cover selection, peeking, flanking)
//   west    - 3 m platform reached by 15/30/44/50 degree ramps and stairs (slope limits, high ground)
//   NE      - zig-zag corridors (chokepoints, losing line of sight, searching)
//   SE      - big block with passages either side (loop routes, squad flanking)
//   SW      - debug corner: one of each piece plus 0.25 / 0.4 / 0.5 m step tests
ArenaLayout buildTestArena();
