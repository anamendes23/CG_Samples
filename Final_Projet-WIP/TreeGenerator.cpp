/*
	CPSC 5700: Computer Graphics
	Ana Carolina de Souza Mendes

	TreeGenerator.cpp: draws 3D tree sketch and writes it to .obj file
*/

#include "glad.h"
#include <glfw3.h>
#include <stdio.h>
#include "GLXtras.h"
#include "Camera.h"
#include "Tree.h"

GLuint vBuffer = 0;
GLuint program = 0;

std::vector<vec3> points;

// Camera
int winW = 400, winH = 400;
float aspectRatio = (float)(winW) / (float)(winH);
Camera camera(aspectRatio, vec3(0, 0, 0), vec3(0, 0, -5));

// vertex shader: operations before the rasterizer
const char* vertexShader = R"(
	#version 130
	in vec3 point;											// 2D point from GPU memory
	uniform mat4 view;
	void main() {
		// REQUIREMENT 1A) transform vertex:
		gl_Position = view * vec4(point, 1);					// 'built-in' variable
	}
)";

// pixel shader: operations after the rasterizer
const char* pixelShader = R"(
	#version 130
	out vec4 pColor;
	void main() {
		// REQUIREMENT 1B) shade pixel:
		pColor = vec4(0, 1, 0, 1);							// r, g, b, alpha
	}
)";

// Interaction

vec2  mouseDown;            // reference for mouse drag
vec2  rotOld, rotNew;       // .x is rotation about Y-axis, .y about X-axis
float rotZ = 0;
vec2  tranOld, tranNew;
float rotSpeed = .3f, tranSpeed = .01f;

void MouseWheel(GLFWwindow* w, double ignore, double spin) {
	rotZ += (spin > 0 ? 1 : -1) * 2.5f;
}

void MouseButton(GLFWwindow* w, int butn, int action, int mods) {
	// called when mouse button pressed or released
	if (action == GLFW_PRESS) {
		double x, y;
		glfwGetCursorPos(w, &x, &y);
		mouseDown = vec2((float)x, (float)y); // save reference for MouseDrag
	}
	if (action == GLFW_RELEASE) {
		rotOld = rotNew;                        // save reference for MouseDrag
		tranOld = tranNew;
	}
}

void MouseMove(GLFWwindow* w, double x, double y) {
	if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) { // drag
		// find mouse drag difference
		vec2 mouse((float)x, (float)y), dif = mouse - mouseDown;
		bool shift = glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
			glfwGetKey(w, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
		if (shift)
			tranNew = tranOld + tranSpeed * vec2(dif.x, -dif.y);    // SHIFT key: translate
		else
			rotNew = rotOld + rotSpeed * dif;                       // rotate
	}
}

void Resize(GLFWwindow* window, int width, int height) {
	camera.Resize(winW = width, winH = height);
	glViewport(0, 0, winW, winH);
}

void Display() {
	// clear screen to grey
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	// enable z-buffer (needed for tetrahedron)
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(program);
	glBindBuffer(GL_ARRAY_BUFFER, vBuffer);

	GLint id = glGetAttribLocation(program, "point");
	glEnableVertexAttribArray(id);
	glVertexAttribPointer(id, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	VertexAttribPointer(program, "point", 3, 0, (void*)0);
	glLineWidth(5.0f);
	
	mat4 view = Translate(tranNew.x, tranNew.y, 0) * RotateY(rotNew.x) * RotateX(rotNew.y) * RotateZ(rotZ);
	SetUniform(program, "view", view);
	glDrawArrays(GL_LINES, 0, points.size());

	view = view * RotateY(90);
	SetUniform(program, "view", view);
	glDrawArrays(GL_LINES, 0, points.size());

	glFlush();
}

void InitVertexBuffer() {
	glGenBuffers(1, &vBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
	int size = points.size() * sizeof(vec3);
	glBufferData(GL_ARRAY_BUFFER, size, &points[0], GL_STATIC_DRAW);
}

int AppEnd(const char *msg) {
	if (msg) printf("%s\n", msg);
	getchar();
	glfwTerminate();
	return msg == NULL? 0 : 1;
}

void Keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main() {
	Tree tree;
	tree.generateTree(points);
	/*
	for (int i = 0; i < points.size(); i++) {
		printf("v %f %f %f\n", points[i].x, points[i].y, points[i].z);
	}
	*/
	if (!glfwInit()) return AppEnd("can't init GLFW\n");
	GLFWwindow *w = glfwCreateWindow(400, 400, "Line test", NULL, NULL);
	// need window to create GL context
	if (!w) return AppEnd("can't open window\n");
	glfwSetWindowPos(w, 100, 100);
	glfwMakeContextCurrent(w);
	// must load OpenGL runtime subroutine pointers
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	printf("GL vendor: %s\n", glGetString(GL_VENDOR));
	printf("GL renderer: %s\n", glGetString(GL_RENDERER));
	printf("GL version: %s\n", glGetString(GL_VERSION));
	printf("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	program = LinkProgramViaCode(&vertexShader, &pixelShader);
	InitVertexBuffer();
	// callbacks
	glfwSetWindowSizeCallback(w, Resize);
	glfwSetCursorPosCallback(w, MouseMove);
	glfwSetMouseButtonCallback(w, MouseButton);
	glfwSetScrollCallback(w, MouseWheel);
	glfwSetKeyCallback(w, Keyboard);

	while (!glfwWindowShouldClose(w)) {
		Display();
		glfwSwapBuffers(w);
		glfwPollEvents();
	}
	// unbind vertex buffer, free GPU memory
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vBuffer);
	glfwDestroyWindow(w);
	return AppEnd(NULL);
}
