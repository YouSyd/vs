#include "tesseract_test.h"
#include "workflow_proj.h"
#include "workflow_ctrl.h"
#include "img_scratch.h"

char datapath[]="E:\\SRC_REF\\tesseract\\self-bulid\\test\\tessdata";

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
char section_name[100][256]={
#define enum_names(enum_code,enum_desc,enum_name) \
enum_desc "(" enum_name ")" ,
//TOSTRING(enum_code) "：" enum_desc "(" enum_name ")" ,
    ENUMNAMES_SECTION
#undef enum_names
};

char page_name[20][256]={
#define enum_names(enum_code,enum_desc,enum_name) \
enum_desc "(" enum_name ")" ,
//TOSTRING(enum_code) "：" enum_desc "(" enum_name ")" ,
    ENUMNAMES_PAGE
#undef enum_names
};

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
TRZ sections[]={
    //直播间主页面标志图（识别页面）
    {
        38,1432,695-38,1500-1432,STREAMER_ROOM_IMG,NULL,STREAMER_ROOM,
    },
    {
        97,72,136,33,STREAMER,NULL,STREAMER_ROOM,
        //97,72,60,33,STREAMER,NULL
    },
    {
        97,107,136,18,THUMBUP_OR_STREAMER_DESC,NULL,STREAMER_ROOM,
    },
    {
        573,83,50,30,ALIVECOUNT,NULL,STREAMER_ROOM,"eng","0123456789",
    },
    {
        20,210,300,70,FORTUNEGIFT_OR_DIAMOND_RANGE,NULL,STREAMER_ROOM,
    },
    //以下均需要在运行中计算
    //---------------------------------------
    //福袋点击区域
    {
        0,0,0,0,FORTUNEGIFT_CLICKRECT,NULL,STREAMER_ROOM,
    },
    //斗钻点击区域
    {
        0,0,0,0,DIAMOND_CLICKRECT,NULL,STREAMER_ROOM,
    },
    //斗钻价值描述区域 这个不一定有，可能程序中会被取消
    {
        0,0,0,0,DOU_DIAMOND_DESC,NULL,STREAMER_ROOM,
    },
    //斗钻倒计时
    {
        18,47,45,18,DOU_DIAMOND_COUNTDOWN,NULL,STREAMER_ROOM,"eng","0123456789:",
    },
    //福袋倒计时
    {
        18,47,45,18,DOU_FORTUNEGIFT_COUNTDOWN,NULL,STREAMER_ROOM,"eng","0123456789:",
    },
    //-------------------------------------------
    //福袋详情页面-详情页标志页图（识别页面）
    {
        230,700,510-230,120,DOU_FORTUNEGIFT_DETAIL_IMG,NULL,FORTUNEGIFT_DETAIL,
    }, 
    //福袋详情页面-倒计时
    {
        277,785,310-277,815-785,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL,NULL,FORTUNEGIFT_DETAIL,"eng","0123456789",
    }, 
    //福袋详情页面-倒计时2
    {
        333,785,366-333,815-785,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL2,NULL,FORTUNEGIFT_DETAIL,"eng","0123456789",
    },
    //福袋详情页面-福袋描述
    {
        260,865,670-260,1020-865,DOU_FORTUNEGIFT_DETAIL_TEXT,NULL,FORTUNEGIFT_DETAIL,
    },
    //福袋详情页面-任务描述 1
    {
        50,1205,670-50,1245-1205,DOU_FORTUNEGIFT_DETAIL_TASK1,NULL,FORTUNEGIFT_DETAIL,
    },
    //福袋详情页面-任务描述 2
    {
        50,1250,670-50,1280-1250,DOU_FORTUNEGIFT_DETAIL_TASK2,NULL,FORTUNEGIFT_DETAIL,
    },
    //福袋详情页面- 参与任务
    {
        50,1370,610,70,DOU_FORTUNEGIFT_DETAIL_EXECTASK,NULL,FORTUNEGIFT_DETAIL,
    },
    
    //----------------------------------------------------
    //斗钻详情页面
    { 
        30,800,660,100,DOU_DIAMOND_DETAIL_IMG,NULL,DIAMOND_DETAIL,
    },
    
        
    //----------------------------------------------------
    //福袋抽奖结果界面-未中奖
    {
        115,430,500,80,DOU_FORTUNEGIFT_DRAW_NOPRIZE_IMG,NULL,FORTUNEGIFT_NOPRIZE,
    },
    {
        135,900,440,60,DOU_FORTUNEGIFT_DRAW_NOPRIZE_EXIT,NULL,FORTUNEGIFT_NOPRIZE,
    },
    
    
    //----------------------------------------------------
    //福袋抽奖结果界面-中奖 
    {
        115,430,500,80,DOU_FORTUNEGIFT_DRAW_JACKPOT_IMG,NULL,FORTUNEGIFT_JACKPOT,
    },
    {
        110,497,505,103,DOU_FORTUNEGIFT_DRAW_JACKPOT_PRIZEDESC,NULL,FORTUNEGIFT_JACKPOT,
    },
    {
        115,875,500,80,DOU_FORTUNEGIFT_DRAW_JACKPOT_CFMAWARD,NULL,FORTUNEGIFT_JACKPOT,
    },
    {
        140,980,46,42,DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO,NULL,FORTUNEGIFT_JACKPOT,
    },
    
    
    //----------------------------------------------------
    //直播间 已经关闭
    {
        254,120,213,49,STREAMER_ROOMCLOSED_IMG,NULL,STREAMER_ROOMCLOSED,
    },
};

pMajor GMajor;
    
/*
 * streamer_room_ocr: 
 * 用途：识别直播间画面要素
 */
