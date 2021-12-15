﻿//***************************************************************************
// GAME2012_Final_BrazeauHuynh.cpp by Max and Kenny - ID: 101082462
//
// Hedge Maze inside castle walls - Final Project for GAME2012 3D Gaphics
//
// Description:
//	A model of a 3d hedge maze inside a castle
//*****************************************************************************

////http://glew.sourceforge.net/
//The OpenGL Extension Wrangler Library (GLEW) is a cross-platform open-source C/C++ extension loading library. 
//GLEW provides efficient run-time mechanisms for determining which OpenGL extensions are supported on the target
//platform. OpenGL core and extension functionality is exposed in a single header file. GLEW has been tested on a 
//variety of operating systems, including Windows, Linux, Mac OS X, FreeBSD, Irix, and Solaris.
//
//http://freeglut.sourceforge.net/
//The OpenGL Utility Toolkit(GLUT) is a library of utilities for OpenGL programs, which primarily perform system - level I / O with the host operating system.
//Functions performed include window definition, window control, and monitoring of keyboardand mouse input.
//Routines for drawing a number of geometric primitives(both in solid and wireframe mode) are also provided, including cubes, spheresand the Utah teapot.
//GLUT also has some limited support for creating pop - up menus..

//OpenGL functions are in a single library named GL (or OpenGL in Windows). Function names begin with the letters glSomeFunction*();
//Shaders are written in the OpenGL Shading Language(GLSL)
//To interface with the window system and to get input from external devices into our programs, we need another library. For the XWindow System, this library is called GLX, for Windows, it is wgl,
//and for the Macintosh, it is agl. Rather than using a different library for each system,
//we use two readily available libraries, the OpenGL Extension Wrangler(GLEW) and the OpenGLUtilityToolkit(GLUT).
//GLEW removes operating system dependencies. GLUT provides the minimum functionality that should be expected in any modern window system.
//OpenGL makes heavy use of defined constants to increase code readability and avoid the use of magic numbers.Thus, strings such as GL_FILL and GL_POINTS are defined in header(#include <GL/glut.h>)

//https://glm.g-truc.net/0.9.9/index.html
////OpenGL Mathematics (GLM) is a header only C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specifications.
///////////////////////////////////////////////////////////////////////

using namespace std;

#include "stdlib.h"
#include "time.h"
#include <GL/glew.h>
#include <GL/freeglut.h> 
#include "prepShader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <iostream>
#include "Shape.h"
#include "Light.h"

#define BUFFER_OFFSET(x)  ((const void*) (x))
#define FPS 60
#define MOVESPEED 0.1f
#define TURNSPEED 0.05f
#define X_AXIS glm::vec3(1,0,0)
#define Y_AXIS glm::vec3(0,1,0)
#define Z_AXIS glm::vec3(0,0,1)
#define XY_AXIS glm::vec3(1,0.9,0)
#define YZ_AXIS glm::vec3(0,1,1)
#define XZ_AXIS glm::vec3(1,0,1)
#define SPEED 0.25f

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

enum keyMasks {
	KEY_FORWARD = 0b00000001,		// 0x01 or   1	or   01
	KEY_BACKWARD = 0b00000010,		// 0x02 or   2	or   02
	KEY_LEFT = 0b00000100,
	KEY_RIGHT = 0b00001000,
	KEY_UP = 0b00010000,
	KEY_DOWN = 0b00100000,
	KEY_MOUSECLICKED = 0b01000000

	// Any other keys you want to add.
};

static unsigned int
program,
vertexShaderId,
fragmentShaderId;

GLuint modelID, viewID, projID;
glm::mat4 View, Projection;

// Our bitflag variable. 1 byte for up to 8 key states.
unsigned char keys = 0; // Initialized to 0 or 0b00000000.

// Texture variables.
GLuint blankID, hedgeID, wallID, wall2ID, coneID, groundID;
GLint width, height, bitDepth;

// Light objects. Now OOP.
AmbientLight aLight(
	glm::vec3(1.0f, 1.0f, 1.0f),	// Diffuse colour.
	0.7f);							// Diffuse strength.

DirectionalLight dLight(
	glm::vec3(1.0f, 5.0f, 0.0f),	// Origin.
	glm::vec3(1.0f, 0.5f, 0.0f),	// Diffuse colour.
	0.5f);							// Diffuse strength.

