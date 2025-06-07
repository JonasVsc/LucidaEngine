#include "lucida/window.h"

static bool shouldClose = false;
LeResult leWindowCreate(const LeWindowCreateInfo *pCreateInfo, LeWindow *pOutWindow)
{
    SDL_Init(SDL_INIT_VIDEO) >= 0 && "could not initialized SDL";

    pOutWindow->handle = SDL_CreateWindow(
        pCreateInfo->title, 
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 
        pCreateInfo->extent.width,
        pCreateInfo->extent.height, 
        SDL_WINDOW_VULKAN
    );

    if (pOutWindow == NULL) 
    {
        return LE_FAIL;
    }

    return LE_SUCCESS;
}

void leWindowUpdate(LeWindow *pWindow)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            pWindow->shouldClose = true;
        }
    }
}

void leWindowDestroy(LeWindow *pWindow)
{
    if (pWindow && pWindow->handle) {
        SDL_DestroyWindow(pWindow->handle);
        pWindow->handle = NULL;
    }
}