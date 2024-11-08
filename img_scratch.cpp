#include "img_scratch.h"
#include "ydark.h"
#include "trzcfg_ctrl.h"
#include "trzcfg.h"
#include <gdiplus.h>
#include "scratchview_ctrl.h"

#include "workflow_proj.h"

LRESULT CALLBACK ScratchViewProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
    LRESULT ret=0;

    switch(msg) {
    case WM_INITMENUPOPUP: {
        HMENU menu=(HMENU)wParam;
        int pos=(UINT)LOWORD(lParam);
        BOOL sysmenu_flag=(BOOL)HIWORD(lParam);
        int item_count;
        MENUINFO mif={0};
        MENUITEMINFO miif={0};
        
        pScratchView psv=ScratchView_GetSettings(hwnd);
        if(!psv) return 0;
        
        item_count=GetMenuItemCount(menu);
        for(int index=0;index<item_count;index++) {
            miif.cbSize=sizeof(miif);
            miif.fMask=MIIM_STATE|MIIM_ID;
            GetMenuItemInfo(menu,index,TRUE,&miif);
            
            miif.cbSize=sizeof(miif);
            miif.fMask=MIIM_ID|MIIM_FTYPE;
            miif.fType=MFT_OWNERDRAW;//Menu Item Owner drawing
            SetMenuItemInfo(menu,index,TRUE,&miif);
        }
        //menu item backgnd color settings.
        mif.cbSize=sizeof(MENUINFO);
        mif.fMask=MIM_BACKGROUND;
        mif.hbrBack=psv->menubkbrush;
        SetMenuInfo(menu,&mif);

        
        //查找菜单窗口
        //HWND hwndMenu=FindWindowEx(hwnd,NULL,"#32768",NULL);
        HWND hwndMenu=FindWindow("#32768",NULL);
        HINSTANCE instanceMenu=(HINSTANCE)GetWindowLongPtr(hwndMenu,GWLP_HINSTANCE);
        HINSTANCE instance=(HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE);
        if(hwndMenu&&instanceMenu==instance) {
            /*
             * 设置窗口过程，并重绘窗口
             */
            PopupMenu_InitialSettings(hwndMenu);
        }

    } break;
    case WM_MEASUREITEM: {
        LPMEASUREITEMSTRUCT lpmis;
        lpmis=(LPMEASUREITEMSTRUCT)lParam;
        
        if(lpmis->CtlType==ODT_MENU) {
            UINT menu_item_id=lpmis->itemID;
            lpmis->itemWidth=200;
            lpmis->itemHeight=((menu_item_id==0x0)?5:28);//menu item & menu spliter

            //放弃该方式
            //if(menu_item_id==ID_OCRRECOGN_RECT) lpmis->itemHeight=28+15;
        }
    } break;
	case WM_DRAWITEM: {
	    UINT ctrl_id=(UINT)wParam;
	    LPDRAWITEMSTRUCT pDraw=(LPDRAWITEMSTRUCT)lParam;
	    
	    if(pDraw->CtlType==ODT_MENU) {
	        PopupMenu_DrawMenuItem(hwnd,pDraw);
	    }
	} break;        
    case WM_CONTEXTMENU: {
        pScratchView psv=ScratchView_GetSettings(hwnd);
        if(!psv) return 1;

        POINT pt={
            GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam)
        };

        if(psv->mode==2||(psv->mode==1&&psv->active_smpidx!=0)) {
            //获取dll hinstance
            HMODULE ctrl_module=LoadLibrary("ScratchView.dll");
            if(ctrl_module==NULL) return 1;


            //...
            HMENU menu=LoadMenu((HINSTANCE)ctrl_module,MAKEINTRESOURCE(IDR_SCRATCHMENU));
            HMENU submenu=GetSubMenu(menu,0);
            TrackPopupMenu(submenu,TPM_LEFTBUTTON,pt.x,pt.y,0,hwnd,NULL);

            DestroyMenu(menu);
            FreeLibrary(ctrl_module);
            return 0;
        }
    } break;
    case WM_DROPFILES: {
        HDROP  hdrop=(HDROP)wParam;
        UINT filecount=DragQueryFile(hdrop,0xFFFFFFFF,NULL,0);
        if(filecount>1) {
            MessageBox(hwnd,"请最多选中一个文件进行拖拽!","提示",MB_ICONINFORMATION|MB_OK);
            DragFinish(hdrop);
            return 0;
        } else {
            POINT ptDrop;
            DragQueryPoint(hdrop, &ptDrop);
            /*
             * 判断ptDrop 是否在对应sample item的矩形框内
             */
            int dropItemIdx=ScratchView_GetSamplesItemIndex(hwnd,ptDrop);
            if(dropItemIdx==-1) {
                char tmp[256]={0};
                sprintf(tmp,"drop postion(%d,%d)",ptDrop.x,ptDrop.y);
                MessageBox(hwnd,"请将选中的文件拖拽至对应的图例方框内!",tmp,MB_OK|MB_ICONINFORMATION);
                DragFinish(hdrop);
                return 0;
            }

            char filename[MAX_PATH]={0};
            DragQueryFile(hdrop,0,filename,MAX_PATH);

            ScratchView_SetSmps(hwnd,(PAGECODE)dropItemIdx,filename);

            DragFinish(hdrop);
        }
        
    } break;
    case WM_LBUTTONDBLCLK: {
        pScratchView psv=ScratchView_GetSettings(hwnd);
        //if(!psv||psv->mode!=2) break;//||!(psv->mode==1&&psv->showRcRecgSwitch==1)

        POINT pt={
            GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam)
        };
        int sampleItemIdx=ScratchView_GetSamplesItemIndex(hwnd,pt);
        if(psv->mode==2) {
            if(psv->mode==2&&sampleItemIdx!=-1&&psv->smps[sampleItemIdx].img_path[0]=='\0') {
                char filename[256]={0};
                OPENFILENAME opfn;
                ZeroMemory(&opfn,sizeof(OPENFILENAME));
                opfn.lStructSize=sizeof(OPENFILENAME);
                opfn.lpstrFilter="图片文件 (*.png;*.jpg;*.bmp)\0*.png;*.jpg;*.bmp\0所有文件 (*.*)\0*.*\0";
                opfn.nFilterIndex=1;
                opfn.lpstrFile=filename;
                opfn.lpstrFile[0]='\0';
                opfn.lpstrTitle=page_name[sampleItemIdx];
                opfn.nMaxFile=sizeof(filename);
                opfn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;

                if(GetOpenFileName(&opfn)) {
                    ScratchView_SetSmps(hwnd,(PAGECODE)sampleItemIdx,filename);
                }
            } else if(sampleItemIdx!=-1) {
                if(psv->active_smpidx!=sampleItemIdx) {
                    psv->active_smpidx=sampleItemIdx;
                    ScratchView_SetImage(hwnd,(psv->smps+psv->active_smpidx)->img_path);
                }
            }
        } else if(psv->mode==1) {

            if(psv->showRcRecgSwitch==1) {
                /*
                 * 双击选择选区
                 */
                RECT rcImg;
                ScratchView_GetZoneRect(hwnd,IMG_VIEW,&rcImg);
                
                POINT ptCheck={
                    pt.x-rcImg.left+psv->offset.x,
                    pt.y-rcImg.top+psv->offset.y
                };

                if(psv->rcSelList&&psv->rcSelIdx>=0) {
                    if(PtInRect(psv->rcSelList[0],ptCheck)) {
                        psv->rcSelIdx=(psv->rcSelIdx+1)%psv->rcSelCount;
                        InvalidateRect(hwnd,NULL,TRUE);
                    } else {
                        psv->rcSelCount=0;
                        psv->rcSelIdx=0;

                        free(psv->rcSelList);
                        psv->rcSelList=NULL;
                    }
                }

                if(psv->rcSelIdx<0||!psv->rcSelList) {

                    if(psv->rcSelList) {
                        free(psv->rcSelList);
                        psv->rcSelList=NULL;
                    }

                    if(!psv->rcSelList) psv->rcSelList=(LPRECT*)calloc(sizeof(LPRECT)*psv->countRecgnoizes,1);
                    psv->rcSelCount=0;
                    
                    for(int idx=0;idx<psv->countRecgnoizes;idx++) {
                        /*
                         * 将点击坐标转化为图像坐标
                         */
                        if(PtInRect(psv->rcRecgnoizes+idx,ptCheck)) {
                            psv->rcSelList[psv->rcSelCount]=psv->rcRecgnoizes+idx;
                            psv->rcSelCount++;
                        }
                    }

                    if(psv->rcSelCount>0) {
                        /*
                         * 排序-按照面积由小到大
                         */
                        //qsort(psv->rcSelList,psv->rcSelCount,sizeof(LPRECT),cmpRectSize);
                        psv->rcSelIdx=0;
                        InvalidateRect(hwnd,NULL,TRUE);
                    }
                }
            }
        }
    } break;
    case WM_PAINT: {
        return ScratchView_Paint(hwnd,wParam,lParam);
    } break;
    case WM_LBUTTONDOWN: {
        pScratchView psv=ScratchView_GetSettings(hwnd);
        if(!psv) return 1;

        POINT pt={
            GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam)
        };
        if(psv->mode==1||psv->mode==2) {
            /*
             * 水平滚动条
             */
            if(ScratchView_HitTest(hwnd,pt)==(int)ZHSTHUMB) {
                SetCapture(hwnd);
                psv->prept.x=pt.x;
                psv->prept.y=-1;
                return 0;
            }
            if(ScratchView_HitTest(hwnd,pt)==(int)ZVSTHUMB) {
                SetCapture(hwnd);
                psv->prept.y=pt.y;
                psv->prept.x=-1;
                return 0;
            }
        }

        if(psv->mode==1) {
            RECT rcSave,rcCfg;
            if(/* ScratchView_GetZoneRect(hwnd,CTRL_CONFIG,&rcCfg)==0|| */
               ScratchView_GetZoneRect(hwnd,CTRL_SAVE,&rcSave)==0) {
                if(PtInRect(&rcSave,pt)/* ||PtInRect(&rcCfg,pt) */) {
                    //MessageBox(hwnd,"You have clicked the save button.","TEST",MB_OK);
                    memset(&trzcfg,0x00,sizeof(TRZ));
                    trzcfg.width=abs(psv->cx);trzcfg.height=abs(psv->cy);
                    trzcfg.x=(psv->cx>0)?psv->pos.x:(psv->pos.x+psv->cx);
                    trzcfg.y=(psv->cy>0)?psv->pos.y:(psv->pos.y+psv->cy);

                    HINSTANCE inst=(HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE);
                    INT_PTR result=DialogBox(inst,MAKEINTRESOURCE(IDD_TRZCFG),hwnd,TRZCfgHandle);
                    HWND frame=GetParent(hwnd);
                    UINT style=(UINT)GetWindowLongPtr(frame,GWL_STYLE);
                    SetWindowLongPtr(frame,GWL_STYLE,(LONG_PTR)(style&~WS_DISABLED));
                    return 0;
                }
            }
            RECT rcCancel;
            if(ScratchView_GetZoneRect(hwnd,CTRL_CANCEL,&rcCancel)==0) {
                if(PtInRect(&rcCancel,pt)) {
                    psv->pos={0,0};
                    psv->cx=0;psv->cy=0;
                    InvalidateRect(hwnd,NULL,TRUE);
                    return 0;
                }
            }
            RECT rcOCR;
            if(ScratchView_GetZoneRect(hwnd,CTRL_OCR,&rcOCR)==0) {
                if(PtInRect(&rcOCR,pt)) {
                    //进行OCR识别
                    //...
                    InvalidateRect(hwnd,NULL,TRUE);
                    return 0;
                }
            }

            RECT rcImg;
            if(ScratchView_GetZoneRect(hwnd,IMG_VIEW,&rcImg)==0) {
                if(PtInRect(&rcImg,pt)) {
                    /*
                     * psv->pos 记录的是 相对于 图像的位置坐标
                     * 不是相对于屏幕窗口的偏移
                     */
                    psv->pos.x=pt.x-rcImg.left+psv->offset.x;
                    psv->pos.y=pt.y-rcImg.top+psv->offset.y;
                    psv->cx=psv->cy=0;
                }
            }

        }

        RECT rcFile;
        if(ScratchView_GetZoneRect(hwnd,CTRL_FILE,&rcFile)==0&&PtInRect(&rcFile,pt)) {
            /*
                * 打开新文件
                */
            char filename[256]={0};
            OPENFILENAME opfn;
            ZeroMemory(&opfn,sizeof(OPENFILENAME));
            opfn.lStructSize=sizeof(OPENFILENAME);
            opfn.lpstrFilter="所有文件\0*.*\0";
            opfn.nFilterIndex=1;
            opfn.lpstrFile=filename;
            opfn.lpstrFile[0]='\0';
            opfn.nMaxFile=sizeof(filename);
            opfn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;

            if(GetOpenFileName(&opfn)) {
                ScratchView_SetImage(hwnd,filename);
                return 0;
            }
        }
        RECT rcExpend;
        if(ScratchView_GetZoneRect(hwnd,CTRL_MODE_ORI,&rcExpend)==0&&PtInRect(&rcExpend,pt)) {
            if(psv->mode!=1) {
                ScratchView_ChangeMode(hwnd,1);
                return 0;
            }
        }
        RECT rcCollapse;
        if(ScratchView_GetZoneRect(hwnd,CTRL_MODE_SCALE,&rcCollapse)==0&&PtInRect(&rcCollapse,pt)) {
            if(psv->mode!=0) {
                ScratchView_ChangeMode(hwnd,0);
                return 0;
            }
        }
        RECT rcLaunch;
        if(ScratchView_GetZoneRect(hwnd,CTRL_LAUNCHWORK,&rcLaunch)==0&&PtInRect(&rcLaunch,pt)) {
            pwfpj_param wfpj=(pwfpj_param)(f->extra);
            if(!wfpj) return 0;

            if(psv->mode==0) {
                //ResumeThread(wfpj->ocrthread);
                return 0;
            }
            
            /*
             * 判断线程是否有效  
             */
            DWORD exitcode;
            BOOL result=GetExitCodeThread(wfpj->ocrthread,&exitcode);
            if(!result) {
                /*
                 * 线程句柄无效
                 * 创建线程，即时启动
                 */
                MessageBox(hwnd,"线程无效，将启动新的线程...","提示",MB_OK);
                if(wfpj->ocrthread!=NULL) CloseHandle(wfpj->ocrthread);
                DWORD threadid;
                wfpj->ocrthread=CreateThread(NULL,0,DouYinOCRJOB,NULL,0,&threadid);
                wfpj->ocrthreadsuspend_ref=0;
            } else {
                if(exitcode!=STILL_ACTIVE) {
                    /*
                     * 线程已经结束
                     */
                    MessageBox(hwnd,"线程已退出，将启动新的线程...","提示",MB_OK);
                    if(wfpj->ocrthread!=NULL) CloseHandle(wfpj->ocrthread);
                    DWORD threadid;
                    wfpj->ocrthread=CreateThread(NULL,0,DouYinOCRJOB,NULL,0,&threadid);
                    wfpj->ocrthreadsuspend_ref=0;
                } else {
                    /*
                     * 线程尚在运行
                     * 判断线程是否挂起
                     */
                    //由于使用非公开windwos sdk接口，该函数在较新版本中可能存在兼容性问题
                    //经过简单测试，该函数不被兼容，baga
                    //if(IsThreadSuspended(wfpj->ocrthread)) {
                    if(wfpj->ocrthreadsuspend_ref>0) {
                        ResumeThread(wfpj->ocrthread);
                        wfpj->ocrthreadsuspend_ref=0;
                    }

                    MessageBox(hwnd,"线程运行中...","提示",MB_OK);
                    return 0;
                }
            }
        }
        RECT rcSuspend;
        if(ScratchView_GetZoneRect(hwnd,CTRL_SUSPENDWORK,&rcSuspend)==0&&PtInRect(&rcSuspend,pt)) {
            if(psv->mode!=0) {
                
                return 0;
            }
        }
        RECT rcConfig;
        if(ScratchView_GetZoneRect(hwnd,CTRL_CONFIG,&rcConfig)==0&&PtInRect(&rcConfig,pt)) {
            if(psv->mode!=2) {
                ScratchView_ChangeMode(hwnd,2);
            }

            // //弹出右键菜单
            // HMODULE ctrl_module=LoadLibrary("ScratchView.dll");
            // if(ctrl_module==NULL) return 1;


            // //...
            // HMENU menu=LoadMenu((HINSTANCE)ctrl_module,MAKEINTRESOURCE(IDR_SCRATCHMENU));
            // HMENU submenu=GetSubMenu(menu,0);
            // POINT pt={
            //     rcConfig.left-1,
            //     rcConfig.bottom-1
            // };
            // ClientToScreen(hwnd,&pt);
            // TrackPopupMenu(submenu,TPM_LEFTBUTTON,pt.x,pt.y,0,hwnd,NULL);

            // DestroyMenu(menu);
            // FreeLibrary(ctrl_module);
            return 0;
        }

    } break;
    case WM_MOUSEMOVE: {
        pScratchView psv=ScratchView_GetSettings(hwnd);
        if(!psv) return 1;

        POINT pt={
            GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam)
        };
        if(psv->mode==1||psv->mode==2) {
            /*
             * 拖拽滚动条
             */
            if(GetCapture()==hwnd) {
                /*
                 * 设置新的offset
                 */
                if(psv->prept.x!=-1) {
                    /*
                     * 水平滚动
                     * 根据鼠标移动的像素 pt.x-psv->prept.x 
                     * 计算 offset 移动的值
                     * pt.x-psv->prept.x  / cxScrollbar-cxScrollThumb  = offsetx / psv->pixels_x - viewcx
                     */
                    RECT rcHorzThumb,rcHorz;
                    ScratchView_GetZoneRect(hwnd,ZHSCROLL,&rcHorz);
                    ScratchView_GetZoneRect(hwnd,ZHSTHUMB,&rcHorzThumb);
                    
                    psv->offset.x+=(double)(pt.x-psv->prept.x)/((rcHorz.right-rcHorz.left)-(rcHorzThumb.right-rcHorzThumb.left))*(psv->pixels_x-(rcHorz.right-rcHorz.left));
                    psv->prept.x=pt.x;
                    ScratchView_GetZoneRect(hwnd,ZHSTHUMB,&rcHorzThumb);
                } else {
                    /*
                     * 垂直滚动
                     */
                    RECT rcVertThumb,rcVert;
                    ScratchView_GetZoneRect(hwnd,ZVSCROLL,&rcVert);
                    ScratchView_GetZoneRect(hwnd,ZVSTHUMB,&rcVertThumb);
                    psv->offset.y+=(double)(pt.y-psv->prept.y)/((rcVert.bottom-rcVert.top)-(rcVertThumb.bottom-rcVertThumb.top))*(psv->pixels_y-(rcVert.bottom-rcVert.top));
                    psv->prept.y=pt.y;
                    ScratchView_GetZoneRect(hwnd,ZVSTHUMB,&rcVertThumb);
                }

                InvalidateRect(hwnd,NULL,TRUE);
                return 0;
            }

            RECT rcImg;
            /*
             * 左键按下
             */
            if(ScratchView_GetZoneRect(hwnd,IMG_VIEW,&rcImg)==0&&(wParam&MK_LBUTTON==MK_LBUTTON)) {
                if(pt.x<rcImg.left) pt.x=rcImg.left;
                if(pt.x>rcImg.right) pt.x=rcImg.right;

                if(pt.y<rcImg.top) pt.y=rcImg.top;
                if(pt.y>rcImg.bottom) pt.y=rcImg.bottom;

                psv->cx=pt.x-rcImg.left+psv->offset.x-psv->pos.x;
                psv->cy=pt.y-rcImg.top+psv->offset.y-psv->pos.y;
            }
        }
        
        psv->pt={pt.x,pt.y};
        InvalidateRect(hwnd,NULL,TRUE);

    } break;
    case WM_LBUTTONUP: {
        if(GetCapture()==hwnd) ReleaseCapture();
    } break;
    case WM_MOUSEWHEEL: {
        int wheel_delta=GET_WHEEL_DELTA_WPARAM(wParam);
        int scroll_pixls=((double)wheel_delta/10)*2*-1;
        pScratchView psv=ScratchView_GetSettings(hwnd);
        if(!psv) return TRUE;

        RECT rcScroll;
        BOOL hasHorzScrollBar=(ScratchView_GetZoneRect(hwnd,ZHSCROLL,&rcScroll)==0);
        BOOL hasVertScrollBar=(ScratchView_GetZoneRect(hwnd,ZVSCROLL,&rcScroll)==0);

        if(hasHorzScrollBar||hasVertScrollBar) {
            if(hasHorzScrollBar&&!hasVertScrollBar) {
                psv->offset.x+=scroll_pixls;
                ScratchView_GetZoneRect(hwnd,ZHSCROLL,&rcScroll);
            }
            else {
                psv->offset.y+=scroll_pixls;
                ScratchView_GetZoneRect(hwnd,ZVSCROLL,&rcScroll);
            }
            InvalidateRect(hwnd,NULL,TRUE);
        }
        return TRUE;
    } break;
    case WM_ERASEBKGND: return 1;
    case WM_NCDESTROY: {
        ScratchView_ClearSettings(hwnd);
    } break;
    case WM_SIZE: {
        /*
         * 更新 offset
         */
        RECT rcScroll;
        BOOL hasHorzScrollBar=(ScratchView_GetZoneRect(hwnd,ZHSCROLL,&rcScroll)==0);
        BOOL hasVertScrollBar=(ScratchView_GetZoneRect(hwnd,ZVSCROLL,&rcScroll)==0);
        pScratchView psv=ScratchView_GetSettings(hwnd);
        if(psv) {
            psv->offset={0,0};
            psv->prept={0,0};
        }
        InvalidateRect(hwnd,NULL,TRUE);
    } break;
    case MSG_SECTIONSETTINGS: {
        pTRZ trz=(pTRZ)wParam;
        TRZCODE trzcode=(TRZCODE)lParam;

        trzcfg.code=trzcode;
        /*
         * 打开对话框
         */
        pScratchView psv=ScratchView_GetSettings(hwnd);
        if(!psv) return 1;

        HINSTANCE inst=(HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE);
        INT_PTR result=DialogBox(inst,MAKEINTRESOURCE(IDD_TRZCFG),hwnd,TRZCfgHandle);
        HWND frame=GetParent(hwnd);
        UINT style=(UINT)GetWindowLongPtr(frame,GWL_STYLE);
        SetWindowLongPtr(frame,GWL_STYLE,(LONG_PTR)(style&~WS_DISABLED));        

    } break;
    case MSG_SETSMPSIMGS: {
        pMajor major=(pMajor)wParam;
        pTRZProjFile trzproj=(pTRZProjFile)lParam;
        pScratchView psv=ScratchView_GetSettings(hwnd);
        if(psv&&psv->smps) {
            /**
             * 此处使用 psv->smps_count 存在隐患
             * 当方案变更，PAGE 数量发生变化之后，可能无法及时更新
             * 伏笔日期：2024年10月17日
             */
            char path[256]={0};
            GetModulePath(path);
            for(int idx=0;idx<psv->smps_count;idx++) {
                /**
                 * 修正目录
                 * 这里有问题...
                 * 这里会对所有的psv->smps更新，有些位置未设置图片 trzproj->page_imgs[idx][0]=='\0'
                 */
                if(trzproj->page_imgs[idx][0]!='\0') {
                    char pageimg_path[256]={0};
                    sprintf(pageimg_path,"%s%s\\%s",path,trzproj->projname,trzproj->page_imgs[idx]);
                    strcpy((psv->smps+idx)->img_path,pageimg_path);//trzproj->page_imgs[idx]);
                }
            }
            InvalidateRect(hwnd,NULL,TRUE);
        }
    } break;
    case MSG_SAVEPROJ: {
        SendMessage(hwnd,WM_COMMAND,MAKEWPARAM(ID_SAVEPROJ,0),0);
        //ScratchView_SaveTRZProjFile(hwnd,GMajor,)
    } break;
    case WM_COMMAND: {
        pScratchView psv=ScratchView_GetSettings(hwnd);
        if(!psv) return -1;

        switch(LOWORD(wParam)) {
        case ID_OCRRECOGN_RECT: {
            psv->showRcRecgSwitch=1;
            int pos[1000*4]={0};
            int pos_count=0;
            int check_ret=opencv_recognize_rectangles((psv->smps+psv->active_smpidx)->img_path,&pos,&pos_count);
            if(check_ret==0) {
                psv->countRecgnoizes=pos_count/4;
                if(psv->countRecgnoizes<=0) break;
                
                if(!psv->rcRecgnoizes) psv->rcRecgnoizes=(LPRECT)calloc(sizeof(RECT)*psv->countRecgnoizes,1);
                
                if(psv->rcRecgnoizes) {
                    int allocated_size=_msize(psv->rcRecgnoizes)/sizeof(RECT);
                    if(psv->countRecgnoizes>allocated_size) {
                        psv->rcRecgnoizes=(LPRECT)realloc(psv->rcRecgnoizes,sizeof(RECT)*psv->countRecgnoizes);
                    }
                }

                if(psv->rcRecgnoizes) {
                    for(int idx=0;idx<psv->countRecgnoizes;idx++) {
                        SetRect(psv->rcRecgnoizes+idx,pos[idx*4+0],pos[idx*4+1],pos[idx*4+2],pos[idx*4+3]);
                    }

                    qsort(psv->rcRecgnoizes,psv->countRecgnoizes,sizeof(RECT),cmpRectSize);
                }
            }
        } break;
        case ID_OCRRECOGN_ECLIPSE: {
            psv->showRcRecgSwitch=1;
            int pos[1000*3]={0};
            int pos_count=0;
            int check_ret=opencv_recognize_circles((psv->smps+psv->active_smpidx)->img_path,&pos,&pos_count);
            if(check_ret==0) {
                //....

            }
        } break;
        case ID_TRZSETTINGS: {
            if(psv->rcSelIdx<0||!psv->rcSelList) {
                MessageBox(hwnd,"请先行识别选区并挑选需要设置的区域！","提示",MB_ICONINFORMATION|MB_OK);
                break;
            }
            memset(&trzcfg,0x00,sizeof(TRZ));
            trzcfg.width=abs(psv->rcSelList[psv->rcSelIdx]->right-psv->rcSelList[psv->rcSelIdx]->left);
            trzcfg.height=abs(psv->rcSelList[psv->rcSelIdx]->bottom-psv->rcSelList[psv->rcSelIdx]->top);
            trzcfg.x=psv->rcSelList[psv->rcSelIdx]->left;
            trzcfg.y=psv->rcSelList[psv->rcSelIdx]->top;
            trzcfg.page=(PAGECODE)psv->active_smpidx;

            HINSTANCE inst=(HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE);
            INT_PTR result=DialogBox(inst,MAKEINTRESOURCE(IDD_TRZCFG),hwnd,TRZCfgHandle);
            HWND frame=GetParent(hwnd);
            UINT style=(UINT)GetWindowLongPtr(frame,GWL_STYLE);
            SetWindowLongPtr(frame,GWL_STYLE,(LONG_PTR)(style&~WS_DISABLED));
        } break;
        case ID_TRZEDIT: {
            if(!psv->smps||psv->active_smpidx<0||psv->active_smpidx>=psv->smps_count) break;

            /*
             * mode=1 大图 
             */
            if((psv->smps+psv->active_smpidx)->img_path[0]!='\0') {
                ScratchView_ChangeMode(hwnd,1);
                ScratchView_SetImage(hwnd,(psv->smps+psv->active_smpidx)->img_path);
            }
        } break;
        case ID_PAGETMPCLIP: {
            if(!psv->smps||psv->active_smpidx<0||psv->active_smpidx>=psv->smps_count) break;

            POINT pt[2]={
                { psv->pos.x,psv->pos.y },
                { psv->pos.x+psv->cx,psv->pos.y+psv->cy}
            };
            int x=min(pt[0].x,pt[1].x);
            int y=min(pt[0].y,pt[1].y);
            RECT rcClip={x,y,x+abs(psv->cx),y+abs(psv->cy)};
            if(IsRectEmpty(&rcClip)) {
                char buffer[256]={0};
                sprintf(buffer,"设定页面[%s]的识别标识图，需要鼠标拖选一个标识图选区！",page_name[psv->active_smpidx]);
                MessageBox(hwnd,buffer,"提示",MB_OK|MB_ICONINFORMATION);
                break;
            }

            /*
             * 保存用于识别PAGE的裁剪模板图
             */
            opencv_savetemplateclips((psv->smps+psv->active_smpidx)->img_path,x,y,abs(psv->cx),abs(psv->cy),(PAGECODE)psv->active_smpidx);
        } break;
        case ID_SAVEPROJ: {
            if(GMajor->activeproj_index<0) {
                char defaultprojname[256]="DefaultProj_Joe";
                if(0!=Major_AddTRZProj(GMajor,defaultprojname,1)) {
                    char errmsg[256]={0};
                    sprintf(errmsg,"当前系统未设置有效的TRZ识别方案。\n"
                                   "系统试图将当前应用的TRZ识别方案纳入应用，但创建默认的TRZ识别方案[%s]失败",
                                   defaultprojname);
                    MessageBox(hwnd,errmsg,"提示",MB_OK|MB_ICONERROR);
                    return -1;
                }
                if(0!=Major_SetCurrProj(GMajor,0)) {
                    char errmsg[256]={0};
                    sprintf(errmsg,"当前系统试图将TRZ识别方案[%s]设置更新为可用方案，但更新失败",
                                   defaultprojname);
                    MessageBox(hwnd,errmsg,"提示",MB_OK|MB_ICONERROR);
                    return -1;
                }
            }

            int cx=psv->img->GetWidth(),cy=psv->img->GetHeight();
            char projname[256]={0};
            sprintf(projname,"%s",GMajor->trzproj_list[GMajor->activeproj_index]);
            char projdesc[256]={0};

            time_t rawtime;
            struct tm* tminfo;
            char time_buffer[256]={0};
            time(&rawtime);
            tminfo=localtime(&rawtime);
            strftime(time_buffer,sizeof(time_buffer),"%Y-%m-%d %H:%M:%S",tminfo);

            sprintf(projdesc,"适用于%d×%d的分辨率图像方案，更新于%s",cx,cy,time_buffer);
            ScratchView_SaveTRZProjFile(hwnd,projname,GMajor,1080,2244,projdesc);
        } break;
        case ID_LOADTRZPROJINDEX: {
            /*
             * 加载索引文件
             */
            Major_LoadProj(GMajor,NULL);
        } break;
        case ID_SAVETRZPROJINDEX: {
            Major_SaveProjs(GMajor);
        } break;
        case ID_TEST: {
            ScrathView_LoadTest(hwnd);
        } break;
        }
    } break;
    }

    ret=CallWindowProc(DefWindowProc,hwnd,msg,wParam,lParam);
    
    return ret;
}

