#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <chrono>
#include "tesseract.h"
#include "hypercube.h"

enum Mode { PARTICLES=0, TESSERACT=1, HYPERCUBE5D=2 };

static int g_mode = PARTICLES;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_4) g_mode = TESSERACT;
    if (key == GLFW_KEY_5) g_mode = HYPERCUBE5D;
}

GLuint compileShader(GLenum type, const char* src){
    GLuint s = glCreateShader(type);
    glShaderSource(s,1,&src,nullptr);
    glCompileShader(s);
    GLint ok=0; glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(!ok){char buf[512]; glGetShaderInfoLog(s,512,nullptr,buf); std::cerr<<"Shader compile: "<<buf<<"\n";}
    return s;
}

int main(){
    if(!glfwInit()){ std::cerr<<"glfwInit failed\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(1024,768,"Lorenz Visuals",nullptr,nullptr);
    if(!win){ std::cerr<<"Failed create window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glfwSetKeyCallback(win, key_callback);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ std::cerr<<"Failed to init GLAD\n"; return -1; }

    const char* vsrc = "#version 330 core\nlayout(location=0) in vec3 a; void main(){ gl_Position = vec4(a,1.0); }";
    const char* fsrc = "#version 330 core\nout vec4 o; void main(){ o = vec4(1.0,1.0,1.0,1.0); }";
    GLuint vs = compileShader(GL_VERTEX_SHADER,vsrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER,fsrc);
    GLuint prog = glCreateProgram(); glAttachShader(prog,vs); glAttachShader(prog,fs); glLinkProgram(prog);

    GLuint vao, vbo; glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);

    auto tp0 = std::chrono::high_resolution_clock::now();
    while(!glfwWindowShouldClose(win)){
        auto tp = std::chrono::high_resolution_clock::now();
        float t = std::chrono::duration<float>(tp - tp0).count();

        std::vector<float> verts;
        if (g_mode == TESSERACT) verts = generate_tesseract_lines(t);
        else if (g_mode == HYPERCUBE5D) verts = generate_5d_hypercube_lines(t);
        else { // particles placeholder: show a rotating triangle
            verts = { cosf(t)*0.5f, -0.4f, 0.0f, -0.4f, 0.4f, 0.0f, 0.4f, -0.4f, 0.0f };
        }

        // upload
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_DYNAMIC_DRAW);

        glViewport(0,0,1024,768);
        glClearColor(0.05f,0.05f,0.08f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);
        glBindVertexArray(vao);
        if (g_mode == PARTICLES) glDrawArrays(GL_TRIANGLES,0, (GLsizei)(verts.size()/3));
        else glDrawArrays(GL_LINES,0, (GLsizei)(verts.size()/3));

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteBuffers(1,&vbo); glDeleteVertexArrays(1,&vao); glDeleteProgram(prog);
    glfwDestroyWindow(win); glfwTerminate();
    return 0;
}
