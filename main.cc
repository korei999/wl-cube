#include "headers/frame.hh"
#ifdef __linux__
#include "platform/wayland/wayland.hh"
#elif _WIN32
// #include "windowsSomething.hh"
#endif

int
main()
{
#ifdef __linux__
    WlClient app("wl-cube");
#elif _WIN32
    /* Win32 app {} */
#endif

    run(&app);
}
