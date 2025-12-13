#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ModelManager.h"
#include "ShaderManager.h"
#include "LightManager.h"
#include "InputManager.h"

#include "PerlinNoise.hpp"

ModelManager models;
ShaderManager shaders;
LightManager lights;

Camera MyCamera;
InputManager inputs(MyCamera);

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
}

void Initialize(void)
{
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	
	shaders.Init();
	models.Initialize();
}

void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFrontFace(GL_CCW); 
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	Shader& MyShader = shaders.MyShader;

	MyShader.Bind();
	MyShader.updateLight(lights.myLight);

	MyCamera.Update();
	MyShader.setUniformVec3("obsShader", MyCamera.getObs());
	MyShader.setUniformMat4("viewMatrix", MyCamera.getView());
	MyShader.setUniformMat4("projectionMatrix", MyCamera.getProjection());

	MyShader.setUniformMat4("modelMatrix", models.MyTerrain->getTerrainMat());
	MyShader.setUniformInt("codCol", 0);
	MyShader.updateMaterial(models.MyTerrain->getMaterial());
	models.MyTerrain->Draw();

	models.Update(MyShader);

	///*MyShader.setUniformMat4("modelMatrix", matStack.top());
	//for (Model* model : models) {
	//	model->Draw();
	//}*/

	shaders.MyInstancingShader.Bind();
	shaders.MyInstancingShader.setUniformMat4("viewMatrix", MyCamera.getView());
	shaders.MyInstancingShader.setUniformMat4("projectionMatrix", MyCamera.getProjection());
	shaders.MyInstancingShader.setUniformInt("codCol", 0);
	models.MyCube->Draw();
	//shaders.MyInstancingShader.setUniformInt("codCol", 1);
	//MyCube.DrawEdges();

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