/*
    CPSC 5700: Computer Graphics
    Ana Carolina de Souza Mendes

    FrogIllumination.cpp: implement different lighting techniques
    Reference: OpenGL programming guide - 9th ed.
*/

#include <glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <vector>
#include "GLXtras.h"
#include "Mesh.h"
#include "Misc.h"               // LoadTexture

// Application Data
const char* objFilename = "./frog.obj";

std::vector<vec3> points;     // 3D mesh vertices for frog
std::vector<vec3> normals;    // 3D mesh normals for frog
std::vector<int3> triangles;  // triplets of vertex indices for frog
std::vector<vec2> uvs;        // uvs for frog

GLuint vBuffer = 0;   // GPU vertex buffer ID
GLuint program = 0;   // GLSL program ID

int lightModel = 1;
const int MAX_LIGHT_MODEL = 4;

// Camera
int winW = 400, winH = 400;
float aspectRatio = (float)(winW) / (float)(winH);
Camera camera(aspectRatio, vec3(0, 0, 0), vec3(0, 0, -5));

// Shaders: vertex shader with view transform, trivial pixel shader
const char *vertexShader = R"(
    #version 330 core
    in vec3 point;
    in vec3 normal;
    in vec2 uv;
    out vec3 vPoint;
    out vec3 vNormal;
    out vec2 vuv;
    uniform mat4 modelview;
    uniform mat4 persp;
    void main() {
        vuv = uv;
        vPoint = (modelview * vec4(point, 1)).xyz;
        vNormal = (modelview * vec4(normal, 0)).xyz;
        gl_Position = persp * vec4(vPoint, 1);
    }
)";

const char *pixelShader = R"(
    #version 330 core
    in vec3 vPoint;
    in vec3 vNormal;
    in vec2 vuv;
    out vec4 pColor;
    uniform vec4 ambient;
    uniform float a = .2;
    uniform vec3 lightPos = vec3(0,1,0); // 3D light location w/ default
    uniform int lightModel = 1;

    float PhongShading() {
        vec3 N = normalize(vNormal);
        vec3 L = normalize(lightPos - vPoint);
        vec3 E = normalize(vPoint);
        vec3 R = reflect(L, N);
        float d = abs(dot(N, L));
        float h = max(0, dot(R, E));
        float s = pow(h, 100);
        float intensity = clamp(a+d+s, 0, 1);

        return intensity;
    }

    vec3 ambientLight(vec3 color) {
        vec3 scatteredLight = ambient.xyz;
        return vec3(color.rgb * scatteredLight);
    }


    void main() {
        float intensity = 1;
        vec3 color = vec3(.7, 1, 0);

        if(lightModel == 2) {
            color = ambientLight(color);
        }
        else if(lightModel == 3) {
            color = ambientLight(color);
            intensity = PhongShading();
        }

        pColor = vec4(intensity * color, 1);
    }
)";

// Initialization

void InitVertexBuffer() {
    // create GPU vertex buffer
    glGenBuffers(1, &vBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
    // allocate memory for vertex positions
    int sizePts = points.size() * sizeof(vec3);
    int sizeUVs = uvs.size() * sizeof(vec2);
    int sizeNormals = normals.size() * sizeof(vec3);
    int totalSize = sizePts + sizeUVs + sizeNormals;
    glBufferData(GL_ARRAY_BUFFER, totalSize, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizePts, &points[0]);
    glBufferSubData(GL_ARRAY_BUFFER, sizePts, sizeNormals, &normals[0]);
    glBufferSubData(GL_ARRAY_BUFFER, sizePts + sizeNormals, sizeUVs, &uvs[0]);
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
    camera.Resize(winW = width, winH = height);
    glViewport(0, 0, winW, winH);
}

void Keyboard(GLFWwindow* w, int c, int scancode, int action, int mods) {
    if (action == GLFW_PRESS)
        switch (c) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(w, GLFW_TRUE);
            break;
        case 'L':
            lightModel++;
            if (lightModel > MAX_LIGHT_MODEL)
                lightModel = 1;
            break;
        default: break;
        }
}

const char* usage = "\
      L: toggle light\n\
         - No lighting\n\
         - Ambient lighting\n\
         - Phong shading\n\
    ";

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

    // get screen size to support resizing
    glfwGetWindowSize(w, &winW, &winH);

    mat4 modelview = Translate(0.5f, -0.5f, 0) * camera.modelview;
    vec4 ambient = vec4(0.7, 0.1, 0.8, 1);

    // update matrices
    SetUniform(program, "modelview", modelview);
    SetUniform(program, "persp", camera.persp);
    SetUniform(program, "lightModel", lightModel);
    SetUniform(program, "ambient", ambient);
    
    int sizePts = points.size() * sizeof(vec3);
    VertexAttribPointer(program, "point", 3, 0, (void *) 0);
    VertexAttribPointer(program, "normal", 3, 0, (void *) sizePts);
    VertexAttribPointer(program, "uv", 2, 0, (void *) (sizePts * 2));

    // draw shaded frog on the bottom right
    glViewport(0, 0, winW, winH);
    glDrawElements(GL_TRIANGLES, 3 * triangles.size(), GL_UNSIGNED_INT, &triangles[0]);

    // draw shaded frog on the top left
    modelview = Translate(-0.5f, 0.5f, 0) * camera.modelview;
    SetUniform(program, "modelview", modelview);
    glDrawElements(GL_TRIANGLES, 3 * triangles.size(), GL_UNSIGNED_INT, &triangles[0]);

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

int main(int argc, char **argv) {
    // init app window
    if (!glfwInit())
        return 1;
    glfwSetErrorCallback(ErrorGFLW);
    GLFWwindow* w = glfwCreateWindow(winW, winH, "Frog Illumination", NULL, NULL);
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
    if (!ReadAsciiObj((char *) objFilename, points, triangles, &normals, &uvs)) {
        printf("failed to read obj file (type any key to continue)\n");
        getchar();
        return 1;
    }
    printf("%i vertices, %i triangles, %i normals, %i uvs\n", points.size(), triangles.size(), normals.size(), uvs.size());
    Normalize(points, .8f);

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
    printf("Usage:\n%s\n", usage);
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
