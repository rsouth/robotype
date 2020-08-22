#include <windows.h>

#include "resource.h"
#include <memory>

//#include <cstring>
#include <string>

//#define WINVER 0x0500

void pressEnter()
{
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    input.ki.wScan = 0;
    input.ki.dwFlags = 0;

    input.ki.wVk = VK_RETURN;
    SendInput( 1, &input, sizeof( INPUT ) );
}

void pressKeyB(char mK)
{
    HKL kbl = GetKeyboardLayout(0);
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.time = 0;
    ip.ki.dwFlags = KEYEVENTF_UNICODE;
    if ((int)mK<65 && (int)mK>90) //for lowercase
    {
        ip.ki.wScan = 0;
        ip.ki.wVk = VkKeyScanEx( mK, kbl );
    }
    else //for uppercase
    {
        ip.ki.wScan = mK;
        ip.ki.wVk = 0;

    }
    ip.ki.dwExtraInfo = 0;
    SendInput(1, &ip, sizeof(INPUT));
}

int getIntervalMillis(HWND hWnd) {
    BOOL bSuccess;
    int intervalSeconds = GetDlgItemInt(hWnd, IDC_INTERVAL, &bSuccess, FALSE);
    if(bSuccess) {
        return intervalSeconds;
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

    // keystroke
    SendMessage(hWnd, WM_COMMAND, 1111, reinterpret_cast<LPARAM>(new std::string(s.substr(0, 1))));

    // start new timer for next substring if necessary
    if (s.length() > 1) {

        // check for \r\n
        if(s.length() >= 2 && s.substr(0, 2) == "\r\n") {
            pressEnter();
            if(s.length() >= 3) {
                s = s.substr(1, s.length() - 1);
            }
        }

        std::string the_str = s.substr(1, s.length() - 1);

        int intervalMillis = getIntervalMillis(hWnd);
        SetTimer(hWnd, (UINT_PTR) (new std::string(the_str))->c_str(), intervalMillis, Timerproc);
    }
}



BOOL CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    switch (Message) {
        case WM_INITDIALOG:
            // This is where we set up the dialog box, and initialise any default values

            SetDlgItemText(hwnd, IDC_TEXT, "This is a string");
            SetDlgItemInt(hwnd, IDC_INTERVAL, 1, FALSE);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case 1111: // use for testing, shows a message dialog of lParam
                {
                    auto *msg = reinterpret_cast<std::string *>(lParam);
//                    MessageBox(hwnd, msg->c_str(), "Warning", MB_OK);

                    pressKeyB(msg->substr(0, 1).c_str()[0]);

                    delete msg;
                }
                    break;
                case IDC_START: {
                    int intervalMillis = getIntervalMillis(hwnd);
                    int textLength = GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT));
                    if (intervalMillis >= 0 && textLength > 0) {
                        // Now we allocate, and get the string into our buffer
                        char *buf;
                        buf = (char *) GlobalAlloc(GPTR, textLength + 1);
                        GetDlgItemText(hwnd, IDC_TEXT, buf, textLength + 1);

                        // kick off the first timer
                        SetTimer(hwnd,                     // handle to main window
                                 (UINT_PTR) buf,           // ptr to string (Rather than timer id)
                                 intervalMillis,   // interval in millis
                                 (TIMERPROC) Timerproc);   // timer callback
                    }
                }
                    break;
                case IDC_ADD: {
                    // When somebody clicks the Add button, first we get the number of
                    // they entered

                    BOOL bSuccess;
                    int nTimes = GetDlgItemInt(hwnd, IDC_INTERVAL, &bSuccess, FALSE);
                    if (bSuccess) {
                        // Then we get the string they entered
                        // First we need to find out how long it is so that we can
                        // allocate some memory

                        int len = GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT));
                        if (len > 0) {
                            // Now we allocate, and get the string into our buffer

                            int i;
                            char *buf;

                            buf = (char *) GlobalAlloc(GPTR, len + 1);
                            GetDlgItemText(hwnd, IDC_TEXT, buf, len + 1);

                            // Now we add the string to the list box however many times
                            // the user asked us to.

                            for (i = 0; i < nTimes; i++) {
                                int index = SendDlgItemMessage(hwnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM) buf);

                                // Here we are associating the value nTimes with the item
                                // just for the heck of it, we'll use it to display later.
                                // Normally you would put some more useful data here, such
                                // as a pointer.
                                SendDlgItemMessage(hwnd, IDC_LIST, LB_SETITEMDATA, (WPARAM) index, (LPARAM) nTimes);
                            }

                            // Dont' forget to free the memory!
                            GlobalFree((HANDLE) buf);
                        } else {
                            MessageBox(hwnd, "You didn't enter anything!", "Warning", MB_OK);
                        }
                    } else {
                        MessageBox(hwnd, "Couldn't translate that number :(", "Warning", MB_OK);
                    }

                }
                    break;
                case IDC_REMOVE: {
                    // When the user clicks the Remove button, we first get the number
                    // of selected items

                    HWND hList = GetDlgItem(hwnd, IDC_LIST);
                    int count = SendMessage(hList, LB_GETSELCOUNT, 0, 0);
                    if (count != LB_ERR) {
                        if (count != 0) {
                            // And then allocate room to store the list of selected items.

                            int i;
                            int *buf = (int *) GlobalAlloc(GPTR, sizeof(int) * count);
                            SendMessage(hList, LB_GETSELITEMS, (WPARAM) count, (LPARAM) buf);

                            // Now we loop through the list and remove each item that was
                            // selected.

                            // WARNING!!!
                            // We loop backwards, because if we removed items
                            // from top to bottom, it would change the indexes of the other
                            // items!!!

                            for (i = count - 1; i >= 0; i--) {
                                SendMessage(hList, LB_DELETESTRING, (WPARAM) buf[i], 0);
                            }

                            GlobalFree(buf);
                        } else {
                            MessageBox(hwnd, "No items selected.", "Warning", MB_OK);
                        }
                    } else {
                        MessageBox(hwnd, "Error counting items :(", "Warning", MB_OK);
                    }
                }
                    break;
                case IDC_CLEAR:
                    SendDlgItemMessage(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
                    break;
                case IDC_LIST:
                    switch (HIWORD(wParam)) {
                        case LBN_SELCHANGE: {
                            // Get the number of items selected.

                            HWND hList = GetDlgItem(hwnd, IDC_LIST);
                            int count = SendMessage(hList, LB_GETSELCOUNT, 0, 0);
                            if (count != LB_ERR) {
                                // We only want to continue if one and only one item is
                                // selected.

                                if (count == 1) {
                                    // Since we know ahead of time we're only getting one
                                    // index, there's no need to allocate an array.

                                    int index;
                                    int err = SendMessage(hList, LB_GETSELITEMS, (WPARAM) 1, (LPARAM) &index);
                                    if (err != LB_ERR) {
                                        // Get the data we associated with the item above
                                        // (the number of times it was added)

                                        int data = SendMessage(hList, LB_GETITEMDATA, (WPARAM) index, 0);

                                        SetDlgItemInt(hwnd, IDC_SHOWCOUNT, data, FALSE);
                                    } else {
                                        MessageBox(hwnd, "Error getting selected item :(", "Warning", MB_OK);
                                    }
                                } else {
                                    // No items selected, or more than one
                                    // Either way, we aren't going to process this.
                                    SetDlgItemText(hwnd, IDC_SHOWCOUNT, "-");
                                }
                            } else {
                                MessageBox(hwnd, "Error counting items :(", "Warning", MB_OK);
                            }
                        }
                            break;
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
    return DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DlgProc);
}
