#include "host_ctrl.h"

#include "dialog.h"
#include "frame.h"
#include "button.h"

INT_PTR WINAPI HostCfgHandle(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int InitialCtrls(HWND hwnd);

// typedef INT_PTR (CALLBACK* SHOWDIALOGPROC)(HINSTANCE, HWND, LPARAM);
extern "C" __declspec(dllexport) INT_PTR CALLBACK ConfirmHost(HINSTANCE hInst, HWND hWndParent, LPARAM lParam);
