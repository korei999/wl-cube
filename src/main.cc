#include "frame.hh"

#ifdef __linux__
#    include "platform/wayland/wayland.hh"
#elif _WIN32
#    include "platform/windows/windows.hh"
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
WinMain([[maybe_unused]] HINSTANCE instance,
        [[maybe_unused]] HINSTANCE previnstance,
        [[maybe_unused]] LPSTR cmdline,
        [[maybe_unused]] int cmdshow)
{
    Win32window app("wl-cube", instance);
    run(&app);

    return 0;
}

int
main()
{
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);
}

#endif
