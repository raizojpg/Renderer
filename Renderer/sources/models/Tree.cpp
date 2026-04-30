#include "Tree.h"
#include "../Profiler.h"

#define INSTANCE_COUNT 64

Tree::Tree(){
    IndexCount = 0;
}

void Tree::CreateVAO(){}

void Tree::CreateVAO(TreeGenParams& p, int seed)
{
    // ------------------------------------------------------------
    // Generate a TREE SKELETON using the L-system
    // ------------------------------------------------------------

    LSystemTreeGenerator gen;
    TreeSkeleton skel = gen.generateSkeleton(p, seed);

    // ------------------------------------------------------------
    // Build a TUBE MESH from that skeleton
    // ------------------------------------------------------------
    
    int radialSides = 3;
    float uvVScale = 0.02f;

    TreeMeshData mesh = TreeMeshBuilder::BuildFromSkeleton(skel, radialSides, uvVScale);
    IndexCount = (int)mesh.Indices.size();

    // ------------------------------------------------------------
    // Create instance colors
    // ------------------------------------------------------------
    glm::vec3 Colors[INSTANCE_COUNT];
    for (int instID = 0; instID < INSTANCE_COUNT; instID++)
    {
        Colors[instID][0] = 0.7f + 0.1f * sinf(instID % 10 * 1.2f);
        Colors[instID][1] = 0.7f + 0.1f * sinf(instID % 10 * 1.5f + 1.0f);
        Colors[instID][2] = 0.7f + 0.1f * sinf(instID % 10 * 1.8f + 2.0f);
    }

    // ------------------------------------------------------------
    // Create instance model matrices
    // ------------------------------------------------------------

    std::mt19937 rng(seed);
    float tx, ty, tz, rx, ry, rz;
    std::uniform_real_distribution<float> transDist(-0.5f * vPatchSize * vTerrainStep, 0.5f * vPatchSize * vTerrainStep);
    std::uniform_real_distribution<float> zDist(-150.0f, -50.0f);
    std::uniform_real_distribution<float> rotDist(-glm::pi<float>() / 16, glm::pi<float>() / 16);

    glm::mat4 MatModel[INSTANCE_COUNT];
    for (int instID = 0; instID < INSTANCE_COUNT; instID++)
    {
        tx = transDist(rng);
        ty = transDist(rng);
        tz = zDist(rng);

        rx = rotDist(rng);
        ry = rotDist(rng);
        rz = rotDist(rng);

        MatModel[instID] =
            glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(1, 0, 0))
            * glm::translate(glm::mat4(1.0f), glm::vec3(tx, ty, tz))
            * glm::rotate(glm::mat4(1.0f), rx, glm::vec3(1, 0, 0))
            * glm::rotate(glm::mat4(1.0f), ry, glm::vec3(0, 1, 0))
            * glm::rotate(glm::mat4(1.0f), rz, glm::vec3(0, 0, 1));
    }

    // ------------------------------------------------------------
    // Create VAO + upload generated mesh buffers to GPU
    // ------------------------------------------------------------
    glGenVertexArrays(1, &VaoId);
    glBindVertexArray(VaoId);

    // Positions VBO (location = 0)
    glGenBuffers(1, &VbPos);
    glBindBuffer(GL_ARRAY_BUFFER, VbPos);
    glBufferData(GL_ARRAY_BUFFER,
        mesh.Positions.size() * sizeof(glm::vec4),
        mesh.Positions.data(),
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)0);

    // Instance Color VBO (location = 1) 
    glGenBuffers(1, &VbCol);
    glBindBuffer(GL_ARRAY_BUFFER, VbCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Colors), Colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
    glVertexAttribDivisor(1, 1);

    // Normals VBO (location = 2)
    glGenBuffers(1, &VbNorm);
    glBindBuffer(GL_ARRAY_BUFFER, VbNorm);
    glBufferData(GL_ARRAY_BUFFER,
        mesh.Normals.size() * sizeof(glm::vec3),
        mesh.Normals.data(),
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);

    // UVs VBO (location = 3)
    glGenBuffers(1, &VbUV);
    glBindBuffer(GL_ARRAY_BUFFER, VbUV);
    glBufferData(GL_ARRAY_BUFFER,
        mesh.UVs.size() * sizeof(glm::vec2),
        mesh.UVs.data(),
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid*)0);

    // Instance Model Matrix VBO (locations = 4,5,6,7)
    glGenBuffers(1, &VbMat);
    glBindBuffer(GL_ARRAY_BUFFER, VbMat);
    glBufferData(GL_ARRAY_BUFFER, sizeof(MatModel), MatModel, GL_STATIC_DRAW);

    for (int i = 0; i < 4; i++){
        glEnableVertexAttribArray(4 + i);
        glVertexAttribPointer(4 + i,
            4, GL_FLOAT, GL_FALSE,
            sizeof(glm::mat4),
            (void*)(sizeof(glm::vec4) * i));
        glVertexAttribDivisor(4 + i, 1);
    }

    // Index buffer EBO
    glGenBuffers(1, &EboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        mesh.Indices.size() * sizeof(uint32_t),
        mesh.Indices.data(),
        GL_STATIC_DRAW);

}

void Tree::Draw(Shader* MyShader)
{
    this->Bind();
    int count = std::min(vTreeInstanceCount, INSTANCE_COUNT);
    ProfilerGL::DrawElementsInstanced(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0, count);
}

Tree::~Tree()
{
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glDeleteBuffers(1, &VbPos);
    glDeleteBuffers(1, &VbCol);
    glDeleteBuffers(1, &VbNorm);
    glDeleteBuffers(1, &VbUV);
    glDeleteBuffers(1, &VbMat);
    glDeleteBuffers(1, &EboId);

    glDeleteVertexArrays(1, &VaoId);
}