PointLight pLights[2] = { 
	{ glm::vec3(16.0f, 1.0f, -15.0f),	// Position.
	10.0f,							// Range.
	1.0f, 4.5f, 75.0f,				// Constant, Linear, Quadratic.   
	glm::vec3(0.0f, 0.0f, 1.0f),	// Diffuse colour.
	5.0f },							// Diffuse strength.

	{ glm::vec3(16.0f, 3.0f, 0.0f),	// Position.
	10.0f,							// Range.
	1.0f, 4.5f, 75.0f,				// Constant, Linear, Quadratic.   
	glm::vec3(0.0f, 1.0f, 0.0f),	// Diffuse colour.
	5.0f } };						// Diffuse strength.

SpotLight sLight(
	glm::vec3(5.0f, 3.0f, -5.0f),	// Position.
	glm::vec3(0.1f, 1.0f, 0.1f),	// Diffuse colour.
	0.0f,							// Diffuse strength.
	glm::vec3(0.0f, -1.0f, 0.0f),   // Direction. Normally opposite because it's used in dot product. See constructor.
	30.0f);							// Edge.

Material mat = { 0.5f, 8 }; // Alternate way to construct an object.

// Camera and transform variables.
float scale = 1.0f, angle = 0.0f;
glm::vec3 position, frontVec, worldUp, upVec, rightVec; // Set by function
GLfloat pitch, yaw;
int lastX, lastY;

// Geometry data.
Grid g_grid(32);
Cube g_cube(3.0f, 3.0f, 3.0f), g_hedge, g_wall(6.0f, 6.0f, 6.0f), g_wall2;
Prism g_prism(12);
Cone g_cone(12);

void timer(int); // Prototype.

void resetView()
{
	position = glm::vec3(10.0f, 5.0f, 25.0f); // Super pulled back because of grid size.
	frontVec = glm::vec3(0.0f, 0.0f, -1.0f);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	pitch = 0.0f;
	yaw = -90.0f;
	// View will now get set only in transformObject
}