int streamer_room_ocr(tesseract::TessBaseAPI* api,pTRZ trz,char* img_file) {
//int test(char* img_file) {
    char* outputText=NULL;
    
    Pix* img=pixRead(img_file);
    InitialSections(api,img,trz);

    char fortunegift_clip_img[256]={0};
    char eh_img_name[256]={0};
    sprintf(fortunegift_clip_img,"./pics/%d.png",(int)FORTUNEGIFT_OR_DIAMOND_RANGE);
    ImgEnhanceBeforeOCR(fortunegift_clip_img,eh_img_name);
    
    int has_diamond=(searchtemplate(fortunegift_clip_img,DIAMOND_CLICKRECT,trz)==0?1:0);
    int has_fortunegift=(searchtemplate(fortunegift_clip_img,FORTUNEGIFT_CLICKRECT,trz)==0?1:0);
    GMajor->hasfg=has_fortunegift;
    
    //计算DOU_FORTUNEGIFT_COUNTDOWN
    if(has_fortunegift==1) {
        //fortunegift_countdown
        pTRZ trz_fc=gettrzbycode(trz,DOU_FORTUNEGIFT_COUNTDOWN);
        if(trz_fc==NULL) {
            printf("Something was wrong (DOU_FORTUNEGIFT_COUNTDOWN)...\n");
        }
        
        //FORTUNEGIFT_OR_DIAMOND_RANGE
        pTRZ trz_fodr=gettrzbycode(trz,FORTUNEGIFT_OR_DIAMOND_RANGE);
        if(trz_fodr==NULL) {
            printf("Something was wrong (FORTUNEGIFT_OR_DIAMOND_RANGE)...\n");
        }
        
        //裁剪出 DOU_FORTUNEGIFT_COUNTDOWN 
        trz_fc->x=trz_fodr->x+trz_fc->x;
        trz_fc->y=trz_fodr->y+trz_fc->y;
        TextTRZ(api,img,trz_fc);
    }
    
    if(has_diamond==1) {
        //DOU_DIAMOND_COUNTDOWN
        pTRZ trz_dc=gettrzbycode(trz,DOU_DIAMOND_COUNTDOWN);
        if(trz_dc==NULL) {
            printf("Something was wrong (DOU_DIAMOND_COUNTDOWN)...\n");
        }
        
        //FORTUNEGIFT_OR_DIAMOND_RANGE
        pTRZ trz_fodr=gettrzbycode(trz,FORTUNEGIFT_OR_DIAMOND_RANGE);
        if(trz_fodr==NULL) {
            printf("Something was wrong (FORTUNEGIFT_OR_DIAMOND_RANGE)...\n");
        }
        
        //裁剪出 DOU_DIAMOND_COUNTDOWN 
        trz_dc->x=trz_fodr->x+trz_dc->x;
        trz_dc->y=trz_fodr->y+trz_dc->y;
        TextTRZ(api,img,trz_dc);
    }
    
    pTRZ trz_streamer=gettrzbycode(trz,STREAMER);
    pTRZ trz_ac=gettrzbycode(trz,ALIVECOUNT);
    pTRZ trz_fc=gettrzbycode(trz,DOU_FORTUNEGIFT_COUNTDOWN);
    int alivecount=0;
    if(1!=sscanf(trz_ac->pText,"%d",&alivecount)) {
        printf("直播间人数转换出现问题[%s]\n",trz_ac->pText);
    }
    GMajor->alivecount=alivecount;
    printf("【%s】的直播间，目前有 %s 人，该直播间福袋:[%s],抖钻[%s]",trz_streamer->pText,trz_ac->pText,
        (has_fortunegift==1)?"√":"×",
        (has_diamond==1)?"√":"×");
    
    if(has_fortunegift==1) printf("，福袋开启剩余时间【%s】\n",trz_fc->pText);

    pixDestroy(&img);
    
    return 0;
}

/*
 * fg_detail_ocr
 * 用途：识别超级福袋详情页面要素 (一步任务)
 */
int fg_detail_ocr(tesseract::TessBaseAPI* api,pTRZ trz,char* img_file) {
    int result=-1;
    Pix* img=pixRead(img_file);
    
    //1.识别倒计时
    pTRZ trz_cd1=gettrzbycode(trz,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL);
    pTRZ trz_cd2=gettrzbycode(trz,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL2);
    if(!trz_cd1||!trz_cd2) {
        printf("福袋详情-识别福袋详情页面出错：未能取到倒计时\n");
        goto fg_detail_ocr_exit;
    }
    trz_cd1->y+=48;trz_cd2->y+=48;
    TextTRZ(api,img,trz_cd1);
    TextTRZ(api,img,trz_cd2);
    printf("倒计时[%s:%s]\n",trz_cd1->pText,trz_cd2->pText);
    
    int minutes=0,seconds=0;
    if(1!=sscanf(trz_cd1->pText,"%d",&minutes)) {
        printf("福袋详情-倒计时分钟%s 转换出错\n",trz_cd1->pText);
        goto fg_detail_ocr_exit;
    }
    if(1!=sscanf(trz_cd2->pText,"%d",&seconds)) {
        printf("福袋详情-倒计时秒数%s 转换出错\n",trz_cd2->pText);
        goto fg_detail_ocr_exit;
    }
    
    GMajor->timestamp=time(NULL);
    GMajor->fg_countdown=minutes*60+seconds;
    //识别福袋内容 DOU_FORTUNEGIFT_DETAIL_TEXT
    pTRZ trz_fdt=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_TEXT);
    if(!trz_fdt) {
        printf("福袋详情-识别福袋详情页面出错：未能取到福袋内容文本\n");
        goto fg_detail_ocr_exit;
    }
    trz_fdt->y+=48;
    TextTRZ(api,img,trz_fdt);
    printf("福袋内容[%s]\n",trz_fdt->pText);
    GMajor->fg_desc=trz_fdt->pText;
    
    //两步任务？一步任务？
    //... 没想好咋做
    //初步思路如下：
    //扫描图片，检测任务类型，一步？两步？执行了几步？当前状态？
    //收集完信息后，外面做决策
    /* 
     * 经过图像对比发现，一个任务和两个任务的图略有区别，(超级福袋)
     * 的起始位置不同
     * 先找到 其位置，以此为基准，判断是一步任务、两步任务
     * 在本机 720 × 1600 ， 两部任务比一步任务 y 小 48
     *
     */
    
    
    pTRZ trz_tk1=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_TASK1);
    if(!trz_tk1) {
        printf("福袋详情-识别福袋详情页面出错：未能取到任务1区域\n");
        goto fg_detail_ocr_exit;
    }
    trz_tk1->y+=48;
    //TextTRZ(api,img,trz_tk1);
    TextTRZEh(api,img_file,trz_tk1);//使用处理后的图片提高识别准确率
    if(trz_tk1->pText) printf("任务1[%s]\n",trz_tk1->pText);
    GMajor->taskstatus[0]=((strstr(trz_tk1->pText,"已达成"))?1:0);    
    
    //检查执行任务文本
    pTRZ trz_et=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_EXECTASK);
    if(!trz_et) {
        printf("福袋详情-识别福袋详情页面出错：未能取到执行任务按钮的文本\n");
        goto fg_detail_ocr_exit;
    }
    TextTRZ(api,img,trz_et);
    printf("执行任务按钮的文本[%s]\n",trz_et->pText);
    
    result=0;
