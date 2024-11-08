/*
 * 该程序用于自动刷取抖音app的福袋与斗钻
 * 福袋与斗钻公用一个预览界面，即直播间初始界面，获取当前直播间人数、有无斗钻或福袋标志，剩余时间
 * 对于福袋直播间，点击福袋，进入福袋详细信息界面，并操作参与抽福袋
 * 程序等待剩余时间，进入福袋抽奖结束界面，确定是否中奖，分为 中奖 和 未中奖 两个界面
 *     未中奖，点击×，回到预览界面，或者 直播结束 界面
 *     中奖，点击领奖等，切换直播间，回到预览界面。
 * 对于斗钻直播间，点击斗钻，进入斗钻详细信息界面，并操作参与抽斗钻
 * 
 * 截图类型：
 * 1、公共预览界面
 * 2、福袋详细信息界面
 * 3、福袋结算界面-未中奖
 * 4、福袋结算界面-中奖
 * 5、斗钻详细信息界面
 * 6、斗钻结算界面-未中
 * 7、斗钻结算界面-中
 * 8、直播结束界面
 *
 * 状态机
 * ------------------------------------------------------                 
 *
 *           +------+--<<---[3]----<<---+
 *           |      |                   |                          
 *           ↓      ↓                   |                      
 * ---->> --[8]--- [1]>+----[2]---->>-->+-----[]
 *           ↑      ↑                   |                          
 *           |      |                   |     
 *           +------+--<<---[4]----<<---+
 *
 *
 *           +------+--<<---[6]----<<---+
 *           |      |                   |                          
 *           ↓      ↓                   |                      
 * ---->> --[8]--- [1]>+----[5]---->>-->+-----[]
 *           ↑      ↑                   |                          
 *           |      |                   |     
 *           +------+--<<---[7]----<<---+
 *
 * Actions:
 * ------------------------------------------------------         
 * 1--->2 : 点击福袋，等待5s，设置定时器，到时候自动截屏分析，确定是由状态2切换到何种状态
 * 2--->3 : 自动弹出
 * 2--->4 : 自动弹出
 * 2--->8 : 自动弹出
 * 3--->1 : 点击屏幕非弹框区
 * 4--->1 : 点击领奖，切换到交易界面，点击回到直播间（这里漏了一个交易界面状态）
 * 8--->1 : 进入其它直播间
 *
 */

#ifndef TESSERACT_TEST_HEADER
#define TESSERACT_TEST_HEADER

#include <stdio.h>
#include <stdlib.h>

#include <opencv2/opencv.hpp>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <windows.h>
#include <winternl.h>

#include "exec_cmd.h"
#include "dialog.h"

#pragma comment(lib,"opencv_calib3d4.lib")
#pragma comment(lib,"opencv_core4.lib")
#pragma comment(lib,"opencv_dnn4.lib")
#pragma comment(lib,"opencv_features2d4.lib")
#pragma comment(lib,"opencv_flann4.lib")
#pragma comment(lib,"opencv_highgui4.lib")
#pragma comment(lib,"opencv_imgcodecs4.lib")
#pragma comment(lib,"opencv_imgproc4.lib")
#pragma comment(lib,"opencv_ml4.lib")
#pragma comment(lib,"opencv_objdetect4.lib")
#pragma comment(lib,"opencv_photo4.lib")
#pragma comment(lib,"opencv_stitching4.lib")
#pragma comment(lib,"opencv_video4.lib")
#pragma comment(lib,"opencv_videoio4.lib")

#pragma comment(lib,"tesseract54.lib")
#pragma comment(lib,"leptonica-1.84.1.lib")

