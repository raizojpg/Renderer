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
#include "Sphere.h"
#include "Tube.h"
#include "Terrain.h"
#include "Cube.h"

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
