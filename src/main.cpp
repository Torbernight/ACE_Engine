#include "core/engine.hpp"

int main()
{
    Engine App("ACE Engine", 500, 500);
    App.init();
    App.mainLoop();
    App.cleanup();
    return 0;
}