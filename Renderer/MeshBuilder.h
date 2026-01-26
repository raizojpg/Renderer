#pragma once
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include "LSystem.h" 

struct TreeMeshData{
    std::vector<glm::vec4> Positions;
    std::vector<glm::vec3> Normals;
    std::vector<glm::vec2> UVs;
    std::vector<uint32_t>  Indices;
};

// Builds a tube mesh from the L-system skeleton.
class TreeMeshBuilder{
public:
    static TreeMeshData BuildFromSkeleton(const TreeSkeleton& skel, int radialSides, float uvVScale);

private:
    // Utility: build an orthonormal basis (T, B) perpendicular to direction N.
    static void MakePerpBasis(const glm::vec3& N, glm::vec3& T, glm::vec3& B);
};
