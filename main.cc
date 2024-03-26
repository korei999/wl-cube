#include "headers/wayland.hh"

int
main()
{
    WlClient app;
    app.init();
    app.mainLoop();
}
