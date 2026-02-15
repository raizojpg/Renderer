#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <random>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

struct Segment
{
    glm::vec3 p0;
    glm::vec3 p1;

    float r0 = 0.0f;
    float r1 = 0.0f;

    int depth = 0;
    int parent = -1;
};

struct TreeSkeleton
{
    std::vector<Segment> segments;
};

struct TreeGenParams
{
    // Grammar
    std::string axiom = "X";
    std::unordered_map<char, std::string> rules; 
    int iterations = 5;

    // Turtle movement / branch sizing
    float initialLength = 1.0f; // Starting length for 'F'
    float initialRadius = 0.15f; // Starting radius for trunk
    float lengthDecay = 0.92f; // Multiply length after each 'F'
    float radiusDecay = 0.90f; // Multiply radius after each 'F'

    // Turtle rotations
    float yawDeg = 25.0f;   // '+' / '-' rotation around local Y
    float pitchDeg = 15.0f; // '&' / '^' rotation around local X
    float rollDeg = 0.0f;   // '/' / '\' rotation around local Z 

    // Randomness
    float angleJitterDeg = 7.0f;  // +/- degrees added to every rotation
    float lengthJitter = 0.10f;   // +/- fraction applied to length per segment 
    float radiusJitter = 0.05f;   // +/- fraction applied to radius per segment

    // Stochastic pruning: if < 1.0, some bracketed branches will be skipped entirely.
    float branchKeepProbability = 1.0f;

    // Stop conditions 
    float minRadius = 0.01f; // Do not emit segments thinner than this
    int maxDepth = 32;       // Do not allow too deep recursion 
};

// ------------------------------------------------------------
// LSystemTreeGenerator
// ------------------------------------------------------------
class LSystemTreeGenerator
{
public:
    TreeSkeleton generateSkeleton(const TreeGenParams& params, uint32_t seed);

private:
    std::string expandGrammar(const TreeGenParams& params);
    TreeSkeleton interpretTurtle(const std::string& commands, const TreeGenParams& params, uint32_t seed);
    TreeSkeleton mergeColinearConsecutiveSegments(const TreeSkeleton& inputSkeleton, float positionEpsilon = 1e-4f, float angleEpsilonDeg = 1.0f);
    inline bool nearlyEqualVec3(const glm::vec3& a, const glm::vec3& b, float eps);
    inline bool colinearDirections(const glm::vec3& d0, const glm::vec3& d1, float dotThreshold);
    
private:

    struct Rng
    {
        std::mt19937 gen;
        std::uniform_real_distribution<float> dist01{ 0.0f, 1.0f };

        explicit Rng(uint32_t seed) : gen(seed) {}

        float next01()
        {
            return dist01(gen);
        }

        float signedRange(float mag)
        {
            return (next01() * 2.0f - 1.0f) * mag;
        }
    };

    struct TurtleState
    {
        glm::vec3 pos{ 0.0f, 0.0f, 0.0f };        // current position
        glm::quat rot{ 1.0f, 0.0f, 0.0f, 0.0f };  // current orientation

        float curLen = 1.0f;   // current step length for 'F'
        float curRad = 0.1f;   // current branch radius for 'F'

        int depth = 0;         // current branch depth
        int parentSeg = -1;    // index of the last emitted segment in this branch
    };

    // Utility: skip characters until the matching closing bracket ']' is found.
    // This is used for stochastic pruning when we decide to ignore a whole branch.
    static void skipUntilMatchingBracket(const std::string& commands, int& i);
};