fg_detail_ocr_exit:    
    pixDestroy(&img);
    return result;
}

/*
 * fg_detail_2task_ocr
 * 用途：识别超级福袋详情页面要素(两步任务)
 */
int fg_detail_2task_ocr(tesseract::TessBaseAPI* api,pTRZ trz,char* img_file) {
    int result=-1;
    Pix* img=pixRead(img_file);
    
    //1.识别倒计时
    pTRZ trz_cd1=gettrzbycode(trz,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL);
    pTRZ trz_cd2=gettrzbycode(trz,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL2);
    if(!trz_cd1||!trz_cd2) {
        printf("福袋详情-识别福袋详情页面出错：未能取到倒计时\n");
        goto fg_detail_2task_ocr_exit;
    }
    TextTRZ(api,img,trz_cd1);
    TextTRZ(api,img,trz_cd2);
    printf("倒计时[%s:%s]\n",trz_cd1->pText,trz_cd2->pText);
    
    int minutes=0,seconds=0;
    if(1!=sscanf(trz_cd1->pText,"%d",&minutes)) {
        printf("福袋详情-倒计时分钟%s 转换出错\n",trz_cd1->pText);
        goto fg_detail_2task_ocr_exit;
    }
    if(1!=sscanf(trz_cd2->pText,"%d",&seconds)) {
        printf("福袋详情-倒计时秒数%s 转换出错\n",trz_cd2->pText);
        goto fg_detail_2task_ocr_exit;
    }
    GMajor->timestamp=time(NULL);
    GMajor->fg_countdown=minutes*60+seconds;
    
    //识别福袋内容 DOU_FORTUNEGIFT_DETAIL_TEXT
    pTRZ trz_fdt=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_TEXT);
    if(!trz_fdt) {
        printf("福袋详情-识别福袋详情页面出错：未能取到福袋内容文本\n");
        goto fg_detail_2task_ocr_exit;
    }
    TextTRZ(api,img,trz_fdt);
    GMajor->fg_desc=trz_fdt->pText;
    printf("福袋内容[%s]\n",trz_fdt->pText);
    
    //两步任务？一步任务？
    //... 没想好咋做
    //初步思路如下：
    //扫描图片，检测任务类型，一步？两步？执行了几步？当前状态？
    //收集完信息后，外面做决策
    /* 
     * 经过图像对比发现，一个任务和两个任务的图略有区别，(超级福袋)
     * 的起始位置不同
     * 先找到 其位置，以此为基准，判断是一步任务、两步任务
     * 在本机 720 × 1600 ， 两部任务比一步任务 y 小 48
     *
     */
    
    
    pTRZ trz_tk1=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_TASK1);
    if(!trz_tk1) {
        printf("福袋详情-识别福袋详情页面出错：未能取到任务1区域\n");
        goto fg_detail_2task_ocr_exit;
    }
    pTRZ trz_tk2=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_TASK2);
    if(!trz_tk2) {
        printf("福袋详情-识别福袋详情页面出错：未能取到任务2区域\n");
        goto fg_detail_2task_ocr_exit;
    }
    //TextTRZ(api,img,trz_tk1);
    TextTRZEh(api,img_file,trz_tk1);//使用处理后的图片提高识别准确率
    if(trz_tk1->pText) printf("任务1[%s]\n",trz_tk1->pText);
    //TextTRZ(api,img,trz_tk2);
    TextTRZEh(api,img_file,trz_tk2);//使用处理后的图片提高识别准确率
    if(trz_tk2->pText) printf("任务2[%s]\n",trz_tk2->pText);
    GMajor->taskstatus[0]=((strstr(trz_tk1->pText,"已达成"))?1:0);    
    GMajor->taskstatus[1]=((strstr(trz_tk2->pText,"已达成"))?1:0);
    
    //检查执行任务文本
    pTRZ trz_et=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_EXECTASK);
    if(!trz_et) {
        printf("福袋详情-识别福袋详情页面出错：未能取到执行任务按钮的文本\n");
        goto fg_detail_2task_ocr_exit;
    }
    TextTRZ(api,img,trz_et);
    printf("执行任务按钮的文本[%s]\n",trz_et->pText);
    
    result=0;
fg_detail_2task_ocr_exit:    
    pixDestroy(&img);
    return result;
}

/*
 * 福袋开奖结果-中奖页面要素识别
 */
int fg_draw_jackpot_ocr(tesseract::TessBaseAPI* api,pTRZ trz,char* img_file) {
    printf("中奖页面.\n");
    
    //将中奖图片存入文件夹 ./pics/award/
    time_t now=time(NULL);
    struct tm* local=localtime(&now);
    char award_name[256]="";
    sprintf(award_name,"./pics/award/%04d_%02d_%02d_%02d_%02d_%02d.png",
        local->tm_year+1900,local->tm_mon+1,local->tm_mday,
        local->tm_hour,local->tm_min,local->tm_sec
    );
    CopyFile(img_file,award_name,TRUE);
    
    //...
    
    return 0;
}

/*
 * 福袋开奖结果 -- 没有中奖 页面要素识别
 */
int fg_draw_noprize_ocr(tesseract::TessBaseAPI* api,pTRZ trz,char* img_file) {
    printf("未中奖页面.\n");
    return 0;
}

int ImgEnhanceBeforeOCR(const char* img_file,char* eh_img_file) {
    cv::Mat image=cv::imread(img_file);
    if(image.empty()) {
        printf("无法打开图像[%s]。\n",img_file);
        return -1;
    }
    
    //灰度处理
    cv::Mat gray;
    cv::cvtColor(image,gray,cv::COLOR_BGR2GRAY);
    
    //自适应阈值
    cv::Mat adaptiveThresh;
    cv::adaptiveThreshold(gray,adaptiveThresh,255,cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 15, -2);
    
    //cv::Mat hline=getStructuringElement(cv::MORPH_RECT,cv::Size(image.cols/16,1),cv::Point(-1,-1));
    //cv::Mat vline=getStructuringElement(cv::MORPH_RECT,cv::Size(1,image.rows/16),cv::Point(-1,-1));
    
    cv::Mat kernel=getStructuringElement(cv::MORPH_RECT,cv::Size(1,1),cv::Point(-1,-1));
    
    //膨胀与腐蚀
    cv::Mat dilated,eroded;
    //cv::Mat kernel=cv::getStructuringElement(cv::MORPH_RECT,cv::Size(2,2));
    cv::erode(adaptiveThresh,eroded,kernel);
    cv::dilate(eroded,dilated,kernel);
    
    // 反色处理
    cv::Mat inverted;
    cv::bitwise_not(dilated,inverted);
        
    //边缘处理
    //cv::blur(inverted,inverted,cv::Size(2,2),cv::Point(-1,-1));
        
    //去噪
    cv::Mat denoised;
    cv::fastNlMeansDenoising(inverted,denoised,50,7,21);
    
    if(eh_img_file[0]=='\0') getehimagename(img_file,eh_img_file);
    cv::imwrite(eh_img_file,denoised);
    
    return 0;
}