int ScratchView_InitialSettings(HWND hwnd) {
    pScratchView psv=(pScratchView)calloc(sizeof(ScratchView),1);
    if(!psv) return -1;
    psv->mode=1;
    psv->offset={0,0};

    psv->smps_count=20;
    psv->smps=(pTRZSmps)calloc(sizeof(TRZSmps),psv->smps_count);
    if(psv->smps) {
        for(int idx=0;idx<psv->smps_count;idx++) {
            pTRZSmps smps=psv->smps+idx;
            smps->code=(PAGECODE)idx;
        }
    }

    HMODULE instance=LoadLibrary("scratchview.dll");
    psv->list=ImageList_Create(24,24,ILC_MASK|ILC_COLOR32,1,1);
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0001)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0002)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0003)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0004)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0005)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0006)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0007)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0008)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0009)));
    FreeLibrary(instance);

    psv->menubkbrush=CreateSolidBrush(RGB(45,45,45));

    SetWindowLongPtr(hwnd,GWLP_USERDATA,(ULONG_PTR)psv);
    return 0;
}

int ScratchView_SetImage(HWND hwnd,char* img) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) return -1;

    memset(psv->img_path,0x00,sizeof(psv->img_path));
    strcpy(psv->img_path,img);
    ScratchView_ChangeMode(hwnd,0);

    //wchar_t img_wchar[256]={0};
    //int wchar_len=MultiByteToWideChar(CP_ACP,0,psv->img_path,-1,NULL,0);
    //MultiByteToWideChar(CP_ACP,0,psv->img_path,-1,img_wchar,wchar_len);
    if(psv->img) {
        delete psv->img;
        psv->img=NULL;
    }

    /*
     * 读取文件内存，这里没有处理异常，后续更正
     */
    char* img_buffer=NULL;
    int img_size;
    HANDLE img_file;
    DWORD dwRead;

    img_file=CreateFile(psv->img_path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(img_file==INVALID_HANDLE_VALUE) {
        MessageBox(hwnd,"打开文件失败","异常",MB_OK|MB_ICONERROR);
        return -1;
    }
    img_size=GetFileSize(img_file,NULL);
    img_buffer=(char*)calloc(sizeof(char)*img_size,1);
    ReadFile(img_file,img_buffer,img_size,&dwRead,NULL);
    if(dwRead<img_size) {
        char read_result_string[256]={0};
        sprintf(read_result_string,"文件尺寸 %d (bytes) , 读取尺寸 %d (bytes)",img_size,dwRead);
        MessageBox(hwnd,read_result_string,"读取文件异常",MB_OK|MB_ICONWARNING);
    }
    CloseHandle(img_file);

    /*
     * 使用流，避免文件占用
     */
    HGLOBAL hGlobal=GlobalAlloc(GMEM_MOVEABLE,img_size);
    void* pData=GlobalLock(hGlobal);
    memcpy(pData,img_buffer,img_size);
    GlobalUnlock(hGlobal);
    IStream* stream=NULL;
    HRESULT hr=CreateStreamOnHGlobal(hGlobal,TRUE,&stream);
    if(img_buffer) free(img_buffer);

    psv->cx=0;
    psv->cy=0;
    psv->pos={0,0};
    psv->img=new Gdiplus::Bitmap(stream);
    stream->Release();
    if(psv->img->GetLastStatus()!=Ok) {
        /*
         * 加载图像出现异常
         */
        MessageBox(hwnd,img,"读取文件内存失败",MB_OK);
        delete psv->img;
        psv->img=NULL;
        psv->pixels_x=psv->pixels_y=0;
        InvalidateRect(hwnd,NULL,TRUE);
        return -1;
    } else {
        psv->pixels_x=psv->img->GetWidth();
        psv->pixels_y=psv->img->GetHeight();
        InvalidateRect(hwnd,NULL,TRUE);
        return 0;
    }
}