#define ENUM_NAMES(name,desc) enum_names(name,desc,#name)
#define ENUMNAMES_SECTION ENUM_NAMES(STREAMER_ROOM_IMG,"直播间标志搜索区域") \
ENUM_NAMES(STREAMER,"主播") \
ENUM_NAMES(THUMBUP_OR_STREAMER_DESC,"点赞描述") \
ENUM_NAMES(ALIVECOUNT,"在线人数") \
ENUM_NAMES(FORTUNEGIFT_OR_DIAMOND_RANGE,"福袋的搜索范围") \
ENUM_NAMES(DOU_DIAMOND_DESC,"斗钻参与人数、价值") \
ENUM_NAMES(DOU_DIAMOND_COUNTDOWN,"斗钻倒计时")\
ENUM_NAMES(DOU_FORTUNEGIFT_COUNTDOWN,"福袋倒计时")\
ENUM_NAMES(FORTUNEGIFT_CLICKRECT,"福袋区域")\
ENUM_NAMES(DIAMOND_CLICKRECT,"抖钻区域")\
\
ENUM_NAMES(DOU_FORTUNEGIFT_DETAIL_IMG,"福袋详情页面识别标志")\
ENUM_NAMES(DOU_FORTUNEGIFT_COUNTDOWN_DETAIL,"详情页面倒计时(分)")\
ENUM_NAMES(DOU_FORTUNEGIFT_COUNTDOWN_DETAIL2,"详情页面倒计时(秒)")\
ENUM_NAMES(DOU_FORTUNEGIFT_TASKRECT,"任务描述选区")\
ENUM_NAMES(DOU_FORTUNEGIFT_DETAIL_TEXT,"详情描述")\
ENUM_NAMES(DOU_FORTUNEGIFT_DETAIL_TASK1,"任务描述")\
ENUM_NAMES(DOU_FORTUNEGIFT_DETAIL_TASK2,"任务描述2")\
ENUM_NAMES(DOU_FORTUNEGIFT_DETAIL_EXECTASK,"任务执行按钮")\
\
ENUM_NAMES(DOU_DIAMOND_DETAIL_IMG,"斗钻详情页面识别标志")\
\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_NOPRIZE_IMG,"福袋未中奖页面识别标志")\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_NOPRIZE_EXIT,"福袋未中奖页面退出按钮")\
\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_JACKPOT_IMG,"福袋中奖页面标志")\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_JACKPOT_PRIZEDESC,"福袋中奖页面-中奖内容描述")\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_JACKPOT_CFMAWARD,"福袋中奖页面-领奖按钮")\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO,"福袋中奖页面-check协议")\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO_CHECKED,"福袋中奖页面-check协议-选中样式")\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_JACKPOT_CLOSEBTN,"福袋中奖页面-关闭按钮")\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_JACKPOT_POPUP,"福袋中奖页面-弹框区域")\
\
ENUM_NAMES(STREAMER_ROOMCLOSED_IMG,"直播间关闭标志搜索区域")\
\
ENUM_NAMES(DOU_FORTUNEGIFT_AWARDFORTUNEGIFT_IMG,"福袋领奖页面标志")\
ENUM_NAMES(DOU_FORTUNEGIFT_AWARDFORTUNEGIFT_AFMBTN,"福袋领奖页面-领取奖品")\
\
ENUM_NAMES(STREAMERROOM_QUIT,"退出直播间主界面")

typedef enum {
#define enum_names(enum_code,enum_desc,enum_name) enum_code,
    ENUMNAMES_SECTION
#undef enum_names
    
}TRZCODE;
extern char section_name[100][256];

#define ENUMNAMES_PAGE ENUM_NAMES(UNKNOWN_PAGE,"未知页面") \
ENUM_NAMES(STREAMER_ROOM,"直播间主界面") \
ENUM_NAMES(FORTUNEGIFT_DETAIL,"福袋详情页面") \
ENUM_NAMES(FORTUNEGIFT_DETAIL_2TASK,"福袋详情页面-两步任务") \
ENUM_NAMES(FORTUNEGIFT_NOPRIZE,"福袋未中奖界面") \
ENUM_NAMES(FORTUNEGIFT_JACKPOT,"福袋中奖界面") \
ENUM_NAMES(DIAMOND_DETAIL,"斗钻详情页面") \
ENUM_NAMES(DIAMOND_NOPRICE,"斗钻未中奖界面") \
ENUM_NAMES(DIAMOND_JACKPOT,"斗钻中奖界面") \
ENUM_NAMES(STREAMER_ROOMCLOSED,"直播间关闭界面") \
ENUM_NAMES(FORTUNEGIFT_AFMAWARD,"领奖确认界面") \
ENUM_NAMES(WATCHLIST,"关注列表界面")

typedef enum {
#define enum_names(enum_code,enum_desc,enum_name) enum_code,
    ENUMNAMES_PAGE
#undef enum_names

}PAGECODE;
extern char page_name[20][256];

typedef struct text_rect_zone {
    int x;
    int y;
    int width;
    int height;
    TRZCODE code;
    
    char* pText;
    
    PAGECODE page;
    char traindata[256];//traindata字库，默认为 chi_sim
    char whitelist[256];// tessedit_char_whitelist 参数
    
} TRZ,*pTRZ;

typedef struct major {
    char host[256]; //主机名

    pTRZ trz;//list
    
    int hasfg;//福袋标记
    int alivecount;//在线人数
    int timestamp;//获取当前倒计时的时间戳
    int fg_countdown;//福袋倒计时剩余秒数
    char* fg_desc;//福袋描述
    PAGECODE currpage;//当前页面
    
    int taskcount;
    int taskstatus[3];

    int proto_checked;//中奖后是否点确认协议 0-未点选 1-点选 -1-未设置

    /*
     * 设置参与的时机
     * 比如倒计时 15 分钟， 可以设置一个 参与的时机 比如 7 分钟 、6.5 分钟
     */
    int attend_countdown;

    /*
     * trz 方案数据
     */
    char* trzproj_buffer;
    char** trzproj_list;
    int trzproj_count;
    int activeproj_index;

    /**
     * strategy code
     * 运行策略
     */
    int strategycode;
    
}Major,*pMajor;

extern pMajor GMajor;
extern TRZ sections[];
extern const int sections_counts;

extern char datapath[256];

#define TRZPROJ_EXTENSION   ".trz"
#define TRZPROJINDEXFILE "TRZProjIndex.txt"

