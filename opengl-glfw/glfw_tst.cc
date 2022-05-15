
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <chrono>
#include <thread>
#include <iostream>

std::chrono::time_point<std::chrono::steady_clock> t1, t2;
std::chrono::duration<float> dt;

GLFWwindow* window;
bool  w1full = false;
int   w1[4] = {30, 30, 640, 480};
float transform[4];
float scale = 2.5f;
float pos[2] = {0.0f, 0.0f};
float mouse[2] = {0.0f, 0.0f};



void key_callback         (GLFWwindow* wnd, int key, int scancode, int action, int mods);
void scroll_callback      (GLFWwindow* wnd, double xoffset, double yoffset);
//void cursorpos_callback   (GLFWwindow* wnd, double xpos, double ypos);
//void mousebutton_callback (GLFWwindow* wnd, int button, int action, int mods);
void window_size_callback(GLFWwindow* wnd, int width, int height);

void compile_shaders ();



GLuint vshader = 0;
GLuint fshader = 0;
GLuint program = 0;

const char* vs = R"vertexshader(
#version 430

layout(location = 0) in vec2  pos;

void main ()
{
    gl_Position = vec4(pos, 0.0, 1.0);
}
)vertexshader";



const char* fs = R"fragmentshader(
#version 430

precision highp float;

uniform vec4 tr;
uniform vec2 mouse;
out vec4 color;

// HSV to RGB by Inigo Quilez https://www.shadertoy.com/view/lsS3Wc
vec3 hsv2rgb (in vec3 c)
{
    vec3 rgb = clamp(abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0);
    return c.z * mix(vec3(1.0), rgb, c.y);
}
// Mandelbrot
float mand(in float x, in float y)
{
    vec2 z = vec2(0.0);
    int i = 0;
    int n = 100;
    for (i=0 ; i<n ; ++i)
    {
        z = vec2(z.x*z.x - z.y*z.y + x, 2.0*z.x*z.y + y);
        if (length(z) > 2.0) break;
    }
    return (float(i)/float(n));
}
// Julia + M
vec3 col(in float x, in float y)
{
    vec2 z = vec2(x,y);
    vec2 c = vec2(mouse.x, mouse.y);
    int i = 0;
    int n = 500;
    for (i=0 ; i<n ; ++i)
    {
        z = vec2(z.x*z.x - z.y*z.y + c.x, 2.0*z.x*z.y + c.y);
        if (length(z) > 2.0) break;
    }
    float t = fract(float(i) / float(n) * 2.0);
    
    return hsv2rgb(vec3(0.1, 0.9, t)) + vec3(0.1*mand(x,y));
}

void main()
{
    float x = tr.x * gl_FragCoord.x + tr.y;
    float y = tr.z * gl_FragCoord.y + tr.w;
    color = vec4(col(x, y), 1.0);
}
)fragmentshader";



int main ()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
    
    //window = glfwCreateWindow(WIDTH, HEIGHT, "NewWorld", glfwGetPrimaryMonitor(), nullptr);
    window = glfwCreateWindow(w1[2], w1[3], "GLFW", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetWindowPos(window, w1[0], w1[1]);
    glfwSwapInterval(1);
    
    if (window == NULL)
    {
        std::cerr << "ERROR: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -2;
    }
    
    glfwSetKeyCallback        (window, key_callback);
    glfwSetScrollCallback     (window, scroll_callback);
    //glfwSetCursorPosCallback  (window, cursorpos_callback);
    //glfwSetMouseButtonCallback(window, mousebutton_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    float screen_quad[] = {
        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f, -1.0f,
        
        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };
    GLuint quadbuffer = 0;
    glGenBuffers(1, &quadbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
    glBufferData(GL_ARRAY_BUFFER, 2*6*sizeof(float), screen_quad, GL_STATIC_DRAW);
    
    compile_shaders();
    glUseProgram(program);
    GLint tr_location = glGetUniformLocation(program, "tr");
    GLint mm_location = glGetUniformLocation(program, "mouse");
    
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    
    while (!glfwWindowShouldClose(window))
    {
        t2 = std::chrono::steady_clock::now();
        dt = t2 - t1;
        
        /*
        float slp = 16666.6f - std::chrono::duration_cast<std::chrono::microseconds>(dt).count();
        if (slp > 200.0f)
        {
            //cout << "Sleep: " << slp << endl;
            //std::this_thread::sleep_for(std::chrono::microseconds((int)slp - 100));
        }
        */
    
        t2 = std::chrono::steady_clock::now();
        dt = t2 - t1;
        //std::cout << "dt: " << std::chrono::duration_cast<std::chrono::microseconds>(dt).count() << std::endl;
        
        t1 = t2;
        
        glfwPollEvents();
        
        glViewport(0, 0, w1[2], w1[3]);
        
        transform[0] =   scale/w1[3];
        transform[1] = -(w1[2]*scale)/(w1[3]*2.0)-pos[0];
        transform[2] =   scale/w1[3];
        transform[3] = - scale/2.0+pos[1];
        glUniform4fv(tr_location, 1, transform);
        glUniform2fv(mm_location, 1, mouse);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glfwSwapBuffers(window);
    }
    
    glDeleteBuffers(1, &quadbuffer);
    glDeleteVertexArrays(1, &vao);
    glfwTerminate();
    return 0;
}



