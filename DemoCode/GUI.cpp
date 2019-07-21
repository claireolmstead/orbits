#include <cstdio>		// for C++ i/o
#include <iostream>
#include <string>
#include <cstddef>
using namespace std;	// to avoid having to use std::

#include <GLEW/glew.h>	// include GLEW
#include <GLFW/glfw3.h>	// include GLFW (which includes the OpenGL header)
#include <glm/glm.hpp>	// include GLM (ideally should only use the GLM headers that are actually used)
#include <glm/gtx/transform.hpp>
using namespace glm;	// to avoid having to use glm::

#include <AntTweakBar.h>

#include "shader.h"

// struct for vertex attributes
struct Vertex
{
	GLfloat position[3];
	GLfloat color[3];
};

// global variables

Vertex g_vertices[] = {
	// vertex 1
	-0.5f, 0.5f, 0.5f,	// position
	1.0f, 0.0f, 1.0f,	// colour
	// vertex 2
	-0.5f, -0.5f, 0.5f,	// position
	1.0f, 0.0f, 0.0f,	// colour
	// vertex 3
	0.5f, 0.5f, 0.5f,	// position
	1.0f, 1.0f, 1.0f,	// colour
	// vertex 4
	0.5f, -0.5f, 0.5f,	// position
	1.0f, 1.0f, 0.0f,	// colour
	// vertex 5
	-0.5f, 0.5f, -0.5f,	// position
	0.0f, 0.0f, 1.0f,	// colour
	// vertex 6
	-0.5f, -0.5f, -0.5f,// position
	0.0f, 0.0f, 0.0f,	// colour
	// vertex 7
	0.5f, 0.5f, -0.5f,	// position
	0.0f, 1.0f, 1.0f,	// colour
	// vertex 8
	0.5f, -0.5f, -0.5f,	// position
	0.0f, 1.0f, 0.0f,	// colour
};

GLuint g_indices[] = {
	0, 1, 2,	// triangle 1
	2, 1, 3,	// triangle 2
	4, 5, 0,	// triangle 3
	0, 5, 1,	// ...
	2, 3, 6,
	6, 3, 7,
	4, 0, 6,
	6, 0, 2,
	1, 5, 3,
	3, 5, 7,
	5, 4, 7,
	7, 4, 6,	// triangle 12
};

GLuint g_IBO = 0;				// index buffer object identifier
GLuint g_VBO = 0;				// vertex buffer object identifier
GLuint g_VAO = 0;				// vertex array object identifier
GLuint g_shaderProgramID = 0;	// shader program identifier
GLuint g_MVP_Index = 0;			// location in shader
glm::mat4 g_modelMatrix[9];		// object model matrices
glm::mat4 g_viewMatrix;			// view matrix
glm::mat4 g_projectionMatrix;	// projection matrix

float g_orbitSpeed = 0.3f;
float g_rotationSpeed[2] = { 0.1f, 0.3f };

static void init(GLFWwindow* window)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// set clear background colour

	glEnable(GL_DEPTH_TEST);	// enable depth buffer test

	// create and compile our GLSL program from the shader files
	g_shaderProgramID = loadShaders("MVP_VS.vert", "ColorFS.frag");

	// find the location of shader variables
	GLuint positionIndex = glGetAttribLocation(g_shaderProgramID, "aPosition");
	GLuint colorIndex = glGetAttribLocation(g_shaderProgramID, "aColor");
	g_MVP_Index = glGetUniformLocation(g_shaderProgramID, "uModelViewProjectionMatrix");

	// initialise model matrix to the identity matrix
	g_modelMatrix[0] = g_modelMatrix[1] = g_modelMatrix[2] = g_modelMatrix[3] = g_modelMatrix[4] = g_modelMatrix[5] = g_modelMatrix[6] = glm::mat4(1.0f);

	// initialise view matrix
    g_viewMatrix = glm::lookAt(glm::vec3(0.0f, 3.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.75f), glm::vec3(0.0f, 7.0f, 0.0f));

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspectRatio = static_cast<float>(width) / height;

	// initialise projection matrix
	g_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	
	// generate identifier for VBO and copy data to GPU
	glGenBuffers(1, &g_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertices), g_vertices, GL_STATIC_DRAW);

	// generate identifier for IBO and copy data to GPU
	glGenBuffers(1, &g_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_indices), g_indices, GL_STATIC_DRAW);

	// generate identifiers for VAO
	glGenVertexArrays(1, &g_VAO);

	// create VAO and specify VBO data
	glBindVertexArray(g_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);
	// interleaved attributes
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));

	glEnableVertexAttribArray(positionIndex);	// enable vertex attributes
	glEnableVertexAttribArray(colorIndex);
}

