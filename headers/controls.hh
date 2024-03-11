#pragma once
#include "math.hh"
#include "input.hh"

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

    f64 deltaTime = 0;
    f64 lastFrameTime = 0;
    f64 moveSpeed = 5.0;

    m4 getLookAt();
    void procMouse();
    void procKeys();
    void updateDeltaTime();
};

extern int pressedKeys[300];
