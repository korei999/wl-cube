#pragma once
#include "ultratypes.h"
#include "../wayland/relative-pointer-unstable-v1.h"

#include <wayland-client-protocol.h>
#ifdef __linux__
#    include <linux/input-event-codes.h>
#elif __FreeBSD__
#    include <dev/evdev/input-event-codes.h>
#endif

struct Mouse
{
    f64 prevAbsX = 0;
    f64 prevAbsY = 0;

    f64 prevRelX = 0;
    f64 prevRelY = 0;

    f64 absX = 0;
    f64 absY = 0;

    f64 relX = 0;
    f64 relY = 0;

    f64 sens = 0.05;
    f64 yaw = -90.0;
    f64 pitch = 0;

    u32 button = BTN_MOUSE;
    u32 state = 0;
};

void keyboardKeymapHandle(void* data, wl_keyboard* keyboard, u32 format, s32 fd, u32 size);
void keyboardEnterHandle(void* data, wl_keyboard* keyboard, u32 serial, wl_surface* surface, wl_array* keys);
void keyboardLeaveHandle(void* data, wl_keyboard* keyboard, u32 serial, wl_surface* surface);
void keyboardKeyHandle(void* data, wl_keyboard* keyboard, u32 serial, u32 time, u32 key, u32 state);
void keyboardModifiersHandle(void* data, wl_keyboard* keyboard, u32 serial, u32 modsDepressed, u32 modsLatched, u32 modsLocked, u32 group);
void keyboardRepeatInfo(void* data, wl_keyboard* keyboard, s32 rate, s32 delay);

void pointerEnterHandle(void* data, wl_pointer* pointer, u32 serial, wl_surface* surface, wl_fixed_t surfaceX, wl_fixed_t surfaceY);
void pointerLeaveHandle(void* data, wl_pointer* pointer, u32 serial, wl_surface* surface);
void pointerMotionHandle(void* data, wl_pointer* pointer, u32 time, wl_fixed_t surfaceX, wl_fixed_t surfaceY);
void pointerButtonHandle(void* data, wl_pointer* pointer, u32 serial, u32 time, u32 button, u32 state);
void pointerAxisHandle(void* data, wl_pointer* pointer, u32 time, u32 axis, wl_fixed_t value);

void relativePointerHandleMotion(void *data, zwp_relative_pointer_v1 *zwp_relative_pointer_v1, u32 utime_hi, u32 utime_lo, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t dx_unaccel, wl_fixed_t dy_unaccel);
