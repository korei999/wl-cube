#pragma once
#include "math.hh"
#include "input.hh"

struct PlayerControls
{
    Mouse mouse;
    v3 pos {0, 0, 3};
    v3 front {0, 0, -1};
    v3 right {1, 0, 0};
    const v3 up {0, 1, 0};

    m4 getLookAt();
    void procMouse();
};
