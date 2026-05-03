#include "core/Application.h"

#include <SDL.h>
#include <SDL_main.h>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    acs::Application app;
    return app.run();
}
