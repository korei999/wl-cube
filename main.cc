#include "headers/frame.hh"
#include "headers/utils.hh"

#ifdef __linux__
#include "platform/wayland/wayland.hh"
#elif _WIN32
#include "platform/windows/windows.hh"
#endif

#ifdef __linux__
int
main()
{
    WlClient app("wl-cube");
    run(&app);
}
#elif _WIN32
int WINAPI
WinMain(HINSTANCE instance,
        [[maybe_unused]] HINSTANCE previnstance,
        [[maybe_unused]] LPSTR cmdline,
        [[maybe_unused]] int cmdshow)
{
    Win32window app("wl-cube", instance);
    try
    {
        run(&app);
    }
    catch (int e)
    {
        if (e != 0)
            CERR("Unhandled exception: {}\n", e);

        CERR("quit...\n");
        return e;
    }

    return 0;
}
int
main()
{
   return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);
}
#endif