// function used to update the scene
static void update_scene()
{
	// static variables for rotation angles
	static float orbitAngle = 0.0f;
	static float rotationAngle[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	float scaleFactor = 0.1f;

	// update rotation angles
	orbitAngle += g_orbitSpeed * scaleFactor;
	rotationAngle[0] += g_rotationSpeed[0] * scaleFactor;
	rotationAngle[1] += g_rotationSpeed[1] * scaleFactor + 0.1f;
    rotationAngle[2] += g_rotationSpeed[0] * scaleFactor - 0.12f;
    rotationAngle[3] += g_rotationSpeed[1] * scaleFactor - 0.2f;
    rotationAngle[4] += g_rotationSpeed[0] * scaleFactor + 0.15f;

	// update model matrix
    //sun
	g_modelMatrix[0] = glm::rotate(rotationAngle[0], glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
    //planet1
	g_modelMatrix[1] = g_modelMatrix[0]
        *glm::rotate(orbitAngle, glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(1.5f, 0.0f, 0.0f))
		* glm::rotate(rotationAngle[1], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::vec3(0.7f, 0.7f, 0.7f));
    //planet2
    g_modelMatrix[2] = g_modelMatrix[0]
        *glm::rotate(orbitAngle, glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::translate(glm::vec3(3.0f, 0.0f, 0.0f))
        * glm::rotate(rotationAngle[2], glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::scale(glm::vec3(0.4f, 0.4f, 0.4f));
    //inner moon1
    g_modelMatrix[3] = g_modelMatrix[2]
        * glm::rotate(orbitAngle, glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::translate(glm::vec3(1.0f, 0.0f, 0.0f))
        * glm::rotate(rotationAngle[3], glm::vec3(0.0f, 1.0f, 0.0f)) //rotate around axis
        * glm::scale(glm::vec3(0.4f, 0.4f, 0.4f));
    //outer moon
    g_modelMatrix[4] = g_modelMatrix[2]
        * glm::rotate(orbitAngle, glm::vec3(0.0f, 0.1f, 0.0f))
        * glm::translate(glm::vec3(1.75f, 0.0f, 0.0f))
        * glm::rotate(rotationAngle[4], glm::vec3(0.0f, 2.0f, 0.0f))
        * glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
    
    //inner orbit
    g_modelMatrix[5] =glm::rotate(rotationAngle[5], glm::vec3(0.5f, 0.0f, 0.0f))
        * glm::scale(glm::vec3(1.5f, 1.5f, 1.5f));
    
    //outer orbit
    g_modelMatrix[6] =g_modelMatrix[0]
         * glm::rotate(rotationAngle[5], glm::vec3(.5f, 0.0f, 0.0f))
        * glm::scale(glm::vec3(3.5f, 3.5f, 3.5f));
    
    //inner moon orbit
    g_modelMatrix[7] = g_modelMatrix[2]
        * glm::rotate(rotationAngle[5], glm::vec3(1.0f, 0.0f, 0.0f))
        * glm::scale(glm::vec3(1.75f, 1.75f, 1.75f));
    
    //outer moon orbit
    g_modelMatrix[8] = g_modelMatrix[2]
    * glm::rotate(rotationAngle[5], glm::vec3(1.0f, 0.0f, 0.0f));
    //* glm::translate(glm::vec3(0.0f, 0.5f, 0.0f));

}

// function used to render the scene
static void render_scene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// clear colour buffer and depth buffer

	glUseProgram(g_shaderProgramID);	// use the shaders associated with the shader program

	glBindVertexArray(g_VAO);		// make VAO active

// sun
	glm::mat4 MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[0];
	// set uniform model transformation matrix
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_indices), g_indices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertices), g_vertices, GL_STATIC_DRAW);
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

// planet 1
	MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[1];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type
    
// planet 2
    MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[2];
    glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);    // display the vertices based on their indices and primitive type
    
// moon1
    MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[3];
    glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);    // display the vertices based on their indices and primitive type