int ClipTRZ(Pix* pix,pTRZ trz) {
    if(!pix||!trz) {
        printf("Clip TRZ 失败：图像或选区数据无效。\n");
        return -1;
    }
    
    //裁剪图片
    BOX* clipBox=boxCreate(trz->x,trz->y,trz->width,trz->height);
    if(clipBox==NULL) {
        printf("创建裁剪区失败,TRZ CODE:%d.\n",trz->code);
    }
    PIX* clipImg=pixClipRectangle(pix,clipBox,NULL);
    if(clipImg==NULL) {
        printf("图片裁剪失败\n");
    } else {
        //写入文件
        char clip_file[256]={0};
        sprintf(clip_file,"./pics/%d.png",(int)trz->code);
        if(0!=pixWrite(clip_file,clipImg,IFF_PNG)) {
            printf("裁剪图片写入文件%s 失败\n",clip_file);
        } else {
            //printf("裁剪图片写入文件%s\n",clip_file);
        }
    }
    
    //释放资源
    pixDestroy(&clipImg);
    boxDestroy(&clipBox);
    
    return 0;
}

int ClipTRZ(Pix* pix,pTRZ trz,char* clip_file) {
    if(!pix||!trz) {
        printf("Clip TRZ 失败：图像或选区数据无效。\n");
        return -1;
    }
    
    //裁剪图片
    BOX* clipBox=boxCreate(trz->x,trz->y,trz->width,trz->height);
    if(clipBox==NULL) {
        printf("创建裁剪区失败,TRZ CODE:%d.\n",trz->code);
    }
    PIX* clipImg=pixClipRectangle(pix,clipBox,NULL);
    if(clipImg==NULL) {
        printf("图片裁剪失败\n");
    } else {
        //写入文件
        if(clip_file[0]=='\0') sprintf(clip_file,"./pics/%d.png",(int)trz->code);
        if(0!=pixWrite(clip_file,clipImg,IFF_PNG)) {
            printf("裁剪图片写入文件%s 失败\n",clip_file);
        } else {
            //printf("裁剪图片写入文件%s\n",clip_file);
        }
    }
    
    //释放资源
    pixDestroy(&clipImg);
    boxDestroy(&clipBox);
    
    return 0;
}

/*
 * TextTRZ的增强版，需要对图像进行预处理
 * 该版本效率极低，一个图片需要3读2写，待优化，后期考虑是否可以使用内存而非文件过度
 */
int TextTRZEh(tesseract::TessBaseAPI* api,const char* img_file,pTRZ trz) {
    char clip_file[256]={0};
    char eh_img_file[256]={0};
    
    //1.对图像指定区域裁切保存
    Pix* pixSource=pixRead(img_file);
    ClipTRZ(pixSource,trz,clip_file);
    pixDestroy(&pixSource);
    
    //2.对裁切图像进行识别前的处理
    // 对 eh_img_file 赋值
    // 增强图为原图名加上 "_Eh"
    getehimagename(clip_file,eh_img_file);
    ImgEnhanceBeforeOCR(clip_file,eh_img_file);
    
    //3.识图
    Pix* pix=pixRead(eh_img_file);
    TextTRZ(api,pix,trz,0);//不需要裁剪
    pixDestroy(&pix);
    return 0;
}

/*
 * 取回的OCR文本附加处理
 */
int TextTRZExtra(pTRZ trz) {
    if(!trz||!trz->pText) return 0;
    
    char* plt=trz->pText;
    size_t len=strlen(plt);
    
    if(plt[len-1]=='\n') plt[len-1]='\0';
    
    //福袋详情内容去回车换行符
    if(trz->code==DOU_FORTUNEGIFT_DETAIL_TEXT&&trz->pText) {
        char* plt_buffer=(char*)calloc(sizeof(char)*(len+1),1);
        if(!plt_buffer) return -1;
        for(int idx=0,dst_idx=0;idx<len;idx++) {
            if(plt[idx]!='\r'&&plt[idx]!='\n') {
                plt_buffer[dst_idx]=plt[idx];
                dst_idx++;
            }
        }
        
        memcpy(trz->pText,plt_buffer,len);
        trz->pText[len]='\0';
        if(plt_buffer) free(plt_buffer);
    }
    return 0;
}

int TextTRZ(tesseract::TessBaseAPI* api,Pix* pix,pTRZ trz,int clip_flag) {
    if(clip_flag!=0) ClipTRZ(pix,trz);  
    
    //设置api的traindata  
    if(trz->traindata[0]=='\0') strcpy(trz->traindata,"chi_sim");
    if(api->Init(datapath,trz->traindata)) {
        printf("Initial Tesseract failed.\n");
        return -1;
    }
    
    //加载图片
    api->SetImage(pix);
    
    //设置tessedit_char_whitelist参数
    if(trz->whitelist[0]!='\0')  api->SetVariable("tessedit_char_whitelist",trz->whitelist);
    else api->SetVariable("tessedit_char_whitelist", "");
        
    int result;
    if(clip_flag!=0) result=TextRectOCR(api,trz->x,trz->y,trz->width,trz->height,&(trz->pText));
    else result=TextRectOCR(api,0,0,trz->width,trz->height,&(trz->pText));
    
    if(result==0) result=TextTRZExtra(trz); 
    
    //终止当前的识别绘画
    api->End();
    
    return result;
}

int TextTRZ(tesseract::TessBaseAPI* api,Pix* pix,pTRZ trz) {
    return TextTRZ(api,pix,trz,1);
}

