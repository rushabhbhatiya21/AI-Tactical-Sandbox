#include "cubeMeshBuilder.h"

void buildCube(
    std::vector<Vertex>& vertices,
    std::vector<Index>& indices)
{
    vertices.clear();
    indices.clear();

    // Each face has 4 unique vertices.
    // Each face normal points outward.
    // Vertex order is counterclockwise when viewed from outside.

    // Front: +Z
    // Back: -Z
    // Right: +X
    // Left: -X
    // Top: +Y
    // Bottom: -Y

    struct Face
    {
        glm::vec3 normal;
        glm::vec3 corners[4];
    };

    const float n = -0.5f;
    const float p = 0.5f;

    const Face faces[] = {
        // Front (+Z)
        { { 0, 0, 1 },{ { n,n,p },{ p,n,p },{ p,p,p },{ n,p,p } } },

        // Back (-Z)
        { { 0, 0,-1 },{ { p,n,n },{ n,n,n },{ n,p,n },{ p,p,n } } },

        // Right (+X)
        { { 1, 0, 0 },{ { p,n,p },{ p,n,n },{ p,p,n },{ p,p,p } } },

        // Left (-X)
        { { -1,0, 0 },{ { n,n,n },{ n,n,p },{ n,p,p },{ n,p,n } } },

        // Top (+Y)
        { { 0, 1, 0 },{ { n,p,p },{ p,p,p },{ p,p,n },{ n,p,n } } },

        // Bottom (-Y)
        { { 0,-1, 0 },{ { n,n,n },{ p,n,n },{ p,n,p },{ n,n,p } } }
    };

    const glm::vec2 uvs[] = {
        { 0,0 },{ 1,0 },{ 1,1 },{ 0,1 }
    };

    for (const Face& face : faces)
    {
        Index base = static_cast<Index>(vertices.size());

        for (int i = 0; i < 4; ++i)
        {
            vertices.push_back({
                face.corners[i],
                face.normal,
                uvs[i]
                });
        }

        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);

        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }
}