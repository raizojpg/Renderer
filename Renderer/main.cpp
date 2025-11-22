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
#include "Cube.h"
#include "Camera.h"
#include "Shader.h"

std::stack<glm::mat4> matStack;
std::vector<Model*> models;
Sphere MySphere(20, 10, 100);
Cube MyCube;
Camera MyCamera;
Shader MyShader;
Shader MyInstancingShader;

float const PI = 3.141592f;
float timeElapsed;
float incr_alpha1 = 0.01f, incr_alpha2 = 0.01f;

GLint winWidth = 1000, winHeight = 600;

void ProcessNormalKeys(unsigned char key, int x, int y)
{
	switch (key) {		
	case '-':
		MyCamera.distR() += 5.0;
		break;
	case '+':
		MyCamera.distR() -= 5.0;
		break;
	}
	if (key == 27)
		exit(0);
}

void ProcessSpecialKeys(int key, int xx, int yy)
{
	switch (key)			
	{
	case GLUT_KEY_LEFT:
		MyCamera.betaR() -= 0.01f;
		break;
	case GLUT_KEY_RIGHT:
		MyCamera.betaR() += 0.01f;
		break;
	case GLUT_KEY_UP:
		MyCamera.alphaR() += incr_alpha1;
		if (abs(MyCamera.alphaR() - PI / 2) < 0.05)
		{
			incr_alpha1 = 0.f;
		}
		else
		{
			incr_alpha1 = 0.01f;
		}
		break;
	case GLUT_KEY_DOWN:
		MyCamera.alphaR() -= incr_alpha2;
		if (abs(MyCamera.alphaR() + PI / 2) < 0.05)
		{
			incr_alpha2 = 0.f;
		}
		else
		{
			incr_alpha2 = 0.01f;
		}
		break;
	}
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
	
	MySphere.CreateVAO();
	MyCube.CreateVAO();

	models.push_back(new Sphere(20, 10, 100));
	models.push_back(new Tube(20, 10, 25));
	for (Model* model : models) {
		model->CreateVAO();
	}

	MyShader.Create("Shader.vert", "Shader.frag");
	MyInstancingShader.Create("InstancingShader.vert", "InstancingShader.frag");
}

void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	timeElapsed = glutGet(GLUT_ELAPSED_TIME);

	MyShader.Bind();
	MyCamera.Update();
	MyShader.setUniformMat4("viewMatrix", MyCamera.getView());
	MyShader.setUniformMat4("projectionMatrix", MyCamera.getProjection());

	glm::mat4 identity = glm::mat4(1.0f);
	matStack.push(identity);
	matStack.push(matStack.top());

	glm::mat4 rotateSun = glm::rotate(glm::mat4(1.0f), (float)0.0001 * timeElapsed, glm::vec3(0.0, 0.0, 1.0));
	matStack.top() *= rotateSun;
	matStack.push(matStack.top());
	
	MyShader.setUniformMat4("modelMatrix", rotateSun);
	matStack.pop();
	MyShader.setUniformInt("codCol", 2);
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
	MySphere.Draw();

	/*MyShader.setUniformMat4("modelMatrix", matStack.top());
	for (Model* model : models) {
		model->Draw();
	}*/

	MyInstancingShader.Bind();
	MyInstancingShader.setUniformMat4("viewMatrix", MyCamera.getView());
	MyInstancingShader.setUniformMat4("projectionMatrix", MyCamera.getProjection());
	MyInstancingShader.setUniformInt("codCol", 0);
	MyCube.Draw();
	//MyInstancingShader.setUniformInt("codCol", 1);
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
	glutCloseFunc(Cleanup);

	glutMainLoop();

	return 0;
}