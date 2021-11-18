/*
	CPSC 5700: Computer Graphics
	Ana Carolina de Souza Mendes

	ColorfulName.cpp: draw multiple triangles to form a colorful name
*/

#include <glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "GLXtras.h"
#include <vector>

// shader program

GLuint vBuffer = 0; // GPU vertex buffer ID, valid if > 0
GLuint program = 0; // GLSL program ID, valid if > 0

const char *vertexShader = R"(
	#version 130
	in vec2 point;
	in vec3 color;
	out vec4 vColor;
	void main() {
		gl_Position = vec4(point, 0, 1);
		vColor = vec4(color, 1);
	}
)";

const char *pixelShader = R"(
	#version 130
	in vec4 vColor;
	out vec4 pColor;
	void main() {
		pColor = vColor;
	}
)";

// the name ANA D S M
float points[][2] = {// A
					 {-.9375f, .1875f}, {-.6875f, .6875f}, {-.4375f, .1875f}, {-.5625f, .1875f},
					 {-.625f, .3125f},  {-.75f, .3125f},   {-.8125f, .1875f},
					 // N
					 {-.25f, .1875f}, {-.25f, .6875f}, {-.125f, .6875f}, { .125f,  .375f}, { .125f, .6875f},
					 { .25f, .6875f}, { .25f, .1875f}, { .125f, .1875f}, {-.125f, .5f},    {-.125f, .1875f},
				     // A
					 {.9375f, .1875f}, {.6875f, .6875f}, {.4375f, .1875f}, {.5625f, .1875f},
					 {.625f, .3125f},  {.75f, .3125f},   {.8125f, .1875f},
					 // D
					 {-.9375f, -.6875f}, {-.9375f, -.1875f}, {-.4375f, -.1875f}, {-.4375f, -.6875f},
					 {-.5625f, -.5625f}, {-.5625f, -.3125f}, {-.8125f, -.3125f}, {-.8125f, -.5625f},
					 // S
					 {-.25f, -.6875f},  {-.25f, -.5625f}, {.125f, -.5625f}, {.125f, -.4688f},
					 {-.25f, -.4688f},  {-.25f, -.1875f}, {.25f, -.1875f},  {.25f, -.3125f},
					 {-.125f, -.3125f}, {-.125f, -.375f}, {.25f, -.375f},   {.25f, -.6875f},
					 // M
					 {.9375f, -.6875f}, {.9375f, -.1875f}, {.8125f, -.1875f}, {.6875f, -.4375f},
					 {.5625f, -.1875f}, {.4375f, -.1875f}, {.4375f, -.6875f}, {.5625f, -.6875f},
					 {.5625f, -.5f},    {.6563f, -.6875f}, {.7188f, -.6875f}, {.8125f, -.5f},  {.8125f, -.6875f}
					 };
float colors[][3] = {// A
					 {0.97f, 0.15f, 0.52f}, {0.56f, 0.04f, 0.72f}, {0.3f, 0.79f, 0.97f}, {0.3f, 0.79f, 0.97f},
					 {0.26f, 0.38f, 0.93f}, {0.23f, 0.05f, 0.64f}, {0.97f, 0.15f, 0.52f},
					 // N
					 {0.97f, 0.15f, 0.52f}, {0.56f, 0.04f, 0.72f}, {0.56f, 0.04f, 0.72f}, {0.23f, 0.05f, 0.64f}, {0.3f, 0.79f, 0.97f},
					 {0.3f, 0.79f, 0.97f},  {0.26f, 0.38f, 0.93f}, {0.26f, 0.38f, 0.93f}, {0.23f, 0.05f, 0.64f}, {0.97f, 0.15f, 0.52f},
					 // A
					 {0.97f, 0.15f, 0.52f}, {0.56f, 0.04f, 0.72f}, {0.3f, 0.79f, 0.97f}, {0.3f, 0.79f, 0.97f},
					 {0.26f, 0.38f, 0.93f}, {0.23f, 0.05f, 0.64f}, {0.97f, 0.15f, 0.52f},
					 // D
					 {0.97f, 0.15f, 0.52f}, {0.56f, 0.04f, 0.72f}, {0.26f, 0.38f, 0.93f}, {0.3f, 0.79f, 0.97f},
					 {0.23f, 0.05f, 0.64f}, {0.23f, 0.05f, 0.64f}, {0.23f, 0.05f, 0.64f}, {0.23f, 0.05f, 0.64f},
					 // S
					 {0.97f, 0.15f, 0.52f}, {0.97f, 0.15f, 0.52f}, {0.56f, 0.04f, 0.72f}, {0.56f, 0.04f, 0.72f},
					 {0.26f, 0.38f, 0.93f}, {0.23f, 0.05f, 0.64f}, {0.97f, 0.15f, 0.52f}, {0.56f, 0.04f, 0.72f},
					 {0.3f, 0.79f, 0.97f},  {0.3f, 0.79f, 0.97f},  {0.23f, 0.05f, 0.64f}, {0.3f, 0.79f, 0.97f},
					 // M
					 {0.97f, 0.15f, 0.52f}, {0.3f, 0.79f, 0.97f}, {0.3f, 0.79f, 0.97f},  {0.26f, 0.38f, 0.93f},
					 {0.3f, 0.79f, 0.97f},  {0.3f, 0.79f, 0.97f}, {0.97f, 0.15f, 0.52f}, {0.97f, 0.15f, 0.52f},
					 {0.56f, 0.04f, 0.72f}, {0.23f, 0.05f, 0.64f},{0.23f, 0.05f, 0.64f}, {0.56f, 0.04f, 0.72f}, {0.97f, 0.15f, 0.52f}
					 };
