/*
    CPSC 5700: Computer Graphics
    Ana Carolina de Souza Mendes

    BumpStore.cpp: use bump mapping to make a quadrilateral look like a stone wall
*/

#include <glad.h>
#include <glfw/glfw3.h>
#include <stdio.h>
#include <time.h>
#include "Camera.h"
#include "Draw.h"
#include "GLXtras.h"
#include "Misc.h"
#include "Text.h"
#include "Widgets.h"

const char *textureFilename = "./Stone_02_COLOR.jpg";
const char *normalFilename = "./Stone_02_NRM.jpg";

// GPU identifiers, app window, camera
GLuint vBuffer = 0, program = 0, textureId = 0, normalId = 0;
int textureUnit = 0, normalUnit = 1;
Camera camera(800, 800, vec3(0,0,0), vec3(0,0,-5));
time_t tEvent = clock();

// a quad
float s = .8f;
float pnts[][3] = {{-s,-s,0}, {-s,s, 0}, {s, s, 0}, {s,-s, 0}};
float nrms[][3] = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}};
float uaxs[][3] = {{1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}};
float vaxs[][3] = {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}};
float uvs[][2]  = {{0, 0},    {0, 1},    {1, 1},    {1, 0}};

// movable light
vec3 light(-.5f, .2f, .3f);
Mover lightMover;
void *picked = NULL, *hover = NULL;
int useTexture = 1;

const char *vertexShader = R"(
    #version 130
    in vec3 point;
    in vec3 normal;
    in vec3 uaxis;
    in vec3 vaxis;
    in vec2 uv;
    out vec3 vPoint;
    out vec3 vNormal;
    out vec3 vUaxis;
    out vec3 vVaxis;
    out vec2 vUv;
    uniform mat4 modelview;
    uniform mat4 persp;
    void main() {
        vPoint =  (modelview * vec4(point, 1)).xyz;
        vNormal = (modelview * vec4(normal, 0)).xyz;
        vUaxis =  (modelview * vec4(uaxis, 0)).xyz;
        vVaxis =  (modelview * vec4(vaxis, 0)).xyz;
        vUv = uv;
        gl_Position = persp * vec4(vPoint, 1);
    }
)";

const char *pixelShader = R"(
    #version 400
    in vec3 vPoint;
    in vec3 vNormal;
    in vec3 vUaxis;
    in vec3 vVaxis;
    in vec2 vUv;
    out vec4 pColor;
    uniform sampler2D textureMap;
    uniform sampler2D bumpMap;
    uniform vec3 light;
    uniform int useTexture = 1;

    float PhongIntensity(vec3 pos, vec3 nrm) {
        float a = .15f, d = 0, s = 0;
        vec3 N = normalize(nrm);            // surface normal
        vec3 E = normalize(pos);            // eye vertex
        vec3 L = normalize(light);
        d = abs(dot(N, L));                 // two-sided diffuse
        return clamp(a+d+s, 0, 1);
    }

    vec3 TransformToLocal(vec3 v, vec3 x, vec3 y, vec3 z) {
        float xx = v.x * x.x + v.y * y.x + v.z * z.x;
        float yy = v.x * x.y + v.y * y.y + v.z * z.y;
        float zz = v.x * x.z + v.y * y.z + v.z * z.z;
        return normalize(vec3(xx, yy, zz));
    }

    vec3 BumpNormal() {
        vec4 bumpV = texture(bumpMap, vUv);
        // map red, grn to [-1,1], blu to [0,1]
        vec3 bv = vec3(2 * bumpV.r - 1, 2 * bumpV.g - 1, bumpV.b);
        return normalize(bv);
    }

    void main() {
        vec3 b = BumpNormal();
        vec3 u = normalize(vUaxis), v = normalize(vVaxis);
        vec3 n = TransformToLocal(b, u, v, normalize(vNormal));
        vec3 color = vec3(1,1,1);
        if (useTexture == 1) color = texture(textureMap, vUv).rgb;
        float intensity = PhongIntensity(vPoint, b);
        pColor = vec4(intensity * color, 1);
    }
)";

