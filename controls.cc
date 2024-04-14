#include "headers/controls.hh"
#include "headers/frame.hh"
#include "headers/utils.hh"

bool pressedKeys[300] {};

static void procMovements(WlClient* self);

void
PlayerControls::procMouse()
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
            f32(cos(toRad(mouse.yaw)) * cos(toRad(mouse.pitch))),
            f32(sin(toRad(mouse.pitch))),
            f32(sin(toRad(mouse.yaw)) * cos(toRad(mouse.pitch)))
    });

    right = v3Norm(v3Cross(front, up));
}

void
procKeysOnce(WlClient* self, u32 key, u32 pressed)
{
    switch (key)
    {
        case KEY_P:
        case KEY_GRAVE:
            if (pressed)
            {
                self->isPaused = !self->isPaused;
                if (self->isPaused)
                    LOG(WARNING, "paused: {}\n", self->isPaused);
            }
            break;

        case KEY_Q:
            if (pressed)
                self->togglePointerRelativeMode();
            break;

        case KEY_ESC:
        case KEY_CAPSLOCK:
            if (pressed)
            {
                self->isRunning = false;
                LOG(OK, "quit...\n");
            }
            break;

        case KEY_R:
            if (pressed)
                incCounter = 0;
            break;

        case KEY_F:
            if (pressed)
                self->toggleFullscreen();
            break;

        case KEY_V:
            if (pressed)
            {
                self->swapInterval = !self->swapInterval;
                EGLD( eglSwapInterval(self->eglDisplay, self->swapInterval) );
                LOG(OK, "swapInterval: {}\n", self->swapInterval);
            }
            break;

        default:
            break;
    }
}

void
PlayerControls::procKeys(WlClient* self)
{
    procMovements(self);

    if (pressedKeys[KEY_I])
    {
        fov += 100.0f * deltaTime;
        LOG(OK, "fov: {:.3f}\n", fov);
    }
    if (pressedKeys[KEY_O])
    {
        fov -= 100.0f * deltaTime;
        LOG(OK, "fov: {:.3f}\n", fov);
    }
    if (pressedKeys[KEY_Z])
    {
        f32 inc = pressedKeys[KEY_LEFTSHIFT] ? (-4.0) : 4.0;
        x += inc * deltaTime;
        LOG(OK, "x: {:.3f}\n", x);
    }
    if (pressedKeys[KEY_X])
    {
        f32 inc = pressedKeys[KEY_LEFTSHIFT] ? (-4.0) : 4.0;
        y += inc * deltaTime;
        LOG(OK, "y: {:.3f}\n", y);
    }
    if (pressedKeys[KEY_C])
    {
        f32 inc = pressedKeys[KEY_LEFTSHIFT] ? (-4.0) : 4.0;
        z += inc * deltaTime;
        LOG(OK, "z: {:.3f}\n", z);
    }
}

static void
procMovements([[maybe_unused]] WlClient* self)
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
    currTime = timeNow();
    deltaTime = currTime - lastFrameTime;
    lastFrameTime = currTime;
}

void 
PlayerControls::updateView()
{
    view = m4LookAt(pos, pos + front, up);
}

void 
PlayerControls::updateProj(f32 fov, f32 aspect, f32 near, f32 far)
{
    proj = m4Pers(fov, aspect, near, far);
}