// moon2
    MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[4];
    glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);    // display the vertices based on their indices and primitive type
    
//circle
    GLuint numOfVertices = 100; //10sides
    GLfloat doublePi = 2.0f * M_PI;
    
    GLfloat c_vert_x[numOfVertices];
    GLfloat c_vert_y[numOfVertices];
    GLfloat c_vert_z[numOfVertices];
    
    GLuint sphere_indices[100] = {0};
    //set vertices
    for(int i = 0; i < numOfVertices; i++){
        c_vert_x[i] = (cos(i * doublePi / 99));
        c_vert_y[i] = (sin(i * doublePi / 99));
        c_vert_z[i] = 0.0f;
        
        sphere_indices[i] = i;
    }
    
    GLfloat allVerts[numOfVertices * 6]; //36 coords, 36more for color
    GLfloat colorVerts[3] = {1.0f, 0.0f, 0.0f};
    
    //make combined array with all vertices.
    for(int k = 0; k < numOfVertices; k++){
        allVerts[k*6] = c_vert_x[k];
        allVerts[(k*6) + 1] = c_vert_y[k];
        allVerts[(k*6) + 2] = c_vert_z[k];
        allVerts[(k*6) + 3] = colorVerts[0];
        allVerts[(k*6) + 4] = colorVerts[1];
        allVerts[(k*6) + 5] = colorVerts[2];
    }
    //inner orbit line
    MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[5];
    glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(allVerts), allVerts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_indices), sphere_indices, GL_STATIC_DRAW);
    glDrawElements(GL_LINE_STRIP, 100, GL_UNSIGNED_INT, 0);
    
    //outer orbit line
    MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[6];
    glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
    glDrawElements(GL_LINE_STRIP, 100, GL_UNSIGNED_INT, 0);
    
    //inner moon orbit line
    MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[7];
    glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
    glDrawElements(GL_LINE_STRIP, 100, GL_UNSIGNED_INT, 0);
    
    //outer moon orbit line
    MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[8];
    glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
    glDrawElements(GL_LINE_STRIP, 100, GL_UNSIGNED_INT, 0);
    
	glFlush();	// flush the pipeline
}

// key press or release callback function
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// quit if the ESCAPE key was press
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
}

// mouse movement callback function
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{

}

// mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

}

// error callback function
static void error_callback(int error, const char* description)
{
	cerr << description << endl;	// output error description
}

int main(void)
{
	GLFWwindow* window = NULL;	// pointer to a GLFW window handle

	glfwSetErrorCallback(error_callback);	// set error callback function

	// initialise GLFW
	if (!glfwInit())
	{
		// if failed to initialise GLFW
		exit(EXIT_FAILURE);
	}

	// minimum OpenGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window and its OpenGL context
	window = glfwCreateWindow(800, 800, "Assignment1 pt. 2 -- Olmstead", NULL, NULL);

	// if failed to create window
	if (window == NULL)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);	// set window context as the current context
	glfwSwapInterval(1);			// swap buffer interval

	// initialise GLEW
	if (glewInit() != GLEW_OK)
	{
		// if failed to initialise GLEW
		cerr << "GLEW initialisation failed" << endl;
		exit(EXIT_FAILURE);
	}

	// set key callback function
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// initialise rendering states
	init(window);

	// variables for simple time management
	float lastUpdateTime = glfwGetTime();
	float currentTime = lastUpdateTime;

	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		currentTime = glfwGetTime();

		// only update if more than 0.02 seconds since last update
		if (currentTime - lastUpdateTime > 0.02)
		{
			update_scene();		// update the scene
			render_scene();		// render the scene

			glfwSwapBuffers(window);	// swap buffers
			glfwPollEvents();			// poll for events

			lastUpdateTime = currentTime;	// update last update time
		}
	}

	// clean up
	glDeleteProgram(g_shaderProgramID);
	glDeleteBuffers(1, &g_IBO);
	glDeleteBuffers(1, &g_VBO);
	glDeleteVertexArrays(1, &g_VAO);

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

