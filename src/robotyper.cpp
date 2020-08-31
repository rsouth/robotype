#include <windows.h>

#include "resource.h"
#include <memory>
#include <string>

void pressEnter() {
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    input.ki.wScan = 0;
    input.ki.dwFlags = 0;

    input.ki.wVk = VK_RETURN;
    SendInput(1, &input, sizeof(INPUT));
}

void pressKeyB(char mK) {
    HKL kbl = GetKeyboardLayout(0);
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.time = 0;
    ip.ki.dwFlags = KEYEVENTF_UNICODE;
    if ((int) mK < 65 && (int) mK > 90) //for lowercase
    {
        ip.ki.wScan = 0;
        ip.ki.wVk = VkKeyScanEx(mK, kbl);
    } else //for uppercase
    {
        ip.ki.wScan = mK;
        ip.ki.wVk = 0;

    }
    ip.ki.dwExtraInfo = 0;
    SendInput(1, &ip, sizeof(INPUT));
}

int getMillis(HWND hWnd, int field) {
    BOOL bSuccess;
    int timeMillis = GetDlgItemInt(hWnd, field, &bSuccess, FALSE);
    if (bSuccess) {
        return timeMillis;
    } else {
        return -1;
    }
}

void CALLBACK Timerproc(HWND hWnd, UINT uMsg, UINT_PTR uintPtr, DWORD dwTime) {
    // stop the timer that sent this message
    KillTimer(hWnd, uintPtr);

    // get the message as a string
    char *bar = (char *) uintPtr;
    std::string s(bar);

    // start new timer for next substring if necessary
    if (s.length() > 1) {
        // check for \r\n
        if (s.rfind("\r\n", 0) == 0) {
            pressEnter();
            s = s.substr(1, s.length());
        } else {
            // fire keystroke
            SendMessage(hWnd, WM_COMMAND, IDC_PROCESS, reinterpret_cast<LPARAM>(new std::string(s.substr(0, 1))));
        }

        std::string the_str = s.substr(1, s.length() - 1);

        int intervalMillis = getMillis(hWnd, IDC_INTERVAL);
        SetTimer(hWnd, (UINT_PTR) (new std::string(the_str))->c_str(), intervalMillis, Timerproc);
    }
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    switch (Message) {
        case WM_INITDIALOG:
            SetDlgItemText(hwnd, IDC_TEXT, "This is a string");
            SetDlgItemInt(hwnd, IDC_INTERVAL, 250, FALSE);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_PROCESS: {
                    auto *msg = reinterpret_cast<std::string *>(lParam);
                    pressKeyB(msg->substr(0, 1).c_str()[0]);
                    delete msg;
                };
                    break;
                case IDC_START: {
                    int intervalMillis = getMillis(hwnd, IDC_DELAY);
                    int textLength = GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT));
                    if (intervalMillis >= 0 && textLength > 0) {
                        // Now we allocate, and get the string into our buffer
                        char *buf;
                        buf = (char *) GlobalAlloc(GPTR, textLength + 1);
                        GetDlgItemText(hwnd, IDC_TEXT, buf, textLength + 1);

                        // kick off the first timer
                        SetTimer(hwnd,                    // handle to main window
                                 (UINT_PTR) buf,          // ptr to string (Rather than timer id)
                                 intervalMillis,          // interval in millis
                                 (TIMERPROC) Timerproc);  // timer callback
                    }
                }
                    break;
            }
            break;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    return DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC) DlgProc);
}
