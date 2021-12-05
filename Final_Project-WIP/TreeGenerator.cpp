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
std::vector<float> radiuses;
int pointsSize;
int radiusSize;

// Camera
int winW = 1000, winH = 1000;
float aspectRatio = (float)(winW) / (float)(winH);
Camera camera(aspectRatio, vec3(0, 0, 0), vec3(0, 0, -5));
vec3 light(.3f, .2f, -.2f);

// interaction
void* picked = NULL, * hover = NULL;
Mover mover;
int isLit = 0, isGreen = 0;

// vertex shader: operations before the rasterizer
const char* vertexShader = R"(
	#version 130
	in vec3 point;
	in float radius;
	uniform mat4 modelview;
	uniform mat4 persp;
	out vec3 vPoint;
	out float vRadius;
	void main() {
		vRadius = radius;
		vPoint = (modelview * vec4(point, 1)).xyz;
		gl_Position = persp * vec4(vPoint, 1);
	}
)";

// geometry shader: change lines into cylinder
const char* geometryShader = R"(
    #version 330 core
	layout (lines) in;
	layout (triangle_strip, max_vertices = 32) out;
	uniform vec4 initPoint;
	uniform mat4 persp;
	uniform mat4 modelview;
	in vec3 vPoint[];
	in float vRadius[];
	out float greenFactor;
	out vec3 gNormal;
	out vec3 gPoint;

	vec3 Getxcross(vec3 p1, vec3 p2)
    {
        vec3 invec = normalize(p2 - p1);
        vec3 ret = cross( invec, vec3(0.0, 0.0, 1.0) );
        if ( length(ret) == 0.0 )
        {
            ret = cross(invec, vec3(0.0, 1.0, 0.0) );
        }
		if ( length(ret) == 0.0 )
        {
            ret = cross(invec, vec3(1.0, 0.0, 0.0) );
        }

        return ret;
    }

	void main() {
		float dist = distance(vPoint[0], initPoint.xyz);
		greenFactor = dist * 0.3f;

		vec3 axis = vPoint[1] - vPoint[0]; // tip - base

        vec3 xcross = normalize(Getxcross(vPoint[1], vPoint[0]));
        vec3 ycross = cross(normalize(axis), xcross);

        float r1 = vRadius[0];
        float r2 = vRadius[1];

        int res = 16;
        for(int i = 0; i < res; i++) {
            float a = i / float(res - 1) * 2.0 * 3.14159;
            float ca = cos(a), sa = sin(a);
            vec3 normal = vec3(ca*xcross.x + sa*ycross.x,
                               ca*xcross.y + sa*ycross.y,
                               ca*xcross.z + sa*ycross.z );

            vec3 p1 = vPoint[0] + r1 * normal;
            vec3 p2 = vPoint[1] + r2 * normal;

            gl_Position = persp * vec4(p1, 1.0);
			gPoint = p1;
            gNormal = normal;
            EmitVertex();

            gl_Position = persp * vec4(p2, 1.0);
			gPoint = p2;
            gNormal = normal;
            EmitVertex();       
        }
        EndPrimitive();
	}
)";

// pixel shader: operations after the rasterizer
const char* pixelShader = R"(
	#version 130
	uniform vec3 light;
	uniform int isLit = 0;
	uniform int isGreen = 0;
	in float greenFactor;
	in vec3 gPoint;
	in vec3 gNormal;
	out vec4 pColor;
	void main() {
		// compute triangle normal for faceted shading
        vec3 N = normalize(gNormal), E = -gPoint;
        bool sideViewer = dot(E, N) < 0;
        // given local lights, compute total diffuse intensity
        float intensity = .2f;
        vec3 L = normalize(light - gPoint);
        bool sideLight = dot(L, N) < 0;
        if (sideLight == sideViewer)
            intensity += max(0, dot(N, L));
        intensity = clamp(intensity, 0, 1);

		if(isLit == 0) intensity = 1;
		
		if(isGreen == 0) {
			pColor = intensity * vec4(.5f + .5f * gNormal, 1);
		}
		else {
			pColor = intensity * vec4(0.6f, greenFactor + 0.3f, 0.1f, 1); // r, g, b, alpha
		}
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
	glClearColor(0, .5f, .8f, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	// enable z-buffer (needed for tetrahedron)
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	// init shader program, set vertex feed for points and colors
	glUseProgram(program);
	glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
	glViewport(0, 0, winW, winH);

	VertexAttribPointer(program, "point", 3, 0, (void*)0);
	VertexAttribPointer(program, "radius", 1, 0, (void*) pointsSize);
	glLineWidth(5.0f);
	// transform light
	vec4 tLight = camera.modelview * vec4(light, 1);
	// send uniforms
	SetUniform(program, "light", (vec3 *) &tLight);
	SetUniform(program, "persp", camera.persp);
	SetUniform(program, "isLit", isLit);
	SetUniform(program, "isGreen", isGreen);
	SetUniform(program, "initPoint", (camera.modelview * vec4(points[0], 1)));

	mat4 view = camera.modelview;
	for (int i = 0; i < 4; i++) {
		view = view * RotateY(45 * i);
		SetUniform(program, "modelview", view);
		glDrawArrays(GL_LINES, 0, points.size());
	}

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
	pointsSize = points.size() * sizeof(vec3);
	radiusSize = radiuses.size() * sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, pointsSize + radiusSize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, pointsSize, &points[0]);
	glBufferSubData(GL_ARRAY_BUFFER, pointsSize, radiusSize, &radiuses[0]);
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

	if (action == GLFW_PRESS) {
		bool shift = mods & GLFW_MOD_SHIFT;
		if (key == 'L')
			isLit = isLit == 0 ? 1 : 0;
		if (key == 'G')
			isGreen = isGreen == 0 ? 1 : 0;
	}
}

const char* usage = "\n\
    L: turn light on and off\n\
    G: change color\n";

int main() {
	Tree tree;
	tree.generateTree(points, radiuses);
	printf("Points: %d, Radius: %d", points.size(), radiuses.size());

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
