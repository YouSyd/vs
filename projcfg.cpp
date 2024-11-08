#include "projcfg.h"

int* pAttendCountDown;
char* pOCRPath;
int* pStrategyCode;

INT_PTR WINAPI ProjCfgHandle(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if(msg==WM_INITDIALOG) {
        /*
         * lParam 包含了外部传入的参数
         */
        int* plt=(int*)*((void**)lParam+0);
        pAttendCountDown=plt;
        pOCRPath=(char*)*((void**)lParam+1);
        pStrategyCode=(int*)*((void**)lParam+2);
        
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
            Edit_GetValue(GetDlgItem(hwnd,IDC_TESSOCRFOLDER),pOCRPath);
            *pStrategyCode=ComboBox_GetCurSel(GetDlgItem(hwnd,IDC_EXECSTRATEGY));
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
    
    HWND ctrl_tessorcfolder=GetDlgItem(hwnd,IDC_TESSOCRFOLDER);
    Edit_InitialSettings(ctrl_tessorcfolder,(char*)"OCR训练集",NULL);
    Edit_SetInternalStyle(ctrl_tessorcfolder,0);
    wchar_t ocr_buffer[256]={0};
    swprintf(ocr_buffer,L"%s",L"Tessseract OCR(Training data)目录");
    Edit_SetCueBannerText(ctrl_tessorcfolder,ocr_buffer);
    Edit_SetTitleOffset(ctrl_tessorcfolder,120);
    // char ocr_text[256]="";
    // sprintf(ocr_text,"%s",pOCRPath);
    Edit_SetText(ctrl_tessorcfolder,pOCRPath);
    SetWindowPos(ctrl_tessorcfolder,NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
    
    HWND ctrl_strategy=GetDlgItem(hwnd,IDC_EXECSTRATEGY);
    ComboBox_InitialSettings(ctrl_strategy,(char*)"页面切换策略");
    ComboBox_SetTitleOffset(ctrl_strategy,120);
    ComboBox_GetSettings(ctrl_strategy)->font_cy=18;
    ComboBox_GetSettings(ctrl_strategy)->is_multicheck=FALSE;
    ComboLBox_InitialSettings(ctrl_strategy,(char*)WC_EDIT);
    ComboBox_SetReadOnly(ctrl_strategy,FALSE);
    char strategies[][256]={
        "持续运行",
        "中奖后自动退出并关机",
        "持续执行2小时后关机",
        "中奖后切换其它关注的直播间"
    };
    int strategy_count=sizeof(strategies)/sizeof(char[256]);
    for(int idx=0;idx<strategy_count;idx++) {
        char buffer[256]={0};
        sprintf(buffer,"%d：%s",idx,strategies[idx]);
        ComboBox_AddItem(ctrl_strategy,buffer,FALSE);
    }
    ComboBox_SetCurSel(ctrl_strategy,*pStrategyCode);

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

