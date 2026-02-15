#include "LSystem.h"

TreeSkeleton LSystemTreeGenerator::generateSkeleton(const TreeGenParams& params, uint32_t seed){
    std::string commands = expandGrammar(params);
    return interpretTurtle(commands, params, seed);
}

std::string LSystemTreeGenerator::expandGrammar(const TreeGenParams& params){
    std::string current = params.axiom;
    std::string next;

    for (int iter = 0; iter < params.iterations; ++iter){
        next.clear();
        for (char c : current){

            auto ruleIt = params.rules.find(c);
            if (ruleIt != params.rules.end()){
                next += ruleIt->second;
            }
            else{
                next.push_back(c);
            }
        }
        current.swap(next);
    }
    return current;
}

void LSystemTreeGenerator::skipUntilMatchingBracket(const std::string& commands, int& i){
    // commands[i] should currently be '[' when called.
    // We want to skip until we reach the matching ']'.
    int bracketBalance = 0;
    for (; i < commands.size(); ++i){
        if (commands[i] == '['){
            bracketBalance++;
        }
        else if (commands[i] == ']'){
            bracketBalance--;

            if (bracketBalance == 0)
                return;
        }
    }
}

TreeSkeleton LSystemTreeGenerator::interpretTurtle(const std::string& commands, const TreeGenParams& params, uint32_t seed){
    TreeSkeleton skeleton;

    // Random generator for jitter / stochastic pruning.
    Rng rng(seed);

    TurtleState turtle;
    turtle.curLen = params.initialLength;
    turtle.curRad = params.initialRadius;

    // Stack holds turtle states for branching.
    // When we see '[', push state.
    // When we see ']', pop state.
    std::vector<TurtleState> stack;

    // Helper lambda: get the turtle's forward direction in world space.
    auto forwardDir = [](const TurtleState& t) -> glm::vec3 
        {
            // Rotate local forward vector by the turtle rotation.
            glm::vec3 f = t.rot * glm::vec3(0.0f, 0.0f, 1.0f);
            float len = glm::length(f);
            if (len < 1e-6f)
                return glm::vec3(0.0f, 0.0f, 1.0f);

            return f / len;
        };

    // Helper lambda: apply +/- jitter to a scalar factor.
    auto jitterFactor = [&](float frac) -> float
        {
            return 1.0f + rng.signedRange(frac);
        };

    // Helper lambda: rotate turtle around a local axis by degrees, with jitter.
    auto rotateLocal = [&](TurtleState& t, const glm::vec3& localAxis, float degrees)
        {
            // Add random angle jitter to make the tree organic.
            float jitter = rng.signedRange(params.angleJitterDeg);
            float angleRad = glm::radians(degrees + jitter);

            // Convert local axis into world axis by rotating it with current rotation.
            glm::vec3 worldAxis = t.rot * localAxis;

            // Build quaternion for that rotation.
            glm::quat q = glm::angleAxis(angleRad, glm::normalize(worldAxis));

            // Rotate in world axis relative to current orientation.
            t.rot = glm::normalize(q * t.rot);
        };

    for (int i = 0; i < commands.size(); ++i){
        const char c = commands[i];
        switch (c){
        case 'F':
        {
            if (turtle.depth > params.maxDepth)
                break;

            // Compute this step's length/radius with jitter.
            float segLen = turtle.curLen * jitterFactor(params.lengthJitter);
            float r0 = turtle.curRad * jitterFactor(params.radiusJitter);
            float r1 = (turtle.curRad * params.radiusDecay) * jitterFactor(params.radiusJitter);

            // Compute end point along current forward direction.
            glm::vec3 p0 = turtle.pos;
            glm::vec3 p1 = turtle.pos + forwardDir(turtle) * segLen;

            // If the branch is too thin, we can avoid emitting a segment.
            // But we still move forward to keep the overall "walk" consistent.
            if (r0 >= params.minRadius && r1 >= params.minRadius){
                
                Segment seg{ p0,p1,r0,r1,turtle.depth, turtle.parentSeg };
                int segIndex = (int)skeleton.segments.size();
                skeleton.segments.push_back(seg);
               
                // This segment becomes the "parent" for the next segment in this branch.
                turtle.parentSeg = segIndex;
            }

            // Move turtle forward to p1 no matter what.
            turtle.pos = p1;

            // Apply decay so branches get shorter and thinner as they grow.
            turtle.curLen *= params.lengthDecay;
            turtle.curRad *= params.radiusDecay;
        }
        break;

        // Yaw left/right:
        case '+': rotateLocal(turtle, glm::vec3(0, 1, 0), +params.yawDeg); break;
        case '-': rotateLocal(turtle, glm::vec3(0, 1, 0), -params.yawDeg); break;

        // Pitch down/up:
        case '&': rotateLocal(turtle, glm::vec3(1, 0, 0), +params.pitchDeg); break;
        case '^': rotateLocal(turtle, glm::vec3(1, 0, 0), -params.pitchDeg); break;

        // Roll (optional):
        case '/': rotateLocal(turtle, glm::vec3(0, 0, 1), +params.rollDeg); break;
        case '\\': rotateLocal(turtle, glm::vec3(0, 0, 1), -params.rollDeg); break;

        case '[':
        {
            // Stochastic pruning
            if (params.branchKeepProbability < 1.0f)
            {
                float r = rng.next01();
                if (r > params.branchKeepProbability){
                    skipUntilMatchingBracket(commands, i);
                    break;
                }
            }

            stack.push_back(turtle);
            turtle.depth++;
        }
        break;

        case ']':
        {
            // Restore previous state if stack is not empty.
            // This returns turtle to the branching point.
            if (!stack.empty()){
                turtle = stack.back();
                stack.pop_back();
            }
        }
        break;

        default:
            break;
        }
    }

    return mergeColinearConsecutiveSegments(skeleton);
}

