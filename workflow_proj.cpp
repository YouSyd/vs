#include "workflow_proj.h"
#include "workflow_ctrl.h"
#include "img_scratch.h"
#include "cmdconsole.h"

pwf_test f;
pwf_test initframemodule(HINSTANCE inst) {
    pwf_test p=(pwf_test)calloc(sizeof(wf_test),1);
    if(!p) return NULL;
    
    p->icon_app=LoadIcon(inst, MAKEINTRESOURCE(0x0001)/*IDC_ICON_DATAVIEW*/);
    p->wrapped_proc=TestProc;
    p->cx=1000;p->cy=600;
    p->inst=inst;
    p->impl=WorkFlow_Test;
    strcpy(p->title,"Work Flow Test.");
    
    return p;
}

pwf_test getframemodule() { return f;}

void clearframemodule(pwf_test p) {
    if(!p) return;
    if(p->extra) free((pwfpj_param)f->extra);
    free(p);

    clearmajor(GMajor);
    GMajor=NULL;
}

int WINAPI WinMain(HINSTANCE hinstance,HINSTANCE hpreinst,PSTR szcmdline,int icmdshow) {
    f=initframemodule(hinstance);
    return LAUNCH_FRAME(f->cx,f->cy,f->impl);
}

int WorkFlow_Test(HWND hwnd) {
    SetWindowLongPtr(hwnd,GWLP_HINSTANCE,(LONG_PTR)f->inst);
    Frame_InitialSettings(hwnd,f->wrapped_proc);
    //取消WS_HSCROLL
    SetWindowLongPtr(hwnd,GWL_STYLE,(((UINT)GetWindowLongPtr(hwnd,GWL_STYLE))&(~WS_HSCROLL)));
    SetWindowText(hwnd,f->title);
    SendMessage(hwnd,WM_SETICON,(WPARAM)ICON_SMALL,(LPARAM)f->icon_app);
    updatencwindow(hwnd);
    
    pFrameStyle fs=Frame_GetSettings(hwnd);
    if(fs) {
        NCPaintEx ncp={
            ProjCfgPaint,
            ProjCfgZone,
            (NCZoneClick)ProjCfgClick
        };
        Frame_AppendNCPaint(hwnd,&ncp);

        NCPaintEx ncptimer={
            ProjTimerPaint,
            ProjTimerZone,
            (NCZoneClick)ProjTimerClick
        };
        Frame_AppendNCPaint(hwnd,&ncptimer);
    }

    if(!f->extra) {
        pwfpj_param wfpj=(pwfpj_param)calloc(sizeof(wfpj_param),1);
        wfpj->frame=hwnd;
        wfpj->ocrthread=NULL;
        wfpj->ocrthreadsuspend_ref=0;
        f->extra=wfpj;
    }
    /*
     * 注册 workflow 控件
     */
    register_window((char*)CTRL_WORKFLOWVIEW,f->inst,(WNDPROC)WorkFlowViewProc);
    /*
     * 我们创建一个workflowview 控件，以便测试
     */
    RECT rcClient;
    GetClientRect(hwnd,&rcClient);
    pwfpj_param param=(pwfpj_param)f->extra;
    if(!param) return -1;
    param->workflow=CreateWindow(CTRL_WORKFLOWVIEW,"",
        WS_CHILD|WS_VISIBLE,
        0,0,350,rcClient.bottom,
        hwnd,(HMENU)0x0001,f->inst,NULL);
    WorkFlowView_InitialSettings(param->workflow);
    NodeMap_Test(param->workflow);

    register_window((char*)CTRL_IMGSCRATCH,f->inst,(WNDPROC)ScratchViewProc);
    param->imgsratch=CreateWindow(CTRL_IMGSCRATCH,"",
        WS_CHILD|WS_VISIBLE,
        350,0,rcClient.right-400-300,(rcClient.bottom-rcClient.top),
        hwnd,(HMENU)0x0002,f->inst,NULL);
    ScratchView_InitialSettings(param->imgsratch);
    ScratchView_SetImage(param->imgsratch,(char*)"E:\\SRC_REF\\tesseract\\self-bulid\\test\\pics\\x.png");

    DWORD threadid;
    param->ocrthread=CreateThread(NULL,0,DouYinOCRJOB,NULL,CREATE_SUSPENDED,&threadid);
    param->ocrthreadsuspend_ref++;

    register_window((char*)CTRL_CMDCONSOLE,f->inst,(WNDPROC)CmdViewProc);
    param->cmdview=CreateWindow(CTRL_CMDCONSOLE,"",WS_CHILD|WS_VISIBLE,
        rcClient.right-400,rcClient.top,400,rcClient.bottom-rcClient.top,
        hwnd,(HMENU)0x0003,f->inst,NULL);
    CmdView_InitialSettings(param->cmdview);
    /*
     * 设置静默运行
     */
    CmdView_GetSettings(param->cmdview)->slience_mode=1;
    

    // char log_file[256]="E:\\SRC_REF\\tesseract\\self-bulid\\test\\vs\\pipe_exec_cmd.log";
    // int fetched=0;
    // char guid_up[50]="5F05D006-3C46-4EBF-B141-9E3EB1F25E6D";//"F44663F6-DFCD-47A4-B868-F58F9B84EC5X";//
    // pCEL cel=(pCEL)calloc(sizeof(CEL)*10,1);
    // int ret=FetchCELRecord(log_file,cel,&fetched,10,guid_up);
    // CmdView_AttachCmdList(param->cmdview,cel,fetched);
    // if(cel) free(cel);

    GMajor=initialmajor();
        
    return 0;
}

