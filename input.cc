#include "wayland/pointer-constraints-unstable-v1.h"
#include "wayland/xdg-shell.h"
#include "headers/controls.hh"
#include "headers/input.hh"
#include "headers/main.hh"
#include "headers/utils.hh"
#include "headers/frame.hh"

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
    for (auto& key : pressedKeys)
        key = 0;
}

void
keyboardKeyHandle(void* data,
                  wl_keyboard* keyboard,
                  u32 serial,
                  u32 time,
                  u32 key,
                  u32 keyState)
{
#ifdef DEBUG
    if (key >= pressedKeys.size())
    {
        if (key > 10000)
        {
            LOG(WARNING, "key '{}' is too big?\n", key);
            return;
        }
        pressedKeys.resize(key + 1, 0);
    }
#endif

    pressedKeys[key] = keyState;
    procKeysOnce(key, keyState);
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
    //
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
    appState.pointerSerial = serial;

    if (appState.pointerRelativeMode)
        wl_pointer_set_cursor(appState.pointer, appState.pointerSerial, NULL, 0, 0);
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
    player.mouse.absX = wl_fixed_to_double(surfaceX);
    player.mouse.absY = wl_fixed_to_double(surfaceY);
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

    player.mouse.button = button;
    player.mouse.state = buttonState;
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

void
relativePointerHandleMotion(void *data,
				            zwp_relative_pointer_v1 *zwp_relative_pointer_v1,
				            u32 utime_hi,
				            u32 utime_lo,
				            wl_fixed_t dx,
				            wl_fixed_t dy,
				            wl_fixed_t dxUnaccel,
				            wl_fixed_t dyUnaccel)
{
    player.mouse.relX += wl_fixed_to_int(dxUnaccel);
    player.mouse.relY += wl_fixed_to_int(dyUnaccel);
}