int InitialSections(tesseract::TessBaseAPI* api,Pix* pix,pTRZ trz) {
    size_t trz_size=_msize(trz)/sizeof(TRZ);
    
    for(int idx=0;idx<trz_size;idx++) {
        if((trz+idx)->code<=FORTUNEGIFT_OR_DIAMOND_RANGE&&
           (trz+idx)->code>=STREAMER) {
            if((trz+idx)->code==ALIVECOUNT) {
                api->SetVariable("tessedit_char_whitelist", "0123456789");
            } else if((trz+idx)->code==DOU_DIAMOND_COUNTDOWN||
                (trz+idx)->code==DOU_DIAMOND_COUNTDOWN) {
                api->SetVariable("tessedit_char_whitelist", "0123456789:");
            }
            if((trz+idx)->code==STREAMER) {
                char eh_name[256]="";
                sprintf(eh_name,"./pics/%d.png",(int)STREAMER);
                TRZ rtmp;
                memcpy(&rtmp,trz+idx,sizeof(rtmp));
                rtmp.x=rtmp.y=0;
                TextTRZEh(api,eh_name,&rtmp);
                (trz+idx)->pText=rtmp.pText; 
            } else TextTRZ(api,pix,trz+idx);
        }
    }
    
    return 0;
}

void ReleaseSections(pTRZ trz) {
    if(!trz) return;
    size_t trz_size=_msize(trz)/sizeof(TRZ);
    
    for(int idx=0;idx<trz_size;idx++) {
        if((trz+idx)->pText) delete[] (trz+idx)->pText;
        (trz+idx)->pText=NULL;
    }
    
    delete[] trz;
}

int TextRectOCR(tesseract::TessBaseAPI* api,int x,int y,int width,int height,char** pText) {
    api->SetRectangle(x,y,width,height);
    char* outputTextUTF8=NULL;
    
    outputTextUTF8=api->GetUTF8Text();
    if(outputTextUTF8==NULL) return -1;
    
    //utf8 转 ACII
    wchar_t* text_wchar=NULL;
    int wchar_length=MultiByteToWideChar(CP_UTF8,0,outputTextUTF8,-1,NULL,0);
    if(wchar_length<=0) {
        delete[] outputTextUTF8;
        outputTextUTF8=NULL;
        return -1;
    }
    
    text_wchar=new wchar_t[wchar_length+1];
    MultiByteToWideChar(CP_UTF8,0,outputTextUTF8,-1,text_wchar,wchar_length);
    
    char* outputText=NULL;
    int gbk_length=WideCharToMultiByte(CP_ACP,0,text_wchar,-1,NULL,0,NULL,NULL);
    if(gbk_length<=0) {
        delete[] outputTextUTF8;
        outputTextUTF8=NULL;
        if(text_wchar) delete[] text_wchar;
        text_wchar=NULL;
        return -1;
    }
    
    outputText=new char[gbk_length+1]; 
    WideCharToMultiByte(CP_ACP,0,text_wchar,-1,outputText,gbk_length,NULL,NULL);
    
    if(outputTextUTF8) delete[] outputTextUTF8;
    outputTextUTF8=NULL;
    if(text_wchar) delete[] text_wchar;
    text_wchar=NULL;
    
    //gbk_length=strlen(outputText);
    //if(outputText[gbk_length-1]=='\n') outputText[gbk_length-1]='\0';
    
    *pText=outputText;
    
    return 0;
}

int getehimagename(const char* img_name,char* eh_name) {
#define EHANCE_IMG_EXT  "_Eh"
    for(int idx=strlen(img_name)-1;idx>=0;idx--) 
        if(img_name[idx]=='.') {
            strncpy(eh_name,img_name,idx);
            strcat(eh_name,EHANCE_IMG_EXT);
            strcat(eh_name,&img_name[idx]);
            //printf("source name:%s, ehance name:%s\n",img_name,eh_name);
            return 0;
        }
    return -1;
}

int searchtemplate(const char* source_img,TRZCODE code,pTRZ list) {
    char template_imgs[][256]={
        "./pics/diamond_template.png",
        "./pics/gift_template.png"
    };
    char* template_img=NULL;
    
    switch((int)code) {
    case DIAMOND_CLICKRECT: 
        template_img=&template_imgs[0][0];
        break;
    case FORTUNEGIFT_CLICKRECT:
        template_img=&template_imgs[1][0];
        break;
    }
    
    //printf("source img:%s,template img:%s\n",source_img,template_img);
    
    cv::Mat source=cv::imread(source_img);
    cv::Mat templt=cv::imread(template_img);
        
    if(source.empty()||templt.empty()) {
        printf("加载图片或者模板图片失败\n");
        return -1;
    }
    
    // 转换为灰度图像
    cv::Mat sourceGray,templateGray;
    cv::cvtColor(source,sourceGray,cv::COLOR_BGR2GRAY);
    cv::cvtColor(templt,templateGray,cv::COLOR_BGR2GRAY);

    cv::Mat result;
    int result_cols=sourceGray.cols-templateGray.cols+1;
    int result_rows=sourceGray.rows-templateGray.rows+1;
    result.create(result_rows,result_cols,CV_32FC1);
    
    //模板匹配
    //TM_SQDIFF_NORMED 使用归一化方差，越小越好
    //TM_CCOEFF_NORMED 相关度，越大越好
    cv::matchTemplate(sourceGray,templateGray,result,cv::TM_CCOEFF_NORMED);
    
    //归一化
    //cv::normalize(result,result,0,1,cv::NORM_MINMAX,-1,cv::Mat());
        
    //找到最佳匹配位置
    double minVal,maxVal;
    cv::Point minLoc,maxLoc,matchLoc;
    cv::minMaxLoc(result,&minVal,&maxVal,&minLoc,&maxLoc);
    printf("匹配最佳位置的方差%lf\n",maxVal);
    
    matchLoc=maxLoc;
    double threshold=0.9;
    if(maxVal>threshold) {
        int x,y,width,height;//注意x,y 为右下角坐标
        x=matchLoc.x;
        y=matchLoc.y;
        width=templt.cols;
        height=templt.rows;
        
        pTRZ trz=gettrzbycode(list,code);
        if(trz) { 
            trz->x=x;trz->y=y;trz->width=width;trz->height=height;
        }
        //printf("找到匹配位置:(%d,%d) %d×%d\n",x,y,width,height);
        
        //在图像上绘制矩形以显示匹配位置
        cv::rectangle(source,matchLoc,cv::Point(matchLoc.x+templt.cols,
            matchLoc.y+templt.rows),cv::Scalar::all(250),2,8,0);
        
        cv::imwrite("./pics/searched.png",source);
        
        return 0;
    } else {
        printf("未找到相似度匹配的图像。\n");
        return -1;
    }
}

