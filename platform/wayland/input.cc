#include "../../headers/controls.hh"
#include "../../headers/frame.hh"
#include "wayland.hh"
#include "../../headers/utils.hh"

void
keyboardKeymapHandler([[maybe_unused]] void* data,
                      [[maybe_unused]] wl_keyboard* keyboard,
                      [[maybe_unused]] u32 format,
                      [[maybe_unused]] s32 fd,
                      [[maybe_unused]] u32 size)
{
    //
}

void
keyboardEnterHandler([[maybe_unused]] void* data,
                     [[maybe_unused]] wl_keyboard* keyboard,
                     [[maybe_unused]] u32 serial,
                     [[maybe_unused]] wl_surface* surface,
                     [[maybe_unused]] wl_array* keys)
{
    LOG(OK, "keyboardEnterHandler\n");

    auto app = (WlClient*)data;

    if (app->bRelativeMode)
        app->enableRelativeMode();
}

void
keyboardLeaveHandler([[maybe_unused]] void* data,
                     [[maybe_unused]] wl_keyboard* keyboard,
                     [[maybe_unused]] u32 serial,
                     [[maybe_unused]] wl_surface* surface)
{
    LOG(OK, "keyboardLeaveHandler\n");

    auto app = (WlClient*)data;

    for (auto& key : pressedKeys)
        key = 0;

    if (app->bRelativeMode)
        app->disableRelativeMode();
}

void
keyboardKeyHandler([[maybe_unused]] void* data,
                   [[maybe_unused]] wl_keyboard* keyboard,
                   [[maybe_unused]] u32 serial,
                   [[maybe_unused]] u32 time,
                   [[maybe_unused]] u32 key,
                   [[maybe_unused]] u32 keyState)
{
    auto app = (WlClient*)data;

#ifdef DEBUG
    if (key >= LEN(pressedKeys))
    {
        LOG(WARNING, "key '{}' is too big?\n", key);
        return;
    }
#endif

    pressedKeys[key] = keyState;
    procKeysOnce(app, key, keyState);
}

void
keyboardModifiersHandler([[maybe_unused]] void* data,
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
keyboardRepeatInfoHandler([[maybe_unused]] void* data,
                          [[maybe_unused]] wl_keyboard* wl_keyboard,
                          [[maybe_unused]] s32 rate,
                          [[maybe_unused]] s32 delay)
{
    //
}


void
pointerEnterHandler([[maybe_unused]] void* data,
                    [[maybe_unused]] wl_pointer* pointer,
                    [[maybe_unused]] u32 serial,
                    [[maybe_unused]] wl_surface* surface,
                    [[maybe_unused]] wl_fixed_t surfaceX,
                    [[maybe_unused]] wl_fixed_t surfaceY)
{
    LOG(OK, "pointerEnterHandler\n");

    auto app = (WlClient*)data;
    app->pointerSerial = serial;

    if (app->bRelativeMode)
    {
        wl_pointer_set_cursor(pointer, serial, nullptr, 0, 0);
    }
    else
    {
        wl_pointer_set_cursor(pointer,
                              serial,
                              app->cursorSurface,
                              app->cursorImage->hotspot_x,
                              app->cursorImage->hotspot_y);
    }
}

void
pointerLeaveHandler([[maybe_unused]] void* data,
                    [[maybe_unused]] wl_pointer* pointer,
                    [[maybe_unused]] u32 serial,
                    [[maybe_unused]] wl_surface* surface)
{
    LOG(OK, "pointerLeaveHandler\n");
}

void
pointerMotionHandler([[maybe_unused]] void* data,
                     [[maybe_unused]] wl_pointer* pointer,
                     [[maybe_unused]] u32 time,
                     [[maybe_unused]] wl_fixed_t surfaceX,
                     [[maybe_unused]] wl_fixed_t surfaceY)
{
    player.mouse.absX = wl_fixed_to_double(surfaceX);
    player.mouse.absY = wl_fixed_to_double(surfaceY);
}

void
pointerButtonHandler([[maybe_unused]] void* data,
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
pointerAxisHandler([[maybe_unused]] void* data,
                   [[maybe_unused]] wl_pointer* pointer,
                   [[maybe_unused]] u32 time,
                   [[maybe_unused]] u32 axis,
                   [[maybe_unused]] wl_fixed_t value)
{
    //
}

void
relativePointerMotionHandler([[maybe_unused]] void *data,
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
