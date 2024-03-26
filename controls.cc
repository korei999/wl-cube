#include "headers/controls.hh"
#include "headers/frame.hh"
#include "headers/utils.hh"

int pressedKeys[300] {};

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
            f32(cos(TO_RAD(mouse.yaw)) * cos(TO_RAD(mouse.pitch))),
            f32(sin(TO_RAD(mouse.pitch))),
            f32(sin(TO_RAD(mouse.yaw)) * cos(TO_RAD(mouse.pitch)))
    });

    right = v3Norm(v3Cross(front, up));
}

void
procKeysOnce(WlClient* self, u32 key, u32 keyState)
{
    switch (key)
    {
        case KEY_P:
        case KEY_GRAVE:
            if (keyState)
            {
                self->isPaused = !self->isPaused;
                if (self->isPaused)
                    LOG(WARNING, "paused: {}\n", self->isPaused);
            }
            break;

        case KEY_Q:
            if (keyState)
                self->togglePointerRelativeMode();
            break;

        case KEY_ESC:
        case KEY_CAPSLOCK:
            self->isRunning = false;
            LOG(OK, "quit...\n");
            break;

        case KEY_R:
            if (keyState)
                incCounter = 0;
            break;

        case KEY_F:
            if (keyState)
                self->toggleFullscreen();
            break;

        case KEY_V:
            if (keyState)
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
}

static void
procMovements(WlClient* self)
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