void init(void)
{
	srand((unsigned)time(NULL));
	// Create shader program executable.
	vertexShaderId = setShader((char*)"vertex", (char*)"cube.vert");
	fragmentShaderId = setShader((char*)"fragment", (char*)"cube.frag");
	program = glCreateProgram();
	glAttachShader(program, vertexShaderId);
	glAttachShader(program, fragmentShaderId);
	glLinkProgram(program);
	glUseProgram(program);

	modelID = glGetUniformLocation(program, "model");
	viewID = glGetUniformLocation(program, "view");
	projID = glGetUniformLocation(program, "projection");
	
	// Projection matrix : 45∞ Field of View, 1:1 ratio, display range : 0.1 unit <-> 100 units
	Projection = glm::perspective(glm::radians(45.0f), 1.0f / 1.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	// Projection = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	resetView();

	// Image loading.
	stbi_set_flip_vertically_on_load(true);

	// Load first image.
	unsigned char* image = stbi_load("stoneGround.jpg", &width, &height, &bitDepth, 0);
	if (!image) { cout << "Unable to load file!" << endl; }
	glGenTextures(1, &blankID);
	glBindTexture(GL_TEXTURE_2D, blankID);
	// Note: image types with native transparency will need to be GL_RGBA instead of GL_RGB.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(image);
	// End first image.

	// Load first image.
	image = stbi_load("HedgeTexture.jpg", &width, &height, &bitDepth, 0);
	if (!image) { cout << "Unable to load file!" << endl; }
	glGenTextures(1, &hedgeID);
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	// Note: image types with native transparency will need to be GL_RGBA instead of GL_RGB.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(image);
	// End first image.

	// Load third image
	image = stbi_load("CastleBricks.jpg", &width, &height, &bitDepth, 0);
	if (!image) { cout << "Unable to load file!" << endl; }
	glGenTextures(1, &wallID);
	glBindTexture(GL_TEXTURE_2D, wallID);
	// Note: image types with native transparency will need to be GL_RGBA instead of GL_RGB.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(image);
	// End third image.

	// Load fourth image
	image = stbi_load("wall2.jpg", &width, &height, &bitDepth, 0);
	if (!image) { cout << "Unable to load file!" << endl; }
	glGenTextures(1, &wall2ID);
	glBindTexture(GL_TEXTURE_2D, wall2ID);
	// Note: image types with native transparency will need to be GL_RGBA instead of GL_RGB.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(image);
	// End fourth image.

	// Load fifth image
	image = stbi_load("cone.jpg", &width, &height, &bitDepth, 0);
	if (!image) { cout << "Unable to load file!" << endl; }
	glGenTextures(1, &coneID);
	glBindTexture(GL_TEXTURE_2D, coneID);
	// Note: image types with native transparency will need to be GL_RGBA instead of GL_RGB.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(image);
	// End fifth image.

	// Load 6 image
	image = stbi_load("ground.jpg", &width, &height, &bitDepth, 0);
	if (!image) { cout << "Unable to load file!" << endl; }
	glGenTextures(1, &groundID);
	glBindTexture(GL_TEXTURE_2D, groundID);
	// Note: image types with native transparency will need to be GL_RGBA instead of GL_RGB.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(image);
	// End 6 image.

	glUniform1i(glGetUniformLocation(program, "texture0"), 0);

	// Setting material values.
	glUniform1f(glGetUniformLocation(program, "mat.specularStrength"), mat.specularStrength);
	glUniform1f(glGetUniformLocation(program, "mat.shininess"), mat.shininess);

	// Setting ambient light.
	glUniform3f(glGetUniformLocation(program, "aLight.base.diffuseColour"), aLight.diffuseColour.x, aLight.diffuseColour.y, aLight.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "aLight.base.diffuseStrength"), aLight.diffuseStrength);

	// Setting directional light.
	glUniform3f(glGetUniformLocation(program, "dLight.base.diffuseColour"), dLight.diffuseColour.x, dLight.diffuseColour.y, dLight.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "dLight.base.diffuseStrength"), dLight.diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "dLight.origin"), dLight.origin.x, dLight.origin.y, dLight.origin.z);

	// Setting point lights.
	glUniform3f(glGetUniformLocation(program, "pLights[0].base.diffuseColour"), pLights[0].diffuseColour.x, pLights[0].diffuseColour.y, pLights[0].diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "pLights[0].base.diffuseStrength"), pLights[0].diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "pLights[0].position"), pLights[0].position.x, pLights[0].position.y, pLights[0].position.z);
	glUniform1f(glGetUniformLocation(program, "pLights[0].constant"), pLights[0].constant);
	glUniform1f(glGetUniformLocation(program, "pLights[0].linear"), pLights[0].linear);
	glUniform1f(glGetUniformLocation(program, "pLights[0].quadratic"), pLights[0].quadratic);

	glUniform3f(glGetUniformLocation(program, "pLights[1].base.diffuseColour"), pLights[1].diffuseColour.x, pLights[1].diffuseColour.y, pLights[1].diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "pLights[1].base.diffuseStrength"), pLights[1].diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "pLights[1].position"), pLights[1].position.x, pLights[1].position.y, pLights[1].position.z);
	glUniform1f(glGetUniformLocation(program, "pLights[1].constant"), pLights[1].constant);
	glUniform1f(glGetUniformLocation(program, "pLights[1].linear"), pLights[1].linear);
	glUniform1f(glGetUniformLocation(program, "pLights[1].quadratic"), pLights[1].quadratic);

	// Setting spot light.
	glUniform3f(glGetUniformLocation(program, "sLight.base.diffuseColour"), sLight.diffuseColour.x, sLight.diffuseColour.y, sLight.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "sLight.base.diffuseStrength"), sLight.diffuseStrength);

	glUniform3f(glGetUniformLocation(program, "sLight.position"), sLight.position.x, sLight.position.y, sLight.position.z);
	glUniform3f(glGetUniformLocation(program, "sLight.direction"), sLight.direction.x, sLight.direction.y, sLight.direction.z);
	glUniform1f(glGetUniformLocation(program, "sLight.edge"), sLight.edgeRad);

	// All VAO/VBO data now in Shape.h! But we still need to do this AFTER OpenGL is initialized.
	g_grid.BufferShape();
	g_cube.BufferShape();
	g_hedge.BufferShape();
	g_wall.BufferShape();
	g_wall2.BufferShape();
	g_prism.BufferShape();
	g_cone.BufferShape();

	// Enable depth testing and face culling. 
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	timer(0); // Setup my recursive 'fixed' timestep/framerate.

	glClearColor(0.1, 0.3, 0.5, 1.0); // Change Background Color
}