int triangles[][3] = {// A - 7 (0)
					  {0, 6, 5}, {0, 1, 5}, {1, 5, 4}, {1, 4, 2}, {4, 3, 2},
					  // N - 10 (7)
					  {7, 16, 15},  {7, 8, 15},   {8, 9, 15},   {9, 15, 10},
					  {15, 10, 14},	{10, 14, 13}, {10, 13, 12}, {10, 11, 12},
					  // A - 7 (17)
					  {17, 23, 22}, {17, 18, 22}, {18, 22, 21}, {18, 21, 19}, {21, 20, 19},
					  // D - 8 (24)
					  {24, 25, 31}, {25, 30, 31}, {25, 30, 26}, {26, 29, 30},
					  {26, 27, 29}, {27, 28, 29}, {27, 28, 24}, {24, 28, 31},
					  // S - 12 (32)
					  {32, 33, 43}, {33, 43, 34}, {34, 43, 42}, {34, 35, 42}, {35, 42, 36},
					  {36, 42, 41}, {36, 41, 37}, {37, 41, 40}, {37, 40, 38}, {38, 40, 39},
					  // M - 13 (42)
					  {44, 45, 55}, {44, 55, 56}, {45, 46, 55}, {46, 47, 55}, {47, 55, 54},
					  {47, 54, 53}, {47, 53, 52}, {47, 48, 52}, {48, 49, 52}, {49, 50, 52}, {50, 51, 52}
					  };

void InitVertexBuffer() {
	// create GPU buffer, make it active, allocate memory and copy vertices
	glGenBuffers(1, &vBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
	// allocate and fill vertex buffer
	int vsize = sizeof(points), csize = sizeof(colors);
	glBufferData(GL_ARRAY_BUFFER, vsize+csize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vsize, points);
	glBufferSubData(GL_ARRAY_BUFFER, vsize, csize, colors);
}

// display

void Display() {
	// clear screen to grey
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);
	// establish vertex fetch for point and for color, then draw triangles
	int vsize = sizeof(points), ntris = sizeof(triangles)/(3*sizeof(int));
	VertexAttribPointer(program, "point", 2, 0, (void *) 0);
	VertexAttribPointer(program, "color", 3, 0, (void *) vsize);
	glDrawElements(GL_TRIANGLES, 3*ntris, GL_UNSIGNED_INT, &triangles[0]);
	glFlush();
}

// application

void Close() {
	// unbind vertex buffer and free GPU memory
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (vBuffer >= 0)
		glDeleteBuffers(1, &vBuffer);
}

void ErrorGFLW(int id, const char *reason) {
	printf("GFLW error %i: %s\n", id, reason);
}

void Keyboard(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main() {
	glfwSetErrorCallback(ErrorGFLW);
	if (!glfwInit())
		return 1;
	GLFWwindow *window = glfwCreateWindow(800, 800, "Colorful Name", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	printf("GL version: %s\n", glGetString(GL_VERSION));
	PrintGLErrors();
	if (!(program = LinkProgramViaCode(&vertexShader, &pixelShader)))
		return 0;
	InitVertexBuffer();
	glfwSetKeyCallback(window, Keyboard);
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(window)) {
		Display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	Close();
	glfwDestroyWindow(window);
	glfwTerminate();
}
