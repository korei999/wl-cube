#pragma once
#include "gmath.hh"
#include "wayland.hh"

#include <linux/input-event-codes.h>

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

struct PlayerControls
{
    /* place proj and view adjecent for nice ubo buffering */
    m4 proj {};
    m4 view {};

    v3 pos {0, 0, 3};
    v3 front {0, 0, -1};
    v3 right {1, 0, 0};
    const v3 up {0, 1, 0};

    f64 deltaTime = 0;
    f64 lastFrameTime = 0;
    f64 moveSpeed = 5.0;

    Mouse mouse {};

    void procMouse();
    void procKeys(WlClient* self);
    void updateDeltaTime();
    void updateView();
    void updateProj(f32 fov, f32 aspect, f32 near, f32 far);
};

void procKeysOnce(WlClient* self, u32 key, u32 pressed);

extern bool pressedKeys[300];
