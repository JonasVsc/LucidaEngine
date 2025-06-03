#include "window.h"

// core
#include "core/log.h"
#include "core/config/config.h"


// std
#include <stdexcept>


Window::Window(Config& lc)
{
    jinfo("window constructor");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        throw std::runtime_error("Could not initialize SDL");
    }

    uint32_t fullscreen{};
    uint32_t resizable{};

    lc.is_window_fullscreen() ? fullscreen = SDL_WINDOW_FULLSCREEN : fullscreen = 0;
    lc.is_window_resizable() ? resizable = SDL_WINDOW_RESIZABLE : resizable = 0;

    m_window = SDL_CreateWindow(
        lc.get_window_title().c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        lc.get_window_width(), lc.get_window_height(),  fullscreen | resizable | SDL_WINDOW_VULKAN);
}

Window::~Window()
{
    jinfo("window destructor");
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void Window::process_events()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            m_closed = true;
            break;
        }
    }
}