int ScratchView_ChangeMode(HWND hwnd,int mode) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) return -1;

    if(psv->mode!=mode) {
        psv->mode=mode;
        psv->cx=psv->cy=0;
        psv->pos={0,0};
        InvalidateRect(hwnd,NULL,TRUE);

        DragAcceptFiles(hwnd,(psv->mode==2)?TRUE:FALSE); 
    }

    return 0;
}

pScratchView ScratchView_GetSettings(HWND hwnd) {
    return (pScratchView)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

void ScratchView_ClearSettings(HWND hwnd) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) {

        if(psv->smps) {
            free(psv->smps);
            psv->smps=NULL;
        }

        if(psv->list) {
            ImageList_Destroy(psv->list);
            psv->list=NULL;
        }
        
        DeleteObject(psv->menubkbrush);

        if(psv->img) {
            delete psv->img;
            psv->img=NULL;
        }

        if(psv->rcRecgnoizes) {
            free(psv->rcRecgnoizes);
            psv->countRecgnoizes=0;
            psv->rcRecgnoizes=NULL;
        }

        if(psv->rcSelList) {
            free(psv->rcSelList);
            psv->rcSelList=NULL;
            psv->rcSelCount=0;
        }

        free(psv);
    }
}

int ScratchView_HitTest(HWND hwnd,POINT pt) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) return -1;

    RECT rcView,rcHSThumb,rcVSThumb;

    if(psv->mode==1||psv->mode==2) {
        if(ScratchView_GetZoneRect(hwnd,ZHSTHUMB,&rcHSThumb)==0) {
            if(PtInRect(&rcHSThumb,pt)) {
                return ZHSTHUMB;
            }
        }
        if(ScratchView_GetZoneRect(hwnd,ZVSTHUMB,&rcVSThumb)==0) {
            if(PtInRect(&rcVSThumb,pt)) {
                return ZVSTHUMB;
            }
        }
    
    }
    return 0;
}

