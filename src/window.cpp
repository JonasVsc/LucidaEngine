#include "window.h"

#include <assert.h>

void leWindowCreate(const LeWindowCreateInfo& createInfo, LeWindow& window)
{
    assert(SDL_Init(SDL_INIT_VIDEO) >= 0 && "could not initialized SDL");

    window.handle = SDL_CreateWindow(
        createInfo.title, 
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 
        createInfo.width,
        createInfo.height, 
        SDL_WINDOW_VULKAN
    );

    assert(window.handle != NULL && "failed to create window");
    window.shouldClose = false;
}

void leWindowPoolEvents(LeWindow &window)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            window.shouldClose = true;
        }
    }
}

void leWindowDestroy(LeWindow &window)
{
    if (window.handle == nullptr)
        return;
    
    SDL_DestroyWindow(window.handle);
    window.handle = NULL;
}
