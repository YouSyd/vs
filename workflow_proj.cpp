#include "workflow_proj.h"
#include "workflow_ctrl.h"
#include "img_scratch.h"
#include "cmdconsole.h"

pwf_test f;
pwf_test initframemodule(HINSTANCE inst) {
    pwf_test p=(pwf_test)calloc(sizeof(wf_test),1);
    if(!p) return NULL;
    
    p->icon_app=LoadIcon(inst, MAKEINTRESOURCE(0x0003)/*IDC_ICON_DATAVIEW*/);
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
}

int WINAPI WinMain(HINSTANCE hinstance,HINSTANCE hpreinst,PSTR szcmdline,int icmdshow) {
    f=initframemodule(hinstance);
    return LAUNCH_FRAME(f->cx,f->cy,f->impl);
}

int WorkFlow_Test(HWND hwnd) {
    SetWindowLongPtr(hwnd,GWLP_HINSTANCE,(LONG_PTR)f->inst);
    Frame_InitialSettings(hwnd,f->wrapped_proc);
    SetWindowText(hwnd,f->title);
    SendMessage(hwnd,WM_SETICON,(WPARAM)ICON_SMALL,(LPARAM)f->icon_app);
    updatencwindow(hwnd);
    
    if(!f->extra) {
        pwfpj_param wfpj=(pwfpj_param)calloc(sizeof(wfpj_param),1);
        wfpj->frame=hwnd;
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

    register_window((char*)CTRL_CMDCONSOLE,f->inst,(WNDPROC)CmdViewProc);
    param->cmdview=CreateWindow(CTRL_CMDCONSOLE,"",WS_CHILD|WS_VISIBLE,
        rcClient.right-400,rcClient.top,400,rcClient.bottom-rcClient.top,
        hwnd,(HMENU)0x0003,f->inst,NULL);
    CmdView_InitialSettings(param->cmdview);

    char log_file[256]="E:\\SRC_REF\\tesseract\\self-bulid\\test\\vs\\pipe_exec_cmd.log";
    int fetched=0;
    char guid_up[50]="5F05D006-3C46-4EBF-B141-9E3EB1F25E6D";//"F44663F6-DFCD-47A4-B868-F58F9B84EC5X";//
    pCEL cel=(pCEL)calloc(sizeof(CEL)*10,1);
    int ret=FetchCELRecord(log_file,cel,&fetched,10,guid_up);
    CmdView_AttachCmdList(param->cmdview,cel,fetched);
    if(cel) free(cel);
        
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