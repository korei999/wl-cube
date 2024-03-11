#include "headers/controls.hh"
#include "headers/main.hh"
#include "headers/utils.hh"

int pressedKeys[300] {
    // [KEY_W] = 0,
    // [KEY_A] = 0,
    // [KEY_S] = 0,
    // [KEY_D] = 0,
    // [KEY_SPACE] = 0,
    // [KEY_LEFTCTRL] = 0,
};

m4 
PlayerControls::getLookAt()
{
    return m4LookAt(this->pos, this->pos + this->front, this->up);
}

void
PlayerControls::procMouse()
{
    auto offsetX = (this->mouse.currX - this->mouse.prevX) * this->mouse.sens;
    auto offsetY = (this->mouse.prevY - this->mouse.currY) * this->mouse.sens;

    this->mouse.prevX = this->mouse.currX;
    this->mouse.prevY = this->mouse.currY;

    this->mouse.yaw += offsetX;
    this->mouse.pitch += offsetY;

    if (this->mouse.pitch > 89.9)
        this->mouse.pitch = 89.9;
    if (this->mouse.pitch < -89.9)
        this->mouse.pitch = -89.9;

    this->front = v3Norm({
        (f32)cos(TO_RAD(this->mouse.yaw)) * (f32)cos(TO_RAD(this->mouse.pitch)),
        (f32)sin(TO_RAD(this->mouse.pitch)),
        (f32)sin(TO_RAD(this->mouse.yaw)) * (f32)cos(TO_RAD(this->mouse.pitch))
    });

    this->right = v3Norm(v3Cross(this->front, this->up));
}

void
PlayerControls::procKeys()
{
    auto moveSpeed = player.moveSpeed * player.deltaTime;

    if (pressedKeys[KEY_Q])
    {
        appState.togglePointerRelativeMode();
    }
    if (pressedKeys[KEY_CAPSLOCK] || pressedKeys[KEY_ESC])
    {
        appState.programIsRunning = false;
        LOG(OK, "quit...\n");
    }
    // if (pressedKeys[KEY_P] && !appState.paused)
    // {
        // appState.paused = true;
        // LOG(WARNING, "paused: {}\n", appState.paused);
    // }

    if (pressedKeys[KEY_W])
    {
        v3 forward = player.front;
        forward.y = 0;
        player.pos += (v3Norm(forward) * moveSpeed);
    }
    if (pressedKeys[KEY_S])
    {
        v3 forward = player.front;
        forward.y = 0;
        player.pos -= (v3Norm(forward) * moveSpeed);
    }
    if (pressedKeys[KEY_A])
    {
        v3 left = v3Norm(v3Cross(player.front, player.up));
        player.pos -= (left * moveSpeed);
    }
    if (pressedKeys[KEY_D])
    {
        v3 left = v3Norm(v3Cross(player.front, player.up));
        player.pos += (left * moveSpeed);
    }
    if (pressedKeys[KEY_SPACE])
    {
        player.pos += (player.up * moveSpeed);
    }
    if (pressedKeys[KEY_LEFTCTRL])
    {
        player.pos -= (player.up * moveSpeed);
    }
}

void
PlayerControls::updateDeltaTime()
{
    f64 currTime = getTimeSec();
    player.deltaTime = currTime - player.lastFrameTime;
    player.lastFrameTime = currTime;
}
