/*
    CPSC 5700: Computer Graphics
    Ana Carolina de Souza Mendes

    AnimalLineDrawing.cpp: read an .obj file and draw only the lines of its triangles.
*/

#include <glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <vector>
#include "GLXtras.h"
#include "Mesh.h"

// Application Data
const char* objFilename = "./frog.obj";
const char* objFilename2 = "./penguin.obj";

std::vector<vec3> points; // 3D mesh vertices
std::vector<vec3> points2; // 3D mesh vertices
std::vector<int3> triangles; // triplets of vertex indices
std::vector<int3> triangles2; // triplets of vertex indices

GLuint vBuffer = 0;   // GPU vertex buffer ID
GLuint program = 0;   // GLSL program ID

// Camera
int winW = 800, winH = 400;
float aspectRatio = (float)(winW / 2) / (float)(winH);
Camera camera(aspectRatio, vec3(0, 0, 0), vec3(0, 0, -5));

// Shaders: vertex shader with view transform, trivial pixel shader

const char *vertexShader = R"(
    #version 130
    in vec3 point;
    uniform mat4 modelview;
    uniform mat4 persp;
    void main() {
        gl_Position = persp * modelview * vec4(point, 1);
    }
)";

const char *pixelShader = R"(
    #version 130
    out vec4 pColor;
    uniform vec4 color = vec4(.7, .7, 0, 1);
    void main() {
        pColor = color;
    }
)";

// Initialization

void InitVertexBuffer() {
    // create GPU vertex buffer
    glGenBuffers(1, &vBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
    // allocate memory for vertex positions
    int sizePts = points.size() * sizeof(vec3);
    int sizePts2 = points2.size() * sizeof(vec3);
    glBufferData(GL_ARRAY_BUFFER, sizePts + sizePts2, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizePts, &points[0]);
    glBufferSubData(GL_ARRAY_BUFFER, sizePts, sizePts2, &points2[0]);
}


void MouseWheel(GLFWwindow *w, double ignore, double spin) {
    bool shift = glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(w, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    camera.MouseWheel(spin > 0, shift);
}

void MouseButton(GLFWwindow *w, int butn, int action, int mods) {
    // called when mouse button pressed or released
    if (action == GLFW_PRESS) {
        double x, y;
        glfwGetCursorPos(w, &x, &y);
        camera.MouseDown((int)x, (int)y);
    }
    if (action == GLFW_RELEASE) {
        camera.MouseUp();
    }
}

void MouseMove(GLFWwindow *w, double x, double y) {
    bool shift = glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(w, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) { // drag
        camera.MouseDrag((int)x, (int)y, shift);
    }
}

void Resize(GLFWwindow* window, int width, int height) {
    camera.Resize(winW = width / 2, winH = height);
    glViewport(0, 0, winW, winH);
}

// Application

void Display(GLFWwindow *w) {
    // clear screen to grey
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    // enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // enable z-buffer (needed for tetrahedron)
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    // init shader program, set vertex feed for points and colors
    glUseProgram(program);

    // update matrices
    SetUniform(program, "modelview", camera.modelview);
    SetUniform(program, "persp", camera.persp);

    VertexAttribPointer(program, "point", 3, 0, (void *) 0);

    // get screen size to support resizing
    glfwGetWindowSize(w, &winW, &winH);

    // draw on left (frog)
    glViewport(0, 0, winW / 2, winH);
    for (int i = 0; i < (int)triangles.size(); i++) {
        glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, &triangles[i]);
    }
    int sizePts = points.size() * sizeof(vec3);
    VertexAttribPointer(program, "point", 3, 0, (void *) sizePts);
    // draw on right (penguin)
    glViewport(winW / 2, 0, winW / 2, winH);
    for (int i = 0; i < (int)triangles2.size(); i++) {
        glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, &triangles2[i]);
    }

    glFlush();
}

void Close() {
    // unbind vertex buffer, free GPU memory
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vBuffer);
}

void ErrorGFLW(int id, const char *reason) {
    printf("GFLW error %i: %s\n", id, reason);
}

void Keyboard(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(int argc, char **argv) {
    // init app window
    if (!glfwInit())
        return 1;
    glfwSetErrorCallback(ErrorGFLW);
    GLFWwindow* w = glfwCreateWindow(winW, winH, "Animal Line Drawing", NULL, NULL);
    if (!w) {
        glfwTerminate();
        return 1;
    }
    glfwSetWindowPos(w, 100, 100);
    glfwMakeContextCurrent(w);
    // init OpenGL
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    program = LinkProgramViaCode(&vertexShader, &pixelShader);

    // read OBJ file and fit object to +/- 1 space
    if (!ReadAsciiObj((char *) objFilename, points, triangles)) {
        printf("failed to read obj file (type any key to continue)\n");
        getchar();
        return 1;
    }
    if (!ReadAsciiObj((char*)objFilename2, points2, triangles2)) {
        printf("failed to read obj file (type any key to continue)\n");
        getchar();
        return 1;
    }
    printf("%i vertices, %i triangles\n", points.size(), triangles.size());
    Normalize(points, .8f);
    Normalize(points2, .8f);

    printf("GL version: %s\n", glGetString(GL_VERSION));
    PrintGLErrors();
    // init shader programs
    InitVertexBuffer();
    // callbacks
    glfwSetWindowSizeCallback(w, Resize);
    glfwSetCursorPosCallback(w, MouseMove);
    glfwSetMouseButtonCallback(w, MouseButton);
    glfwSetScrollCallback(w, MouseWheel);
    glfwSetKeyCallback(w, Keyboard);
    // event loop
    while (!glfwWindowShouldClose(w)) {
        Display(w);
        glfwPollEvents();
        glfwSwapBuffers(w);
    }
    Close();
    glfwDestroyWindow(w);
    glfwTerminate();
}
