#include "projcfg.h"

int* pAttendCountDown;

INT_PTR WINAPI ProjCfgHandle(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if(msg==WM_INITDIALOG) {
        /*
         * lParam 包含了外部传入的参数
         */
        int* plt=(int*)lParam;
        pAttendCountDown=plt;
        
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
            Edit_GetValue(GetDlgItem(hwnd,IDC_ATTEND),pAttendCountDown);
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
    HWND ctrl_attend=GetDlgItem(hwnd,IDC_ATTEND);
    Edit_InitialSettings(ctrl_attend,(char*)"参与时机(秒)",NULL);
    Edit_SetInternalStyle(ctrl_attend,1);
    wchar_t attend_buffer[256]={0};
    swprintf(attend_buffer,L"%s",L"设定参与距离开奖的秒数");
    Edit_SetCueBannerText(ctrl_attend,attend_buffer);
    Edit_SetTitleOffset(ctrl_attend,120);
    char attend_text[256]="";
    sprintf(attend_text,"%d",*pAttendCountDown);
    Edit_SetText(ctrl_attend,attend_text);
    SetWindowPos(ctrl_attend,NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
    
    HWND confirm=GetDlgItem(hwnd,IDC_CONFIRM);
    Btn_InitialSettings(confirm);
    ApplyBtnStyle(confirm,ROUNDEDGE);

    HWND cancel=GetDlgItem(hwnd,IDC_CANCEL);
    Btn_InitialSettings(cancel);
    ApplyBtnStyle(cancel,ROUNDEDGE);
    return 0;
}

INT_PTR CALLBACK ConfirmProj(HINSTANCE hInst, HWND hWndParent, LPARAM lParam) {
    /*
     * 文档说 DialogBoxParam 这种是模态，但似乎无法阻止我对父窗口的操作.
     * CreateDialogParam 创建的是非模态
     * 
     */
    return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PROJCFG),hWndParent,ProjCfgHandle,lParam);
}

