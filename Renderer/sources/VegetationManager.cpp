#include "VegetationManager.h"

VegetationManager::VegetationManager()
{
	treesTypeA = CreateTreesTypeA();
    treesTypeB = CreateTreesTypeB();
    treesTypeC = CreateTreesTypeC();
}

VegetationManager::~VegetationManager(){}

std::vector<std::shared_ptr<Tree>> VegetationManager::CreateTreesTypeA()
{
    std::vector<std::shared_ptr<Tree>> trees;
    std::shared_ptr<Tree> tree;
    TreeGenParams p;
    int seed = 1338;

    // X creates branching, F can be expanded to add more segments.
    
    // LOD 0
    p.axiom = "X";
    p.rules['X'] = "F[&+XX][-XX]FX";
    p.rules['F'] = "FF";

    p.iterations = 4;

    p.initialLength = 150.0f;
    p.initialRadius = 60.0f;
    p.lengthDecay = 0.95f;
    p.radiusDecay = 0.94f;

    p.yawDeg = 35.0f;
    p.pitchDeg = 25.0f;
    p.rollDeg = 12.0f;

    p.angleJitterDeg = 6.0f;
    p.branchKeepProbability = 0.9f;
    p.minRadius = 8.0f;

    tree = std::make_shared<Tree>();
    tree->CreateVAO(p, seed);
	trees.push_back(tree);
    trees.push_back(tree);


    // LOD 1
    p.rules['X'] = "F[&+XX][-XX]FX";
    p.iterations = 4;
    p.initialLength = 180.0f;
    p.minRadius = 16.0f;

    tree = std::make_shared<Tree>();
    tree->CreateVAO(p, seed);
    trees.push_back(tree);
    trees.push_back(tree);

    // LOD 2
    p.rules['X'] = "F[&+XX][-XX]FX";
    p.iterations = 3;
    p.initialLength = 220.0f;

    tree = std::make_shared<Tree>();
    tree->CreateVAO(p, seed);
    trees.push_back(tree);
    trees.push_back(tree);


    // LOD 3
    p.rules['X'] = "F[&+X][-X]FX";
    p.iterations = 3;
    p.initialLength = 220.0f;

    tree = std::make_shared<Tree>();
    tree->CreateVAO(p, seed);
    trees.push_back(tree);


    // LOD 4
    p.rules['X'] = "FX";
    p.iterations = 1;
    p.initialLength = 2000.0f;

    tree = std::make_shared<Tree>();
    tree->CreateVAO(p, seed);
    trees.push_back(tree);


    // LOD 5
    tree = std::make_shared<Tree>();
    tree->CreateVAO();
    trees.push_back(tree);

	return trees;
}

std::vector<std::shared_ptr<Tree>> VegetationManager::CreateTreesTypeB()
{
    std::vector<std::shared_ptr<Tree>> trees;
    std::shared_ptr<Tree> tree;
    TreeGenParams p;
    int seed = 1338;

    // LOD 0
    p.axiom = "X";
    p.rules['X'] = "F[&+XFX][-XFX]FX";
    p.rules['F'] = "FXF";

    p.iterations = 4;

    p.initialLength = 220.0f;
    p.initialRadius = 80.0f;
    p.lengthDecay = 0.95f;
    p.radiusDecay = 0.94f;

    p.yawDeg = 35.0f;
    p.pitchDeg = 25.0f;
    p.rollDeg = 12.0f;

    p.angleJitterDeg = 6.0f;
    p.branchKeepProbability = 0.9f;
    p.minRadius = 8.0f;

    tree = std::make_shared<Tree>();
    tree->CreateVAO(p, seed);
    trees.push_back(tree);
    trees.push_back(tree);
    

    // LOD 1
    p.rules['X'] = "F[&+XFX][-XFX]FX";
    p.iterations = 4;
    p.initialLength = 240.0f;
    p.initialRadius = 80.0f;
    p.lengthDecay = 0.95f;
    p.minRadius = 16.0f;

    tree = std::make_shared<Tree>();
    tree->CreateVAO(p, seed);
    trees.push_back(tree);
    trees.push_back(tree);


    // LOD 2
    p.rules['X'] = "F[&+XX][-XX]FX";
    p.iterations = 2;
    p.initialLength = 2000.0f;
    p.initialRadius = 60.0f;
    p.lengthDecay = 0.40f;

    tree = std::make_shared<Tree>();
    tree->CreateVAO(p, seed);
    trees.push_back(tree);


    // LOD 3
    p.rules['X'] = "FX";
    p.iterations = 1;
    p.initialLength = 3000.0f;

    tree = std::make_shared<Tree>();
    tree->CreateVAO(p, seed);
    trees.push_back(tree);


    // LOD 4
    tree = std::make_shared<Tree>();
    tree->CreateVAO();
    trees.push_back(tree);

    return trees;
}

std::vector<std::shared_ptr<Tree>> VegetationManager::CreateTreesTypeC()
{
    std::vector<std::shared_ptr<Tree>> trees;
    std::shared_ptr<Tree> tree;
    TreeGenParams p;
    int seed = 1338;

    // LOD 0
    p.axiom = "X";
    p.rules['X'] = "F[&+XFX][-XFX]X";
    p.rules['F'] = "X";

    p.iterations = 3;

    p.initialLength = 180.0f;
    p.initialRadius = 70.0f;
    p.lengthDecay = 0.95f;
    p.radiusDecay = 0.94f;

    p.yawDeg = 35.0f;
    p.pitchDeg = 25.0f;
    p.rollDeg = 12.0f;

    p.angleJitterDeg = 6.0f;
    p.branchKeepProbability = 0.9f;
    p.minRadius = 16.0f;

    tree = std::make_shared<Tree>();
    tree->CreateVAO(p, seed);
    trees.push_back(tree);
    trees.push_back(tree);


    // LOD 1
    p.rules['X'] = "FF[&+XFX][-XFX]X";
    p.iterations = 2;
    p.initialLength = 250.0f;

    tree = std::make_shared<Tree>();
    tree->CreateVAO(p, seed);
    trees.push_back(tree);
    trees.push_back(tree);


    // LOD 2
    tree = std::make_shared<Tree>();
    tree->CreateVAO();
    trees.push_back(tree);

    return trees;
}
