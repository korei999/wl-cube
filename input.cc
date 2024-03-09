#include "headers/main.hh"
#include "headers/input.hh"
#include "glueCode/xdg-shell-client-protocol.h"
#include "headers/utils.hh"

#ifdef __linux__
#    include <linux/input-event-codes.h>
#elif __FreeBSD__
#    include <dev/evdev/input-event-codes.h>
#endif

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
                  u32 state)
{
    //
    LOG(OK, "key: {}\tstate: {}\n", key, state);
    switch (key)
    {
        default:
            break;

        case KEY_ESC:
            if (state == 0)
            {
                programIsRunning = false;
                LOG(OK, "Good bye!\n");
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
    //
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
    //
}

void
pointerButtonHandle(void* data,
                    wl_pointer* pointer,
                    u32 serial,
                    u32 time,
                    u32 button,
                    u32 state)
{
    wl_seat* seat = (wl_seat*)data;

    if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED)
        xdg_toplevel_move(xdg_toplevel, seat, serial);
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
