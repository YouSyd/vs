#ifndef IMG_SCRATCH_HEADER
#define IMG_SCRATCH_HEADER

#include <stdio.h>
#include <stdlib.h>

#include <opencv2/opencv.hpp>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <gdiplus.h>

#include "tesseract_test.h"

#include "PopupMenu.h"

#define CTRL_IMGSCRATCH "ImgScratch"
#define SCROLLBAR_PIXLS 13
#define THUMB_MIN_PIXLS 50

/*
 * trz方案文件
 */
typedef struct trz_projfile {
    /*
     * 方案名称的描述信息
     */
    char projname[256];
    char projdesc[256];

    /* 
     * 图像分辨率
     */
    int cx;
    int cy;

    /*
     * 各页面 图像名称
     */
    char page_imgs[20][256];//存储相对路径文件
    int pages_count;

    TRZ trz[100];//区域设定信息
    int trzs_count;
}TRZProjFile,*pTRZProjFile;

/*
 * 手机截图，用于定义选区 
 * 支持缩放，原大小滚动，文件可以设置默认打开文件，也可以手动指定打开问题
 * 显示模式: 全图、原尺寸，支持窗口内滚动
 * 支持点击选择区域，调整选区等
 */
typedef struct trz_proj {
    char projname[256];
    int width;
    int height;

    pTRZ trz_list;
    int trz_count;
} TRZProj,*pTRZProj;

typedef struct trz_samples{
    PAGECODE code;
    char img_path[256]; //存储绝对路径文件
    char pagedesc[256];

    pTRZ trz_list;
    int trz_count;
}TRZSmps,*pTRZSmps;

//////////////////////////////////////////////////////
/*****************************************************
 * 测试或者调试代码
 */
#ifdef SCRATCHVIEW_DBGTEST
typedef int (*PaintTestPlt)(HWND,HDC,LPRECT,POINT);
typedef int (*CmdTestPlt)(HWND);

typedef struct SVDBGTest{
    PaintTestPlt paint;
    CmdTestPlt cmd;
}SVDbgTest,*pSVDbgTest;
#endif
//////////////////////////////////////////////////////

typedef struct scratchview {
    HIMAGELIST list;
    HBRUSH menubkbrush;
    /*
     * 滚动条 位移
     */
    POINT offset;

    /*
     * 当前鼠标位置
     */
    POINT pt;

    /* 
     * 选区起点，选区高宽 
     */
    POINT pos;
    int cx;
    int cy;

    /*
     * 图像
     * 是该记录图像的内存对象 gdiplus::Image 还是记录 文件名呢？
     */
    char img_path[256];
    Gdiplus::Bitmap* img;
    int pixels_x; /* 图像分辨率 */
    int pixels_y;
    
    /*
     * 显示模式 0-自适应窗口
     *         1-原始尺寸
     *         2-samples 选图界面
     */
    int mode;

    /*
     * 处理滚动条滚动,左键位置
     */
    POINT prept;

    /* 图例*/
    pTRZSmps smps;
    int smps_count;

    /*
     * 激活的smp item 
     * 设置了激活的smp item， 在mode==1时，会进行模式匹配搜索
     */
    int active_smpidx;

    /* 输出或接口输入的依据*/
    pTRZProj proj;

    int showRcRecgSwitch;
    LPRECT rcRecgnoizes;
    int countRecgnoizes;

    LPRECT* rcSelList;
    int rcSelCount;
    int rcSelIdx; /* 当前选中 */

#ifdef SCRATCHVIEW_DBGTEST
    SVDbgTest svtest;
#endif
} ScratchView,*pScratchView;

int ScratchView_InitialSettings(HWND hwnd);
pScratchView ScratchView_GetSettings(HWND hwnd);
int ScratchView_SetImage(HWND hwnd,char* img);
int ScratchView_ChangeMode(HWND hwnd,int mode);
void ScratchView_ClearSettings(HWND hwnd);

#define IMG_VIEW        20
#define CTRL_PANEL      21
#define CTRL_MODE_SCALE 22 /* 自适应缩略图 */
#define CTRL_MODE_ORI   23 /* 原始尺寸 */
#define CTRL_SELRECT    24 /* 设置选区 */
#define CTRL_FILE       25 /* 打开新的图片 */
#define CTRL_CONFIG     26 /* 打开选区配置 与选区保存按钮同操作*/
#define SAMPLES_VIEW    27 /* 多模板页面视图 mode==2*/
#define CTRL_SAVE       28 /* 选区的保留按钮*/
#define CTRL_CANCEL     29 /* 选区的撤销按钮*/
#define CTRL_OCR        32 /* 选区的OCR识别 */
#define CTRL_LAUNCHWORK 30 /* 启动工作流程*/
#define CTRL_SUSPENDWORK    31 /* 暂停工作流程*/
int ScratchView_GetZoneRect(HWND hwnd,int poscode,LPRECT prc);
int ScratchView_GetZoneRect_SamplesItem(HWND hwnd,int index,LPRECT prc);
int ScratchView_GetSamplesItemIndex(HWND hwnd,const POINT pt);
int ScratchView_HitTest(HWND hwnd,POINT pt);

int ScratchView_SetSmps(HWND hwnd,PAGECODE pagecode,char* filename);

LRESULT CALLBACK ScratchViewProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
int ScratchView_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam);
int ScrathView_LoadTest(HWND hwnd);

/*
 * TRZ 方案文件可能有很多个，可能会多次编辑更新
 * 1、方案的命名与组织
 *    方案包含一个数据结构文件，TRZ编辑的基准图片组
 *    方案的名字与组织方案的文件夹同名，数据结构的名字为【方案文件夹的名字】.trz
 *    图片组的文件名为对应页面的名称
 *    方案文件夹置于运行程序的同级目录下
 *    那么如何知道哪些文件夹是属于方案文件夹呢？
 *    需要一个方案索引文件 TRZProjIndex.txt
 *    格式如下 ：
 *    方案数量-字符串-换行
 *    方案名-字符串-换行
 *    ....
 * 2、方案的选择与加载
 *    寻找同级目录下的 方案索引 TRZProjIndex.txt
 *    解析
 *    新增方案、删除方案等，同步更新方案目录的文件和数据
 * 3、方案页面及选区的更新，支持按照页面更新
 *    指定的方案，更新特定的页面基准图片以及对应的选区数据
 * 4、方案选区的预览
 *    选定的方案页面，标识出设定的选区
 * 
 * 
 */
int ScratchView_SaveTRZProjFile(HWND hwnd,char* proj,pMajor major,int cx,int cy,const char* proj_desc);
int ScratchView_AlterTRZProjFile(HWND hwnd,char* proj,PAGECODE code,const char* pagefile,pTRZ list,int list_count);
int ScratchView_LoadTRZProjFile(HWND hwnd,char* proj);

int cmpRectSize(const void* a,const void* b);
/*
 * 设定TRZ的消息
 * WPARAM pTRZ :指向需要修正坐标的TRZ
 * LPARAM TRZCODE :选区代码 
 */
#define MSG_SECTIONSETTINGS (UINT)(WM_USER+0x0010)

/*
 * 更换TRZProj 对应的smps
 * WPARAM major: GMajor
 * LPARAM pTRZProjFile: ptrzprojfile
 */
#define MSG_SETSMPSIMGS     (UINT)(WM_USER+0x0011)

/*
 * 将指定的TRZProj 写入TRZProjFile
 * WPARAM major: GMajor
 * LPARAM proj name: proj
 */
#define MSG_SAVEPROJ    (UINT)(WM_USER+0x0012)


#endif