/*
 * --------------------------------------------------
 * wasted.
 * --------------------------------------------------
 */
int searchtemplate2(const char* source_img,const char* template_img) {
    
    //printf("source img:%s,template img:%s\n",source_img,template_img);
    
    cv::Mat source=cv::imread(source_img);
    cv::Mat templt=cv::imread(template_img,cv::IMREAD_UNCHANGED);
        
    if(source.empty()||templt.empty()) {
        printf("加载图片或者模板图片失败\n");
        return -1;
    }
    
    // 转换为灰度图像
    //cv::Mat sourceGray,templateGray;
    //cv::cvtColor(source,sourceGray,cv::COLOR_BGR2GRAY);
    //cv::cvtColor(templt,templateGray,cv::COLOR_BGR2GRAY);
    
    //template 提取非透明部分
    cv::Mat sourceNonTrans;
    cv::extractChannel(source,sourceNonTrans,3);//取alpha通道
        
    cv::Mat templtNonTrans;
    cv::extractChannel(templt,templtNonTrans,3);
    
    cv::Mat result;
    cv::matchTemplate(sourceNonTrans,templtNonTrans,result,cv::TM_CCORR_NORMED);
    
    //归一化
    //cv::normalize(result,result,0,1,cv::NORM_MINMAX,-1,cv::Mat());
        
    //找到最佳匹配位置
    double minVal,maxVal;
    cv::Point minLoc,maxLoc,matchLoc;
    cv::minMaxLoc(result,&minVal,&maxVal,&minLoc,&maxLoc);
    printf("匹配最佳位置的方差范围:%lf~%lf\n",minVal,maxVal);
    
    matchLoc=maxLoc;
    double threshold=0.9;
    if(maxVal>threshold) {
        int x,y,width,height;
        x=matchLoc.x+templt.cols;
        y=matchLoc.y+templt.rows;
        width=templt.cols;
        height=templt.rows;
        printf("找到匹配位置:(%d,%d) %d×%d\n",x,y,width,height);
        
        //在图像上绘制矩形以显示匹配位置
        cv::rectangle(source,matchLoc,cv::Point(matchLoc.x+templt.cols,
            matchLoc.y+templt.rows),cv::Scalar::all(250),2,8,0);
        
        cv::imwrite("./pics/searched.png",source);
        
        return 0;
    } else {
        printf("未找到相似度匹配的图像。\n");
        return -1;
    }
}
/* -------------------------------------------------- */


pTRZ gettrzbycode(pTRZ list,TRZCODE code) {
    if(list==NULL) return NULL;
     
    int length=_msize(list)/sizeof(TRZ);
    for(int idx=0;idx<length;idx++) {
        if((list+idx)->code==code) {
            pTRZ section=list+idx;
            if(!section||section->height<=0||section->width<=0) {
                /*
                 * 发送消息
                 */
                if(f&&f->extra) {
                    pwfpj_param wfpj=(pwfpj_param)f->extra;
                    SendMessage(wfpj->imgsratch,MSG_SECTIONSETTINGS,(WPARAM)section,(LPARAM)code);
                }
            }

            return list+idx;
        }
    }
    return NULL;
}

PAGECODE GetPageCode(const char* source_img,pTRZ list) {
    return GetPageCode(NULL,source_img,list);
}
/*
 * 识别当前页面代码
 */
PAGECODE GetPageCode(tesseract::TessBaseAPI* api,const char* source_img,pTRZ list) {
    
    //页面标志图
    //请勿随意调整以下模板文件、页面代码、搜索区域的顺序，
    //顺序经过设计，即便同步调整也可能会导致错误。
    char template_imgs[][256]={
        "./pics/fortunegift_draw_jackpot_template.png",//中奖模板图
        "./pics/fortunegift_draw_noprize_template.png",//未中奖模板图
        "./pics/streamer_room_template.png",
        "./pics/fortunegift_detail_template.png",
        "./pics/diamond_detail_template.png",
    };
    //页面代码
    PAGECODE pages[]={
        FORTUNEGIFT_JACKPOT,//福袋中奖界面
        FORTUNEGIFT_NOPRIZE,//福袋未中奖界面
        STREAMER_ROOM,
        FORTUNEGIFT_DETAIL,
        DIAMOND_DETAIL,
    };
    //主图待搜索区域
    TRZCODE zones[]={
        DOU_FORTUNEGIFT_DRAW_JACKPOT_IMG,//福袋中奖页面标志
        DOU_FORTUNEGIFT_DRAW_NOPRIZE_IMG,//福袋未中奖页面识别标志
        STREAMER_ROOM_IMG,
        DOU_FORTUNEGIFT_DETAIL_IMG,
        DOU_DIAMOND_DETAIL_IMG,
    };
        
    char* template_img=NULL;
    
    for(int idx=0;idx<sizeof(template_imgs)/sizeof(char[256]);idx++) {
        template_img=&template_imgs[idx][0];
                
        pTRZ trz=gettrzbycode(list,zones[idx]);
        if(trz==NULL) {
            printf("获取TRZ信息失败,trz code:%d\n",zones[idx]);
            continue;
        }
        
        cv::Mat img=cv::imread(source_img);
        cv::Mat source(img,cv::Rect(trz->x,trz->y,trz->width,trz->height));
        cv::Mat templt=cv::imread(template_img);
            
        if(source.empty()||templt.empty()) {
            printf("加载图片[%s]或者模板图片[%s]失败\n",source_img,template_img);
            return UNKNOWN_PAGE;
        }
        
        // 转换为灰度图像
        cv::Mat sourceGray,templateGray;
        cv::cvtColor(source,sourceGray,cv::COLOR_BGR2GRAY);
        cv::cvtColor(templt,templateGray,cv::COLOR_BGR2GRAY);
    
        cv::Mat result;
        int result_cols=sourceGray.cols-templateGray.cols+1;
        int result_rows=sourceGray.rows-templateGray.rows+1;
        result.create(result_rows,result_cols,CV_32FC1);
        
        //模板匹配
        //TM_SQDIFF_NORMED 使用归一化方差，越小越好
        //TM_CCOEFF_NORMED 相关度，越大越好
        cv::matchTemplate(sourceGray,templateGray,result,cv::TM_CCOEFF_NORMED);
            
        //找到最佳匹配位置
        double minVal,maxVal;
        cv::Point minLoc,maxLoc,matchLoc;
        cv::minMaxLoc(result,&minVal,&maxVal,&minLoc,&maxLoc);
        
        matchLoc=maxLoc;
        double threshold=0.9;
        if(maxVal>threshold) {
            PAGECODE code=pages[idx];
            if(FORTUNEGIFT_DETAIL==pages[idx]) {
                //trz->x=matchLoc.x;trz->y=matchLoc.y;
                int y_offset=trz->y+matchLoc.y;
                if(y_offset>=700&&y_offset<=740) {
                    //两部任务模式 718
                    printf("两步任务模式,template Y:%d\n",y_offset);
                    
                    code=FORTUNEGIFT_DETAIL_2TASK;
                } else {
                    //一步任务模式 766, offset = 766-718=48;
                    printf("一步任务模式,template Y:%d\n",y_offset);
                }
            }
            
            //printf("source img:%s,template img:%s,",source_img,template_img);
            //printf("匹配最佳位置的方差:%lf\n",maxVal);
            return code;
        }  
    }
    
    //如果找图没找到，开启文字识别
    // 直播间已经关闭 STREAMER_ROOMCLOSED_IMG
    pTRZ trz_rc=gettrzbycode(list,STREAMER_ROOMCLOSED_IMG);
    
    if(!api) {
        tesseract::TessBaseAPI* api_new=new tesseract::TessBaseAPI();
        TextTRZEh(api_new,source_img,trz_rc);
        if(api_new) delete api_new;
    } else {
        TextTRZEh(api,source_img,trz_rc);
    }
        
    if(strstr(trz_rc->pText,"直播已结束")) {
        return STREAMER_ROOMCLOSED;
    }
    
    return UNKNOWN_PAGE;
}

