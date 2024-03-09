#pragma once
#include "ultratypes.h"
#include <wayland-client-protocol.h>

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
