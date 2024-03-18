#include "wayland/pointer-constraints-unstable-v1.h"
#include "wayland/xdg-shell.h"
#include "headers/controls.hh"
#include "headers/input.hh"
#include "headers/main.hh"
#include "headers/utils.hh"
#include "headers/frame.hh"

void
keyboardKeymapHandle([[maybe_unused]] void* data,
                     [[maybe_unused]] wl_keyboard* keyboard,
                     [[maybe_unused]] u32 format,
                     [[maybe_unused]] s32 fd,
                     [[maybe_unused]] u32 size)
{
    //
}

void
keyboardEnterHandle([[maybe_unused]] void* data,
                    [[maybe_unused]] wl_keyboard* keyboard,
                    [[maybe_unused]] u32 serial,
                    [[maybe_unused]] wl_surface* surface,
                    [[maybe_unused]] wl_array* keys)
{
    //
}

void
keyboardLeaveHandle([[maybe_unused]] void* data,
                    [[maybe_unused]] wl_keyboard* keyboard,
                    [[maybe_unused]] u32 serial,
                    [[maybe_unused]] wl_surface* surface)
{
    for (auto& key : pressedKeys)
        key = 0;
}

void
keyboardKeyHandle([[maybe_unused]] void* data,
                  [[maybe_unused]] wl_keyboard* keyboard,
                  [[maybe_unused]] u32 serial,
                  [[maybe_unused]] u32 time,
                  [[maybe_unused]] u32 key,
                  [[maybe_unused]] u32 keyState)
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
keyboardModifiersHandle([[maybe_unused]] void* data,
                        [[maybe_unused]] wl_keyboard* keyboard,
                        [[maybe_unused]] u32 serial,
                        [[maybe_unused]] u32 modsDepressed,
                        [[maybe_unused]] u32 modsLatched,
                        [[maybe_unused]] u32 modsLocked,
                        [[maybe_unused]] u32 group)
{
    //
}

void
keyboardRepeatInfo([[maybe_unused]] void* data,
                   [[maybe_unused]] wl_keyboard* wl_keyboard,
                   [[maybe_unused]] s32 rate,
                   [[maybe_unused]] s32 delay)
{
    LOG(OK, "rate: {}\tdelay: {}\n", rate, delay);
}


void
pointerEnterHandle([[maybe_unused]] void* data,
                   [[maybe_unused]] wl_pointer* pointer,
                   [[maybe_unused]] u32 serial,
                   [[maybe_unused]] wl_surface* surface,
                   [[maybe_unused]] wl_fixed_t surfaceX,
                   [[maybe_unused]] wl_fixed_t surfaceY)
{
    appState.pointerSerial = serial;

    if (appState.pointerRelativeMode)
        wl_pointer_set_cursor(appState.pointer, appState.pointerSerial, NULL, 0, 0);
}

void
pointerLeaveHandle([[maybe_unused]] void* data,
                   [[maybe_unused]] wl_pointer* pointer,
                   [[maybe_unused]] u32 serial,
                   [[maybe_unused]] wl_surface* surface)
{
    //
}

void
pointerMotionHandle([[maybe_unused]] void* data,
                    [[maybe_unused]] wl_pointer* pointer,
                    [[maybe_unused]] u32 time,
                    [[maybe_unused]] wl_fixed_t surfaceX,
                    [[maybe_unused]] wl_fixed_t surfaceY)
{
    player.mouse.absX = wl_fixed_to_double(surfaceX);
    player.mouse.absY = wl_fixed_to_double(surfaceY);
}

void
pointerButtonHandle([[maybe_unused]] void* data,
                    [[maybe_unused]] wl_pointer* pointer,
                    [[maybe_unused]] u32 serial,
                    [[maybe_unused]] u32 time,
                    [[maybe_unused]] u32 button,
                    [[maybe_unused]] u32 buttonState)
{
    player.mouse.button = button;
    player.mouse.state = buttonState;
}

void
pointerAxisHandle([[maybe_unused]] void* data,
                  [[maybe_unused]] wl_pointer* pointer,
                  [[maybe_unused]] u32 time,
                  [[maybe_unused]] u32 axis,
                  [[maybe_unused]] wl_fixed_t value)
{
    //
}

void
relativePointerHandleMotion([[maybe_unused]] void *data,
				            [[maybe_unused]] zwp_relative_pointer_v1 *zwp_relative_pointer_v1,
				            [[maybe_unused]] u32 utime_hi,
				            [[maybe_unused]] u32 utime_lo,
				            [[maybe_unused]] wl_fixed_t dx,
				            [[maybe_unused]] wl_fixed_t dy,
				            [[maybe_unused]] wl_fixed_t dxUnaccel,
				            [[maybe_unused]] wl_fixed_t dyUnaccel)
{
    player.mouse.relX += wl_fixed_to_int(dxUnaccel);
    player.mouse.relY += wl_fixed_to_int(dyUnaccel);
}