int ScratchView_GetZoneRect(HWND hwnd,int poscode,LPRECT prc) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) return -1;

    RECT rc;
    GetClientRect(hwnd,&rc);
    int viewcx=rc.right-rc.left;
    int viewcy=rc.bottom-rc.top;
    BOOL hasvertbar=FALSE;
    BOOL hashorzbar=FALSE;
    /* 识别模式 */
    if(psv->mode==1||psv->mode==2) {
        hasvertbar=viewcy<psv->pixels_y;//垂直滚动条
        hashorzbar=viewcx<psv->pixels_x;//水平滚动条
    } 

/*
 * 图像视图区 
 */
    int panel_cy=35;
    
    switch(poscode) {
    case ZHSTHUMB: {
        if(!hashorzbar) return -1;
        int contentcx=viewcx-(hasvertbar?SCROLLBAR_PIXLS:0);
        int xpage=contentcx;
        int xrange1=0;
        int xrange2=psv->pixels_x-xpage;
        /*
         * contentcx 展示视图区
         * xrange : offsetx 值域范围
         * xpage : 页宽
         * psv->pixels_x 内容宽度
         * 等式：
         * 
         */
        if(psv->offset.x<xrange1) psv->offset.x=xrange1;
        if(psv->offset.x>xrange2) psv->offset.x=xrange2;

        int horzcx=((double)contentcx)*contentcx/psv->pixels_x;
        horzcx=((horzcx<THUMB_MIN_PIXLS)?THUMB_MIN_PIXLS:horzcx);
        int horz_offset=((double)(contentcx-horzcx)*psv->offset.x)/(psv->pixels_x-contentcx);
        SetRect(prc,rc.left+horz_offset,
            rc.bottom-SCROLLBAR_PIXLS,
            rc.left+horz_offset+horzcx,
            rc.bottom-1);
    } break;
    case ZHSCROLL: {
        if(!hashorzbar) return -1;
        int contentcx=viewcx-(hasvertbar?SCROLLBAR_PIXLS:0);
        int xpage=contentcx;
        int xrange1=0;
        int xrange2=psv->pixels_x-xpage;
        /*
         * contentcx 展示视图区
         * xrange : offsetx 值域范围
         * xpage : 页宽
         * psv->pixels_x 内容宽度
         * 等式：
         * 
         */
        if(psv->offset.x<xrange1) psv->offset.x=xrange1;
        if(psv->offset.x>xrange2) psv->offset.x=xrange2;

        SetRect(prc,rc.left,
            rc.bottom-SCROLLBAR_PIXLS,
            rc.right-(hasvertbar?SCROLLBAR_PIXLS:0),
            rc.bottom-1);
    } break;
    case ZVSTHUMB: {
        if(!hasvertbar) return -1;
        int contentcy=viewcy-(hashorzbar?SCROLLBAR_PIXLS:0);
        int ypage=contentcy;
        int yrange1=0;
        int yrange2=psv->pixels_y-ypage;
        /*
         * contentcx 展示视图区
         * xrange : offsetx 值域范围
         * xpage : 页宽
         * psv->pixels_x 内容宽度
         * 等式：
         * 
         */
        if(psv->offset.y<yrange1) psv->offset.y=yrange1;
        if(psv->offset.y>yrange2) psv->offset.y=yrange2;

        int vertcy=((double)contentcy)*contentcy/psv->pixels_y;
        vertcy=((vertcy<THUMB_MIN_PIXLS)?THUMB_MIN_PIXLS:vertcy);
        int vert_offset=((double)(contentcy-vertcy)*psv->offset.y)/(psv->pixels_y-contentcy);
        SetRect(prc,rc.right-SCROLLBAR_PIXLS,
            rc.top+vert_offset,
            rc.right-1,
            rc.top+vert_offset+vertcy);        
    } break;
    case ZVSCROLL: {
        if(!hasvertbar) return -1;
        int contentcy=viewcy-(hashorzbar?SCROLLBAR_PIXLS:0);
        int ypage=contentcy;
        int yrange1=0;
        int yrange2=psv->pixels_y-ypage;
        /*
         * contentcx 展示视图区
         * xrange : offsetx 值域范围
         * xpage : 页宽
         * psv->pixels_x 内容宽度
         * 等式：
         * 
         */
        if(psv->offset.y<yrange1) psv->offset.y=yrange1;
        if(psv->offset.y>yrange2) psv->offset.y=yrange2;

        // int vertcy=((double)contentcy)*contentcy/psv->pixels_y;
        // int vert_offset=((double)(contentcy-vertcy)*psv->offset.y)/(psv->pixels_y-contentcy);
        SetRect(prc,rc.right-SCROLLBAR_PIXLS,
            rc.top,
            rc.right-1,
            rc.bottom-1);   
    } break;
    case IMG_VIEW: {
        int viewcx,viewcy;
        POINT ptbase={rc.right,rc.bottom};
        SetRect(prc,rc.left,rc.top,rc.right,rc.bottom);
        if(psv->mode==0) {
            /*
             * 图像纵横比
             */
            double ratio=(double)psv->pixels_x/psv->pixels_y;
            viewcx=rc.right-rc.left;
            viewcy=rc.bottom-rc.top;
            double view_ratio=(double)viewcx/viewcy;

            if(ratio>view_ratio) {
                /*
                 * 以 viewcx 为基准进行比例适配
                 */
                if(viewcx<psv->pixels_x) {
                    /*
                     * 图像会被压缩
                     */
                    int imgviewcx=viewcx;
                    int imgviewcy=imgviewcx/((double)psv->pixels_x/psv->pixels_y);
                    int offsety=(viewcy-imgviewcy)>>1;
                    SetRect(
                        prc,
                        rc.left,
                        rc.top+offsety,
                        rc.right,
                        rc.top+offsety+imgviewcy
                    );
                } else {
                    /*
                     * 原图尺寸
                     */
                    int imgviewcx=psv->pixels_x;
                    int imgviewcy=psv->pixels_y;
                    int offsetx=(viewcx-imgviewcx)>>1;
                    int offsety=(viewcy-imgviewcy)>>1;
                    SetRect(prc,rc.left+offsetx,rc.top+offsety,
                        rc.left+offsetx+imgviewcx,rc.top+offsety+imgviewcy
                    );
                }
            } else {
                /*
                 * 以 viewcy 为基准进行比例适配
                 */
                if(viewcy<psv->pixels_y) {
                    int imgviewcy=viewcy;
                    int imgviewcx=imgviewcy*((double)psv->pixels_x/psv->pixels_y);
                    int offsetx=(viewcx-imgviewcx)>>1;
                    SetRect(prc,
                        rc.left+offsetx,rc.top,rc.left+offsetx+imgviewcx,rc.bottom
                    );
                } else {
                    int imgviewcy=psv->pixels_y;
                    int imgviewcx=psv->pixels_x;

                    int offsetx=(viewcx-imgviewcx)>>1;
                    int offsety=(viewcy-imgviewcy)>>1;
                    SetRect(prc,rc.left+offsetx,rc.top+offsety,
                        rc.left+offsetx+imgviewcx,rc.top+offsety+imgviewcy
                    );
                }
            }
            break;
        }
        if(psv->mode==1) {

            /*
             * 当前具有垂直滚动条
             * 说明 右侧有滚动条占据
             * 视图右侧区域需要调整
             */
            if(hasvertbar) {
                ptbase.x-=SCROLLBAR_PIXLS;//垂直滚动条
                prc->right=ptbase.x;
            }
            if(hashorzbar) {
                ptbase.y-=SCROLLBAR_PIXLS;//水平滚动条
                prc->bottom=ptbase.y;
            }
        } 

        /*
         * 调整prc居中位置
         * 以长度 psv->pixels_x/psv->pixels_y 为准
         */
        viewcx=rc.right-prc->left;
        viewcy=prc->bottom-prc->top;
        if(psv->pixels_x<viewcx) {
            prc->left+=(viewcx-psv->pixels_x)>>1;
        }
        if(psv->pixels_y<viewcy) {
            prc->top+=(viewcy-psv->pixels_y)>>1;
        }

        if(prc->left+psv->pixels_x<prc->right) prc->right=prc->left+psv->pixels_x;
        if(prc->top+psv->pixels_y<prc->bottom) prc->bottom=prc->top+psv->pixels_y;
    } break;
    case CTRL_PANEL: {
        //if(psv->mode!=1) return -1;
        int panel_cx= panel_cy*8;
        int offsetx=(rc.right-rc.top-panel_cx)>>1;
        SetRect(prc,rc.left+offsetx,rc.top,rc.left+offsetx+panel_cx,rc.top+panel_cy);
    } break;
    case CTRL_MODE_SCALE: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_PANEL,prc)!=0) return -1;
        prc->right=prc->left+panel_cy;

        OffsetRect(prc,panel_cy*1,0);
    } break;
    case CTRL_MODE_ORI: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_PANEL,prc)!=0) return -1;
        prc->right=prc->left+panel_cy;

        OffsetRect(prc,panel_cy*2,0);
    } break;
    case CTRL_FILE: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_PANEL,prc)!=0) return -1;
        prc->right=prc->left+panel_cy;

        OffsetRect(prc,panel_cy*3,0);
    } break;
    case CTRL_CONFIG: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_PANEL,prc)!=0) return -1;
        prc->right=prc->left+panel_cy;

        OffsetRect(prc,panel_cy*4,0);
    } break;
    case CTRL_LAUNCHWORK: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_PANEL,prc)!=0) return -1;
        prc->right=prc->left+panel_cy;

        OffsetRect(prc,panel_cy*5,0);

    } break;
    case CTRL_SUSPENDWORK: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_PANEL,prc)!=0) return -1;
        prc->right=prc->left+panel_cy;

        OffsetRect(prc,panel_cy*6,0);

    } break;
    case CTRL_SELRECT: {
        /*
         * mode==1时，选中的区域
         */
        RECT rcImg;
        if(ScratchView_GetZoneRect(hwnd,IMG_VIEW,&rcImg)!=0) return -1;
        if(psv->mode!=1||(psv->cx==0&&psv->cy==0)) return -1;

        RECT rcScreenImg;
        CopyRect(&rcScreenImg,&rcImg);
        OffsetRect(&rcScreenImg,-psv->offset.x,-psv->offset.y);

        RECT rcTag={
            (psv->cx>0?psv->pos.x:psv->pos.x+psv->cx),
            (psv->cy>0?psv->pos.y:psv->pos.y+psv->cy),
            (psv->cx>0?psv->pos.x:psv->pos.x+psv->cx)+abs(psv->cx),
            (psv->cy>0?psv->pos.y:psv->pos.y+psv->cy)+abs(psv->cy)
        };
        OffsetRect(&rcTag,rcScreenImg.left,rcScreenImg.top);
        CopyRect(prc,&rcTag);

    } break;
    case CTRL_SAVE: {
        RECT rcSel;
        if(ScratchView_GetZoneRect(hwnd,CTRL_SELRECT,&rcSel)!=0) return -1;
        SetRect(prc,rcSel.left,rcSel.top-26,rcSel.left+26,rcSel.top);
    } break;
    case CTRL_CANCEL: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_SAVE,prc)!=0) return -1;
        OffsetRect(prc,26,0);
    } break;
    case CTRL_OCR: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_CANCEL,prc)!=0) return -1;
        OffsetRect(prc,26,0);
    } break;
    case SAMPLES_VIEW: {
        if(psv->mode!=2) return -1;

        /* 
         * 默认返回第一行第一列的数据
         * 以固定高宽列示
         */
        int paddingcx=25,paddingcy=15;
        int cell_x=100,cell_y=350;

        //计算一行最多显示多少列
        //line_cells*cell_x+paddingcx*(line_cells+1) <= viewcx;
        int line_cells=viewcx/cell_x;
        if(line_cells*cell_x+paddingcx*(line_cells+1)>viewcx) line_cells--;
        POINT pt={
            paddingcx+(viewcx-(line_cells*cell_x+paddingcx*(line_cells+1)))>>1,
            paddingcy
        };

        //int cell_x=viewcx/3;
        //int cell_y=viewcy/3;
        SetRect(prc,pt.x,pt.y,pt.x+cell_x,pt.y+cell_y);
    } break;
    }

    return 0;
}

