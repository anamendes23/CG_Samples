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
#include "Draw.h"
#include "Tree.h"
#include "VecMat.h"
#include "Widgets.h"

GLuint vBuffer = 0;
GLuint program = 0;

std::vector<vec3> points;

// Camera
int winW = 1000, winH = 1000;
float aspectRatio = (float)(winW) / (float)(winH);
Camera camera(aspectRatio, vec3(0, 0, 0), vec3(0, 0, -5));
vec3 light(.3f, .2f, -.2f);

// interaction
void* picked = NULL, * hover = NULL;
Mover mover;

// vertex shader: operations before the rasterizer
const char* vertexShader = R"(
	#version 130
	in vec3 point;											// 2D point from GPU memory
	uniform mat4 modelview;
	uniform mat4 persp;
	void main() {
		// REQUIREMENT 1A) transform vertex:
		gl_Position = persp * modelview * vec4(point, 1);					// 'built-in' variable
	}
)";

// geometry shader: change lines into cylinder
const char* geometryShader = R"(
    #version 330 core
	layout (lines) in;
	layout (triangle_strip, max_vertices = 24) out;
	//layout (line_strip, max_vertices = 2) out;
	uniform vec4 initPoint;
	out float greenFactor;

	mat4 RotateZ(float theta) {
		float angle = 3.14159265358f/180.f * theta;
		mat4 c;
		c[0][0] = cos(angle);
		c[1][1] = cos(angle);
		c[1][0] = sin(angle);
		c[0][1] = -sin(angle);
		return c;
	}

	mat4 getCoordSys(vec4 p1, vec4 p2) {
		vec4 direction = p2 - p1;
		vec4 yAxis = vec4(0, 1, 0, 0);
		float cosAngle = dot(vec4(direction.xyz, 0), yAxis);
		float angle = acos(cosAngle);
		return RotateZ(angle);
	}
	
	void main() {
		vec4 p1 = gl_in[0].gl_Position;
		vec4 p2 = gl_in[1].gl_Position;
		float dist = distance(p1, initPoint);
		greenFactor = dist * 0.07f;
		
		mat4 rotMatrix = getCoordSys(p1, p2);
		/*
		gl_Position = gl_in[0].gl_Position;
		EmitVertex();
		gl_Position = rotMatrix * gl_in[0].gl_Position;
		EmitVertex();
		EndPrimitive();
		*/

		float e1 = 0.02f;
		float e2 = 0.01f;

		// ------------- FRONT ----------------------
		
		gl_Position = gl_in[0].gl_Position + vec4(-e1, -e1, 0, 0);
		EmitVertex();

		gl_Position = gl_in[1].gl_Position + vec4(-e2, -e2, 0, 0);
		EmitVertex();

		gl_Position = gl_in[1].gl_Position + vec4(e2, -e2, 0, 0);
		EmitVertex();

		EndPrimitive();

		gl_Position = gl_in[0].gl_Position + vec4(-e1, -e1, 0, 0);
		EmitVertex();

		gl_Position = gl_in[1].gl_Position + vec4(e2, -e2, 0, 0);
		EmitVertex();

		gl_Position = gl_in[0].gl_Position + vec4(e1, -e1, 0, 0);
		EmitVertex();

		EndPrimitive();

		// ------------- LEFT ----------------------
		
		gl_Position = gl_in[0].gl_Position + vec4(-e1, -e1, 0, 0);
		EmitVertex();

		gl_Position = gl_in[1].gl_Position + vec4(-e2, -e2, 0, 0);
		EmitVertex();

		gl_Position = gl_in[0].gl_Position + vec4(-e1, e1, 0, 0);
		EmitVertex();

		EndPrimitive();

		gl_Position = gl_in[1].gl_Position + vec4(-e2, -e2, 0, 0);
		EmitVertex();

		gl_Position = gl_in[1].gl_Position + vec4(-e2, e2, 0, 0);
		EmitVertex();

		gl_Position = gl_in[0].gl_Position + vec4(-e1, e1, 0, 0);
		EmitVertex();

		EndPrimitive();

		// ------------- BACK ----------------------
		
		gl_Position = gl_in[0].gl_Position + vec4(-e1, e1, 0, 0);
		EmitVertex();

		gl_Position = gl_in[1].gl_Position + vec4(-e2, e2, 0, 0);
		EmitVertex();

		gl_Position = gl_in[1].gl_Position + vec4(e2, e2, 0, 0);
		EmitVertex();

		EndPrimitive();

		gl_Position = gl_in[1].gl_Position + vec4(e2, e2, 0, 0);
		EmitVertex();

		gl_Position = gl_in[0].gl_Position + vec4(e1, e1, 0, 0);
		EmitVertex();

		gl_Position = gl_in[0].gl_Position + vec4(-e1, e1, 0, 0);
		EmitVertex();

		EndPrimitive();

		// ------------- RIGHT ----------------------
		
		gl_Position = gl_in[0].gl_Position + vec4(e1, e1, 0, 0);
		EmitVertex();

		gl_Position = gl_in[1].gl_Position + vec4(e2, e2, 0, 0);
		EmitVertex();

		gl_Position = gl_in[1].gl_Position + vec4(e2, -e2, 0, 0);
		EmitVertex();

		EndPrimitive();

		gl_Position = gl_in[1].gl_Position + vec4(e2, -e2, 0, 0);
		EmitVertex();

		gl_Position = gl_in[0].gl_Position + vec4(e1, e1, 0, 0);
		EmitVertex();

		gl_Position = gl_in[0].gl_Position + vec4(e1, -e1, 0, 0);
		EmitVertex();

		EndPrimitive();
		
	}
)";

