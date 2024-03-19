#include "headers/controls.hh"
#include "headers/main.hh"
#include "headers/frame.hh"
#include "headers/utils.hh"

std::vector<int> pressedKeys(300, 0);

static void procMovements();

void
PlayerControls::procMouse()
{
    if (appState.pointerRelativeMode)
    {
        auto offsetX = (mouse.relX - mouse.prevRelX) * mouse.sens;
        auto offsetY = (mouse.prevRelY - mouse.relY) * mouse.sens;

        mouse.prevRelX = mouse.relX;
        mouse.prevRelY = mouse.relY;

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
    // else
    // {
        // LOG(OK, "absX: {:.3f},\tabsY: {:.3f}\n", mouse.absX, mouse.absY);
    // }
}

void
procKeysOnce(u32 key, u32 keyState)
{
    switch (key)
    {
        case KEY_P:
        case KEY_GRAVE:
            if (keyState)
            {
                appState.paused = !appState.paused;
                if (appState.paused)
                    LOG(WARNING, "paused: {}\n", appState.paused);
            }
            break;

        case KEY_Q:
            if (keyState)
                appState.togglePointerRelativeMode();
            break;

        case KEY_ESC:
        case KEY_CAPSLOCK:
            appState.programIsRunning = false;
            LOG(OK, "quit...\n");
            break;

        case KEY_R:
            if (keyState)
                incCounter = 0;
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
        v3 forward {player.front.x, 0.0f, player.front.z};
        combinedMove += (v3Norm(forward));
    }
    if (pressedKeys[KEY_S])
    {
        v3 forward {player.front.x, 0.0f, player.front.z};
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

    f32 len = v3Length(combinedMove);
    if (len > 0)
        combinedMove = v3Norm(combinedMove, len);

    player.pos += combinedMove * moveSpeed;
}

void
PlayerControls::updateDeltaTime()
{
    f64 currTime = timeNow();
    player.deltaTime = currTime - player.lastFrameTime;
    player.lastFrameTime = currTime;
}

void 
PlayerControls::updateView()
{
    view = m4LookAt(player.pos, player.pos + player.front, player.up);
}

void 
PlayerControls::updateProj(f32 fov, f32 aspect, f32 near, f32 far)
{
    proj = m4Pers(fov, aspect, near, far);
}
