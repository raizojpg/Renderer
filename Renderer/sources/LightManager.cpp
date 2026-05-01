#include "LightManager.h"

LightManager::LightManager()
{
	myLight.position = glm::vec4(0.35f, -0.25f, -1.0f, 0.0f);
	myLight.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	myLight.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	myLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
	myLight.attenuation = glm::vec3(0.70f, 0.0f, 0.0f);
}

LightManager::~LightManager(){}
