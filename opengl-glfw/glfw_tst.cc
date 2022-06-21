
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


GLFWwindow* window;
bool  window_fullscreen = false;
int   w1[4] = {30, 100, 640, 480};
int   w2[4] = {30, 100, 640, 480};
float transform[4];
float scale    = 2.5f;
float pos[2]   = {0.0f, 0.0f};
float mouse[2] = {0.0f, 0.0f};
double mxold, myold;
bool grabbed = false;
bool param_freeze = false;



void key_callback         (GLFWwindow* wnd, int key, int scancode, int action, int mods);
void scroll_callback      (GLFWwindow* wnd, double xoffset, double yoffset);
void cursorpos_callback   (GLFWwindow* wnd, double xpos, double ypos);
void mousebutton_callback (GLFWwindow* wnd, int button, int action, int mods);
void window_size_callback (GLFWwindow* wnd, int width, int height);

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
    
    window = glfwCreateWindow(w1[2], w1[3], "GLFW", nullptr, nullptr);
    glfwSetWindowPos(window, w1[0], w1[1]);
    
    glfwMakeContextCurrent(window);
    
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
    glfwSetCursorPosCallback  (window, cursorpos_callback);
    glfwSetMouseButtonCallback(window, mousebutton_callback);
    glfwSetWindowSizeCallback (window, window_size_callback);
    
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
        glfwPollEvents();
        
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


inline bool numeric (char c)
{
    return ( (c>='0' && c<='9') || (c=='.') || (c=='-') );
}

void key_callback (GLFWwindow* /*wnd*/, int key, int /*scancode*/, int action, int mods)
{
    if (action != GLFW_PRESS) return;
    
    
    if (key == GLFW_KEY_ESCAPE) { glfwSetWindowShouldClose(window, GL_TRUE); return; }
    
    else if (key == (GLFW_KEY_C) && (mods & GLFW_MOD_CONTROL))
    {
        std::string params = "(" + std::to_string(mouse[0]) + ", " + std::to_string(mouse[1]) + ")";
        glfwSetClipboardString(NULL, params.c_str());
        std::cout << "To clipboard: " << params << std::endl;
        return;
    }
    
    else if (key == (GLFW_KEY_V) && (mods & GLFW_MOD_CONTROL))
    {
        std::string params = glfwGetClipboardString(NULL);
        size_t comma = params.find(',');
        if (comma != std::string::npos)
        {
            std::string m00 = params.substr(0,comma);
            std::string m11 = params.substr(comma+1);
            std::string m0, m1;
            for (size_t i=0 ; i<m00.size() ; ++i) { if (numeric(m00[i])) { m0 += m00[i]; } }
            for (size_t i=0 ; i<m11.size() ; ++i) { if (numeric(m11[i])) { m1 += m11[i]; } }
            
            mouse[0] = std::atof(m0.c_str());
            mouse[1] = std::atof(m1.c_str());
            param_freeze = true;
            
            std::cout << "Pasted: (" << mouse[0] << ", " << mouse[1] << ")" << std::endl;
        }
        else
        {
            std::cout << "Couldn't parse: " << params << std::endl;
        }
        return;
    }
    
    else if (key == (GLFW_KEY_Q))
    {
        param_freeze = !param_freeze;
        return;
    }
    
    else if (key == GLFW_KEY_F11)
    {
        if (window_fullscreen)
        {
            w1[0] = w2[0];
            w1[1] = w2[1];
            w1[2] = w2[2];
            w1[3] = w2[3];
            glfwSetWindowMonitor(window, nullptr, w1[0], w1[1], w1[2], w1[3], 0);
            glViewport(0, 0, w1[2], w1[3]);
            
            //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            window_fullscreen = false;
        }
        else
        {
            w2[0] = w1[0];
            w2[1] = w1[1];
            w2[2] = w1[2];
            w2[3] = w1[3];
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
            w1[2] = mode->width;
            w1[3] = mode->height;
            glViewport(0, 0, w1[2], w1[3]);
            
            //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            window_fullscreen = true;
        }
    }
}
void scroll_callback      (GLFWwindow* /*wnd*/, double /*xoffset*/, double yoffset)
{
    //std::cout << "Scroll Y: " << yoffset << std::endl;
    if (yoffset > 0) { scale *= 0.8f;  }
    else             { scale *= 1.25f; }
}
void cursorpos_callback   (GLFWwindow* /*wnd*/, double xpos, double ypos)
{
    //std::cout << "M " << xpos << " - " << ypos << std::endl;
    if (grabbed)
    {
        float a = scale/w1[3];
        pos[0] += float(xpos-mxold) * a;
        pos[1] += float(ypos-myold) * a;
        mxold = xpos;
        myold = ypos;
    }
    else if (!param_freeze)
    {
        mouse[0] = transform[0] * float(xpos) + transform[1];
        mouse[1] = transform[2] * (w1[3]-float(ypos)) + transform[3];
    }
}
void mousebutton_callback (GLFWwindow* /*wnd*/, int /*button*/, int action, int /*mods*/)
{
    //std::cout << "MB " << action << std::endl;
    if   (action == GLFW_PRESS)
    {
        grabbed = true;
        glfwGetCursorPos(window, &mxold, &myold);
    }
    else
    {
        grabbed = false;
    }
}
void window_size_callback(GLFWwindow* /*wnd*/, int width, int height)
{
    //std::cout << "SIZE: " << width << " - " << height << std::endl;
    w1[2] = width;
    w1[3] = height;
    glViewport(0, 0, w1[2], w1[3]);
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