int ScratchView_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) return 0;

    PAINTSTRUCT ps={0};
    RECT rc,rcMem;
    GetClientRect(hwnd,&rc);
    OffsetRect(&rc,-rc.left,-rc.top);
    CopyRect(&rcMem,&rc);
    int cx=rcMem.right,cy=rcMem.bottom;
    if(!(cx>0&&cy>0)) return 0;

    HDC hdc=BeginPaint(hwnd,&ps);
    HDC memdc=CreateCompatibleDC(hdc);
    HBITMAP bmp=CreateCompatibleBitmap(hdc,cx,cy);
    HBITMAP prebmp=(HBITMAP)SelectObject(memdc,bmp);
    HBRUSH brushbk=CreateSolidBrush(RGB(20,20,20));
    FillRect(memdc,&rcMem,brushbk);
    DeleteObject(brushbk);

    
    Gdiplus::Graphics graphics(memdc);
    //wchar_t img_wchar[256]={0};
    //int wchar_len=MultiByteToWideChar(CP_ACP,0,psv->img_path,-1,NULL,0);
    //MultiByteToWideChar(CP_ACP,0,psv->img_path,-1,img_wchar,wchar_len);
    Gdiplus::Bitmap* img=psv->img;
    if(!img) {
        psv->pixels_x=psv->pixels_y=0;
    } else {//return 0;

        psv->pixels_x=img->GetWidth();
        psv->pixels_y=img->GetHeight();
    }
    if(psv->pixels_x<=0||psv->pixels_y<=0) {
        HICON iconWarning=LoadIcon(NULL,IDI_WARNING);
        ICONINFO iconInfo={0};
        BITMAP iconbmp;
        GetIconInfo(iconWarning,&iconInfo);
        GetObject(iconInfo.hbmMask,sizeof(BITMAP),&iconbmp);
        if(iconInfo.hbmMask) {

            DeleteObject(iconInfo.hbmColor);
        }
        DeleteObject(iconInfo.hbmMask);
        int iconHeight=iconbmp.bmHeight;
        int iconWidth=iconbmp.bmWidth;
        
        char error_msg[256]={0};
        sprintf(error_msg,"文件[%s]打开失败(%d)",psv->img_path,GetLastError());
        RECT rcErr,rcText,rcIcon;
        int errcx=450,errcy=100;
        POINT ptErr={(cx-errcx)>>1,(cy-errcy)>>1};
        SetRect(&rcErr,ptErr.x,ptErr.y,ptErr.x+errcx,ptErr.y+errcy);
        SetRect(&rcIcon,rcErr.left+15,((rcErr.bottom-rcErr.top-iconHeight)>>1)+rcErr.top,rcErr.left+15+iconWidth,((rcErr.bottom-rcErr.top-iconHeight)>>1)+rcErr.top+iconHeight);
        SetRect(&rcText,rcIcon.right+15,rcErr.top+25,rcErr.right-15,rcErr.bottom-15);

        HPEN penWarning=CreatePen(PS_SOLID,1,RGB(0,100,200));
        HBRUSH brushWarning=CreateSolidBrush(RGB(45,45,45));
        SetGraphicsMode(memdc,GM_ADVANCED);
        SetBkMode(memdc,TRANSPARENT);
        SelectObject(memdc,penWarning);
        SelectObject(memdc,brushWarning);
        Rectangle(memdc,rcErr.left,rcErr.top,rcErr.right,rcErr.bottom);
        DeleteObject(penWarning);
        DeleteObject(brushWarning);
        
        DrawIconEx(memdc,rcIcon.left,rcIcon.top,iconWarning,iconWidth,iconHeight,0,0,DI_NORMAL);
        SetTextColor(memdc,RGB(255,242,0));
        SetBkMode(memdc,TRANSPARENT);
        LOGFONT lf={0};
        GetObject(GetStockObject(SYSTEM_FONT),sizeof(LOGFONT),&lf);
        lf.lfHeight=18;
        lf.lfWeight=22;
        strcpy(lf.lfFaceName,"微软雅黑");
        HFONT font=CreateFontIndirect(&lf);
        HFONT prefont=(HFONT)SelectObject(memdc,font);
        DrawText(memdc,error_msg,-1,&rcText,DT_VCENTER|DT_LEFT|DT_WORDBREAK);
        SelectObject(memdc,prefont);
        DeleteObject(font);
        goto SCRATCHVIEWPAINT_EXIT;
    }

    RECT rcImg;
    ScratchView_GetZoneRect(hwnd,IMG_VIEW,&rcImg);
    POINT ptOffset={
        rcImg.left-psv->offset.x,
        rcImg.top-psv->offset.y
    };
    /*
     * mode==1 原始尺寸
     */
    if(psv->mode==1) {
        /*
         * 1.根据图像的像素，当前视窗的大小 判断有无滚动条
         *   如果有，取offset信息 ， 计算图像视窗、滚动条视窗RECT
         * 2.绘制
         * 
         * 该控件需要一个 hittest函数, 对点击位置进行判断
         * 一个getzonerect 函数 ， 对模式位置进行定义
         */
        
        /*
         * 绘制图像
         */
        POINT pt={rcImg.left,rcImg.top};
        
        graphics.DrawImage(img,
            Gdiplus::Rect(rcImg.left,rcImg.top,rcImg.right-rcImg.left,rcImg.bottom-rcImg.top),
            psv->offset.x,psv->offset.y,
            rcImg.right-rcImg.left,rcImg.bottom-rcImg.top,
            Gdiplus::UnitPixel
        );

        if(psv->showRcRecgSwitch==1) {
            HBRUSH brushRecg=CreateSolidBrush(RGB(0,100,200));
            RECT rcSel={0,0,0,0};
            for(int idx=0;idx<psv->countRecgnoizes;idx++) {
                RECT rcRecgTmp;
                CopyRect(&rcRecgTmp,psv->rcRecgnoizes+idx);
                OffsetRect(&rcRecgTmp,pt.x-psv->offset.x,pt.y-psv->offset.y);
                if(psv->rcSelList&&(psv->rcSelList[psv->rcSelIdx]==psv->rcRecgnoizes+idx)) {
                    CopyRect(&rcSel,&rcRecgTmp);
                    //FrameRect(memdc,&rcRecgTmp,brushSelRect);
                } else {
                    FrameRect(memdc,&rcRecgTmp,brushRecg);
                }
            }
            if(!IsRectEmpty(&rcSel)) {
                HBRUSH hNullBrush=(HBRUSH)GetStockObject(NULL_BRUSH);
                HBRUSH hOldBrush=(HBRUSH)SelectObject(memdc, hNullBrush);
                //设置画笔宽度
                HPEN penSelRect=CreatePen(PS_SOLID,2,RGB(100,0,0));
                HPEN preSelRectPen=(HPEN)SelectObject(memdc,penSelRect);
                //SelectObject(memdc,brushSelRect);
                //FrameRect(memdc,&rcSel,brushRecg);
                Rectangle(memdc,rcSel.left,rcSel.top,rcSel.right,rcSel.bottom);
                SelectObject(memdc,preSelRectPen);
                SelectObject(memdc,hOldBrush);
                DeleteObject(penSelRect);
                //DeleteObject(brushSelRect);
            }
            DeleteObject(brushRecg);
        }

        if(psv->cx!=0&&psv->cy!=0) {
            /*
             * 坐标转换逻辑
             * 1. rcImg的坐标为屏幕坐标
             * 2. rcImg进行offset偏移，得到图像的屏幕坐标
             * 3. psv->pos 是 图像坐标
             * 处理思路如下
             * 1. OffsetRect(&rcImg,-psv->pos.x,-psv->pox.y); 
             *    得到图像的0坐标相对于屏幕的位置
             * 2. rcTag 记录的是相对于图像的位置，需要转换出其屏幕坐标
             *    第1步得到的 {rcImg.left,rcImg.top} 对应图像的 {0,0}
             *    故需要对rcTag 进行偏移
             *    OffsetRect(&rcTag,-rcImg.left,-rcImg.top);
             */

            RECT rcTag;
            if(ScratchView_GetZoneRect(hwnd,CTRL_SELRECT,&rcTag)==0) {
                HBRUSH brushtag=CreateSolidBrush(RGB(35,35,35));
                FrameRect(memdc,&rcTag,brushtag);
                RECT rcOps={rcTag.left,rcTag.top-26,rcTag.left+26*3,rcTag.top};
                FillRect(memdc,&rcOps,brushtag);
                DeleteObject(brushtag);
            }
            
            LOGFONT lf={0};
            GetObject(GetStockObject(SYSTEM_FONT),sizeof(LOGFONT),&lf);
            lf.lfHeight=18;
            lf.lfWeight=22;
            strcpy(lf.lfFaceName,"微软雅黑");
            HFONT font=CreateFontIndirect(&lf);
            HFONT prefont=(HFONT)SelectObject(memdc,font);
            SetTextColor(memdc,RGB(0,50,100));
            SetBkMode(memdc,TRANSPARENT);
            RECT rcTagText{rcImg.left,rcTag.bottom,rcTag.right,rcTag.bottom+25};
            char tagtext[256]={0};
            POINT pos={
                (psv->cx>0)?psv->pos.x:(psv->pos.x+psv->cx),
                (psv->cy>0)?psv->pos.y:(psv->pos.y+psv->cy)
            };
            sprintf(tagtext,"位置：(%d,%d) %d×%d",pos.x,pos.y,abs(psv->cx),abs(psv->cy));
            DrawText(memdc,tagtext,-1,&rcTagText,DT_VCENTER|DT_RIGHT);
            SelectObject(memdc,prefont);
            DeleteObject(font);

            HBRUSH brushCtrls=CreateSolidBrush(RGB(200,200,200));
            RECT rcSave,rcCancel,rcOCR;
            if(ScratchView_GetZoneRect(hwnd,CTRL_SAVE,&rcSave)==0) {
                if(PtInRect(&rcSave,psv->pt)) {
                    FillRect(memdc,&rcSave,brushCtrls);
                }

                InflateRect(&rcSave,-5,-5);
                HICON iSave=ImageList_GetIcon(psv->list,0,ILD_NORMAL);
                DrawIconEx(memdc,rcSave.left,rcSave.top,iSave,16,16,0,0,DI_NORMAL);
                DestroyIcon(iSave);
            }
            if(ScratchView_GetZoneRect(hwnd,CTRL_CANCEL,&rcCancel)==0) {
                if(PtInRect(&rcCancel,psv->pt)) {
                    FillRect(memdc,&rcCancel,brushCtrls);
                }
                InflateRect(&rcCancel,-5,-5);
                HICON iCancel=ImageList_GetIcon(psv->list,1,ILD_NORMAL);
                DrawIconEx(memdc,rcCancel.left,rcCancel.top,iCancel,16,16,0,0,DI_NORMAL);
                DestroyIcon(iCancel);
            }
            if(ScratchView_GetZoneRect(hwnd,CTRL_OCR,&rcOCR)==0) {
                if(PtInRect(&rcOCR,psv->pt)) {
                    FillRect(memdc,&rcOCR,brushCtrls);
                }
                InflateRect(&rcOCR,-5,-5);
                HICON iOCR=ImageList_GetIcon(psv->list,8,ILD_NORMAL);
                DrawIconEx(memdc,rcOCR.left,rcOCR.top,iOCR,16,16,0,0,DI_NORMAL);
                DestroyIcon(iOCR);
            }

            DeleteObject(brushCtrls);
        }
    } else if(psv->mode==0) {
        /*
         * 绘制图像 ，图像可能会被压缩
         */
        graphics.DrawImage(img,Gdiplus::Rect(rcImg.left,rcImg.top,rcImg.right-rcImg.left,rcImg.bottom-rcImg.top));
    } else if(psv->mode==2) {
        /*
         * 图例模式
         * 在该案中，默认有9个图例
         * 默认将视图区域分割为 3×3的区域
         */
        int multi_count=0;
        for(int idx=0;idx<20&&page_name[idx][0]!='\0';idx++) multi_count++;
        
        RECT cell;
        ScratchView_GetZoneRect(hwnd,SAMPLES_VIEW,&cell);

        LOGFONT lf={0};
        GetObject(GetStockObject(SYSTEM_FONT),sizeof(LOGFONT),&lf);
        lf.lfHeight=18;
        lf.lfWeight=22;
        strcpy(lf.lfFaceName,"微软雅黑");
        HFONT font=CreateFontIndirect(&lf);
        SetBkMode(memdc,TRANSPARENT);
        SetTextColor(memdc,RGB(166,166,166));
        HFONT prefont=(HFONT)SelectObject(memdc,font);
        HBRUSH brushcell=CreateSolidBrush(RGB(53,53,53));
        HBRUSH brushcellhover=CreateSolidBrush(RGB(0,53,106));
        Gdiplus::Graphics graphics(memdc);
        int page_count=0;
        POINT ptMax={0,0};
        for(int idx=0;page_name[idx][0]!='\0';idx++,page_count++) {
            RECT cell_cur;
            ScratchView_GetZoneRect_SamplesItem(hwnd,idx,&cell_cur);
            if(cell_cur.right>ptMax.x) ptMax.x=cell_cur.right;
            if(cell_cur.bottom+15>ptMax.y) ptMax.y=cell_cur.bottom+15;

            OffsetRect(&cell_cur,-psv->offset.x,-psv->offset.y);
            if(cell_cur.bottom<0||cell_cur.top>cy) continue;

            RECT rcTitle={cell_cur.left,cell_cur.bottom-30,cell_cur.right,cell_cur.bottom};
            char titleText[256]={0};//page_name[idx];
            strcpy(titleText,page_name[idx]);
            if(strstr(titleText,"(")) *(strstr(titleText,"("))='\0';
            DrawText(memdc,titleText,-1,&rcTitle,DT_VCENTER|DT_CENTER|DT_SINGLELINE);

            RECT rcSmpImg={cell_cur.left,cell_cur.top,cell_cur.right,cell_cur.bottom-30};
            InflateRect(&rcSmpImg,-1,-1);
            if(psv->smps&&(psv->smps+idx)->img_path[0]!='\0') {
                wchar_t img_wchar[256]={0};
                int wchar_len=MultiByteToWideChar(CP_ACP,0,(psv->smps+idx)->img_path,-1,NULL,0);
                MultiByteToWideChar(CP_ACP,0,(psv->smps+idx)->img_path,-1,img_wchar,wchar_len);
                
                Gdiplus::Image smpImg(img_wchar);
                int maxWidth=rcSmpImg.right-rcSmpImg.left;
                int maxHeight=rcSmpImg.bottom-rcSmpImg.top;

                float rcRatio=(float)maxWidth/maxHeight;
                float imgRatio=(float)smpImg.GetWidth()/smpImg.GetHeight();
                POINT rcOffset={0,0};
                if(rcRatio>imgRatio) {
                    /*以高度为限制*/
                    rcOffset.x=((int)(maxWidth-imgRatio*maxHeight))>>1;
                } else {
                    rcOffset.y=((int)(maxHeight-maxWidth/imgRatio))>>1;
                }
                
                int rcImgWidth=maxWidth-rcOffset.x*2;
                int rcImgHeight=maxHeight-rcOffset.y*2;

                if(rcImgWidth>smpImg.GetWidth()) {
                    rcOffset.x=(maxWidth-smpImg.GetWidth())>>1;
                    rcOffset.y=(maxHeight-smpImg.GetHeight())>>1;
                }

                /*
                 * 使用 thumbnail 希望提升一下绘制的速度
                 */
                Gdiplus::Image* smpThumbnail=smpImg.GetThumbnailImage(maxWidth-rcOffset.x*2,maxHeight-rcOffset.y*2);
                graphics.DrawImage(smpThumbnail,rcSmpImg.left+rcOffset.x,rcSmpImg.top+rcOffset.y);
                delete smpThumbnail;
                //graphics.DrawImage(&smpImg,Gdiplus::Rect(rcSmpImg.left+rcOffset.x,rcSmpImg.top+rcOffset.y,(rcSmpImg.right-rcSmpImg.left-rcOffset.x*2),(rcSmpImg.bottom-rcSmpImg.top-rcOffset.y*2)));
            }

            if(PtInRect(&cell_cur,psv->pt)) {
                FrameRect(memdc,&cell_cur,brushcellhover);
            } else {
                FrameRect(memdc,&cell_cur,brushcell);
            }
        }
        DeleteObject(brushcell);
        DeleteObject(brushcellhover);
        SelectObject(memdc,prefont);
        DeleteObject(font);
        
        psv->pixels_x=ptMax.x;
        psv->pixels_y=ptMax.y;

    }

    if(psv->mode==1||psv->mode==2) {
        /*
         * 绘制滚动条
         */
        //水平滚动条
        RECT rcHorz,rcHorzThumb;
        if(ScratchView_GetZoneRect(hwnd,ZHSCROLL,&rcHorz)==0) {
            ScratchView_GetZoneRect(hwnd,ZHSTHUMB,&rcHorzThumb);

            COLORREF colorBar=RGB(5,5,5);
            HBRUSH brushBar=CreateSolidBrush(colorBar);
            FillRect(memdc,&rcHorz,brushBar);
            DeleteObject(brushBar);

            Gdiplus::GraphicsPath path;
            Gdiplus::LinearGradientBrush pbrush(Rect(rcHorzThumb.left,rcHorzThumb.top,rcHorzThumb.right-rcHorzThumb.left,rcHorzThumb.bottom-rcHorzThumb.top),
                Gdiplus::Color(255,53,53,53),
                Gdiplus::Color(155,0,0,0),
                LinearGradientModeVertical
            );
            graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
            path.AddArc(rcHorzThumb.left,rcHorzThumb.top,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,90,180);
            path.AddArc(rcHorzThumb.right-SCROLLBAR_PIXLS,rcHorzThumb.top,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,-90,180);
            graphics.FillPath(&pbrush,&path);
        }

        //垂直滚动条
        RECT rcVert,rcVertThumb;
        if(ScratchView_GetZoneRect(hwnd,ZVSCROLL,&rcVert)==0) {
            ScratchView_GetZoneRect(hwnd,ZVSTHUMB,&rcVertThumb);
            
            COLORREF colorBar=RGB(5,5,5);
            HBRUSH brushBar=CreateSolidBrush(colorBar);
            FillRect(memdc,&rcVert,brushBar);
            DeleteObject(brushBar);

            Gdiplus::GraphicsPath path;
            Gdiplus::LinearGradientBrush pbrush(Rect(rcVertThumb.left,rcVertThumb.top,rcVertThumb.right-rcVertThumb.left,rcVertThumb.bottom-rcVertThumb.top),
                Gdiplus::Color(255,53,53,53),
                Gdiplus::Color(155,0,0,0),
                LinearGradientModeHorizontal
            );
            graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
            path.AddArc(rcVertThumb.left,rcVertThumb.top,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,180,180);
            path.AddArc(rcVertThumb.left,rcVertThumb.bottom-SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,0,180);
            graphics.FillPath(&pbrush,&path);
        }
    }

