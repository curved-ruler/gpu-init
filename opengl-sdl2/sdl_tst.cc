
#include <glad/glad.h>
#include <SDL2/SDL.h>

SDL_Window*   window  = 0;
//SDL_Renderer* sdl_rrr = 0;

int main()
{
    int ret = SDL_Init(SDL_INIT_EVERYTHING);
    if(ret != 0) { return -1; }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    //SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        
    window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
    if(window == 0) { return -2; }
    
    //sdl_rrr = SDL_CreateRenderer(window, -1, 0);
    //if(sdl_rrr == 0) { return -3; }

    //SDL_SetRenderDrawColor(sdl_rrr, 0, 0, 0, 255);
    //SDL_RenderClear(sdl_rrr);
    //SDL_RenderPresent(sdl_rrr);
    
    SDL_GLContext context;
    context = SDL_GL_CreateContext(window);
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    
    while (true)
    {
        SDL_Event event;
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                break;
            
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                {
                    break;
                }
                /*
                if (event.key.keysym.scancode == SDL_SCANCODE_V && ((SDL_GetModState() & KMOD_CTRL) != 0) && SDL_HasClipboardText()) //Ctrl-V
                {
                    double flo, fla;
                    unsigned long key;

                    if ((SDL_GetModState() & KMOD_SHIFT) != 0)
                        sscanf_s(SDL_GetClipboardText(), "%lf,%lf", &flo, &fla);
                    else
                        sscanf_s(SDL_GetClipboardText(), "%lf,%lf", &fla, &flo);

                    key = GetFileKeyFromCoords(flo, fla);
                    log("Clipboard: %f, %f\n", flo, fla);
                    center = DEMPosition(fla, flo, center.GetLevelCount());
                    reload = true;
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_A || event.key.keysym.scancode == SDL_SCANCODE_LEFT)
                {
                    cam.rotate(-1);
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_D || event.key.keysym.scancode == SDL_SCANCODE_RIGHT)
                {
                    cam.rotate(1);
                }
                
                if (event.key.keysym.scancode == SDL_SCANCODE_LEFT)
                {
                    center.PrevX();
                    reload = true;
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT)
                {
                    center.NextX();
                    reload = true;
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_UP)
                {
                    center.NextY();
                    reload = true;
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_DOWN)
                {
                    center.PrevY();
                    reload = true;
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_PAGEDOWN)
                {
                    center.Down();
                    reload = true;
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_PAGEUP)
                {
                    center.Up();
                    reload = true;
                }
            }
            
            if (event.type == SDL_MOUSEWHEEL)
            {
                cam.zoom(event.wheel.y);
            }

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
            {
                mouse_drag = true;
            }

            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
            {
                mouse_drag = false;
            }

            if (event.type == SDL_MOUSEMOTION && mouse_drag)
            {
                cam.move(event.motion.xrel, event.motion.yrel);
            }
            */
            }
        }
    }
    
    //SDL_DestroyRenderer(sdl_rrr);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}