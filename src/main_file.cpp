/*
Zegar mechaniczny
Wiktor Tomczak 160069
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

float speed_x=0;
float speed_y=0;
float aspectRatio=1;

float timeSpeed = 1.0f; //default 1, tempo przebiegu czasu
int clockTimeS = 0; //czas w sekundach 
float timeSdelta = 0; //część ułamkowa sekundy
float rotationPerTick = 2.0f * PI / 60.0f;

ShaderProgram *sp;

Model cylinder1, cylinder2, cylinder3;
Model tooth1, tooth2, tooth3;
Model pendulum;
Model hand1;
Model cube;
Model clockFace;
Model sun;
Model wall;
Model mask;
Model plush;

GLuint texR;
GLuint texMetal1, texMetal2, texMetal3;
GLuint texPen1;
GLuint texWd1;
GLuint texClk;
GLuint texSun;
GLuint texWall;
GLuint texMask;
GLuint texPlush;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -6.0f);
float yaw = 0.0f;   // left/right
float pitch = 0.0f; // up/down
float cameraSpeed = 2.0f;

//Procedura obsługi błędów
void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

float getClockAngle()
{
	float baseAngle = rotationPerTick * clockTimeS;
	float extraAngle = rotationPerTick; //domyślnie maksymalny, w pierwszej części sekundy jest przesuwany
	if (timeSdelta <= 0.5)
	{
		float t = timeSdelta / 0.5f; // przesuwanie przez 0.3 z 1 sekundy
		float easedT = t * t * (3.0f - 2.0f * t); // płynne przesuwanie wskazówki za pomocą funkcji wielomianowej
		extraAngle = easedT * rotationPerTick;
	}
	return baseAngle + extraAngle;
}

void updateCamera(GLFWwindow* window, float deltaTime)
{
	float moveSpeed = 3.0f * cameraSpeed * deltaTime;
	float turnSpeed = 1.0f * cameraSpeed * deltaTime;

	glm::vec3 front = glm::vec3(
		cos(pitch) * sin(yaw), // x
		0, // y :  no flying up/down here for forward/back
		cos(pitch) * cos(yaw) // z
	);
	front = glm::normalize(front);
	glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
	glm::vec3 up = glm::vec3(0, 1, 0);

	// Movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += moveSpeed * front; //move forward
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= moveSpeed * front; //move backward
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= moveSpeed * right; //move left
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += moveSpeed * right; //move right

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		cameraPos += moveSpeed * up;    //move up

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
		cameraPos -= moveSpeed * up;    //move down

	// rotation 
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		yaw += turnSpeed;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		yaw -= turnSpeed;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		pitch += turnSpeed;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		pitch -= turnSpeed;

	// Prevent pitch from going over 90 (flipping backwards when rotating up or down)
	float pitchLimit = glm::radians(89.0f);
	pitch = glm::clamp(pitch, -pitchLimit, pitchLimit);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_LEFT) speed_x = PI / 2;
		if (key == GLFW_KEY_RIGHT) speed_x = -PI / 2;
		if (key == GLFW_KEY_UP) speed_y = -PI / 2;
		if (key == GLFW_KEY_DOWN) speed_y = PI / 2;
		if (key == GLFW_KEY_EQUAL && timeSpeed <= 256) timeSpeed *= 2;
		if (key == GLFW_KEY_MINUS && timeSpeed >= 0.03125) timeSpeed /= 2;
		if (key == GLFW_KEY_BACKSPACE)
		{
			timeSpeed = 1; 
			timeSdelta = 0;
		}
		if (key == GLFW_KEY_N) // ustaw aktualny czas systemowy
		{
			std::time_t now = std::time(nullptr);
			std::tm localTime;
			localtime_s(&localTime, &now);
			clockTimeS = localTime.tm_hour * 3600 + localTime.tm_min * 60 + localTime.tm_sec;
		}
	}

	if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) speed_x = 0;
		if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) speed_y = 0;
	}
}

void windowResizeCallback(GLFWwindow* window,int width,int height)
{
    if (height==0) return;
    aspectRatio=(float)width/(float)height;
    glViewport(0,0,width,height);
}



GLuint readTexture(const char* filename)
{
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);
	//Wczytanie do pamięci komputera
	std::vector<unsigned char> image; //Alokuj wektor do wczytania obrazka
	unsigned width, height; //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);
	//Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return tex;
}

GLuint createColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	unsigned char pixel[] = { r, g, b, a }; // Single RGBA pixel

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, pixel);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return tex;
}


//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window)
{
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0.71f, 0.81f, 0.85f,1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glfwSetWindowSizeCallback(window,windowResizeCallback);
	glfwSetKeyCallback(window,keyCallback);

	texR = readTexture("assets/textures/sky.png");
	texMetal1 = readTexture("assets/textures/metal1.png");
	texMetal2 = readTexture("assets/textures/metal2.png");
	texMetal3 = readTexture("assets/textures/metal3.png");
	texPen1 = readTexture("assets/textures/pendulum.png");
	texWd1 = readTexture("assets/textures/wood1.png");
	texClk = readTexture("assets/textures/clock.png");
	texSun = readTexture("assets/textures/sun.png");
	texWall = readTexture("assets/textures/wall.png");
	texMask = readTexture("assets/textures/mask.png");
	texPlush = readTexture("assets/textures/plush.png");

	cylinder1.loadFromFile("assets/models/cylinder.fbx");
	cylinder1.setTextures(texMetal1, texR);
	cylinder2.loadFromFile("assets/models/cylinder.fbx");
	cylinder2.setTextures(texMetal2, texR);
	cylinder3.loadFromFile("assets/models/cylinder.fbx");
	cylinder3.setTextures(texMetal3, texR);

	tooth1.loadFromFile("assets/models/tooth.glb");
	tooth1.setTextures(texMetal1, texR);
	tooth2.loadFromFile("assets/models/tooth.glb");
	tooth2.setTextures(texMetal2, texR);
	tooth3.loadFromFile("assets/models/tooth.glb");
	tooth3.setTextures(texMetal3, texR);

	pendulum.loadFromFile("assets/models/pendulum.fbx");
	pendulum.setTextures(texPen1, texR);

	hand1.loadFromFile("assets/models/hand1.glb");
	hand1.setTextures(createColor(158, 150, 75), texR);

	cube.loadFromFile("assets/models/cube.fbx");
	cube.setTextures(texWd1, texR);

	clockFace.loadFromFile("assets/models/cylinder.fbx");
	clockFace.setTextures(texClk, texR);

	sun.loadFromFile("assets/models/sun.fbx");
	sun.setTextures(texSun, texSun);
	sun.setShaded(false);

	wall.loadFromFile("assets/models/wall.fbx");
	wall.setTextures(texWall, texR);

	mask.loadFromFile("assets/models/mask.fbx");
	mask.setTextures(texMask, texR);

	plush.loadFromFile("assets/models/plush.fbx");
	plush.setTextures(texPlush, texR);

	sp=new ShaderProgram("shaders/v_simplest.glsl",NULL,"shaders/f_simplest.glsl");
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) 
{
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
    delete sp;
}




//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float angle_x, float angle_y)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//kierunek w którym patrzy kamera
	glm::vec3 front = glm::vec3(
		cos(pitch) * sin(yaw),
		sin(pitch),
		cos(pitch) * cos(yaw)
	);

	glm::vec3 target = cameraPos + glm::normalize(front);
	glm::vec3 up = glm::vec3(0, 1, 0);
	glm::mat4 V = glm::lookAt(cameraPos, target, up);

	glm::mat4 P = glm::perspective(80.0f * PI / 180.0f, aspectRatio, 0.01f, 80.0f);

	//główna macierz pozycyjna + rotacja strzałkami
	glm::mat4 M = glm::mat4(1.0f);
	M = glm::rotate(M, angle_y, glm::vec3(1.0f, 0.0f, 0.0f));
	M = glm::rotate(M, angle_x, glm::vec3(0.0f, 1.0f, 0.0f));

	sp->use();
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	glUniform4f(sp->u("lp1"), cameraPos.x, cameraPos.y - 1, cameraPos.z, 1);
	glUniform4f(sp->u("lp2"), 0.0f, 40.0f, -5.0f, 1);

	//wahadło
	glm::mat4 Mpen = M;
	Mpen = glm::scale(Mpen, glm::vec3(0.0025f, 0.0025f, 0.0025f));
	Mpen = glm::rotate(Mpen, sin(PI * 2 * timeSdelta - 1) / 23, glm::vec3(0.0f, 0.0f, 1.0f));
	Mpen = glm::translate(Mpen, glm::vec3(0.0f, -27.0f, -27.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mpen));
	pendulum.draw(sp);


	// Zębatka sekundowa
	glm::mat4 Mg1 = M;
	Mg1 = glm::scale(Mg1, glm::vec3(0.5f, 0.5f, 0.05f));
	Mg1 = glm::rotate(Mg1, 0.5f * PI, glm::vec3(1.0f, 0.0f, 0.0f));
	Mg1 = glm::rotate(Mg1, getClockAngle(), glm::vec3(0.0f, 1.0f, 0.0f));
	// Rysuj model
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mg1));
	cylinder1.draw(sp);
	// rysowanie 60 zębów
	for (int i = 0; i < 60; i++)
	{
		glm::mat4 Mth = Mg1;
		Mth = glm::rotate(Mth, PI * i / 30.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		Mth = glm::translate(Mth, glm::vec3(0.0f, -0.8f, 1.0f));
		Mth = glm::scale(Mth, glm::vec3(0.01f, 0.16f, 0.015f));
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mth));
		tooth1.draw(sp);
	}
	//druga część 6 zębów 
	glm::mat4 Mg2 = glm::scale(Mg1, glm::vec3(0.08f, 1.0f, 0.08f));
	Mg2 = glm::translate(Mg2, glm::vec3(0.0f, 1.8f, 0.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mg2));
	cylinder1.draw(sp);

	for (int i = 0; i < 6; i++)
	{
		glm::mat4 Mth = Mg2;
		Mth = glm::rotate(Mth, PI * i / 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		Mth = glm::translate(Mth, glm::vec3(0.0f, -0.8f, 1.2f));
		Mth = glm::scale(Mth, glm::vec3(0.08f, 0.16f, 0.1f));
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mth));
		tooth1.draw(sp);
	}
	//"wał napędowy" do wskazówki sekundy
	glm::mat4 Mc1 = glm::scale(Mg2, glm::vec3(1.0f, 3.2f, 1.0f));
	Mc1 = glm::translate(Mc1, glm::vec3(0.0f, -1.3f, 0.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mc1));
	cylinder1.draw(sp);

	//wskazówka sekundowa
	glm::mat4 Mh1 = glm::scale(Mg1, glm::vec3(0.02f, 0.08f, 0.69f));
	Mh1 = glm::translate(Mh1, glm::vec3(0.0f, -67.0f, -1.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mh1));
	cube.draw(sp);
	//końcówka wskazówki
	glm::mat4 Mh2 = glm::scale(Mg1, glm::vec3(0.003f, 0.03f, 0.003f));
	Mh2 = glm::translate(Mh2, glm::vec3(0.0f, -175.0f, -510.0f));
	Mh2 = glm::rotate(Mh2, 0.5f * PI, glm::vec3(1.0f, 0.0f, 0.0f));
	Mh2 = glm::rotate(Mh2, PI, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mh2));
	hand1.draw(sp);


	//zębatka sekunda---minuta
	glm::mat4 Mg3 = M;
	Mg3 = glm::scale(Mg3, glm::vec3(0.5f, 0.5f, 0.05f));
	Mg3 = glm::translate(Mg3, glm::vec3(0.0f, 1.28f, 2.2f));
	Mg3 = glm::rotate(Mg3, 0.5f * PI, glm::vec3(1.0f, 0.0f, 0.0f));
	Mg3 = glm::rotate(Mg3, -getClockAngle()/10 - 0.022f, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mg3));
	cylinder2.draw(sp);
	// 60 zębów
	for (int i = 0; i < 60; i++)
	{
		glm::mat4 Mth = Mg3;
		Mth = glm::rotate(Mth, PI * i / 30.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		Mth = glm::translate(Mth, glm::vec3(0.0f, -0.8f, 1.0f));
		Mth = glm::scale(Mth, glm::vec3(0.01f, 0.16f, 0.015f));
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mth));
		tooth2.draw(sp);
	}
	// 10 zębów
	glm::mat4 Mg4 = glm::scale(Mg3, glm::vec3(0.12f, 1.0f, 0.12f));
	Mg4 = glm::translate(Mg4, glm::vec3(0.0f, 1.8f, 0.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mg4));
	cylinder2.draw(sp);

	for (int i = 0; i < 10; i++)
	{
		glm::mat4 Mth = Mg4;
		Mth = glm::rotate(Mth, PI * i / 5.0f + 0.14f, glm::vec3(0.0f, 1.0f, 0.0f));
		Mth = glm::translate(Mth, glm::vec3(0.0f, -0.8f, 0.8f));
		Mth = glm::scale(Mth, glm::vec3(0.067f, 0.16f, 0.09f));
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mth));
		tooth2.draw(sp);
	}


	// Zębatka minutowa
	glm::mat4 Mg5 = M;
	Mg5 = glm::scale(Mg5, glm::vec3(0.5f, 0.5f, 0.05f));
	Mg5 = glm::translate(Mg5, glm::vec3(0.0f, 0.0f, 4.4f));
	Mg5 = glm::rotate(Mg5, 0.5f * PI, glm::vec3(1.0f, 0.0f, 0.0f));
	Mg5 = glm::rotate(Mg5, getClockAngle()/60, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mg5));
	cylinder3.draw(sp);
	// 60 zębów
	for (int i = 0; i < 60; i++)
	{
		glm::mat4 Mth = Mg5;
		Mth = glm::rotate(Mth, PI * i / 30.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		Mth = glm::translate(Mth, glm::vec3(0.0f, -0.8f, 1.0f));
		Mth = glm::scale(Mth, glm::vec3(0.01f, 0.16f, 0.015f));
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mth));
		tooth3.draw(sp);
	}
	// 20 zębów
	glm::mat4 Mg6 = glm::scale(Mg5, glm::vec3(0.35f, 1.0f, 0.35f));
	Mg6 = glm::translate(Mg6, glm::vec3(0.0f, 1.8f, 0.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mg6));
	cylinder3.draw(sp);
	for (int i = 0; i < 20; i++)
	{
		glm::mat4 Mth = Mg6;
		Mth = glm::rotate(Mth, PI * i / 10.0f - 0.08f, glm::vec3(0.0f, 1.0f, 0.0f));
		Mth = glm::translate(Mth, glm::vec3(0.0f, -0.8f, 1.1f));
		Mth = glm::scale(Mth, glm::vec3(0.0228f, 0.16f, 0.0342f));
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mth));
		tooth3.draw(sp);
	}

	//"wał napędowy" do wskazówki minuty
	glm::mat4 Mc2 = glm::scale(Mg6, glm::vec3(0.2f, 5.6f, 0.2f));
	Mc2 = glm::translate(Mc2, glm::vec3(0.0f, -1.17f, 0.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mc2));
	cylinder3.draw(sp);

	//wskazówka minutowa
	glm::mat4 Mh3 = glm::scale(Mg5, glm::vec3(0.04f, 0.08f, 0.5f));
	Mh3 = glm::translate(Mh3, glm::vec3(0.0f, -126.5f, -1.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mh3));
	cube.draw(sp);
	//końcówka wskazówki min
	glm::mat4 Mh4 = glm::scale(Mg5, glm::vec3(0.005f, 0.05f, 0.005f));
	Mh4 = glm::translate(Mh4, glm::vec3(0.0f, -199.5f, -252.0f));
	Mh4 = glm::rotate(Mh4, 0.5f * PI, glm::vec3(1.0f, 0.0f, 0.0f));
	Mh4 = glm::rotate(Mh4, PI, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mh4));
	hand1.draw(sp);

	
	//zębatka minuta---godzina
	glm::mat4 Mg7 = M;
	Mg7 = glm::scale(Mg7, glm::vec3(0.35f, 0.35f, 0.05f));
	Mg7 = glm::translate(Mg7, glm::vec3(0.0f, 1.8286f, 6.6f));
	Mg7 = glm::rotate(Mg7, 0.5f * PI, glm::vec3(1.0f, 0.0f, 0.0f));
	Mg7 = glm::rotate(Mg7, -getClockAngle() / 120, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mg7));
	cylinder1.draw(sp);
	// 40 zębów
	for (int i = 0; i < 40; i++)
	{
		glm::mat4 Mth = Mg7;
		Mth = glm::rotate(Mth, PI * i / 20.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		Mth = glm::translate(Mth, glm::vec3(0.0f, -0.8f, 1.04f));
		Mth = glm::scale(Mth, glm::vec3(0.0115f, 0.16f, 0.017f));
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mth));
		tooth1.draw(sp);
	}
	// 10 zębów
	glm::mat4 Mg8 = glm::scale(Mg7, glm::vec3(0.1714f, 1.0f, 0.1714f));
	Mg8 = glm::translate(Mg8, glm::vec3(0.0f, 1.8f, 0.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mg8));
	cylinder1.draw(sp);
	for (int i = 0; i < 10; i++)
	{
		glm::mat4 Mth = Mg8;
		Mth = glm::rotate(Mth, PI * i / 5.0f + 0.185f, glm::vec3(0.0f, 1.0f, 0.0f));
		Mth = glm::translate(Mth, glm::vec3(0.0f, -0.8f, 0.8f));
		Mth = glm::scale(Mth, glm::vec3(0.067f, 0.16f, 0.09f));
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mth));
		tooth1.draw(sp);
	}

	// zębatka godzinna
	glm::mat4 Mg9 = M;
	Mg9 = glm::scale(Mg9, glm::vec3(0.5f, 0.5f, 0.05f));
	Mg9 = glm::translate(Mg9, glm::vec3(0.0f, 0.0f, 8.8f));
	Mg9 = glm::rotate(Mg9, 0.5f * PI, glm::vec3(1.0f, 0.0f, 0.0f));
	Mg9 = glm::rotate(Mg9, getClockAngle() / 720, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mg9));
	cylinder2.draw(sp);
	// 60 zębów
	for (int i = 0; i < 60; i++)
	{
		glm::mat4 Mth = Mg9;
		Mth = glm::rotate(Mth, PI * i / 30.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		Mth = glm::translate(Mth, glm::vec3(0.0f, -0.8f, 1.0f));
		Mth = glm::scale(Mth, glm::vec3(0.01f, 0.16f, 0.015f));
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mth));
		tooth2.draw(sp);
	}

	// wał napędowy godziny
	glm::mat4 Mc3 = glm::scale(Mg9, glm::vec3(0.06f, 8.0f, 0.06f));
	Mc3 = glm::translate(Mc3, glm::vec3(0.0f, -0.893f, 0.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mc3));
	cylinder2.draw(sp);

	//wskazówka godzinna
	glm::mat4 Mh5 = glm::scale(Mg9, glm::vec3(0.04f, 0.08f, 0.25f));
	Mh5 = glm::translate(Mh5, glm::vec3(0.0f, -188.0f, -1.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mh5));
	cube.draw(sp);
	//końcówka wskazówki godz
	glm::mat4 Mh6 = glm::scale(Mg9, glm::vec3(0.005f, 0.05f, 0.005f));
	Mh6 = glm::translate(Mh6, glm::vec3(0.0f, -298.5f, -152.0f));
	Mh6 = glm::rotate(Mh6, 0.5f * PI, glm::vec3(1.0f, 0.0f, 0.0f));
	Mh6 = glm::rotate(Mh6, PI, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mh6));
	hand1.draw(sp);


	// holder for intermiediate gears
	glm::mat4 Mgx = M;
	Mgx = glm::scale(Mgx, glm::vec3(0.032f, 0.032f, 0.32f));
	Mgx = glm::translate(Mgx, glm::vec3(0.0f, 20.0f, 0.55f));
	Mgx = glm::rotate(Mgx, 0.5f * PI, glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mgx));
	cylinder3.draw(sp);



	//clock face
	glm::mat4 Mf1 = M;
	Mf1 = glm::scale(Mf1, glm::vec3(0.95f, 0.95f, 0.02f));
	Mf1 = glm::rotate(Mf1, 0.5f * PI, glm::vec3(1.0f, 0.0f, 0.0f));
	Mf1 = glm::translate(Mf1, glm::vec3(0.0f, -11.5f, 0.0f));

	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mf1));
	clockFace.draw(sp);
	

	//drugie źródło światła
	glm::mat4 Ms = M;
	Ms = glm::translate(Ms, glm::vec3(0.0f, 40.0f, -5.0f));
	Ms = glm::scale(Ms, glm::vec3(0.1f, 0.1f, 0.1f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Ms));
	sun.draw(sp);


	//ściana
	glm::mat4 Mw = M;
	Mw = glm::rotate(Mw, 0.5f * PI, glm::vec3(0.0f, 1.0f, 0.0f));
	Mw = glm::rotate(Mw, 0.5f * PI, glm::vec3(1.0f, 0.0f, 0.0f));
	Mw = glm::scale(Mw, glm::vec3(0.3f, 0.6f, 0.85f));
	Mw = glm::translate(Mw, glm::vec3(0.65f, 0.0f, -0.2f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mw));
	wall.draw(sp);

	//dekor
	glm::mat4 Mm = M;
	Mm = glm::rotate(Mm, PI, glm::vec3(0.0f, 1.0f, 0.0f));
	Mm = glm::scale(Mm, glm::vec3(0.0005f, 0.0005f, 0.0005f));
	Mm = glm::translate(Mm, glm::vec3(0.0f, 3069.0f, 530.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mm));
	mask.draw(sp);

	glm::mat4 Mf = M;
	Mf = glm::rotate(Mf, 0.5f * PI, glm::vec3(1.0f, 0.0f, 0.0f));
	Mf = glm::rotate(Mf, PI, glm::vec3(0.0f, 1.0f, 0.0f));
	Mf = glm::scale(Mf, glm::vec3(0.2f, 0.2f, 0.2f));
	Mf = glm::translate(Mf, glm::vec3(-5.0f, -1.0f, 12.1f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mf));
	plush.draw(sp);

	glfwSwapBuffers(window);
}


int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit())
	{ //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_SAMPLES, 4); //włącz 4x MSAA

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK)
	{ //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące

	//Główna pętla
	float angle_x = 0;
	float angle_y = 0;
	float lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		// --- Time tracking ---
		float currentTime = glfwGetTime();
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		timeSdelta += deltaTime * timeSpeed;
		if (timeSdelta >= 1.0f)
		{
			clockTimeS++;
			timeSdelta -= 1.0f;
		}

		// --- Update camera movement and rotation smoothly ---
		updateCamera(window, deltaTime);

		// --- Update object rotation (if you still want this for object animation) ---
		angle_x += speed_x * deltaTime;
		angle_y += speed_y * deltaTime;

		// --- Draw everything ---
		drawScene(window, angle_x, angle_y);

		// --- Handle events ---
		glfwPollEvents();
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