#ifdef TESSERACT_UNITTEST
int main() {
    
    //PageAnalysis_Test();
    Flows();
    
    printf("Exit Normally.\n");
    return 0;
}
#endif

DWORD WINAPI DouYinOCRJOB(LPVOID lParam) {
    /*
     * 启动流程
     */

    Flows();

    return 0;
}

void Flows() {
    GMajor=(pMajor)calloc(sizeof(Major),1);
    if(GMajor==NULL) return;
    
    
    //参数初始化
    int sections_size=sizeof(sections)/sizeof(TRZ);
    pTRZ trz=new TRZ[sections_size];
    memcpy(trz,sections,sizeof(sections));
    GMajor->trz=trz;
    tesseract::TessBaseAPI* api=new tesseract::TessBaseAPI();
    
    pwfpj_param wfpj=(pwfpj_param)f->extra;;

    //根据页面类型确定执行页面切换的策略
    int refresh_flag=1;
    while(refresh_flag) {
        //1. 捕捉图像
        int retry_count=0;
        while(retry_count<=5) {
            char cmd3[256]="adb exec-out screencap -p > ./pics/x.png";
            if(0!=subproc_execmd((char*)"cmd",(char*)cmd3)) {retry_count++;printf("Command Execution failed.\n");Sleep(1000);}
            else break;
                
            Sleep(15);
        }
        
        //2.分析图像界面，识别页面，并解析页面内容
        PAGECODE page=PageAnalysis(api,trz);
        if(wfpj->workflow) SendMessage(wfpj->workflow,MSG_NODELIGHTING,(WPARAM)page,(LPARAM)0);

        //3. 确定下一个页面
        NextPageStrategies_TRACEONE(trz,page);
    }
    

    ReleaseSections(trz);
    if(api) delete api; 
    if(GMajor) free(GMajor);
}

/*
 * 无论当前给定的是什么页面，识别出来是否相关，如果相关，从页面获取所需的所有信息。
 * 面对未知的界面，将反复调用该函数
 */
PAGECODE PageAnalysis(tesseract::TessBaseAPI* api,pTRZ trz) {
    char img_file[][256]={
        "./pics/x.png"
    };
    int idx=0;
    PAGECODE page=GetPageCode(api,img_file[idx],trz);
    
    switch(page) {
    case FORTUNEGIFT_JACKPOT: 
        fg_draw_jackpot_ocr(api,trz,img_file[idx]);
    break;
    case FORTUNEGIFT_NOPRIZE: 
        fg_draw_noprize_ocr(api,trz,img_file[idx]);
    break;
    case STREAMER_ROOM: 
        streamer_room_ocr(api,trz,img_file[idx]);
    break;
    case STREAMER_ROOMCLOSED: 
        {
            printf("直播已结束.\n");
        }
    break;
    case FORTUNEGIFT_DETAIL: 
        GMajor->taskcount=1;
        fg_detail_ocr(api,trz,img_file[idx]);
    break;
    case FORTUNEGIFT_DETAIL_2TASK: 
        GMajor->taskcount=2;
        fg_detail_2task_ocr(api,trz,img_file[idx]);
    break;
    case DIAMOND_DETAIL: 
        //diamond_detail_ocr(api,trz,img_file[idx]);
    break;
    default: 
        printf("Unknown PAGE,image file [%s]\n",img_file[idx]);
    }
    return page;
}

/*
 * 获取下一个页面的策略
 * 这里可以有很多种策略，比如在一个直播间持续蹲守，
 * 可以切换不同的直播间，寻找人少、或者价值更高的直播间
 * 根据时间、或者货品类型切换等
 * 这将成为一个扩展接口，根据各自的需要自行实现
 * NextPageStrategies_TRACEONE 实现的是对一个特定直播间的蹲守策略
 * 蹲守规则如下：
 * 如果该直播间尚在直播，持续检查福袋，直至中奖或者直播间关闭。
 */