//---------------------------------------------------------------------
//
// calculateView
//
void calculateView()
{
	frontVec.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	frontVec.y = sin(glm::radians(pitch));
	frontVec.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	frontVec = glm::normalize(frontVec);
	rightVec = glm::normalize(glm::cross(frontVec, worldUp));
	upVec = glm::normalize(glm::cross(rightVec, frontVec));

	View = glm::lookAt(
		position, // Camera position
		position + frontVec, // Look target
		upVec); // Up vector
}

//---------------------------------------------------------------------
//
// transformModel
//
void transformObject(glm::vec3 scale, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 translation) {
	glm::mat4 Model;
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, translation);
	Model = glm::rotate(Model, glm::radians(rotationAngle), rotationAxis);
	Model = glm::scale(Model, scale);
	
	// We must now update the View.
	calculateView();

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(projID, 1, GL_FALSE, &Projection[0][0]);
}

//---------------------------------------------------------------------
//
// display
//
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, blankID);
	transformObject(glm::vec3(5.0f, 0.2f, 7.0f), X_AXIS, 0.0f, glm::vec3(14.0f, 0.0f, -19.0f));
	g_cube.DrawShape(GL_TRIANGLES);

	glBindTexture(GL_TEXTURE_2D, groundID);

	// Grid. Note: I rendered it solid!
	transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, -90.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	g_grid.DrawShape(GL_TRIANGLES);

	// Grid. Note: I rendered it solid!
	transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, -90.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	g_grid.DrawShape(GL_LINES);

	// Front Left Wall.
	glBindTexture(GL_TEXTURE_2D, wallID);
	transformObject(glm::vec3(15.0f, 5.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(0.0f, 0.0f, -1.0f));
	g_wall.DrawShape(GL_TRIANGLES);

	// Front Right Wall.
	transformObject(glm::vec3(15.0f, 5.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(17.0f, 0.0f, -1.0f));
	g_wall.DrawShape(GL_TRIANGLES);

	// Left Wall.
	transformObject(glm::vec3(2.0f, 5.0f, -32.0f), X_AXIS, 0.0f, glm::vec3(-1.0f, 0.0f, 0.0f));
	g_wall.DrawShape(GL_TRIANGLES);

	// Right Wall.
	transformObject(glm::vec3(2.0f, 5.0f, -32.0f), X_AXIS, 0.0f, glm::vec3(31.0f, 0.0f, 0.0f));
	g_wall.DrawShape(GL_TRIANGLES);

	// Back Wall.
	transformObject(glm::vec3(32.0f, 5.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(0.0f, 0.0f, -33.0f));
	g_wall.DrawShape(GL_TRIANGLES);

	// List of hedges
	// 1
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(13.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(11.0f, 0.0f, -3.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 2
	transformObject(glm::vec3(1.0f, 2.0f, 5.0f), X_AXIS, 0.0f, glm::vec3(23.0f, 0.0f, -8.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 3
	transformObject(glm::vec3(6.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(25.0f, 0.0f, -3.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 4
	transformObject(glm::vec3(4.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(24.0f, 0.0f, -5.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 5
	transformObject(glm::vec3(1.0f, 2.0f, 5.0f), X_AXIS, 0.0f, glm::vec3(27.0f, 0.0f, -10.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 6
	transformObject(glm::vec3(3.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(24.0f, 0.0f, -10.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 7
	transformObject(glm::vec3(1.0f, 2.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(29.0f, 0.0f, -6.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 8
	transformObject(glm::vec3(1.0f, 2.0f, 7.0f), X_AXIS, 0.0f, glm::vec3(29.0f, 0.0f, -14.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 9
	transformObject(glm::vec3(5.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(24.0f, 0.0f, -12.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 10
	transformObject(glm::vec3(1.0f, 2.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(23.0f, 0.0f, -12.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 11
	transformObject(glm::vec3(1.0f, 2.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(25.0f, 0.0f, -9.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 12
	transformObject(glm::vec3(3.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(21.0f, 0.0f, -14.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 12b
	transformObject(glm::vec3(4.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(25.0f, 0.0f, -14.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 13
	transformObject(glm::vec3(7.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(25.0f, 0.0f, -16.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 14
	transformObject(glm::vec3(1.0f, 2.0f, 16.0f), X_AXIS, 0.0f, glm::vec3(23.0f, 0.0f, -30.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 15
	transformObject(glm::vec3(6.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(24.0f, 0.0f, -18.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 16
	transformObject(glm::vec3(6.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(25.0f, 0.0f, -20.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 17
	transformObject(glm::vec3(6.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(24.0f, 0.0f, -30.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 18
	transformObject(glm::vec3(1.0f, 2.0f, 8.0f), X_AXIS, 0.0f, glm::vec3(29.0f, 0.0f, -29.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 19
	transformObject(glm::vec3(1.0f, 2.0f, 8.0f), X_AXIS, 0.0f, glm::vec3(25.0f, 0.0f, -28.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 20
	transformObject(glm::vec3(1.0f, 2.0f, 7.0f), X_AXIS, 0.0f, glm::vec3(27.0f, 0.0f, -28.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 21
	transformObject(glm::vec3(4.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(17.0f, 0.0f, -30.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 22
	transformObject(glm::vec3(6.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(15.0f, 0.0f, -28.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 23
	transformObject(glm::vec3(1.0f, 2.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(21.0f, 0.0f, -30.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 24
	transformObject(glm::vec3(1.0f, 2.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(15.0f, 0.0f, -31.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 25
	transformObject(glm::vec3(12.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(11.0f, 0.0f, -26.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 26
	transformObject(glm::vec3(1.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(11.0f, 0.0f, -28.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 27
	transformObject(glm::vec3(1.0f, 2.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(13.0f, 0.0f, -30.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 28
	transformObject(glm::vec3(19.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(2.0f, 0.0f, -24.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 29
	transformObject(glm::vec3(1.0f, 2.0f, 9.0f), X_AXIS, 0.0f, glm::vec3(21.0f, 0.0f, -24.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 30
	transformObject(glm::vec3(1.0f, 2.0f, 9.0f), X_AXIS, 0.0f, glm::vec3(21.0f, 0.0f, -13.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 31
	transformObject(glm::vec3(4.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(17.0f, 0.0f, -10.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 32
	transformObject(glm::vec3(1.0f, 2.0f, 5.0f), X_AXIS, 0.0f, glm::vec3(19.0f, 0.0f, -8.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 33
	transformObject(glm::vec3(1.0f, 2.0f, 5.0f), X_AXIS, 0.0f, glm::vec3(17.0f, 0.0f, -9.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 34
	transformObject(glm::vec3(1.0f, 2.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(15.0f, 0.0f, -7.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 35
	transformObject(glm::vec3(2.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(15.0f, 0.0f, -8.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 36
	transformObject(glm::vec3(2.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(13.0f, 0.0f, -5.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 37
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 5.0f), X_AXIS, 0.0f, glm::vec3(11.0f, 0.0f, -8.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 38
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(13.0f, 0.0f, -9.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 39
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(2.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(13.0f, 0.0f, -10.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 40
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(15.0f, 0.0f, -11.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 41 - Center Room Bottom
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(9.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(12.0f, 0.0f, -12.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 42 - Center Room Top
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(7.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(13.0f, 0.0f, -20.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 43a - Center Room Top Left
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(13.0f, 0.0f, -19.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 43b - Center Room Bottom Left
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(13.0f, 0.0f, -15.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 44a - Center Room Top Right
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(19.0f, 0.0f, -19.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 44b - Center Room Bottom Right
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(19.0f, 0.0f, -15.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 45
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(7.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(13.0f, 0.0f, -22.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 46
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 14.0f), X_AXIS, 0.0f, glm::vec3(11.0f, 0.0f, -23.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 47
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 5.0f), X_AXIS, 0.0f, glm::vec3(9.0f, 0.0f, -30.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 48
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(3.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(10.0f, 0.0f, -30.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 49
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 4.0f), X_AXIS, 0.0f, glm::vec3(7.0f, 0.0f, -31.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 50
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 4.0f), X_AXIS, 0.0f, glm::vec3(5.0f, 0.0f, -30.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 51
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 5.0f), X_AXIS, 0.0f, glm::vec3(3.0f, 0.0f, -31.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 52
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(5.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(3.0f, 0.0f, -26.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 53
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 6.0f), X_AXIS, 0.0f, glm::vec3(1.0f, 0.0f, -31.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 54
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(2.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(2.0f, 0.0f, -22.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 55
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(3.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(1.0f, 0.0f, -20.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 56
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(4.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(6.0f, 0.0f, -20.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 57
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(2.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(6.0f, 0.0f, -22.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 58
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(5.0f, 0.0f, -22.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 59
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(9.0f, 0.0f, -23.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 60
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(8.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(2.0f, 0.0f, -18.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 61
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 7.0f), X_AXIS, 0.0f, glm::vec3(9.0f, 0.0f, -17.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 62
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(2.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(9.0f, 0.0f, -10.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 63
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(4.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(7.0f, 0.0f, -8.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 64
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 8.0f), X_AXIS, 0.0f, glm::vec3(7.0f, 0.0f, -16.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 65
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(7.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(3.0f, 0.0f, -4.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 66
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(9.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(1.0f, 0.0f, -2.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 67
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 8.0f), X_AXIS, 0.0f, glm::vec3(3.0f, 0.0f, -12.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 68
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 11.0f), X_AXIS, 0.0f, glm::vec3(1.0f, 0.0f, -13.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 69!!!
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(1.0f, 2.0f, 6.0f), X_AXIS, 0.0f, glm::vec3(5.0f, 0.0f, -12.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 70
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(5.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(5.0f, 0.0f, -6.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 71
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(5.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(1.0f, 0.0f, -14.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// 72
	glBindTexture(GL_TEXTURE_2D, hedgeID);
	transformObject(glm::vec3(5.0f, 2.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(2.0f, 0.0f, -16.0f));
	g_hedge.DrawShape(GL_TRIANGLES);

	// Hedge List Done.

	// wall top thingy - front wall
	glBindTexture(GL_TEXTURE_2D, wall2ID);
	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(13.0f, 5.0f, 0.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(13.0f, 5.0f, -1.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(10.0f, 5.0f, 0.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(10.0f, 5.0f, -1.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(7.0f, 5.0f, 0.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(7.0f, 5.0f, -1.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(4.0f, 5.0f, 0.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(4.0f, 5.0f, -1.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(1.0f, 5.0f, 0.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(1.0f, 5.0f, -1.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(17.0f, 5.0f, 0.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(17.0f, 5.0f, -1.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(20.0f, 5.0f, 0.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(20.0f, 5.0f, -1.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(23.0f, 5.0f, 0.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(23.0f, 5.0f, -1.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(26.0f, 5.0f, 0.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(26.0f, 5.0f, -1.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(29.0f, 5.0f, 0.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(29.0f, 5.0f, -1.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	// wall top thingy - back wall
	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(13.0f, 5.0f, -31.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(13.0f, 5.0f, -33.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(10.0f, 5.0f, -31.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(10.0f, 5.0f, -33.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(7.0f, 5.0f, -31.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(7.0f, 5.0f, -33.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(4.0f, 5.0f, -31.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(4.0f, 5.0f, -33.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(1.0f, 5.0f, -31.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(1.0f, 5.0f, -33.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(17.0f, 5.0f, -31.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(17.0f, 5.0f, -33.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(0.75f, 1.2f, 0.5f), X_AXIS, 0.0f, glm::vec3(15.70f, 5.0f, -31.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(0.75f, 1.2f, 0.5f), X_AXIS, 0.0f, glm::vec3(15.70f, 5.0f, -33.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(20.0f, 5.0f, -31.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(20.0f, 5.0f, -33.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(23.0f, 5.0f, -31.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(23.0f, 5.0f, -33.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(26.0f, 5.0f, -31.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(26.0f, 5.0f, -33.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(29.0f, 5.0f, -31.5f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), X_AXIS, 0.0f, glm::vec3(29.0f, 5.0f, -33.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	// wall top thingy - right wall
	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(31.0f, 5.0f, -29.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(32.5f, 5.0f, -29.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(31.0f, 5.0f, -26.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(32.5f, 5.0f, -26.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(31.0f, 5.0f, -23.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(32.5f, 5.0f, -23.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(31.0f, 5.0f, -20.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(32.5f, 5.0f, -20.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(31.0f, 5.0f, -17.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(32.5f, 5.0f, -17.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(31.0f, 5.0f, -14.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(32.5f, 5.0f, -14.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(31.0f, 5.0f, -11.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(32.5f, 5.0f, -11.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(31.0f, 5.0f, -8.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(32.5f, 5.0f, -8.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(31.0f, 5.0f, -5.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(32.5f, 5.0f, -5.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(31.0f, 5.0f, -2.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(32.5f, 5.0f, -2.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	// wall top thingy - left wall
	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(0.5f, 5.0f, -29.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(-1.0f, 5.0f, -29.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(0.5f, 5.0f, -26.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(-1.0f, 5.0f, -26.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(0.5f, 5.0f, -23.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(-1.0f, 5.0f, -23.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(0.5f, 5.0f, -20.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(-1.0f, 5.0f, -20.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(0.5f, 5.0f, -17.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(-1.0f, 5.0f, -17.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(0.5f, 5.0f, -14.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(-1.0f, 5.0f, -14.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(0.5f, 5.0f, -11.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(-1.0f, 5.0f, -11.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(0.5f, 5.0f, -8.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(-1.0f, 5.0f, -8.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(0.5f, 5.0f, -5.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(-1.0f, 5.0f, -5.0f));
	g_wall2.DrawShape(GL_TRIANGLES);


	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(0.5f, 5.0f, -2.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 0.8f, 0.5f), Y_AXIS, 90.0f, glm::vec3(-1.0f, 5.0f, -2.0f));
	g_wall2.DrawShape(GL_TRIANGLES);

	//towers

	transformObject(glm::vec3(3.0f, 8.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(-1.5f, 0.0f, -1.5f));
	g_prism.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(3.0f, 8.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(-1.5f, 0.0f, -33.5f));
	g_prism.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(3.0f, 8.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(30.5f, 0.0f, -1.5f));
	g_prism.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(3.0f, 8.0f, 3.0f), X_AXIS, 0.0f, glm::vec3(30.5f, 0.0f, -33.5f));
	g_prism.DrawShape(GL_TRIANGLES);

	//tower cones

	glBindTexture(GL_TEXTURE_2D, coneID);
	transformObject(glm::vec3(4.0f, 5.0f, 4.0f), X_AXIS, 0.0f, glm::vec3(-2.0f, 8.0f, -2.0f));
	g_cone.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(4.0f, 5.0f, 4.0f), X_AXIS, 0.0f, glm::vec3(-2.0f, 8.0f, -34.0f));
	g_cone.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(4.0f, 5.0f, 4.0f), X_AXIS, 0.0f, glm::vec3(30.0f, 8.0f, -34.0f));
	g_cone.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(4.0f, 5.0f, 4.0f), X_AXIS, 0.0f, glm::vec3(30.0f, 8.0f, -2.0f));
	g_cone.DrawShape(GL_TRIANGLES);

	//// Plane.
	//transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, -90.0f, glm::vec3(0.0f, -0.1f, 0.0f));
	//g_grid.DrawShape(GL_TRIANGLES);

	//// Cube.
	//transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(6.0f, 0.0f, 0.0f));
	//g_cube.DrawShape(GL_TRIANGLES);

	//// Prism.
	//transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(4.0f, 2.0f, -1.0f));
	//glUniform1f(glGetUniformLocation(program, "mat.specularStrength"), 1.0f);
	//glUniform1f(glGetUniformLocation(program, "mat.shininess"), 128);
	//g_prism.DrawShape(GL_TRIANGLES);
	//
	//// Prism2.
	//transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(4.0f, 0.0f, -1.0f));
	//glUniform1f(glGetUniformLocation(program, "mat.specularStrength"), mat.specularStrength);
	//glUniform1f(glGetUniformLocation(program, "mat.shininess"), mat.shininess);
	//g_prism.DrawShape(GL_TRIANGLES);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glutSwapBuffers(); // Now for a potentially smoother render.
}

void idle() // Not even called.
{
	glutPostRedisplay();
}

void parseKeys()
{
	if (keys & KEY_FORWARD)
		position += frontVec * MOVESPEED;
	if (keys & KEY_BACKWARD)
		position -= frontVec * MOVESPEED;
	if (keys & KEY_LEFT)
		position -= rightVec * MOVESPEED;
	if (keys & KEY_RIGHT)
		position += rightVec * MOVESPEED;
	if (keys & KEY_UP)
		position += upVec * MOVESPEED;
	if (keys & KEY_DOWN)
		position -= upVec * MOVESPEED;
}

void timer(int) { // Tick of the frame.
	// Get first timestamp
	int start = glutGet(GLUT_ELAPSED_TIME);
	// Update call
	parseKeys();
	// Display call
	glutPostRedisplay();
	// Calling next tick
	int end = glutGet(GLUT_ELAPSED_TIME);
	glutTimerFunc((1000 / FPS) - (end-start), timer, 0);
}

// Keyboard input processing routine.
// Keyboard input processing routine.
void keyDown(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case 'w':
		if (!(keys & KEY_FORWARD))
			keys |= KEY_FORWARD; // keys = keys | KEY_FORWARD
		break;
	case 's':
		if (!(keys & KEY_BACKWARD))
			keys |= KEY_BACKWARD;
		break;
	case 'a':
		if (!(keys & KEY_LEFT))
			keys |= KEY_LEFT;
		break;
	case 'd':
		if (!(keys & KEY_RIGHT))
			keys |= KEY_RIGHT;
		break;
	case 'r':
		if (!(keys & KEY_UP))
			keys |= KEY_UP;
		break;
	case 'f':
		if (!(keys & KEY_DOWN))
			keys |= KEY_DOWN;
		break;
	default:
		break;
	}
}

void keyDownSpec(int key, int x, int y) // x and y is mouse location upon key press.
{
	switch (key)
	{
	case GLUT_KEY_UP: // Up arrow.
		if (!(keys & KEY_FORWARD))
			keys |= KEY_FORWARD; // keys = keys | KEY_FORWARD
		break;
	case GLUT_KEY_DOWN: // Down arrow.
		if (!(keys & KEY_BACKWARD))
			keys |= KEY_BACKWARD;
		break;
	default:
		break;
	}
}

void keyUp(unsigned char key, int x, int y) // x and y is mouse location upon key press.
{
	switch (key)
	{
	case 'w':
		keys &= ~KEY_FORWARD; // keys = keys & ~KEY_FORWARD. ~ is bitwise NOT.
		break;
	case 's':
		keys &= ~KEY_BACKWARD;
		break;
	case 'a':
		keys &= ~KEY_LEFT;
		break;
	case 'd':
		keys &= ~KEY_RIGHT;
		break;
	case 'r':
		keys &= ~KEY_UP;
		break;
	case 'f':
		keys &= ~KEY_DOWN;
		break;
	case ' ':
		resetView();
		break;
	default:
		break;
	}
}

void keyUpSpec(int key, int x, int y) // x and y is mouse location upon key press.
{
	switch (key)
	{
	case GLUT_KEY_UP:
		keys &= ~KEY_FORWARD; // keys = keys & ~KEY_FORWARD
		break;
	case GLUT_KEY_DOWN:
		keys &= ~KEY_BACKWARD;
		break;
	default:
		break;
	}
}

void mouseMove(int x, int y)
{
	if (keys & KEY_MOUSECLICKED)
	{
		pitch += (GLfloat)((y - lastY) * TURNSPEED);
		yaw -= (GLfloat)((x - lastX) * TURNSPEED);
		lastY = y;
		lastX = x;
	}
}

void mouseClick(int btn, int state, int x, int y)
{
	if (state == 0)
	{
		lastX = x;
		lastY = y;
		keys |= KEY_MOUSECLICKED; // Flip flag to true
		glutSetCursor(GLUT_CURSOR_NONE);
		//cout << "Mouse clicked." << endl;
	}
	else
	{
		keys &= ~KEY_MOUSECLICKED; // Reset flag to false
		glutSetCursor(GLUT_CURSOR_INHERIT);
		//cout << "Mouse released." << endl;
	}
}

//---------------------------------------------------------------------
//
// clean
//
void clean()
{
	cout << "Cleaning up!" << endl;
	glDeleteTextures(1, &blankID);
	glDeleteTextures(1, &hedgeID);
	glDeleteTextures(1, &wallID);
	glDeleteTextures(1, &wall2ID);
	glDeleteTextures(1, &coneID);
	glDeleteTextures(1, &groundID);
}

//---------------------------------------------------------------------
//
// main
//
int main(int argc, char** argv)
{
	//Before we can open a window, theremust be interaction between the windowing systemand OpenGL.In GLUT, this interaction is initiated by the following function call :
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
	glutSetOption(GLUT_MULTISAMPLE, 8);

	//if you comment out this line, a window is created with a default size
	glutInitWindowSize(1024, 1024);

	//the top-left corner of the display
	glutInitWindowPosition(450, 0);

	glutCreateWindow("GAME2012_Final_BrazeauHuynh");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init(); // Our own custom function.

	glutDisplayFunc(display);
	glutKeyboardFunc(keyDown);
	glutSpecialFunc(keyDownSpec);
	glutKeyboardUpFunc(keyUp);
	glutSpecialUpFunc(keyUpSpec);

	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMove); // Requires click to register.

	atexit(clean); // This useful GLUT function calls specified function before exiting program. 
	glutMainLoop();

	return 0;
}