int NodeMap_Test(HWND hwnd) {
    pMAPS mps=WorkFlowView_GetSettings(hwnd);
    if(!mps) return -1;
        
    pNodeMap map=mps->map;
    NODEMAP_addnode(map,(char*)"开始");//1
    NODEMAP_addnode(map,(char*)"直播间主界面");//2
    NODEMAP_addnode(map,(char*)"福袋详情界面");//3
    NODEMAP_addnode(map,(char*)"福袋中奖界面");//4
    NODEMAP_addnode(map,(char*)"领奖界面"); //5
    NODEMAP_addnode(map,(char*)"福袋未中奖界面");//6
    NODEMAP_addnode(map,(char*)"直播间关闭界面");
    NODEMAP_addnode(map, (char*)"退出");
    
    NODEMAP_connect(map,1,2);
    NODEMAP_connect(map,2,3);
    NODEMAP_connect(map,3,4);
    NODEMAP_connect(map,3,6);
    NODEMAP_connect(map,4,5);
    NODEMAP_connect(map,5,8);
    NODEMAP_connect(map,1,7);
    
    NODEMAP_map(map);
    WorkFlowView_DefaultLayout(hwnd);
    
    return 0;
}

LRESULT CALLBACK TestProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
    LRESULT ret=0;
    pFrameStyle fs=Frame_GetSettings(hwnd);
    
    if(msg==WM_ERASEBKGND) return 1;

    if(fs&&fs->proc) ret=CallWindowProc(proc_window,hwnd,msg,wParam,lParam);
    else ret=CallWindowProc(DefWindowProc,hwnd,msg,wParam,lParam);
        
    if(msg==WM_DESTROY) {
        clearframemodule(f);
    } else if(msg==WM_SIZE) {
        HWND wf_hwnd=GetDlgItem(hwnd,0x0001);
        HWND is_hwnd=GetDlgItem(hwnd,0x0002);
        HWND cv_hwnd=GetDlgItem(hwnd,0x0003);
        int cx=LOWORD(lParam);
        int cy=HIWORD(lParam);
        SetWindowPos(wf_hwnd,NULL,0,0,350,cy,SWP_NOZORDER|SWP_SHOWWINDOW);
        SetWindowPos(is_hwnd,NULL,350,0,cx-350-400,cy,SWP_NOZORDER|SWP_SHOWWINDOW);
        SetWindowPos(cv_hwnd,NULL,cx-400,0,400,cy,SWP_NOZORDER|SWP_SHOWWINDOW);
    }
    
    return ret;
}

/*
typedef int (*NCPaintPlt)(HWND,HDC,const LPRECT,POINT);
typedef int (*NCZoneCalc)(HWND,EFNCZone,LPRECT,BOOL);
typedef struct ncpaintex {
    NCPaintPlt paint;
    NCZoneCalc zone_calc;
    struct ncpaintex* next;
}NCPaintEx,*pNCPaintEx;
 */
int ProjCfgPaint(HWND hwnd,HDC hdc,CONST LPRECT prc,POINT cur) {
    RECT rcCfg,rcIcon;
    if(0!=ProjCfgZone(hwnd,(EFNCZone)0,&rcCfg,TRUE)) return -1;
    CopyRect(&rcIcon,&rcCfg);
    InflateRect(&rcIcon,-2,-2);
    //获取配置icon
    HICON icoCfg=LoadIcon(f->inst,MAKEINTRESOURCE(0x0003));
    HICON icoCfgHover=LoadIcon(f->inst,MAKEINTRESOURCE(0x0002));
    if(PtInRect(&rcCfg,cur)) DrawIconEx(hdc,rcIcon.left,rcIcon.top,icoCfgHover,rcIcon.right-rcIcon.left,rcIcon.bottom-rcIcon.top,0,0,DI_NORMAL);
    else DrawIconEx(hdc,rcIcon.left,rcIcon.top,icoCfg,rcIcon.right-rcIcon.left,rcIcon.bottom-rcIcon.top,0,0,DI_NORMAL);

    return 0;
}

int ProjCfgZone(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL allign_flag) {
    RECT rcCfg;
    Frame_GetNCZoneRect(hwnd,ZMIN,&rcCfg,allign_flag);
    OffsetRect(&rcCfg,-(rcCfg.right-rcCfg.left+5),0);
    CopyRect(prc,&rcCfg);
    return 0;
}

