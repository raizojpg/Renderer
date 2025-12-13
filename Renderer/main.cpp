#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <stack>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Sphere.h"
#include "Tube.h"
#include "Terrain.h"
#include "Cube.h"
#include "Camera.h"
#include "ShaderManager.h"
#include "LightManager.h"
#include "MaterialManager.h"
#include "InputManager.h"

#include "PerlinNoise.hpp"

ShaderManager shaders;
MaterialManager materials;
LightManager lights;

std::stack<glm::mat4> matStack;
std::vector<Model*> models;
Sphere MySphere(20, 10, 100);
Cube MyCube;
Terrain MyTerrain(256, 256,	50);

Camera MyCamera;
InputManager inputs(MyCamera);

float timeElapsed;

GLint winWidth = 1000, winHeight = 600;

void ProcessNormalKeys(unsigned char key, int x, int y) {
	inputs.ProcessNormalKeys(key, x, y);
}

void ProcessSpecialKeys(int key, int x, int y) {
	inputs.ProcessSpecialKeys(key, x, y);
}

void MouseButton(int button, int state, int x, int y) {
	inputs.MouseButton(button, state, x, y);
}

void MouseMotion(int x, int y) {
	inputs.MouseMotion(x, y);
}

void ReshapeFunction(GLint newWidth, GLint newHeight)
{
	glViewport(0, 0, newWidth, newHeight);
	winWidth = newWidth;
	winHeight = newHeight;
	MyCamera.widthR() = winWidth / 10, MyCamera.heightR() = winHeight / 10;
}

void Cleanup(void){
	for (Model* model : models) {
		delete model;
	}
}

void Initialize(void)
{
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	
	shaders.Init();

	MySphere.CreateVAO();
	MyCube.CreateVAO();
	MyTerrain.CreateVAO();
	MyTerrain.setMaterial(materials.opaque);

	models.push_back(new Sphere(20, 10, 100));
	models.push_back(new Tube(20, 10, 25));
	for (Model* model : models) {
		model->CreateVAO();
	}

}

void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFrontFace(GL_CCW); 
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	
	timeElapsed = glutGet(GLUT_ELAPSED_TIME);

	Shader& MyShader = shaders.MyShader;

	MyShader.Bind();
	MyShader.updateLight(lights.myLight);

	MyCamera.Update();
	MyShader.setUniformVec3("obsShader", MyCamera.getObs());
	MyShader.setUniformMat4("viewMatrix", MyCamera.getView());
	MyShader.setUniformMat4("projectionMatrix", MyCamera.getProjection());

	MyShader.setUniformMat4("modelMatrix", MyTerrain.getTerrainMat());
	MyShader.setUniformInt("codCol", 0);
	MyShader.updateMaterial(MyTerrain.getMaterial());
	MyTerrain.Draw();

	glm::mat4 identity = glm::mat4(1.0f);
	matStack.push(identity);
	matStack.push(matStack.top());

	glm::mat4 rotateSun = glm::rotate(glm::mat4(1.0f), (float)0.0001 * timeElapsed, glm::vec3(0.0, 0.0, 1.0));
	matStack.top() *= rotateSun;
	matStack.push(matStack.top());
	
	MyShader.setUniformMat4("modelMatrix", rotateSun);
	matStack.pop();
	MyShader.setUniformInt("codCol", 2);
	MySphere.setMaterial(materials.metallic);
	MyShader.updateMaterial(MySphere.getMaterial());
	MySphere.Draw();

	glm::mat4 scalePlanet = glm::scale(glm::mat4(1.0f), glm::vec3(0.4, 0.4, 0.4));
	glm::mat4 rotatePlanetAxis = glm::rotate(glm::mat4(1.0f), (float)0.001 * timeElapsed, glm::vec3(0.0, 0.0, 1.0));
	glm::mat4 rotatePlanet = glm::rotate(glm::mat4(1.0f), (float)0.0005 * timeElapsed, glm::vec3(-0.1, 0.0, 1.0));
	glm::mat4 translatePlanet = glm::translate(glm::mat4(1.0f), glm::vec3(150.0, 0.0, 0.0));

	matStack.push(matStack.top());
	matStack.top() *= rotatePlanet;
	matStack.top() *= translatePlanet;
	matStack.push(matStack.top());
	matStack.top() *= rotatePlanetAxis;
	matStack.top() *= scalePlanet;
	
	MyShader.setUniformMat4("modelMatrix", matStack.top());
	matStack.pop();
	MyShader.setUniformInt("codCol", 3);
	MySphere.setMaterial(materials.matte);
	MyShader.updateMaterial(MySphere.getMaterial());
	MySphere.Draw();

	glm::mat4 scaleSatellite = glm::scale(glm::mat4(1.0f), glm::vec3(0.1, 0.1, 0.1));
	glm::mat4 rotateSatelliteAxis = glm::rotate(glm::mat4(1.0f), (float)0.005 * timeElapsed, glm::vec3(0.0, 0.0, 1.0));
	glm::mat4 rotateSatellite = glm::rotate(glm::mat4(1.0f), (float)0.0015 * timeElapsed, glm::vec3(0.2, 0.0, 1.0));
	glm::mat4 translateSatellite = glm::translate(glm::mat4(1.0f), glm::vec3(50.0, 0.0, 0.0));
	
	matStack.top() *= rotateSatellite;
	matStack.top() *= translateSatellite;
	matStack.top() *= rotateSatelliteAxis;
	matStack.top() *= scaleSatellite;

	MyShader.setUniformMat4("modelMatrix", matStack.top());
	matStack.pop();
	MyShader.setUniformInt("codCol", 4);
	MySphere.setMaterial(materials.opaque);
	MyShader.updateMaterial(MySphere.getMaterial());
	MySphere.Draw();

	///*MyShader.setUniformMat4("modelMatrix", matStack.top());
	//for (Model* model : models) {
	//	model->Draw();
	//}*/

	shaders.MyInstancingShader.Bind();
	shaders.MyInstancingShader.setUniformMat4("viewMatrix", MyCamera.getView());
	shaders.MyInstancingShader.setUniformMat4("projectionMatrix", MyCamera.getProjection());
	shaders.MyInstancingShader.setUniformInt("codCol", 0);
	MyCube.Draw();
	////shaders.MyInstancingShader.setUniformInt("codCol", 1);
	////MyCube.DrawEdges();

	glutSwapBuffers();
	glFlush();
}


int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Render");

	glewInit();

	Initialize();
	glutReshapeFunc(ReshapeFunction);
	glutDisplayFunc(RenderFunction);
	glutIdleFunc(RenderFunction);
	glutKeyboardFunc(ProcessNormalKeys);
	glutSpecialFunc(ProcessSpecialKeys);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutCloseFunc(Cleanup);

	glutMainLoop();

	return 0;
}