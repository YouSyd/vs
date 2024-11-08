#include "trzcfg.h"

/*
 * 1、选择页面，然后根据选择的页面，调整页面的选区，这里有一个联动
 * 2、该界面需要一个传参中介，有两种思路：
 *     a) 程序的堆中有个地方，分配了所有页面的所有选区内容
 *     b) 一个中介，此处修改填充
 * 
 * 我的想法：
 *  外面分配内存
 *  此处根据页面和选区找到已经分配好的参数，完成填充/覆盖
 * 所以，逻辑是，根据页面、选区的设置，自动调出其已维护的信息，并进行修改
 * 由于位置信息由外部传入，所以这里只需要调整页面和选区
 * 当点击保存，数据会覆盖，点击取消，操作会取消
 */

TRZ trzcfg;

INT_PTR WINAPI TRZCfgHandle(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if(msg==WM_INITDIALOG) {
        WNDPROC pre_proc=(WNDPROC)GetWindowLongPtr(hwnd,GWLP_WNDPROC);
        Frame_InitialSettings(hwnd,DialogWindowProc);
        Frame_GetSettings(hwnd)->pre_proc=pre_proc;
        SetWindowLongPtr(hwnd,GWL_STYLE,~WS_MAXIMIZEBOX&GetWindowLongPtr(hwnd,GWL_STYLE));
        updatencwindow(hwnd);
        
        InitialCtrls(hwnd);
        CenterMsg(hwnd);
        return (INT_PTR)TRUE;
    } else if(msg==WM_CLOSE) {
        /*
         * 保存数据，写入 TRZ
         */
        //...

        EndDialog(hwnd,(INT_PTR)0);
        return (INT_PTR)TRUE;
    } else if(msg==WM_COMMAND) {
        switch(LOWORD(wParam)) {
        case IDC_CONFIRM: {
            /*
             * 根据设置的page 和 section
             * 回写 GMajor->trz
             */
            //...
            PAGECODE page=UNKNOWN_PAGE;
            TRZCODE  trzcode;
            int idx=0;
            if(-1!=(idx=ComboBox_GetCurSel(GetDlgItem(hwnd,IDC_PAGE)))) {
                char seltext[256]={0};
                ComboBox_GetLBText(GetDlgItem(hwnd,IDC_PAGE),idx,seltext);
                int itemkey=0;
                if(1==sscanf(seltext,"%d",&itemkey)) {
                    page=(PAGECODE)itemkey;
                }
            }
            if(-1!=(idx=ComboBox_GetCurSel(GetDlgItem(hwnd,IDC_ZONE)))) {
                char seltext[256]={0};
                ComboBox_GetLBText(GetDlgItem(hwnd,IDC_ZONE),idx,seltext);
                int itemkey=0;
                if(1==sscanf(seltext,"%d",&itemkey)) {
                    trzcode=(TRZCODE)itemkey;
                }
            }

            if(!GMajor||!GMajor->trz) {
                MessageBox(hwnd,"很抱歉指定选区尚未进行管理，可能是某些地方出现了问题。","异常",MB_ICONWARNING|MB_OK);
                break;
            }
            pTRZ trz=gettrzbycode(GMajor->trz,trzcode);
            if(trz&&(trz->height>0&&trz->width>0)) {
                char presettings[256]={0};
                sprintf(presettings,"指定选区的坐标已经进行了设置。\n(%d,%d) %d×%d\n你确定要修改吗？",trz->x,trz->y,trz->width,trz->height);
                if(IDYES!=MessageBox(hwnd,presettings,"提示",MB_YESNO|MB_ICONINFORMATION|MB_ICONQUESTION)) {
                    break;
                }
            } else if(!trz) {
                MessageBox(hwnd,"很抱歉指定选区尚未进行管理，可能是某些地方出现了问题。","异常",MB_ICONWARNING|MB_OK);
                break;
            }

            Edit_GetValue(GetDlgItem(hwnd,IDC_POSX),&trz->x);
            Edit_GetValue(GetDlgItem(hwnd,IDC_POSY),&trz->y);
            Edit_GetValue(GetDlgItem(hwnd,IDC_WIDTH),&trz->width);
            Edit_GetValue(GetDlgItem(hwnd,IDC_HEIGHT),&trz->height);

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
    COLORREF color_bk=RGB(15,15,15);
    HWND ctrl_x=GetDlgItem(hwnd,IDC_POSX);
    Edit_InitialSettings(ctrl_x,(char*)"选区坐标(x)",NULL);
    Edit_SetInternalStyle(ctrl_x,1);
    wchar_t x_buffer[256]={0};
    swprintf(x_buffer,L"%s",L"(x)");
    Edit_SetCueBannerText(ctrl_x,x_buffer);
    Edit_SetTitleOffset(ctrl_x,120);
    char xtext[256]="";
    sprintf(xtext,"%d",trzcfg.x);
    Edit_SetText(ctrl_x,xtext);
    SetWindowPos(ctrl_x,NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
    
    HWND ctrl_y=GetDlgItem(hwnd,IDC_POSY);
    Edit_InitialSettings(ctrl_y,(char*)"选区坐标(y)",NULL);
    Edit_SetInternalStyle(ctrl_y,1);
    wchar_t y_buffer[256]={0};
    swprintf(y_buffer,L"%s",L"(y)");
    Edit_SetCueBannerText(ctrl_y,y_buffer);
    Edit_SetTitleOffset(ctrl_y,120);
    char ytext[256]="";
    sprintf(ytext,"%d",trzcfg.y);
    Edit_SetText(ctrl_y,ytext);
    SetWindowPos(ctrl_y,NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
    
    HWND ctrl_page=GetDlgItem(hwnd,IDC_PAGE);
    ComboBox_InitialSettings(ctrl_page,(char*)"页面类型");
    ComboBox_SetTitleOffset(ctrl_page,120);
    ComboBox_GetSettings(ctrl_page)->font_cy=18;
    ComboBox_GetSettings(ctrl_page)->is_multicheck=FALSE;
    ComboLBox_InitialSettings(ctrl_page,(char*)WC_EDIT);
    ComboBox_SetReadOnly(ctrl_page,FALSE);
    int page_count=sizeof(page_name)/sizeof(char[256]);
    for(int idx=0;idx<page_count&&page_name[idx][0]!='\0';idx++) {
        char buffer[256]={0};
        sprintf(buffer,"%d：%s",idx,page_name[idx]);
        ComboBox_AddItem(ctrl_page,buffer,FALSE);
    }
    ComboBox_SetCurSel(ctrl_page,trzcfg.page);

    HWND ctrl_zone=GetDlgItem(hwnd,IDC_ZONE);
    ComboBox_InitialSettings(ctrl_zone,(char*)"页面选区");
    ComboBox_SetTitleOffset(ctrl_zone,120);
    ComboBox_GetSettings(ctrl_zone)->font_cy=18;
    ComboBox_GetSettings(ctrl_zone)->is_multicheck=FALSE;
    ComboLBox_InitialSettings(ctrl_zone,(char*)WC_EDIT);
    ComboBox_SetReadOnly(ctrl_zone,FALSE);
    int zone_count=sizeof(section_name)/sizeof(char[256]);
    if(trzcfg.page==0)
        for(int idx=0;idx<zone_count&&section_name[idx][0]!='\0';idx++) {
            char buffer[256]={0};
            sprintf(buffer,"%d：%s",idx,section_name[idx]);
            ComboBox_AddItem(ctrl_zone,buffer,FALSE);
        }
    else {
        for(int idx=0;idx<sections_counts;idx++) {
            if(sections[idx].page==trzcfg.page) {
                char buffer[256]={0};
                sprintf(buffer,"%d：%s",sections[idx].code,section_name[sections[idx].code]);
                ComboBox_AddItem(ctrl_zone,buffer,FALSE);
            }
        }
    }
    ComboBox_SetCurSel(ctrl_zone,trzcfg.code);

    HWND ctrl_width=GetDlgItem(hwnd,IDC_WIDTH);
    Edit_InitialSettings(ctrl_width,(char*)"选区宽度",NULL);
    Edit_SetInternalStyle(ctrl_width,1);
    wchar_t width_buffer[256]={0};
    swprintf(width_buffer,L"%s",L"(width)");
    Edit_SetCueBannerText(ctrl_width,width_buffer);
    Edit_SetTitleOffset(ctrl_width,120);
    char widthtext[256]="";
    sprintf(widthtext,"%d",trzcfg.width);
    Edit_SetText(ctrl_width,widthtext);
    SetWindowPos(ctrl_width,NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);

    HWND ctrl_height=GetDlgItem(hwnd,IDC_HEIGHT);
    Edit_InitialSettings(ctrl_height,(char*)"选区高度",NULL);
    Edit_SetInternalStyle(ctrl_height,1);
    wchar_t height_buffer[256]={0};
    swprintf(height_buffer,L"%s",L"(height)");
    Edit_SetCueBannerText(ctrl_height,height_buffer);
    Edit_SetTitleOffset(ctrl_height,120);
    char heighttext[256]="";
    sprintf(heighttext,"%d",trzcfg.height);
    Edit_SetText(ctrl_height,heighttext);
    SetWindowPos(ctrl_height,NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
    
    HWND ctrl_traindataset=GetDlgItem(hwnd,IDC_TRAINDATASET);
    Edit_InitialSettings(ctrl_traindataset,(char*)"OCR训练集",NULL);
    Edit_SetInternalStyle(ctrl_traindataset,0);
    wchar_t traindataset_buffer[256]={0};
    swprintf(traindataset_buffer,L"%s",L"(eng/chi_sim...)");
    Edit_SetCueBannerText(ctrl_traindataset,traindataset_buffer);
    Edit_SetTitleOffset(ctrl_traindataset,120);
    //Edit_SetText(ctrl_width,rsicfg.product_name);
    SetWindowPos(ctrl_traindataset,NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
    
    HWND ctrl_whitelist=GetDlgItem(hwnd,IDC_WHITELIST);
    Edit_InitialSettings(ctrl_whitelist,(char*)"字符白名单",NULL);
    Edit_SetInternalStyle(ctrl_whitelist,0);
    wchar_t whitelist_buffer[256]={0};
    swprintf(whitelist_buffer,L"%s",L"(白名单)");
    Edit_SetCueBannerText(ctrl_whitelist,whitelist_buffer);
    Edit_SetTitleOffset(ctrl_whitelist,120);
    //Edit_SetText(ctrl_width,rsicfg.product_name);
    SetWindowPos(ctrl_whitelist,NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);

    HWND confirm=GetDlgItem(hwnd,IDC_CONFIRM);
    Btn_InitialSettings(confirm);
    ApplyBtnStyle(confirm,ROUNDEDGE);

    HWND cancel=GetDlgItem(hwnd,IDC_CANCEL);
    Btn_InitialSettings(cancel);
    ApplyBtnStyle(cancel,ROUNDEDGE);
    return 0;
}