pMajor initialmajor();
void resetmajor(pMajor major);
void clearmajor(pMajor major);
#define Major_Initial initialmajor
#define Major_Reset resetmajor
#define Major_Clear clearmajor
//int Major_AlterTRZProj(pMajor major,PAGECODE page,pTRZ list,int listcount);
int Major_AddTRZProj(pMajor major,char* proj);
int Major_AddTRZProj(pMajor major,char* proj,int writebackindexfile);
int Major_SetCurrProj(pMajor major,char* proj);
int Major_SetCurrProj(pMajor major,int idx);
int Major_SetCurrProj(pMajor major,int idx,int copyfromfile);
int Major_SaveProjs(pMajor major);
/*
 * 将指定的proj 加载至当前运行环境
 */
int Major_LoadProj(pMajor major,char* proj);

int TextRectOCR(tesseract::TessBaseAPI* api,int x,int y,int width,int height,char** pText);
int ClipTRZ(Pix* pix,pTRZ trz);
int ClipTRZ(Pix* pix,pTRZ trz,char* clip_file);
int TextTRZ(tesseract::TessBaseAPI* api,Pix* pix,pTRZ trz,int clip_flag);
int TextTRZ(tesseract::TessBaseAPI* api,Pix* pix,pTRZ trz);
int InitialSections(tesseract::TessBaseAPI* api,Pix* pix,pTRZ trz);
void ReleaseSections(pTRZ trz);

PAGECODE GetPageCode(tesseract::TessBaseAPI* api,const char* source_img,pTRZ list);
PAGECODE GetPageCode(const char* source_img,pTRZ list);

int searchtemplate(const char* source_img,TRZCODE code,pTRZ list);
int searchtemplate(const char* source_img,TRZCODE code,pTRZ list,int update_trz);
pTRZ gettrzbycode(pTRZ list,TRZCODE code);
pTRZ gettrzbycode(pTRZ list,TRZCODE code,int checktrz);

int getehimagename(const char* img_name,char* eh_name);

int ImgEhanceBeforeOCR(const char* img_file,char* eh_img_file);
int TextTRZEh(tesseract::TessBaseAPI* api,const char* img_file,pTRZ trz);

int streamer_room_ocr(tesseract::TessBaseAPI* api,pTRZ trz,char* img_file);
int fg_detail_ocr(tesseract::TessBaseAPI* api,pTRZ trz,char* img_file);
int fg_detail_ocr_prepare_dynamic(pTRZ list,char* img);

/*********************************************************/
// test code.
/*
 * PageAnalysis_Test:测试页面识别
 */    
void PageAnalysis_Test();
//...

/*********************************************************/

int NextPageStrategies_TRACEONE(pTRZ trz,PAGECODE currpage);
PAGECODE PageAnalysis(tesseract::TessBaseAPI* api,pTRZ trz);
void Flows();

int GetModulePath(char* path);

DWORD WINAPI DouYinOCRJOB(LPVOID lParam);
typedef NTSTATUS (NTAPI *NtQueryInformationThreadFunc)(
    HANDLE ThreadHandle,
    THREADINFOCLASS ThreadInformationClass,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength,
    PULONG ReturnLength
);
// 定义 THREAD_BASIC_INFORMATION 结构体
typedef struct _THREAD_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PVOID TebBaseAddress;
    CLIENT_ID ClientId;
    KAFFINITY AffinityMask;
    KPRIORITY Priority;
    KPRIORITY BasePriority;
    ULONG_PTR UniqueThread;
    ULONG_PTR SuspendCount;
} THREAD_BASIC_INFORMATION;

// 定义 THREADINFOCLASS 枚举类型
// typedef enum /*_THREADINFOCLASS*/ {
//     ThreadBasicInformation = 0,
//     ThreadTimes = 1,
//     ThreadPriority = 2,
//     ThreadBasePriority = 3,
//     ThreadAffinityMask = 4,
//     ThreadImpersonationToken = 5,
//     ThreadDescriptorTableEntry = 6,
//     ThreadEnableAlignmentFaultFixup = 7,
//     ThreadEventPair_Reusable = 8,
//     ThreadQuerySetWin32StartAddress = 9,
//     ThreadZeroTlsCell = 10,
//     ThreadPerformanceCount = 11,
//     ThreadAmILastThread = 12,
//     ThreadIdealProcessor = 13,
//     ThreadPriorityBoost = 14,
//     ThreadSetTlsArrayAddress = 15,
//     ThreadIsIoPending = 16,
//     ThreadHideFromDebugger = 17
// } THREADINFOCLASS;
BOOL IsThreadSuspended(HANDLE threadHandle);


/*************************************************/
/*
 * opencv 基本形状的检测
 */
int opencv_recognize_rectangles(char* filename,void* prc_list,int* prc_count);
int opencv_recognize_circles(char* filename,int range1,int range2,void* pcl_list,int* pcl_count);
int opencv_recognize_circles(char* filename,void* pcl_list,int* pcl_count);

//裁剪图片
int opencv_savetemplateclips(char* filename,int x,int y,int cx,int cy,char* template_file);
int opencv_savetemplateclips(char* filename,int x,int y,int cx,int cy,PAGECODE page);

#endif
