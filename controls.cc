#include "headers/controls.hh"
#include "headers/main.hh"
#include "headers/utils.hh"

std::vector<int> pressedKeys(300, 0);

static void procMovements();

m4 
PlayerControls::getLookAt()
{
    return m4LookAt(this->pos, this->pos + this->front, this->up);
}

void
PlayerControls::procMouse()
{
    if (appState.pointerRelativeMode)
    {
        auto offsetX = (mouse.currX - mouse.prevX) * mouse.sens;
        auto offsetY = (mouse.prevY - mouse.currY) * mouse.sens;

        mouse.prevX = mouse.currX;
        mouse.prevY = mouse.currY;

        mouse.yaw += offsetX;
        mouse.pitch += offsetY;

        if (mouse.pitch > 89.9)
            mouse.pitch = 89.9;
        if (mouse.pitch < -89.9)
            mouse.pitch = -89.9;

        front = v3Norm({
                (f32)cos(TO_RAD(mouse.yaw)) * (f32)cos(TO_RAD(mouse.pitch)),
                (f32)sin(TO_RAD(mouse.pitch)),
                (f32)sin(TO_RAD(mouse.yaw)) * (f32)cos(TO_RAD(mouse.pitch))
                });

        right = v3Norm(v3Cross(front, up));
    }
}

void
procKeysOnce(u32 key, u32 keyState)
{
    switch (key)
    {
        case KEY_P:
        case KEY_GRAVE:
            if (keyState == PRESSED)
            {
                appState.paused = !appState.paused;
                if (appState.paused)
                    LOG(WARNING, "paused: {}\n", appState.paused);
            }
            break;

        case KEY_Q:
            if (keyState == PRESSED)
                appState.togglePointerRelativeMode();
            break;

        case KEY_ESC:
        case KEY_CAPSLOCK:
            appState.programIsRunning = false;
            LOG(OK, "quit...\n");
            break;

        default:
            break;
    }
}

void
PlayerControls::procKeys()
{
    procMovements();
}

static void
procMovements()
{
    f64 moveSpeed = player.moveSpeed * player.deltaTime;

    v3 combinedMove {};
    if (pressedKeys[KEY_W])
    {
        v3 forward = player.front;
        forward.y = 0;
        combinedMove += (v3Norm(forward));
    }
    if (pressedKeys[KEY_S])
    {
        v3 forward = player.front;
        forward.y = 0;
        combinedMove -= (v3Norm(forward));
    }
    if (pressedKeys[KEY_A])
    {
        v3 left = v3Norm(v3Cross(player.front, player.up));
        combinedMove -= (left);
    }
    if (pressedKeys[KEY_D])
    {
        v3 left = v3Norm(v3Cross(player.front, player.up));
        combinedMove += (left);
    }
    if (pressedKeys[KEY_SPACE])
    {
        combinedMove += player.up;
    }
    if (pressedKeys[KEY_LEFTCTRL])
    {
        combinedMove -= player.up;
    }

    if (v3Length(combinedMove) > 0)
        combinedMove = v3Norm(combinedMove);

    player.pos += combinedMove * moveSpeed;
}

void
PlayerControls::updateDeltaTime()
{
    f64 currTime = getTimeSec();
    player.deltaTime = currTime - player.lastFrameTime;
    player.lastFrameTime = currTime;
}
