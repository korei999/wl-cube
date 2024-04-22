#pragma once
#include "../../headers/app.hh"

#include <windows.h>

struct Win32window : App
{
    HINSTANCE instance;
    HWND window;
    HDC deviceContext;
    HGLRC glContext;
    WNDCLASSEXW windowClass;

    Win32window(std::string_view name, HINSTANCE _instance);
    virtual ~Win32window() override;

    virtual void init() override;
    virtual void disableRelativeMode() override;
    virtual void enableRelativeMode() override;
    virtual void togglePointerRelativeMode() override;
    virtual void toggleFullscreen() override;
    virtual void setCursorImage(std::string_view cursorType) override;
    virtual void setFullscreen() override;
    virtual void unsetFullscreen() override;
    virtual void bindGlContext() override;
    virtual void unbindGlContext() override;
    virtual void setSwapInterval(int interval) override;
    virtual void toggleVSync() override;
    virtual void swapBuffers() override;
    virtual void procEvents() override;
    virtual void showWindow() override;
};