void key_callback (GLFWwindow* /*wnd*/, int key, int /*scancode*/, int action, int mods)
{
    if (action != GLFW_PRESS) return;
    
    
    if (key == GLFW_KEY_ESCAPE) { glfwSetWindowShouldClose(window, GL_TRUE); return; }
    
    else if (key == (GLFW_KEY_C) && (mods & GLFW_MOD_CONTROL))
    {
        glfwSetClipboardString(NULL, "clipboard test");
        return;
    }
    
    else if (key == (GLFW_KEY_V) && (mods & GLFW_MOD_CONTROL))
    {
        std::cout << "Pasted: " << glfwGetClipboardString(NULL) << std::endl;
        return;
    }
    
    
    else if (key == GLFW_KEY_F11)
    {
        if (w1full)
        {
            glfwSetWindowMonitor(window, nullptr, 0, 0, w1[2], w1[3], 0);
            //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetWindowPos(window, w1[0], w1[1]);
            w1full = false;
        }
        else
        {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
            //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            w1full = true;
        }
    }
}
void scroll_callback      (GLFWwindow* /*wnd*/, double /*xoffset*/, double yoffset)
{
    //std::cout << yoffset << std::endl;
    if (yoffset < 0) { scale *= 0.8f; }
    else             { scale *= 1.25f; }
}
//void cursorpos_callback   (GLFWwindow* wnd, double xpos, double ypos);
//void mousebutton_callback (GLFWwindow* wnd, int button, int action, int mods);
void window_size_callback(GLFWwindow* /*wnd*/, int width, int height)
{
    std::cout << "SIZE: " << width << " - " << height << std::endl;
}

void compile_shaders ()
{
    GLint success = 0;
    GLint length = 0;
    vshader = glCreateShader(GL_VERTEX_SHADER);
    fshader = glCreateShader(GL_FRAGMENT_SHADER);
    program = glCreateProgram();
    GLchar* log = 0;
    
    
    //const GLchar* s1 = vsSource.c_str();
    glShaderSource(vshader, 1, &vs, NULL);
    glCompileShader(vshader);
    
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderiv(vshader, GL_INFO_LOG_LENGTH, &length);
        log = new GLchar[length+2];
        GLint x;
        glGetShaderInfoLog(vshader, length+1, &x, log);
        log[length+1] = 0;
        std::cout << "Vertex shader ERROR:" << std::endl << log << std::endl;
        delete[] log;
        return;
    }
    
    
    //const GLchar* s2 = fsSource.c_str();
    glShaderSource(fshader, 1, &fs, NULL);
    glCompileShader(fshader);
    
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderiv(fshader, GL_INFO_LOG_LENGTH, &length);
        log = new GLchar[length+2];
        GLint x;
        glGetShaderInfoLog(fshader, length+1, &x, log);
        log[length+1] = 0;
        std::cout << "Fragment shader ERROR:" << std::endl << std::endl;
        delete[] log;
        return;
    }

    
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);

    glLinkProgram(program);
    
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        log = new GLchar[length+2];
        GLint x;
        glGetProgramInfoLog(program, length+1, &x, log);
        log[length+1] = 0;
        std::cout << "Shader link ERROR:" << std::endl << log << std::endl;
        delete[] log;
        return;
    }
}
