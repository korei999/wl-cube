#pragma once
#include "gmath.hh"
#include "input.hh"
#include "wayland.hh"

enum KeyState : int
{
    RELEASED,
    PRESSED
};

struct PlayerControls
{
    Mouse mouse;
    v3 pos {0, 0, 3};
    v3 front {0, 0, -1};
    v3 right {1, 0, 0};
    const v3 up {0, 1, 0};

    m4 proj {};
    m4 view {};

    f64 deltaTime = 0;
    f64 lastFrameTime = 0;
    f64 moveSpeed = 5.0;

    void procMouse();
    void procKeys();
    void updateDeltaTime();
    void updateView();
    void updateProj(f32 fov, f32 aspect, f32 near, f32 far);
};

void procKeysOnce(WlClient* self, u32 key, u32 keyState);

extern int pressedKeys[300];