#ifdef SCRATCHVIEW_DBGTEST
    if(psv->svtest.paint) psv->svtest.paint(hwnd,memdc,&rcMem,ptOffset);
#endif
SCRATCHVIEWPAINT_EXIT:
    /*
     * 绘制 panel
     */
    RECT rcPanel;
    if(ScratchView_GetZoneRect(hwnd,CTRL_PANEL,&rcPanel)==0) {
        if(PtInRect(&rcPanel,psv->pt)) {
            Gdiplus::GraphicsPath path;
            int slashoffset=rcPanel.bottom-rcPanel.top;
            path.AddLine((int)rcPanel.left, (int)rcPanel.top,(int)rcPanel.left+slashoffset,(int)rcPanel.bottom);
            path.AddLine((int)rcPanel.left+slashoffset,(int)rcPanel.bottom,(int)rcPanel.right-slashoffset,(int)rcPanel.bottom);
            path.AddLine((int)rcPanel.right-slashoffset,(int)rcPanel.bottom,(int)rcPanel.right,(int)rcPanel.top);
            path.AddLine((int)rcPanel.right,(int)rcPanel.top,(int)rcPanel.left,(int)rcPanel.top);

            Gdiplus::LinearGradientBrush panelBrush(Gdiplus::Rect(rcPanel.left,rcPanel.top,rcPanel.right-rcPanel.left,rcPanel.bottom-rcPanel.top),
                Gdiplus::Color(255,55,55,55),
                Gdiplus::Color(255,35,35,35),
                LinearGradientModeVertical
            );
            graphics.FillPath(&panelBrush,&path);

            HBRUSH brushicon=CreateSolidBrush(RGB(10,10,10));
            RECT rcFile,rcExpend,rcCollapse,rcMultiMode,rcLaunch,rcSuspend;
            if(ScratchView_GetZoneRect(hwnd,CTRL_FILE,&rcFile)==0) {
                if(PtInRect(&rcFile,psv->pt)) {
                    FillRect(memdc,&rcFile,brushicon);
                }                    
                HICON icon=ImageList_GetIcon(psv->list,2,ILD_NORMAL);
                InflateRect(&rcFile,-5,-5);
                DrawIconEx(memdc,rcFile.left,rcFile.top,icon,24,24,0,0,DI_NORMAL);
                //DrawIconEx(memdc,rcFile.left,rcFile.top,icon);
                DestroyIcon(icon);
            }
            if(ScratchView_GetZoneRect(hwnd,CTRL_MODE_ORI,&rcExpend)==0) {
                if(PtInRect(&rcExpend,psv->pt)) {
                    FillRect(memdc,&rcExpend,brushicon);
                }
                HICON icon=ImageList_GetIcon(psv->list,3,ILD_NORMAL);
                InflateRect(&rcExpend,-5,-5);
                DrawIconEx(memdc,rcExpend.left,rcExpend.top,icon,24,24,0,0,DI_NORMAL);
                DestroyIcon(icon);
            }
            if(ScratchView_GetZoneRect(hwnd,CTRL_MODE_SCALE,&rcCollapse)==0) {
                if(PtInRect(&rcCollapse,psv->pt)) {
                    FillRect(memdc,&rcCollapse,brushicon);
                }
                HICON icon=ImageList_GetIcon(psv->list,4,ILD_NORMAL);
                InflateRect(&rcCollapse,-5,-5);
                DrawIconEx(memdc,rcCollapse.left,rcCollapse.top,icon,24,24,0,0,DI_NORMAL);
                DestroyIcon(icon);
            }
            if(ScratchView_GetZoneRect(hwnd,CTRL_CONFIG,&rcMultiMode)==0) {
                if(PtInRect(&rcMultiMode,psv->pt)) {
                    HBRUSH brushCustom=CreateSolidBrush(RGB(45,45,45));
                    //FillRect(memdc,&rcMultiMode,brushicon);
                    FillRect(memdc,&rcMultiMode,brushCustom);
                    DeleteObject(brushCustom);
                }                    
                HICON icon=ImageList_GetIcon(psv->list,5,ILD_NORMAL);
                InflateRect(&rcMultiMode,-5,-5);
                DrawIconEx(memdc,rcMultiMode.left,rcMultiMode.top,icon,24,24,0,0,DI_NORMAL);
                DestroyIcon(icon);
            }
            if(ScratchView_GetZoneRect(hwnd,CTRL_LAUNCHWORK,&rcLaunch)==0) {
                if(PtInRect(&rcLaunch,psv->pt)) {
                    FillRect(memdc,&rcLaunch,brushicon);
                }                    
                HICON icon=ImageList_GetIcon(psv->list,6,ILD_NORMAL);
                InflateRect(&rcLaunch,-5,-5);
                DrawIconEx(memdc,rcLaunch.left,rcLaunch.top,icon,24,24,0,0,DI_NORMAL);
                DestroyIcon(icon);
            }
            if(ScratchView_GetZoneRect(hwnd,CTRL_SUSPENDWORK,&rcSuspend)==0) {
                if(PtInRect(&rcSuspend,psv->pt)) {
                    FillRect(memdc,&rcSuspend,brushicon);
                }                    
                HICON icon=ImageList_GetIcon(psv->list,7,ILD_NORMAL);
                InflateRect(&rcSuspend,-5,-5);
                DrawIconEx(memdc,rcSuspend.left,rcSuspend.top,icon,24,24,0,0,DI_NORMAL);
                DestroyIcon(icon);
            }

            DeleteObject(brushicon);
        }
    }

    BitBlt(hdc,0,0,cx,cy,memdc,0,0,SRCCOPY);
    SelectObject(memdc,prebmp);
    DeleteObject(bmp);
    DeleteDC(memdc);
    
    EndPaint(hwnd,&ps);
    return 0;
}

