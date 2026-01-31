#pragma once
#include <vector>
#include <memory>
#include "models/Tree.h"

class VegetationManager
{
public:

	VegetationManager();
	~VegetationManager();

	std::vector<std::shared_ptr<Tree>> CreateTreesTypeA();
	std::vector<std::shared_ptr<Tree>> CreateTreesTypeB();
	std::vector<std::shared_ptr<Tree>> CreateTreesTypeC();

public:
	std::vector<std::vector<short>> lodMap;

	std::vector<std::shared_ptr<Tree>> treesTypeA;
	std::vector<std::shared_ptr<Tree>> treesTypeB;
	std::vector<std::shared_ptr<Tree>> treesTypeC;

};

