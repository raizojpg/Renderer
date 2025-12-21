#pragma once
#include <vector>
#include <stack>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "ShaderManager.h"
#include "MaterialManager.h"

#include "Model.h"
#include "Terrain.h"
#include "models/Sphere.h"
#include "models/Tube.h"
#include "models/Cube.h"

class ModelManager {
public:
    ModelManager();
    void Initialize();
    void Update(Shader& MyShader);
    ~ModelManager();

public:
    MaterialManager materials;
    float timeElapsed;

    std::vector<Model*> models;
    std::stack<glm::mat4> matStack;

    Sphere* MySphere;
    Cube* MyCube;
    Terrain* MyTerrain;
};