int ScratchView_GetZoneRect_SamplesItem(HWND hwnd,int index,LPRECT prc) {
    RECT rc;
    GetClientRect(hwnd,&rc);

    int cx=rc.right-rc.left,cy=rc.bottom-rc.top;
    int paddingcx=25,paddingcy=15;
    int cell_width=200,cell_height=350;

    int cols=cx/cell_width;
    if(cols*cell_width+(cols+1)*paddingcx>cx) cols--;
    if(cols<=0) cols=1;

    POINT pt={
        paddingcx+((cx-(cols*cell_width+(cols+1)*paddingcx))>>1),
        paddingcy
    };

    if(pt.x<=0) pt.x=paddingcx;

    int row=index/cols,col=index%cols;
    SetRect(prc,pt.x,pt.y,pt.x+cell_width,pt.y+cell_height);
    OffsetRect(prc,col*(cell_width+paddingcx),row*(cell_height+paddingcy));
    return 0;
}

int ScratchView_GetSamplesItemIndex(HWND hwnd,const POINT pt) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) return -1;

    for(int idx=0;page_name[idx][0]!='\0';idx++) {
        RECT rcItem;
        ScratchView_GetZoneRect_SamplesItem(hwnd,idx,&rcItem);
        OffsetRect(&rcItem,-psv->offset.x,-psv->offset.y);
        if(PtInRect(&rcItem,pt)) return idx;
    }

    return -1;
}

int ScratchView_SetSmps(HWND hwnd,PAGECODE pagecode,char* filename) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) return -1;

    char* extensions=PathFindExtension(filename);
    if(!strstr(extensions,"jpg")&&
       !strstr(extensions,"png")&&
       !strstr(extensions,"bmp")) {
        char msghint[256]={0};
        sprintf(msghint,"当前选择的文件[%s]格式不被支持，请选择合适的图片格式!\n目前支持*.jpg *.png *.bmp的图片",filename);
        MessageBox(hwnd,msghint,"提示",MB_OK|MB_ICONINFORMATION);
        return -1;
    }

    if(!psv->smps) {
        psv->smps=(pTRZSmps)calloc(sizeof(TRZSmps)*20,1);

        for(int idx=0;page_name[idx][0]!='\0';idx++) {
            psv->smps[idx].code=(PAGECODE)idx;
            strcpy(psv->smps[idx].pagedesc,page_name[idx]);
            psv->smps[idx].trz_count=0;
            psv->smps[idx].trz_list=NULL;
        }
    }

    if(!(psv->smps+(int)pagecode)) return -1;
    strcpy((psv->smps+(int)pagecode)->img_path,filename);

    /*
     * 清除 识别矩形的相关设置
     */
    if(psv->rcRecgnoizes) {
        if(psv->rcSelList) {
            free(psv->rcSelList);
            psv->rcSelList=NULL;
            psv->rcSelCount=0;
            psv->rcSelIdx=-1;
        }

        psv->showRcRecgSwitch=0;
        free(psv->rcRecgnoizes);
        psv->rcRecgnoizes=NULL;
        psv->countRecgnoizes=0;
    }

    return 0;
}

