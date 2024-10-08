#include "host.h"

char* sel_host;
char* host_list;

INT_PTR WINAPI HostCfgHandle(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if(msg==WM_INITDIALOG) {
        /*
         * lParam 包含了外部传入的参数
         */
        char* plt=(char*)lParam;
        host_list=plt;
        sel_host=plt+256;
        WNDPROC pre_proc=(WNDPROC)GetWindowLongPtr(hwnd,GWLP_WNDPROC);
        Frame_InitialSettings(hwnd,DialogWindowProc);
        Frame_GetSettings(hwnd)->pre_proc=pre_proc;
        SetWindowLongPtr(hwnd,GWL_STYLE,~WS_MAXIMIZEBOX&GetWindowLongPtr(hwnd,GWL_STYLE));
        updatencwindow(hwnd);
        
        InitialCtrls(hwnd);
        CenterMsg(hwnd);
        return (INT_PTR)TRUE;
    } else if(msg==WM_CLOSE) {
        EndDialog(hwnd,(INT_PTR)0);
        return (INT_PTR)TRUE;
    } else if(msg==WM_COMMAND) {
        switch(LOWORD(wParam)) {
        case IDC_CONFIRM: {
            ComboBox_GetText(GetDlgItem(hwnd,IDC_HOST),sel_host,256);
            SendMessage(hwnd,WM_CLOSE,0,0);
        } break;
        case IDC_CANCEL: {
            SendMessage(hwnd,WM_CLOSE,0,0);
        } break;
        }
    }

    return (INT_PTR)FALSE;
}

int InitialCtrls(HWND hwnd) {
    HWND ctrl_host=GetDlgItem(hwnd,IDC_HOST);
    ComboBox_InitialSettings(ctrl_host,(char*)"主机地址");
    ComboBox_SetTitleOffset(ctrl_host,120);
    ComboBox_GetSettings(ctrl_host)->font_cy=18;
    ComboBox_GetSettings(ctrl_host)->is_multicheck=FALSE;
    ComboLBox_InitialSettings(ctrl_host,(char*)WC_EDIT);
    ComboBox_SetReadOnly(ctrl_host,FALSE);
    char* plt=host_list;
    while(*plt!='\0') {
        //host 列表
        ComboBox_AddItem(ctrl_host,plt,FALSE);
        plt+=strlen(plt)+1;
    }

    HWND confirm=GetDlgItem(hwnd,IDC_CONFIRM);
    Btn_InitialSettings(confirm);
    ApplyBtnStyle(confirm,ROUNDEDGE);

    HWND cancel=GetDlgItem(hwnd,IDC_CANCEL);
    Btn_InitialSettings(cancel);
    ApplyBtnStyle(cancel,ROUNDEDGE);
    return 0;
}

INT_PTR CALLBACK ConfirmHost(HINSTANCE hInst, HWND hWndParent, LPARAM lParam) {
    /*
     * 文档说 DialogBoxParam 这种是模态，但似乎无法阻止我对父窗口的操作.
     * CreateDialogParam 创建的是非模态
     * 
     */
    return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_HOSTCFG), hWndParent, HostCfgHandle, lParam);
}

