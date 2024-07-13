#include <windowsx.h>

#include "input.hh"
#include "utils.hh"
#include "../../frame.hh"

LRESULT CALLBACK
windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Win32window* pWin = (Win32window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg)
    {
        case WM_DESTROY:
            LOG(OK, "WM_DESTROY\n");
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
            pWin->wWidth = LOWORD(lParam);
            pWin->wHeight = HIWORD(lParam);
            break;

        case WM_NCCREATE:
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
            break;

        case WM_LBUTTONDOWN:
            break;

        case WM_KEYUP:
        case WM_KEYDOWN:
            {
                WPARAM keyCode = wParam;
                bool isUp = !((lParam >> 31) & 1);
                switch (keyCode)
                {
                    case 'W':
                        pressedKeys[KEY_W] = isUp;
                        break;

                    case 'A':
                        pressedKeys[KEY_A] = isUp;
                        break;

                    case 'S':
                        pressedKeys[KEY_S] = isUp;
                        break;

                    case 'D':
                        pressedKeys[KEY_D] = isUp;
                        break;

                    case ' ':
                        pressedKeys[KEY_SPACE] = isUp;
                        break;

                    case VK_CONTROL:
                        pressedKeys[KEY_LEFTCTRL] = isUp;
                        break;

                    default:
                        break;
                };
            }
            break;

        case WM_MOUSEMOVE:
            player.mouse.absX = GET_X_LPARAM(lParam);
            player.mouse.absY = GET_Y_LPARAM(lParam);

            player.mouse.relX = GET_X_LPARAM(lParam);
            player.mouse.relY = GET_Y_LPARAM(lParam);
            break;

        default:
            break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
