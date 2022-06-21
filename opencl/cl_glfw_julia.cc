
#include <iostream>
#include <iomanip>
#include <string>

#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))



GLFWwindow* window;
bool  window_fullscreen = false;
int   w1[4] = {30, 100, 640, 480};
int   w2[4] = {30, 100, 640, 480};

float* screenbuf;
size_t workunits[2];

float trm[6];
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



GLuint vao     = 0;
GLuint vshader = 0;
GLuint fshader = 0;
GLuint program = 0;
GLuint tex     = 0;
GLuint quadbuffer = 0;

cl_mem screen_d, trm_d;
cl_context context;





std::string kernel_str = R"kernelstr(

// HSV to RGB by Inigo Quilez https://www.shadertoy.com/view/lsS3Wc
float3 hsv2rgb (float3 c)
{
    float3 rgb = clamp( fabs(fmod( (float3)(c.x*6.0) + (float3)(0.0,4.0,2.0), 6.0 )-(float3)(3.0)) - (float3)(1.0), (float3)(0.0), (float3)(1.0) );
    return c.z * mix((float3)(1.0), rgb, c.y);
}
// Mandelbrot
float mand(float x, float y)
{
    float2 z = 0.0;
    int i = 0;
    int n = 100;
    for (i=0 ; i<n ; ++i)
    {
        z = (float2)(z.x*z.x - z.y*z.y + x, 2.0*z.x*z.y + y);
        if (length(z) > 2.0) break;
    }
    return ( (float)(i) / (float)(n) );
}
// Julia + M
float3 col(float x, float y, float mx, float my)
{
    float2 z = (float2)(x,y);
    float2 c = (float2)(mx, my);
    int i = 0;
    int n = 500;
    for (i=0 ; i<n ; ++i)
    {
        z = (float2)(z.x*z.x - z.y*z.y + c.x, 2.0*z.x*z.y + c.y);
        if (length(z) > 2.0) break;
    }
    float flo;
    float t = fract( (float)(i) / (float)(n) * 2.0, &flo );
    
    return hsv2rgb( (float3)(0.1, 0.9, t) ) + (float3)(0.1*mand(x,y));
}

kernel void main(global float* screen, constant float* trm)
{
    int i = get_global_id(0);
    int j = get_global_id(1);
    int w = get_global_size(0);

    float x = trm[0] * i + trm[1];
    float y = trm[2] * j + trm[3];
    float3  c = col(x, y, trm[4], trm[5]);
    
    screen[(j*w+i)*4 + 0] = c.x;
    screen[(j*w+i)*4 + 1] = c.y;
    screen[(j*w+i)*4 + 2] = c.z;
    screen[(j*w+i)*4 + 3] = 1.0;
}
)kernelstr";

const char* vs = R"vertexshader(
#version 430 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 texin;
out vec2 texf;

void main ()
{
    gl_Position = vec4(pos, 0.0, 1.0);
    texf = texin;
}
)vertexshader";



const char* fs = R"fragmentshader(
#version 430 core

precision mediump float;

in vec2 texf;
out vec4 col;
uniform sampler2D texsampler;

void main()
{
    col = texture(texsampler, texf);
}
)fragmentshader";


void compile_shaders ();


int glfw_init()
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
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    float screen_quad[] = {
        -1.0f,  1.0f,    0.0f, 1.0f,
        -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,    1.0f, 0.0f,
        
        -1.0f,  1.0f,    0.0f, 1.0f,
         1.0f, -1.0f,    1.0f, 0.0f,
         1.0f,  1.0f,    1.0f, 1.0f
    };
    
    glGenBuffers(1, &quadbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
    glBufferData(GL_ARRAY_BUFFER, 4*6*sizeof(float), screen_quad, GL_STATIC_DRAW);
    
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    compile_shaders();
    glUseProgram(program);
    
    return 0;
}

void cl_init()
{
    
}