inline bool LSystemTreeGenerator::nearlyEqualVec3(const glm::vec3& a, const glm::vec3& b, float eps)
{
    return glm::length(a - b) <= eps;
}

inline bool LSystemTreeGenerator::colinearDirections(const glm::vec3& d0, const glm::vec3& d1, float dotThreshold)
{
    return std::abs(glm::dot(d0, d1)) >= dotThreshold;
}

TreeSkeleton LSystemTreeGenerator::mergeColinearConsecutiveSegments(const TreeSkeleton& inputSkeleton, float positionEpsilon, float angleEpsilonDeg)
{
    TreeSkeleton mergedSkeleton;
    const float colinearDotThreshold = std::cos(glm::radians(angleEpsilonDeg));
    std::vector<int> mergedIndex(inputSkeleton.segments.size(), -1);

    for (int i = 0; i < inputSkeleton.segments.size(); ++i){
        const Segment& inputSeg = inputSkeleton.segments[i];
        int remappedParentIndex = (inputSeg.parent >= 0) ? mergedIndex[inputSeg.parent] : -1;

        Segment candidateSeg = inputSeg;
        candidateSeg.parent = remappedParentIndex;

        bool merged = false;
        if (!mergedSkeleton.segments.empty())
        {
            int lastMergedIndex = mergedSkeleton.segments.size() - 1;
            Segment& lastMergedSeg = mergedSkeleton.segments.back();

            // Same branch ?
            if (candidateSeg.parent == lastMergedIndex)
            {
                // Connected ?
                if (nearlyEqualVec3(lastMergedSeg.p1, candidateSeg.p0, positionEpsilon))
                {
                    glm::vec3 prevVec = lastMergedSeg.p1 - lastMergedSeg.p0;
                    glm::vec3 currVec = candidateSeg.p1 - candidateSeg.p0;
                    float prevLen = glm::length(prevVec);
                    float currLen = glm::length(currVec);

                    if (prevLen > 1e-8f && currLen > 1e-8f)
                    {
                        glm::vec3 prevDir = prevVec / prevLen;
                        glm::vec3 currDir = currVec / currLen;

                        // Colinear ?
                        if (colinearDirections(prevDir, currDir, colinearDotThreshold))
                        {
                            lastMergedSeg.p1 = candidateSeg.p1;
                            lastMergedSeg.r1 = candidateSeg.r1;
                            merged = true;
                            mergedIndex[i] = lastMergedIndex;
                        }
                    }
                }
            }
        }

        if (!merged)
        {
            mergedSkeleton.segments.push_back(candidateSeg);
            mergedIndex[i] = mergedSkeleton.segments.size() - 1;
        }
    }

    return mergedSkeleton;
}
