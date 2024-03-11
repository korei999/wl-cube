#include "headers/controls.hh"

m4 
PlayerControls::getLookAt()
{
    return m4LookAt(this->pos, this->pos + this->front, this->up);
}

void
PlayerControls::procMouse()
{
    auto offsetX = (this->mouse.prevX - this->mouse.currX) * this->mouse.sens;
    auto offsetY = (this->mouse.currY - this->mouse.prevY) * this->mouse.sens;

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