int main()
{
    int succ = glfw_init();
    if (succ != 0) return succ;
    
    cl_int errNum;
    
    cl_platform_id platform;
    errNum = clGetPlatformIDs(1, &platform, NULL);
    
    char info[200];
    errNum = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 200, info, NULL);
    std::cout << "Platform name: " << info << std::endl;
    
    cl_device_id device;
    errNum = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
    
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &errNum);
    if (errNum != CL_SUCCESS) { std::cout << "ERROR: creating context" << std::endl; return -1; }
    
    cl_program program;
    const char* kc = kernel_str.c_str();
    size_t kclen   = kernel_str.size();
    program = clCreateProgramWithSource(context, 1, &kc, &kclen, &errNum);
    if (errNum != CL_SUCCESS) { std::cout << "ERROR: creating program" << std::endl; return -2; }
    
    errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (errNum != CL_SUCCESS)
    {
        char buildLog[50000];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);
        std::cout << "ERROR building kernels: " << std::endl;
        std::cout << buildLog << std::endl;
        clReleaseProgram(program);
        clReleaseContext(context);
        return -1;
    }
    
    cl_kernel kernel;
    kernel = clCreateKernel(program, "main", &errNum);
    
    cl_command_queue queue;
    queue = clCreateCommandQueue(context, device, 0, &errNum);
    
    trm_d    = clCreateBuffer(context, CL_MEM_READ_ONLY,              6*sizeof(float), NULL, &errNum);
    screen_d = clCreateBuffer(context, CL_MEM_WRITE_ONLY, w1[2]*w1[3]*4*sizeof(float), NULL, &errNum);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &screen_d);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &trm_d);
    
    
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), BUFFER_OFFSET(0) );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), BUFFER_OFFSET(2 * sizeof(float)) );
    
    screenbuf = new float[ w1[2]*w1[3]*4 ];
    
    //glBindTexture(GL_TEXTURE_2D, tex);
    //float tst = 0.0f;
    //for (int i=0 ; i<w1[2]*w1[3] ; ++i) { screenbuf[i*4] = tst; screenbuf[i*4+1] = tst; screenbuf[i*4+2] = tst; screenbuf[i*4+3] = 1.0f; tst+=1.0f/256.0f; if (tst > 1.0f) { tst = 0.0f; } }
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w1[2], w1[3], 0, GL_RGBA, GL_FLOAT, screenbuf);
    
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        
        trm[0] =   scale/w1[3];
        trm[1] = -(w1[2]*scale)/(w1[3]*2.0)-pos[0];
        trm[2] =   scale/w1[3];
        trm[3] = - scale/2.0+pos[1];
        //trm[4] = mouse[0];
        //trm[5] = mouse[1];
        
        workunits[0] = w1[2];
        workunits[1] = w1[3];
        
        clEnqueueWriteBuffer(queue, trm_d, CL_TRUE, 0, 6*sizeof(float), trm, 0, NULL, NULL);
        clEnqueueNDRangeKernel(queue, kernel, 2, NULL, workunits, NULL, 0, NULL, NULL);
        clFinish(queue);
        clEnqueueReadBuffer(queue, screen_d, CL_TRUE, 0, w1[2]*w1[3]*4*sizeof(float), screenbuf, 0, NULL, NULL);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w1[2], w1[3], 0, GL_RGBA, GL_FLOAT, screenbuf);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glfwSwapBuffers(window);
    }
    
    clReleaseMemObject(screen_d);
    clReleaseMemObject(trm_d);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);
    
    delete[] screenbuf;
    
    glDeleteBuffers(1, &quadbuffer);
    glDeleteTextures(1, &tex);
    glDeleteVertexArrays(1, &vao);
    glfwTerminate();
    
    return 0;
}


void resize(int width, int height)
{
    w1[2] = width;
    w1[3] = height;
    glViewport(0, 0, w1[2], w1[3]);
    
    delete[] screenbuf;
    screenbuf = new float[ w1[2]*w1[3]*4 ];
    
    cl_int errNum;
    clReleaseMemObject(screen_d);
    screen_d = clCreateBuffer(context, CL_MEM_WRITE_ONLY, w1[2]*w1[3]*4*sizeof(float), NULL, &errNum);
    
    glDeleteTextures(1, &tex);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}


void key_callback (GLFWwindow* /*wnd*/, int key, int /*scancode*/, int action, int mods)
{
    if (action != GLFW_PRESS) return;
    
    
    if (key == GLFW_KEY_ESCAPE) { glfwSetWindowShouldClose(window, GL_TRUE); return; }
    
    else if (key == (GLFW_KEY_C) && (mods & GLFW_MOD_CONTROL))
    {
        glfwSetClipboardString(NULL, "clipboard test");
        std::cout << "Clipboard updated" << std::endl;
        return;
    }
    
    else if (key == (GLFW_KEY_V) && (mods & GLFW_MOD_CONTROL))
    {
        std::cout << "Pasted: " << glfwGetClipboardString(NULL) << std::endl;
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
            
            resize(w1[2], w1[3]);
            
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
            
            resize(w1[2], w1[3]);
            
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
        trm[4] = trm[0] * float(xpos) + trm[1];
        trm[5] = trm[2] * (w1[3]-float(ypos)) + trm[3];
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
    resize(width, height);
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
