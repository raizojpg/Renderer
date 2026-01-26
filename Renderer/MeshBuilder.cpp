#include "MeshBuilder.h"
#include <glm/gtc/constants.hpp> 
#include <glm/gtx/norm.hpp>

// ------------------------------------------------------------
// Create a stable perpendicular basis around an axis N.
// N must be normalized.
// Output:
//  T = tangent
//  B = bitangent
// such that (T, B, N) form an orthonormal basis.
// ------------------------------------------------------------
void TreeMeshBuilder::MakePerpBasis(const glm::vec3& N, glm::vec3& T, glm::vec3& B)
{
    // Pick a vector that is not parallel to N, so cross product is not near zero.
    glm::vec3 helper = (fabs(N.y) < 0.99f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);

    // T is perpendicular to N
    T = glm::normalize(glm::cross(helper, N));

    // B is perpendicular to both N and T
    B = glm::normalize(glm::cross(N, T));
}

// ------------------------------------------------------------
// Build a tube mesh from skeleton segments.
// Each segment gets two rings of vertices: one at p0, one at p1.
// Then we connect ring i to ring i+1 with triangles.
// ------------------------------------------------------------
TreeMeshData TreeMeshBuilder::BuildFromSkeleton(const TreeSkeleton& skel, int radialSides, float uvVScale)
{
    TreeMeshData mesh;
    radialSides = std::max(radialSides, 3);
    // We want UV "v" to increase along the tree length.
    // We'll accumulate a "vOffset" as we add segments.
    float vOffset = 0.0f;

    for (size_t s = 0; s < skel.segments.size(); s++)
    {
        const Segment& seg = skel.segments[s];
        glm::vec3 p0 = seg.p0;
        glm::vec3 p1 = seg.p1;
        glm::vec3 axis = p1 - p0; // direction of the segment

        // If segment is too small, skip it
        if (glm::length2(axis) < 1e-8f)
            continue;

        float segLen = glm::length(axis);
        glm::vec3 N = axis / segLen; // normalized axis direction

        glm::vec3 T, B;
        MakePerpBasis(N, T, B);

        uint32_t baseVertex = (uint32_t)mesh.Positions.size();

        float v0 = vOffset;
        float v1 = vOffset + segLen * uvVScale;

        // Build ring at p0 and ring at p1. Each ring has radialSides vertices.
        for (int i = 0; i < radialSides; i++){

            // Angle around the tube
            float a = (float)i / (float)radialSides * glm::two_pi<float>();

            // Direction around circumference in the plane perpendicular to N
            glm::vec3 circleDir = cos(a) * T + sin(a) * B;

            // Vertex positions on the two rings
            glm::vec3 vtx0 = p0 + circleDir * seg.r0;
            glm::vec3 vtx1 = p1 + circleDir * seg.r1;

            // Normals point outwards
            glm::vec3 normal = glm::normalize(circleDir);

            // U coordinate around tube
            float u = (float)i / (float)radialSides;

            // Push vertex for ring 0 (p0)
            mesh.Positions.push_back(glm::vec4(vtx0, 1.0f));
            mesh.Normals.push_back(normal);
            mesh.UVs.push_back(glm::vec2(u, v0));

            // Push vertex for ring 1 (p1)
            mesh.Positions.push_back(glm::vec4(vtx1, 1.0f));
            mesh.Normals.push_back(normal);
            mesh.UVs.push_back(glm::vec2(u, v1));
        }

        // Build indices between the two rings.
        for (int i = 0; i < radialSides; i++){

            int next = (i + 1) % radialSides;

            uint32_t i0 = baseVertex + (uint32_t)(i * 2);       // ring0 current
            uint32_t i1 = baseVertex + (uint32_t)(i * 2 + 1);   // ring1 current
            uint32_t j0 = baseVertex + (uint32_t)(next * 2);    // ring0 next
            uint32_t j1 = baseVertex + (uint32_t)(next * 2 + 1);// ring1 next

            mesh.Indices.push_back(i0);
            mesh.Indices.push_back(j0);
            mesh.Indices.push_back(j1);

            mesh.Indices.push_back(i0);
            mesh.Indices.push_back(j1);
            mesh.Indices.push_back(i1);
        }

        // Increase vOffset so the next segment continues UVs upward
        vOffset = v1;
    }

    return mesh;
}
