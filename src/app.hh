#pragma once

#include <string_view>

struct App
{
    int wWidth = 1920;
    int wHeight = 1080;

    bool bRunning = false;
    bool bConfigured = false;
    int swapInterval = 1;

    bool bPaused = false;
    bool bRelativeMode = false;
    bool bFullscreen = false;

    std::string_view name;

    virtual ~App() = default;

    virtual void init() = 0;
    virtual void disableRelativeMode() = 0;
    virtual void enableRelativeMode() = 0;
    virtual void togglePointerRelativeMode() = 0;
    virtual void toggleFullscreen() = 0;
    virtual void setCursorImage(std::string_view cursorType) = 0;
    virtual void setFullscreen() = 0;
    virtual void unsetFullscreen() = 0;
    virtual void bindGlContext() = 0;
    virtual void unbindGlContext() = 0;
    virtual void setSwapInterval(int interval) = 0;
    virtual void toggleVSync() = 0;
    virtual void swapBuffers() = 0;
    virtual void procEvents() = 0;
    virtual void showWindow() = 0;
};
