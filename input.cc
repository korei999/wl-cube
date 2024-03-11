#include "headers/main.hh"
#include "headers/input.hh"
#include "glueCode/xdg-shell-client-protocol.h"
#include "headers/utils.hh"

void
keyboardKeymapHandle(void* data,
                     wl_keyboard* keyboard,
                     u32 format,
                     s32 fd,
                     u32 size)
{
    //
}

void
keyboardEnterHandle(void* data,
                    wl_keyboard* keyboard,
                    u32 serial,
                    wl_surface* surface,
                    wl_array* keys)
{
    //
}

void
keyboardLeaveHandle(void* data,
                    wl_keyboard* keyboard,
                    u32 serial,
                    wl_surface* surface)
{
    //
}

void
keyboardKeyHandle(void* data,
                  wl_keyboard* keyboard,
                  u32 serial,
                  u32 time,
                  u32 key,
                  u32 keyState)
{
    //
    LOG(OK, "key: {}\tstate: {}\n", key, keyState);
    switch (key)
    {
        default:
            break;

        case KEY_CAPSLOCK:
        case KEY_ESC:
            if (keyState == 0)
            {
                appState.programIsRunning = false;
                LOG(OK, "quit...\n");
            }
            break;

        case KEY_SPACE:
            if (keyState == 1)
            {
                appState.paused = !appState.paused;
                LOG(WARNING, "paused: {}\n", appState.paused);
            }
            break;
    }
}

void
keyboardModifiersHandle(void* data,
                        wl_keyboard* keyboard,
                        u32 serial,
                        u32 modsDepressed,
                        u32 modsLatched,
                        u32 modsLocked,
                        u32 group)
{
    // LOG(OK, "mods_depressed: {}\tmods_latched: {}\tgroup: {}\n", mods_depressed, mods_latched, group);
}

void
keyboardRepeatInfo(void* data,
                   wl_keyboard* wl_keyboard,
                   s32 rate,
                   s32 delay)
{
    LOG(OK, "rate: {}\tdelay: {}\n", rate, delay);
}


void
pointerEnterHandle(void* data,
                   wl_pointer* pointer,
                   u32 serial,
                   wl_surface* surface,
                   wl_fixed_t surfaceX,
                   wl_fixed_t surfaceY)
{
    //
}

void
pointerLeaveHandle(void* data,
                   wl_pointer* pointer,
                   u32 serial,
                   wl_surface* surface)
{
    //
}

void
pointerMotionHandle(void* data,
                    wl_pointer* pointer,
                    u32 time,
                    wl_fixed_t surfaceX,
                    wl_fixed_t surfaceY)
{
    mouse.lastX = wl_fixed_to_double(surfaceX);
    mouse.lastY = wl_fixed_to_double(surfaceY);
}

void
pointerButtonHandle(void* data,
                    wl_pointer* pointer,
                    u32 serial,
                    u32 time,
                    u32 button,
                    u32 buttonState)
{
    // wl_seat* seat = (wl_seat*)data;
    // if (button == BTN_LEFT && buttonState == WL_POINTER_BUTTON_STATE_PRESSED)
        // xdg_toplevel_move(appState.xdgToplevel, seat, serial);

    mouse.button = button;
    mouse.state = buttonState;
}

void
pointerAxisHandle(void* data,
                  wl_pointer* pointer,
                  u32 time,
                  u32 axis,
                  wl_fixed_t value)
{
    //
}