int cmpRectSize(const void* a,const void* b) {
    LPRECT prcA=(LPRECT)a;
    LPRECT prcB=(LPRECT)b;

    return (abs(prcA->right-prcA->left)*abs(prcA->bottom-prcA->top)-
            abs(prcB->right-prcB->left)*abs(prcB->bottom-prcB->top));
}

int ScratchView_SaveTRZProjFile(HWND hwnd,char* proj,pMajor major,int cx,int cy,const char* proj_desc) {
    /*
     * 似乎存在字节对齐的pack问题
     * 将采用填充结构TRZProjFile的方式写入
     */
    TRZProjFile tpf={0};
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) return -1;
    
    char path[256]={0};
    GetModulePath(path);
    char folder[256]={0};
    sprintf(folder,"%s%s\\",path,proj);

    CreateDirectory(folder,NULL);

    char projfilename[256]={0};
    sprintf(projfilename,"%s%s%s",folder,proj,TRZPROJ_EXTENSION);
    // 使用 CreateFile 创建新文件
    HANDLE hFile=CreateFile(
        projfilename,               // 文件路径
        GENERIC_WRITE,          // 写入权限
        0,                      // 独占访问 (不共享)
        NULL,                   // 默认安全属性
        CREATE_ALWAYS,             // 如果文件不存在则创建新文件
        FILE_ATTRIBUTE_NORMAL,  // 常规文件属性
        NULL                    // 不使用模板文件
    );

    // 检查文件是否成功创建
    if(hFile==INVALID_HANDLE_VALUE) {
        char buffer[256]={0};
        sprintf(buffer,"文件[%s]创建失败，错误代码:%d",projfilename,GetLastError());
        MessageBox(hwnd,buffer,"异常",MB_OK|MB_ICONWARNING);
        return -1;
    }

    char proj_desc_buffer[256]={0};
    strcpy(proj_desc_buffer,proj_desc);

    strcpy(tpf.projname,proj);
    strcpy(tpf.projdesc,proj_desc_buffer);
    tpf.cx=cx;tpf.cy=cy;
    for(int idx=0;idx<psv->smps_count;idx++) {
        if((psv->smps+idx)->img_path[0]!='\0') {
            char* filename=page_name[idx];
            char imgfile[256]={0};
            char* imgextplt=strrchr((psv->smps+idx)->img_path,'.');
            if(!imgextplt) continue;
            
            sprintf(imgfile,"%s%s%s",folder,page_name[idx],imgextplt);
            CopyFile((psv->smps+idx)->img_path,imgfile,FALSE);
            strcpy(tpf.page_imgs[idx],page_name[idx]);
            strcat(tpf.page_imgs[idx],imgextplt);
        }
    }
    tpf.pages_count=psv->smps_count;

    int trz_count=_msize(major->trz)/sizeof(TRZ);
    for(int idx=0;idx<100;idx++) {
        if(idx<trz_count) {
            memcpy(&tpf.trz[idx],major->trz+idx,sizeof(TRZ));
            tpf.trz[idx].pText=NULL;
        } else break;
    }
    tpf.trzs_count=trz_count;

    // 将数据写入文件
    DWORD bytesWritten;
    BOOL result=WriteFile(
        hFile, 
        &tpf, 
        sizeof(tpf), 
        &bytesWritten, 
        NULL 
    );

    // 关闭文件句柄
    CloseHandle(hFile);

    return 0;
}

int ScratchView_AlterTRZProjFile(HWND hwnd,char* proj,PAGECODE code,const char* pagefile,pTRZ list,int list_count) {
  
    HANDLE hFile=CreateFile(proj,GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if (hFile==INVALID_HANDLE_VALUE) {
        char buffer[256]={0};
        sprintf(buffer,"无法打开文件，错误代码: %d",GetLastError());
        MessageBox(hwnd,buffer,"异常",MB_OK|MB_ICONWARNING);
        return -1;
    }

    // 定位到文件中的指定位置，例如从字节偏移量 10 开始
    int offset_pagename=sizeof(char[256])+sizeof(int)*2;
    int offset_trzs=offset_pagename+sizeof(char[20][256])+sizeof(int);

    if(pagefile) {
        DWORD dwPtr=SetFilePointer(hFile,offset_pagename+sizeof(char[256])*(int)code,NULL,FILE_BEGIN);
        if (dwPtr==INVALID_SET_FILE_POINTER) {
            char buffer[256]={0};
            sprintf(buffer,"无法设置文件指针，错误代码:  %d",GetLastError());
            MessageBox(hwnd,buffer,"异常",MB_OK|MB_ICONWARNING);
            CloseHandle(hFile);
            return -1;
        }

        // 修改该位置的内容
        DWORD bytesWritten;
        BOOL result=WriteFile(hFile,pagefile,256, &bytesWritten, NULL);
        if(!result) {
            char buffer[256]={0};
            sprintf(buffer,"写入文件失败，错误代码: %d",GetLastError());
            MessageBox(hwnd,buffer,"异常",MB_OK|MB_ICONWARNING);
            CloseHandle(hFile);
            return -1;
        }
    }

    if(list_count>0&&list) {
        for(int idx=0,trz_idx=0;idx<list_count&&trz_idx<list_count;idx++) {
            DWORD dwPtr=SetFilePointer(hFile,offset_pagename+sizeof(char[256])*idx,NULL,FILE_BEGIN);
            if (dwPtr==INVALID_SET_FILE_POINTER) {
                char buffer[256]={0};
                sprintf(buffer,"无法设置文件指针，错误代码:  %d",GetLastError());
                MessageBox(hwnd,buffer,"异常",MB_OK|MB_ICONWARNING);
                CloseHandle(hFile);
                return -1;
            }
            TRZ trz;
            DWORD dwRead;
            ReadFile(hFile,&trz,sizeof(TRZ),&dwRead,NULL);
            if(trz.code==list[idx].code) {
                // 修改该位置的内容
                DWORD bytesWritten;
                BOOL result=WriteFile(hFile,&trz,sizeof(trz),&bytesWritten,NULL);
                if(!result) {
                    char buffer[256]={0};
                    sprintf(buffer,"写入文件失败，错误代码: %d",GetLastError());
                    MessageBox(hwnd,buffer,"异常",MB_OK|MB_ICONWARNING);
                    CloseHandle(hFile);
                    return -1;
                }
                trz_idx++;
            }
        }
    }

    // 关闭文件句柄
    CloseHandle(hFile);

    return 0;

}

int ScratchView_LoadTRZProjFile(HWND hwnd,char* proj) {
    // HANDLE hFile=CreateFile(proj,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    // if (hFile==INVALID_HANDLE_VALUE) {
    //     char buffer[256]={0};
    //     sprintf(buffer,"无法打开文件，错误代码: %d",GetLastError());
    //     MessageBox(hwnd,buffer,"异常",MB_OK|MB_ICONWARNING);
    //     return -1;
    // }

    // DWORD bytesRead;
    // TRZProjFile  trzproj={0};
    // ReadFile(hFile,&trzproj,sizeof(TRZProjFile),&bytesRead,NULL);
    // CloseHandle(hFile);

    // /*
    //  */
    // if(GMajor->trz&&trzproj.trzs_count>0) {
    //     memcpy(GMajor->trz,&trzproj.trz,trzproj.trzs_count*sizeof(TRZ));
    // }
    if(0!=Major_SetCurrProj(GMajor,proj)) {
        char msgbuffer[256]={0};
        sprintf(msgbuffer,"加载方案[%s]失败!",proj);
        MessageBox(hwnd,msgbuffer,"提示",MB_OK|MB_ICONWARNING);
        return -1;
    }

    // pScratchView psv=ScratchView_GetSettings(hwnd);
    // if(psv&&psv->smps) {
    //     for(int idx=0;idx<psv->smps_count;idx++) {
    //         strcpy((psv->smps+idx)->img_path,trzproj.page_imgs[idx]);
    //     }
    // }



    return 0;
}

#ifdef SCRATCHVIEW_DBGTEST
int PaintFGDetail(HWND hwnd,HDC hdc,LPRECT prc,POINT offset) {
    pTRZ trz_min=gettrzbycode(GMajor->trz,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL);
    RECT rcMin={
        trz_min->x,trz_min->y,
        trz_min->x+trz_min->width,trz_min->y+trz_min->height
    };

    pTRZ trz_sec=gettrzbycode(GMajor->trz,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL2);
    RECT rcSec={
        trz_sec->x,trz_sec->y,
        trz_sec->x+trz_sec->width,trz_sec->y+trz_sec->height
    };

    pTRZ trz_btn=gettrzbycode(GMajor->trz,DOU_FORTUNEGIFT_DETAIL_EXECTASK);
    RECT rcBtn={
        trz_btn->x,trz_btn->y,
        trz_btn->x+trz_btn->width,trz_btn->y+trz_btn->height
    };
    /** 
     * 绘图
     */
    HBRUSH hNullBrush=(HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH hOldBrush=(HBRUSH)SelectObject(hdc, hNullBrush);
    //设置画笔宽度
    HPEN pen=CreatePen(PS_SOLID,3.5,RGB(100,100,100));
    HPEN prePen=(HPEN)SelectObject(hdc,pen);
    OffsetRect(&rcMin,offset.x,offset.y);
    Rectangle(hdc,rcMin.left,rcMin.top,rcMin.right,rcMin.bottom);
    DrawText(hdc,"分",-1,&rcMin,DT_VCENTER|DT_CENTER);
    OffsetRect(&rcSec,offset.x,offset.y);
    Rectangle(hdc,rcSec.left,rcSec.top,rcSec.right,rcSec.bottom);
    DrawText(hdc,"秒",-1,&rcSec,DT_VCENTER|DT_CENTER);
    OffsetRect(&rcBtn,offset.x,offset.y);
    Rectangle(hdc,rcBtn.left,rcBtn.top,rcBtn.right,rcBtn.bottom);
    DrawText(hdc,"执行按钮",-1,&rcBtn,DT_VCENTER|DT_CENTER);

    SelectObject(hdc,prePen);
    SelectObject(hdc,hOldBrush);
    DeleteObject(pen);
    return 0;
}
int CmdFGDetail(HWND hwnd) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) return -1;

    char file[256]={0};
    GetModulePath(file);
    strcat(file,"pics\\x.png");
    tesseract::TessBaseAPI* api=new tesseract::TessBaseAPI();
    //1.调用识图，更新 
    PAGECODE page=GetPageCode(api,file,GMajor->trz);
    if(page==FORTUNEGIFT_DETAIL||page==FORTUNEGIFT_DETAIL_2TASK) {
        fg_detail_ocr_prepare_dynamic(GMajor->trz,file);
    } else {
        MessageBox(hwnd,"未能有效识别出页面类型","页面",MB_OK);
    }
    if(api) delete api; 

    psv->svtest.paint=PaintFGDetail;
    InvalidateRect(hwnd,NULL,TRUE);

    return 0;
}
#endif

int ScrathView_LoadTest(HWND hwnd) {
#ifdef SCRATCHVIEW_DBGTEST
    CmdFGDetail(hwnd);
#endif
    return 0;
}