// pixel shader: operations after the rasterizer
const char* pixelShader = R"(
	#version 130
	uniform vec3 light;
	in float greenFactor;
	out vec4 pColor;
	void main() {
		// REQUIREMENT 1B) shader pixel:
		pColor = vec4(0.6f, greenFactor + 0.3f, 0.1f, 1);	// r, g, b, alpha
	}
)";

int WindowHeight(GLFWwindow* w) {
	int width, height;
	glfwGetWindowSize(w, &width, &height);
	return height;
}

bool Shift(GLFWwindow* w) {
	return glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
		glfwGetKey(w, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
}

void MouseWheel(GLFWwindow* w, double ignore, double spin) {
	camera.MouseWheel(spin > 0, Shift(w));
}

void MouseButton(GLFWwindow* w, int butn, int action, int mods) {
	if (action == GLFW_PRESS) {
		double x, y;
		glfwGetCursorPos(w, &x, &y);
		y = WindowHeight(w) - y;
		picked = NULL;
		if (MouseOver(x, y, light, camera.fullview)) {
			mover.Down(&light, (int)x, (int)y, camera.modelview, camera.persp);
			picked = &mover;
		}
		if (picked == NULL) {
			camera.MouseDown(x, y);
			picked = &camera;
		}
	}
	if (action == GLFW_RELEASE)
		camera.MouseUp();
}

void MouseMove(GLFWwindow* w, double x, double y) {
	y = WindowHeight(w) - y;
	if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (picked == &mover)
			mover.Drag((int)x, (int)y, camera.modelview, camera.persp);
		if (picked == &camera)
			camera.MouseDrag((int)x, (int)y, Shift(w));
	}
	else
		hover = MouseOver(x, y, light, camera.fullview) ? (void*)&light : NULL;
}

void Resize(GLFWwindow* window, int width, int height) {
	camera.Resize(winW = width, winH = height);
	glViewport(0, 0, width, height);
}

void Display() {
	// clear screen to blue
	glClearColor(0, .4f, .7f, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program);
	glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
	glViewport(0, 0, winW, winH);

	VertexAttribPointer(program, "point", 3, 0, (void*)0);
	glLineWidth(5.0f);
	// transform light
	vec4 tLight = camera.modelview * vec4(light, 1);
	// send uniforms
	mat4 modelview = camera.modelview;
	SetUniform(program, "light", (vec3 *) &tLight);
	SetUniform(program, "modelview", modelview);
	SetUniform(program, "persp", camera.persp);
	SetUniform(program, "initPoint", vec4(points[0], 1));
	glDrawArrays(GL_LINES, 0, points.size());
	
	modelview = modelview * RotateY(45);
	SetUniform(program, "modelview", modelview);
	glDrawArrays(GL_LINES, 0, points.size());

	modelview = modelview * RotateY(45);
	SetUniform(program, "modelview", modelview);
	glDrawArrays(GL_LINES, 0, points.size());

	modelview = modelview * RotateY(45);
	SetUniform(program, "modelview", modelview);
	glDrawArrays(GL_LINES, 0, points.size());
	
	glDisable(GL_DEPTH_TEST);
	UseDrawShader(ScreenMode());
	UseDrawShader(camera.fullview);
	bool lVisible = IsVisible(light, camera.fullview);
	Disk(light, 12, hover == &light ? lVisible ? vec3(1, 0, 0) : vec3(1, 0, 1) : lVisible ? vec3(1, 1, 0) : vec3(0, 1, 1));
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

	if (!glfwInit()) return AppEnd("can't init GLFW\n");
	GLFWwindow *w = glfwCreateWindow(winW, winH, "Tree", NULL, NULL);
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
	program = LinkProgramViaCode(&vertexShader, NULL, NULL, &geometryShader, &pixelShader);
	InitVertexBuffer();
	// callbacks
	glfwSetWindowSizeCallback(w, Resize);
	glfwSetCursorPosCallback(w, MouseMove);
	glfwSetMouseButtonCallback(w, MouseButton);
	glfwSetScrollCallback(w, MouseWheel);
	glfwSetKeyCallback(w, Keyboard);

	while (!glfwWindowShouldClose(w)) {
		glfwGetWindowSize(w, &winW, &winH);
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