void Display() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    // access GPU vertex buffer
    glUseProgram(program);
    glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
    // maps
    glActiveTexture(GL_TEXTURE0+textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureId);
    SetUniform(program, "textureMap", textureUnit);
    glActiveTexture(GL_TEXTURE0+normalUnit);
    glBindTexture(GL_TEXTURE_2D, normalId);
    SetUniform(program, "bumpMap", normalUnit);
    // associate position input to shader with position array in vertex buffer
    VertexAttribPointer(program, "point", 3, 0, (void *) 0);
    VertexAttribPointer(program, "normal", 3,  0, (void *) sizeof(pnts));
    VertexAttribPointer(program, "uaxis", 3, 0, (void *) (2*sizeof(pnts)));
    VertexAttribPointer(program, "vaxis", 3, 0, (void *) (3*sizeof(pnts)));
    VertexAttribPointer(program, "uv", 2, 0, (void *) (4*sizeof(pnts)));
    // transform light
    vec4 xl(camera.modelview*vec4(light, 0.f));
    vec3 xlight(xl.x, xl.y, xl.z);
    // set uniforms, render quad
    SetUniform(program, "modelview", camera.modelview);
    SetUniform(program, "persp", camera.persp);
    SetUniform3v(program, "light", 1, &xlight.x);
    SetUniform(program, "useTexture", useTexture);
    glDrawArrays(GL_QUADS, 0, 4);
    // draw light sources
    UseDrawShader(camera.fullview);
    glDisable(GL_DEPTH_TEST);
    if ((float) (clock()-tEvent)/CLOCKS_PER_SEC < 1)
        Disk(light, 12, IsVisible(light, camera.fullview) ? vec3(1,0,0) : vec3(0,0,1));
    glFlush();
}

// Mouse Handlers
bool Shift(GLFWwindow *w) {
    return glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
           glfwGetKey(w, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
}

int WindowHeight(GLFWwindow *w) {
    int width, height;
    glfwGetWindowSize(w, &width, &height);
    return height;
}

void MouseButton(GLFWwindow *w, int butn, int action, int mods) {
    double x, y;
    glfwGetCursorPos(w, &x, &y);
    y = WindowHeight(w)-y;          // invert y for upward-increasing screen space
    picked = NULL;
    if (action == GLFW_PRESS) {
        float dsq = ScreenDistSq(x, y, light, camera.fullview);
        if (dsq < 150) {
            picked = &lightMover;
            lightMover.Down(&light, (int) x, (int) y, camera.modelview, camera.persp);
        }
        if (picked == NULL) {
            picked = &camera;
            camera.MouseDown(x, y);
        }
    }
    if (action == GLFW_RELEASE)
        camera.MouseUp();
}

void MouseMove(GLFWwindow *w, double x, double y) {
    tEvent = clock();
    if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) { // drag
        y = WindowHeight(w)-y; // invert y for upward-increasing screen space
        if (picked == &lightMover)
            lightMover.Drag((int) x, (int) y, camera.modelview, camera.persp);
        else
            camera.MouseDrag(x, y, Shift(w));
    }
}

void MouseWheel(GLFWwindow *w, double ignore, double spin) {
    camera.MouseWheel(spin > 0, Shift(w));
}

void Resize(GLFWwindow *w, int width, int height) {
    camera.Resize(width, height);
    glViewport(0, 0, width, height);
}

void Keyboard(GLFWwindow *w, int c, int scancode, int action, int mods) {
    if (action == GLFW_PRESS)
        switch (c) {
            case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(w, GLFW_TRUE); break;
            case 'T': useTexture = 1-useTexture; break;
            default: break;
        }
}

const char *usage = "\
      T: toggle texture\n\
";

void InitVertexBuffer() {
    // init vertex buffer
    glGenBuffers(1, &vBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
    int sPnts = sizeof(pnts), sUvs = sizeof(uvs);
    glBufferData(GL_ARRAY_BUFFER, 4 * sPnts + sUvs, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sPnts, pnts);
    glBufferSubData(GL_ARRAY_BUFFER, sPnts, sPnts, nrms);
    glBufferSubData(GL_ARRAY_BUFFER, 2 * sPnts, sPnts, uaxs);
    glBufferSubData(GL_ARRAY_BUFFER, 3 * sPnts, sPnts, vaxs);
    glBufferSubData(GL_ARRAY_BUFFER, 4 * sPnts, sUvs, uvs);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4); // anti-alias
    GLFWwindow *w = glfwCreateWindow(800, 800, "Bump-mapped Stone", NULL, NULL);
    glfwMakeContextCurrent(w);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    program = LinkProgramViaCode(&vertexShader, &pixelShader);
    int width, height;
    glfwGetWindowSize(w, &width, &height);
    InitVertexBuffer();
    // init bump map
    textureId = LoadTexture(textureFilename, textureUnit);
    normalId = LoadTexture(normalFilename, normalUnit);
    // callbacks
    glfwSetCursorPosCallback(w, MouseMove);
    glfwSetMouseButtonCallback(w, MouseButton);
    glfwSetScrollCallback(w, MouseWheel);
    glfwSetKeyCallback(w, Keyboard);
    glfwSetWindowSizeCallback(w, Resize);
    printf("Usage:\n%s\n", usage);
    while (!glfwWindowShouldClose(w)) {
        Display();
        glfwSwapBuffers(w);
        glfwPollEvents();
    }
    glDeleteBuffers(1, &textureId);
    glfwDestroyWindow(w);
    glfwTerminate();
}