int NextPageStrategies_TRACEONE(pTRZ trz,PAGECODE currpage) {
    /*
     * 获取当前直播间的page，计算出下一个页面
     * 如果下一个页面需要界面点击或者操作，执行
     * 如果下一个页面需要定时等待，等待
     * 如果下一个页面需要循环尝试，循环
     * 直至切换至下一个页面。
     */
    switch(currpage) {
    case STREAMER_ROOM: {
        //获取详情,是否有福袋？
        if(GMajor->hasfg) {
            pTRZ trz_fodr=gettrzbycode(trz,FORTUNEGIFT_OR_DIAMOND_RANGE);
            if(!trz_fodr) return -1;
            pTRZ trz_fc=gettrzbycode(trz,FORTUNEGIFT_CLICKRECT);
            if(!trz_fc) return -1;
            
            char cmd_tapfg[256]={0};
            sprintf(cmd_tapfg,"input tap %d %d",trz_fodr->x+trz_fc->x+(trz_fc->width>>1),trz_fodr->y+trz_fc->y+(trz_fc->height>>1));
            subproc_execmd(NULL,cmd_tapfg);
            Sleep(1040);
        } else {
            //等30秒
            Sleep(30*1000);
        }
    } break;
    case FORTUNEGIFT_DETAIL: {} break;
    case FORTUNEGIFT_DETAIL_2TASK: {
        //检查任务是否已经执行
        for(int idx=0;idx<2;idx++) {
            printf("任务%d:%s\n",idx+1,GMajor->taskstatus[idx]==0?"未完成":"已完成");
            if(GMajor->taskstatus[idx]==0) {
                //执行参与
                pTRZ trz_btn=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_EXECTASK);
                if(!trz_btn) return -1;
                
                char cmd_tapfg[256]={0};
                sprintf(cmd_tapfg,"input tap %d %d",trz_btn->x+(trz_btn->width>>1),trz_btn->y+(trz_btn->height>>1));
                subproc_execmd(NULL,cmd_tapfg);
                Sleep(1200);
                
                //这里实际上可能要break，并执行新的页面判断
            }
        }
        /*
         * 如果两个任务都是已经完成的，点击参与不会自动退出当前页面
         * 该界面的下一步界面需要切回 直播间
         */
         
        //截图，检查当前页面
        int retry_count=0;
        while(retry_count<=5) {
            char cmd3[256]="adb exec-out screencap -p > ./pics/x.png";
            if(0!=subproc_execmd((char*)"cmd",(char*)cmd3)) {retry_count++;printf("Command Execution failed.\n");Sleep(1000);}
            else break;
                
            Sleep(105);
        }
        PAGECODE page=GetPageCode("./pics/x.png",trz);
        if(page==FORTUNEGIFT_DETAIL_2TASK) {
            pTRZ trz_fodr=gettrzbycode(trz,FORTUNEGIFT_OR_DIAMOND_RANGE);
            if(!trz_fodr) return -1;
            pTRZ trz_fc=gettrzbycode(trz,FORTUNEGIFT_CLICKRECT);
            if(!trz_fc) return -1;
            
            char cmd_tapfg[256]={0};
            sprintf(cmd_tapfg,"input tap %d %d",trz_fodr->x+trz_fc->x+(trz_fc->width>>1),trz_fodr->y+trz_fc->y+(trz_fc->height>>1));
            subproc_execmd(NULL,cmd_tapfg);
            Sleep(1040);
        }
        
        //等待
        time_t stamp=GMajor->fg_countdown+GMajor->timestamp;
        struct tm* local_time=localtime(&stamp);
        char time_str[256]="";
        strftime(time_str,sizeof(time_str),"%H:%M:%S",local_time);
        printf("开始休眠，预计福袋在%d秒后[%s]开奖.\n",GMajor->fg_countdown,time_str);
        int countdown_seconds=GMajor->fg_countdown+GMajor->timestamp-time(NULL);
        if(countdown_seconds>=30) {
            Sleep((countdown_seconds+1)*1000);
        } else if(countdown_seconds>0) {
            //切回主界面等待30秒
            pTRZ trz_fodr=gettrzbycode(trz,FORTUNEGIFT_OR_DIAMOND_RANGE);
            if(!trz_fodr) return -1;
            pTRZ trz_fc=gettrzbycode(trz,FORTUNEGIFT_CLICKRECT);
            if(!trz_fc) return -1;
            
            char cmd_tapfg[256]={0};
            sprintf(cmd_tapfg,"input tap %d %d",trz_fodr->x+trz_fc->x+(trz_fc->width>>1),trz_fodr->y+trz_fc->y+(trz_fc->height>>1));
            subproc_execmd(NULL,cmd_tapfg);
            Sleep((countdown_seconds+1)*1000);          
        }
    } break;
    case FORTUNEGIFT_NOPRIZE: {
        //关闭页面，继续蹲守
        printf("抱歉未中奖。\n");
        pTRZ trz_close=gettrzbycode(trz,DOU_FORTUNEGIFT_DRAW_NOPRIZE_EXIT);
        if(!trz_close) return -1;
                
        char cmd_tapfg[256]={0};
        sprintf(cmd_tapfg,"input tap %d %d",trz_close->x+(trz_close->width>>1),trz_close->y+(trz_close->height>>1));
        subproc_execmd(NULL,cmd_tapfg);
        Sleep(1400);
    } break;
    case FORTUNEGIFT_JACKPOT: {} break;
    default: {} break;
    }
    return 0;
}

void PageAnalysis_Test() {
    char img_file[][256]={
        "./pics/x.png",
        "./pics/a20.png",
        "./pics/jackpot.png",
        "./pics/a3.png",
        "./pics/stream_closed.png",  
    };
    
    int sections_size=sizeof(sections)/sizeof(TRZ);
    pTRZ trz=new TRZ[sections_size];
    memcpy(trz,sections,sizeof(sections));
    
    tesseract::TessBaseAPI* api=new tesseract::TessBaseAPI();    
    
    for(int idx=0;idx<sizeof(img_file)/sizeof(char[256]);idx++) {
        PAGECODE page=GetPageCode(api,img_file[idx],trz);

        switch(page) {
        case FORTUNEGIFT_JACKPOT: 
            fg_draw_jackpot_ocr(api,trz,img_file[idx]);
        break;
        case FORTUNEGIFT_NOPRIZE: 
            fg_draw_noprize_ocr(api,trz,img_file[idx]);
        break;
        case STREAMER_ROOM: 
            streamer_room_ocr(api,trz,img_file[idx]);
        break;
        case STREAMER_ROOMCLOSED: 
            {
                printf("直播已结束.\n");
            }
        break;
        case FORTUNEGIFT_DETAIL: 
            fg_detail_ocr(api,trz,img_file[idx]);
        break;
        case FORTUNEGIFT_DETAIL_2TASK: 
            fg_detail_2task_ocr(api,trz,img_file[idx]);
        break;
        case DIAMOND_DETAIL: 
            //diamond_detail_ocr(api,trz,img_file[idx]);
        break;
        default: 
            printf("Unknown PAGE,image file [%s]\n",img_file[idx]);
        }
    }
    
    ReleaseSections(trz);
    if(api) delete api;
    
}