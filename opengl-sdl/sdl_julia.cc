
#include <chrono>
#include <thread>
#include <iostream>

#include <glad/glad.h>
#include <SDL2/SDL.h>

//std::chrono::time_point<std::chrono::steady_clock> t1, t2;
//std::chrono::duration<float> dt;

SDL_Window*   window  = 0;
SDL_Renderer* sdl_rrr = 0;
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

GLuint vshader = 0;
GLuint fshader = 0;
GLuint program = 0;
void compile_shaders ();

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



inline bool numeric (char c)
{
    return ( (c>='0' && c<='9') || (c=='.') || (c=='-') );
}



int main()
{
    int ret = SDL_Init(SDL_INIT_EVERYTHING);
    if(ret != 0) { return -1; }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    //SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        
    window = SDL_CreateWindow("SDL2", w1[0], w1[1], w1[2], w1[3], SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if(window == 0) { std::cout << SDL_GetError() << std::endl; SDL_ClearError(); return -1; }
    
    sdl_rrr = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED/* | SDL_RENDERER_PRESENTVSYNC*/);
    if(sdl_rrr == 0) { std::cout << SDL_GetError() << std::endl; SDL_ClearError(); return -2; }

    //SDL_SetRenderDrawColor(sdl_rrr, 0, 0, 0, 255);
    //SDL_RenderClear(sdl_rrr);
    //SDL_RenderPresent(sdl_rrr);
    
    SDL_GLContext context;
    context = SDL_GL_CreateContext(window);
    if ( !gladLoadGLLoader(SDL_GL_GetProcAddress) )
    {
        std::cout << "ERROR: glad init" << std::endl; return -3;
    }
    
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
    
    glViewport(0, 0, w1[2], w1[3]);
    
    // vsync or manual sleep didn't work in X11
    //t1 = std::chrono::steady_clock::now();
    while (true)
    {
        /*
        t2 = std::chrono::steady_clock::now();
        dt = t2 - t1;
        
        float slp = 16666.6f - std::chrono::duration_cast<std::chrono::microseconds>(dt).count();
        if (slp > 200.0f)
        {
            std::this_thread::sleep_for(std::chrono::microseconds((int)slp - 1000));
        }
    
        t2 = std::chrono::steady_clock::now();
        dt = t2 - t1;
        
        t1 = t2;
        */
        
        SDL_Event event;
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                break;
            
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                {
                    break;
                }
                else if (event.key.keysym.scancode == SDL_SCANCODE_Q)
                {
                    param_freeze = !param_freeze;
                }
                else if (event.key.keysym.scancode == SDL_SCANCODE_F11)
                {
                    if (window_fullscreen)
                    {
                        w1[0] = w2[0];
                        w1[1] = w2[1];
                        w1[2] = w2[2];
                        w1[3] = w2[3];
                        SDL_SetWindowFullscreen(window, 0);
                        glViewport(0, 0, w1[2], w1[3]);
                        
                        window_fullscreen = false;
                    }
                    else
                    {
                        w2[0] = w1[0];
                        w2[1] = w1[1];
                        w2[2] = w1[2];
                        w2[3] = w1[3];
                        SDL_DisplayMode mode;
                        SDL_GetCurrentDisplayMode(SDL_GetWindowDisplayIndex(window), &mode);
                        SDL_SetWindowDisplayMode(window, &mode);
                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                        w1[2] = mode.w;
                        w1[3] = mode.h;
                        glViewport(0, 0, w1[2], w1[3]);
                        
                        window_fullscreen = true;
                    }
                }
                else if (event.key.keysym.scancode == SDL_SCANCODE_C && ((SDL_GetModState() & KMOD_CTRL) != 0)) //Ctrl-C
                {
                    std::string params = "(" + std::to_string(mouse[0]) + ", " + std::to_string(mouse[1]) + ")";
                    SDL_SetClipboardText(params.c_str());
                    std::cout << "To clipboard: " << params << std::endl;
                }
                else if (event.key.keysym.scancode == SDL_SCANCODE_V && ((SDL_GetModState() & KMOD_CTRL) != 0)) //Ctrl-V
                {
                    if (SDL_HasClipboardText() )
                    {
                        std::string params = SDL_GetClipboardText();
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
                    }
                }
            }
            
            else if (event.type == SDL_MOUSEWHEEL)
            {
                if (event.wheel.y > 0) { scale *= 0.8f;  }
                else                   { scale *= 1.25f; }
            }

            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
            {
                grabbed = true;
            }

            else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
            {
                grabbed = false;
            }

            else if (event.type == SDL_MOUSEMOTION)
            {
                if (grabbed)
                {
                    float a = scale/w1[3];
                    pos[0] += float(event.motion.xrel) * a;
                    pos[1] += float(event.motion.yrel) * a;
                }
                else if (!param_freeze)
                {
                    mouse[0] = transform[0] *        float(event.motion.x)  + transform[1];
                    mouse[1] = transform[2] * (w1[3]-float(event.motion.y)) + transform[3];
                }
            }
            
            else if (event.type == SDL_WINDOWEVENT)
            {
                if (event.window.event == SDL_WINDOWEVENT_MOVED)
                {
                    w1[0] = event.window.data1;
                    w1[1] = event.window.data2;
                }
                else if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    w1[2] = event.window.data1;
                    w1[3] = event.window.data2;
                    glViewport(0, 0, w1[2], w1[3]);
                }
            }
        }
        
        transform[0] =   scale/w1[3];
        transform[1] = -(w1[2]*scale)/(w1[3]*2.0)-pos[0];
        transform[2] =   scale/w1[3];
        transform[3] = - scale/2.0+pos[1];
        glUniform4fv(tr_location, 1, transform);
        glUniform2fv(mm_location, 1, mouse);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        SDL_GL_SwapWindow(window);
    }
    
    
    glDeleteBuffers(1, &quadbuffer);
    glDeleteVertexArrays(1, &vao);
    
    SDL_DestroyRenderer(sdl_rrr);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
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