int ProjCfgClick(HWND hwnd,EFNCZone zone) {
    if(!f||!f->extra||!GMajor) {
        MessageBox(hwnd,"Proj 配置参数无效或尚未初始化！","异常",MB_OK|MB_ICONINFORMATION);
        return -1;
    }
    pwfpj_param wfpj=(pwfpj_param)f->extra;
    HMODULE module_proj=LoadLibrary("projcfg.dll");
    if(module_proj) {
        SHOWDIALOGPROC cfmproj=(SHOWDIALOGPROC)GetProcAddress(module_proj,"ConfirmProj");
        if(cfmproj) {
            void* projaddr[]={
                &GMajor->attend_countdown,
                &datapath[0],
                &GMajor->strategycode
            };
            cfmproj((HINSTANCE)module_proj,wfpj->frame,(LPARAM)projaddr);
        } else {
            MessageBox(hwnd,"fuck, proc addr not found.","异常",MB_OK|MB_ICONINFORMATION);
        }
        FreeLibrary(module_proj);
    } else {
        MessageBox(hwnd,"加载Proj配置失败","异常",MB_OK|MB_ICONINFORMATION);
    }
    return 0;
}

int ProjTimerPaint(HWND hwnd,HDC hdc,CONST LPRECT prc,POINT cur) {
    /*
     * 获取 GMajor
     */
    if(!GMajor) return 0;
    // fg_countdown - 截屏的 倒计时
    // timestamp - 截屏时的 倒计时
    // fg_countdown + timestamp - time(NULL) ---> 倒计时还剩多久
    // 现在逻辑调整为 [0 ~ attend_countdown]
    int countdown_seconds=GMajor->fg_countdown+GMajor->timestamp-time(NULL);
    if(countdown_seconds<=0||countdown_seconds>GMajor->attend_countdown&&GMajor->attend_countdown!=0) return 0;

    RECT rcTimer;
    if(0!=ProjTimerZone(hwnd,(EFNCZone)0,&rcTimer,TRUE)) return -1;
    BOOL isHover=PtInRect(&rcTimer,cur);
    InflateRect(&rcTimer,0,-2);
    Gdiplus::Graphics graphics(hdc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::SolidBrush brush(isHover?Gdiplus::Color(0,100,200):Gdiplus::Color(50,51,50));  // 绿色画刷
    
    // 创建圆角矩形路径
    Gdiplus::GraphicsPath path;
    int diameter=rcTimer.bottom-rcTimer.top;  // 半圆直径
    path.AddArc(rcTimer.left,rcTimer.top,diameter,diameter,90,180);
    path.AddLine(rcTimer.left+diameter/2,rcTimer.top, rcTimer.right-diameter/2,rcTimer.top);
    path.AddArc(rcTimer.right-diameter,rcTimer.top,diameter,diameter,270,180);
    path.AddLine(rcTimer.right-diameter/2,rcTimer.bottom,rcTimer.left+diameter/2,rcTimer.bottom);
    
    // 填充路径
    graphics.FillPath(&brush, &path);
    // 设置字体
    Gdiplus::FontFamily fontFamily(L"Consolas");
    Gdiplus::Font font(&fontFamily, rcTimer.bottom-rcTimer.top-2,Gdiplus::FontStyleRegular,Gdiplus::UnitPixel);
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentFar); // 右对齐
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter); // 垂直居中

    // 设置文本区域
    Gdiplus::RectF layoutRect(rcTimer.left+5,rcTimer.top+1,rcTimer.right-rcTimer.left-10,rcTimer.bottom-rcTimer.top-2);

    int minutes=countdown_seconds/60;
    int seconds=countdown_seconds%60;
    char countdown_str[256]={0};
    sprintf(countdown_str,"%02d:%02d",minutes,seconds);
    // 将 countdown_str 转换为宽字符
    wchar_t wcountdown_str[256];
    mbstowcs(wcountdown_str, countdown_str, strlen(countdown_str) + 1);
    // 绘制文本
    // 绘制文本
    Gdiplus::SolidBrush textBrush((countdown_seconds<=30)?Gdiplus::Color(255,255,0,0):Gdiplus::Color(255, 255, 255, 255)); // 白色画刷
    graphics.DrawString(wcountdown_str, -1, &font, layoutRect, &format, &textBrush);

    return 0;

}

int ProjTimerZone(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL allign_flag) {
    RECT rcTimer;
    Frame_GetNCZoneRect(hwnd,ZMIN,&rcTimer,allign_flag);
    OffsetRect(&rcTimer,-(rcTimer.right-rcTimer.left+5)*2,0);
    CopyRect(prc,&rcTimer);
    prc->left=prc->right-100;

    return 0;
}

int ProjTimerClick(HWND hwnd,EFNCZone zone) {
    if(!GMajor) return 0;
    int countdown_seconds=GMajor->fg_countdown+GMajor->timestamp-time(NULL);
    if(countdown_seconds<=0||countdown_seconds>GMajor->attend_countdown&&GMajor->attend_countdown!=0) return -1;

    //MessageBox(hwnd,"paint...","",MB_OK);
    return 0;
}
