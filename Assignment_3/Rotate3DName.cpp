/*
    CPSC 5700: Computer Graphics
    Ana Carolina de Souza Mendes

    Rotate3DName.cpp: rotate name in response to user input.
*/

#include <glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <vector>
#include "GLXtras.h"
#include <time.h>

// Application Data

GLuint vBuffer = 0;   // GPU vertex buffer ID
GLuint program = 0;   // GLSL program ID

// the name ANA D S M
float points[][3] = {// A
                     {-.9375f, .1875f, 0}, {-.6875f, .6875f, 0}, {-.4375f, .1875f, 0}, {-.5625f, .1875f, 0},
                     {-.625f, .3125f, 0},  {-.75f, .3125f, 0},   {-.8125f, .1875f, 0},
                     // N
                     {-.25f, .1875f, 0}, {-.25f, .6875f, 0}, {-.125f, .6875f, 0}, { .125f,  .375f, 0}, { .125f, .6875f, 0},
                     { .25f, .6875f, 0}, { .25f, .1875f, 0}, { .125f, .1875f, 0}, {-.125f, .5f, 0},    {-.125f, .1875f, 0},
                     // A
                     {.9375f, .1875f, 0}, {.6875f, .6875f, 0}, {.4375f, .1875f, 0}, {.5625f, .1875f, 0},
                     {.625f, .3125f, 0},  {.75f, .3125f, 0},   {.8125f, .1875f, 0},
                     // D
                     {-.9375f, -.6875f, 0}, {-.9375f, -.1875f, 0}, {-.4375f, -.1875f, 0}, {-.4375f, -.6875f, 0},
                     {-.5625f, -.5625f, 0}, {-.5625f, -.3125f, 0}, {-.8125f, -.3125f, 0}, {-.8125f, -.5625f, 0},
                     // S
                     {-.25f, -.6875f, 0},  {-.25f, -.5625f, 0}, {.125f, -.5625f, 0}, {.125f, -.4688f, 0},
                     {-.25f, -.4688f, 0},  {-.25f, -.1875f, 0}, {.25f, -.1875f, 0},  {.25f, -.3125f, 0},
                     {-.125f, -.3125f, 0}, {-.125f, -.375f, 0}, {.25f, -.375f, 0},   {.25f, -.6875f, 0},
                     // M
                     {.9375f, -.6875f, 0}, {.9375f, -.1875f, 0}, {.8125f, -.1875f, 0}, {.6875f, -.4375f, 0},
                     {.5625f, -.1875f, 0}, {.4375f, -.1875f, 0}, {.4375f, -.6875f, 0}, {.5625f, -.6875f, 0},
                     {.5625f, -.5f, 0},    {.6563f, -.6875f, 0}, {.7188f, -.6875f, 0}, {.8125f, -.5f, 0},  {.8125f, -.6875f, 0}
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

int a1Triangles[][3] = {
    {0, 6, 5}, {0, 1, 5}, {1, 5, 4}, {1, 4, 2}, {4, 3, 2}
};

int nTriangles[][3] = {
    {7, 16, 15},  {7, 8, 15},   {8, 9, 15},   {9, 15, 10},
    {15, 10, 14}, {10, 14, 13}, {10, 13, 12}, {10, 11, 12},
};

int a2Triangles[][3] = {
    {17, 23, 22}, {17, 18, 22}, {18, 22, 21}, {18, 21, 19}, {21, 20, 19},
};

int dTriangles[][3] = {
    {24, 25, 31}, {25, 30, 31}, {25, 30, 26}, {26, 29, 30},
    {26, 27, 29}, {27, 28, 29}, {27, 28, 24}, {24, 28, 31},
};

int sTriangles[][3] = {
    {32, 33, 43}, {33, 43, 34}, {34, 43, 42}, {34, 35, 42}, {35, 42, 36},
    {36, 42, 41}, {36, 41, 37}, {37, 41, 40}, {37, 40, 38}, {38, 40, 39},
};

int mTriangles[][3] = {
    {44, 45, 55}, {44, 55, 56}, {45, 46, 55}, {46, 47, 55}, {47, 55, 54},
    {47, 54, 53}, {47, 53, 52}, {47, 48, 52}, {48, 49, 52}, {49, 50, 52}, {50, 51, 52}
};

const int numLetters = 6;
float xOffSet[numLetters] = { 0.6875f, 0, -0.6875f, 0.6875f, 0, -0.6875f };

// Shaders: vertex shader with view transform, trivial pixel shader

const char *vertexShader = R"(
    #version 130
    in vec3 point;
    in vec3 color;
    out vec4 vColor;
    uniform mat4 view;
    void main() {
        gl_Position = view*vec4(point, 1);
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

// Initialization

void InitVertexBuffer() {
    // create GPU vertex buffer
    glGenBuffers(1, &vBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points)+sizeof(colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
}

// Animation

time_t starTime = clock();
float degPerSec = 30, setAngle = 0;

// Interaction

vec2  mouseDown;            // reference for mouse drag
vec2  rotOld, rotNew;       // .x is rotation about Y-axis, .y about X-axis
float rotZ = 0, scale = 1;
vec2  tranOld, tranNew;
float rotSpeed = .3f, tranSpeed = .01f;

void MouseWheel(GLFWwindow *w, double ignore, double spin) {
    bool shift = glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(w, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    if (shift)
        scale = scale * (spin > 0 ? 1.5 : 0.5);
    else
        rotZ += (spin > 0 ? 1 : -1)*2.5f;
}

void MouseButton(GLFWwindow *w, int butn, int action, int mods) {
    // called when mouse button pressed or released
    if (action == GLFW_PRESS) {
        double x, y;
        glfwGetCursorPos(w, &x, &y);
        mouseDown = vec2((float) x, (float) y); // save reference for MouseDrag
    }
    if (action == GLFW_RELEASE) {
        rotOld = rotNew;                        // save reference for MouseDrag
        tranOld = tranNew;
    }
}

void MouseMove(GLFWwindow *w, double x, double y) {
    if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) { // drag
        // find mouse drag difference
        vec2 mouse((float) x, (float) y), dif = mouse-mouseDown;
        bool shift = glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                     glfwGetKey(w, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        if (shift)
            tranNew = tranOld+tranSpeed*vec2(dif.x, -dif.y);    // SHIFT key: translate
        else
            rotNew = rotOld+rotSpeed*dif;                       // rotate
    }
}

// Application

void Display() {
    // clear screen to grey
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    // enable z-buffer (needed for tetrahedron)
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    // init shader program, set vertex feed for points and colors
    glUseProgram(program);
    VertexAttribPointer(program, "point", 3, 0, (void *) 0);
    VertexAttribPointer(program, "color", 3, 0, (void *) sizeof(points));
    // update view transformation
    // get angle to rotate on Y axis
    float dt = (float)(clock() - starTime) / CLOCKS_PER_SEC;
    setAngle = dt * degPerSec;
    
    for (int i = 0; i < numLetters; i++) {
        //mat4 view = Translate(-xOffSet[i], 0, 0) * RotateY(setAngle) * Translate(xOffSet[i], 0, 0);
        mat4 view = Scale(scale) * Translate(tranNew.x, tranNew.y, 0) * RotateY(rotNew.x) * RotateX(rotNew.y) * RotateZ(rotZ) 
            * Translate(-xOffSet[i], 0, 0) * RotateY(setAngle) * Translate(xOffSet[i], 0, 0);
        SetUniform(program, "view", view);

        switch (i) {
        case 0:
            glDrawElements(GL_TRIANGLES, sizeof(a1Triangles) / sizeof(int), GL_UNSIGNED_INT, &a1Triangles[0]);
            break;
        case 1:
            glDrawElements(GL_TRIANGLES, sizeof(nTriangles) / sizeof(int), GL_UNSIGNED_INT, &nTriangles[0]);
            break;
        case 2:
            glDrawElements(GL_TRIANGLES, sizeof(a2Triangles) / sizeof(int), GL_UNSIGNED_INT, &a2Triangles[0]);
            break;
        case 3:
            glDrawElements(GL_TRIANGLES, sizeof(dTriangles) / sizeof(int), GL_UNSIGNED_INT, &dTriangles[0]);
            break;
        case 4:
            glDrawElements(GL_TRIANGLES, sizeof(sTriangles) / sizeof(int), GL_UNSIGNED_INT, &sTriangles[0]);
            break;
        case 5:
            glDrawElements(GL_TRIANGLES, sizeof(mTriangles) / sizeof(int), GL_UNSIGNED_INT, &mTriangles[0]);
            break;
        }
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
    GLFWwindow* w = glfwCreateWindow(800, 800, "Rotate 3D Name", NULL, NULL);
    if (!w) {
        glfwTerminate();
        return 1;
    }
    glfwSetWindowPos(w, 100, 100);
    glfwMakeContextCurrent(w);
    // init OpenGL
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    printf("GL version: %s\n", glGetString(GL_VERSION));
    PrintGLErrors();
    // init shader programs
    program = LinkProgramViaCode(&vertexShader, &pixelShader);
    InitVertexBuffer();
    // callbacks
    glfwSetCursorPosCallback(w, MouseMove);
    glfwSetMouseButtonCallback(w, MouseButton);
    glfwSetScrollCallback(w, MouseWheel);
    glfwSetKeyCallback(w, Keyboard);
    // event loop
    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(w)) {
        Display();
        glfwPollEvents();
        glfwSwapBuffers(w);
    }
    Close();
    glfwDestroyWindow(w);
    glfwTerminate();
}
