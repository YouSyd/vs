#include "exec_cmd.h"
#include <windows.h>
#include <objbase.h>

#include "workflow_proj.h"
#include "CmdConsole.h"
#include "img_scratch.h"

int initialsubproc(pExecPipes p) {
    char cmd_line[256]={0};
    sprintf(cmd_line,"adb -s %s shell",GMajor->host);
    return initialsubproc(p,(char*)cmd_line);
}

int initialsubproc(pExecPipes p,char* cmd_line) {
    //printf("%s-->%s\n",__func__,cmd_line);
    int slience_mode=0;
    if(f->extra) {
        pwfpj_param wfpj=(pwfpj_param)f->extra;
        HWND cmdview_hwnd=wfpj->cmdview;
        pCmdView cdv=CmdView_GetSettings(cmdview_hwnd);
        if(cdv) slience_mode=cdv->slience_mode;
    }

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength=sizeof(SECURITY_ATTRIBUTES); 
    saAttr.bInheritHandle=TRUE; 
    saAttr.lpSecurityDescriptor=NULL;

    p->begin=time(NULL);    
    if(!CreatePipe(&p->subproc_out_rd,&p->subproc_out_wt,&saAttr,0)) {
        printf("创建子程序output端管道失败。\n");
        return -1;
    }
    
    if(!CreatePipe(&p->subproc_in_rd,&p->subproc_in_wt,&saAttr,0)) {
        printf("创建子程序input端管道失败。\n");
        return -1;
    }
    //标准输入所对应管道的出口不能是标准输入，将由程序接管控制
    //if(!SetHandleInformation(p->subproc_in_wt,HANDLE_FLAG_INHERIT,0)) {return -1;};
    
    //启动执行命令的命令行 使用adb上下文
    char* cmd_ctx=cmd_line;
    int cmd_length=strlen(cmd_line);
    p->input_len=cmd_length;
    if(cmd_length>0) {
        if(p->input_buffer==NULL) p->input_buffer=(char*)calloc(sizeof(char)*(256),1);
        memcpy(p->input_buffer,cmd_line,cmd_length);
    }
    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFO siStartInfo;
    BOOL bSuccess=FALSE;
    ZeroMemory(&piProcInfo,sizeof(PROCESS_INFORMATION));
    
    ZeroMemory(&siStartInfo,sizeof(STARTUPINFO));
    siStartInfo.cb=sizeof(STARTUPINFO); 
    siStartInfo.hStdError=p->subproc_out_wt;
    siStartInfo.hStdOutput=p->subproc_out_wt;
    siStartInfo.hStdInput=p->subproc_in_rd;
    siStartInfo.dwFlags=STARTF_USESTDHANDLES;
    if(slience_mode==1) {
        /*
         * 静默模式下，子进程在后台运行
         */
        siStartInfo.dwFlags=siStartInfo.dwFlags|STARTF_USESHOWWINDOW;
        siStartInfo.wShowWindow=SW_HIDE;
    }
 
    // Create the child process. 
    bSuccess=CreateProcess(NULL, 
        (LPSTR)cmd_ctx,     // command line 
        NULL,          // process security attributes 
        NULL,          // primary thread security attributes 
        TRUE,          // handles are inherited 
        0,  //((slience_mode==1)?DETACHED_PROCESS:0), // creation flags 
        NULL,          // use parent's environment 
        NULL,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION 
   
    // If an error occurs, exit the application. 
    if(!bSuccess) {
        printf("子进程创建失败。\n");
        return -1;
    } else {
        //printf("创建子进程 %s\n",cmd_ctx);
        p->subproc=piProcInfo.hProcess;
        p->subproc_id=piProcInfo.dwProcessId;
        p->subthread=piProcInfo.hThread;
        
        return 0;
    }
    
    return -1;
}

void releasesubproc(pExecPipes p) {    
    if(INVALID_HANDLE_VALUE==p->subproc) CloseHandle(p->subproc);
    if(INVALID_HANDLE_VALUE==p->subthread) CloseHandle(p->subthread);
    
    //释放资源
    if(INVALID_HANDLE_VALUE==p->subproc_in_wt) CloseHandle(p->subproc_in_wt);
    if(INVALID_HANDLE_VALUE==p->subproc_out_rd) CloseHandle(p->subproc_out_rd);

    if(p->input_buffer) {
        free(p->input_buffer);
        p->input_buffer=NULL;
    }
    if(p->output_buffer) {
        free(p->output_buffer);
        p->output_buffer=NULL;
    }
    //printf("%s\n",__func__);
}

int subproc_execmd(char* cmd_line,char* cmd_buffer) {
    char log_file[256]="E:\\SRC_REF\\tesseract\\self-bulid\\test\\vs\\pipe_exec_cmd.log";  
    return subproc_execmd(cmd_line,cmd_buffer,log_file);
}

int subproc_execmd(char* cmd_line,char* cmd_buffer,void* cb_param) {
    return subproc_execmd(cmd_line,cmd_buffer,NULL,cb_param);
}

int subproc_execmd(char* cmd_line,char* cmd_buffer,subproc_analysis analysis,void* cb_param) {
    //printf("%s....\n",__func__);

    char cmd[1024]={0};
    int offset=0; 
    int result=-1;
    DWORD errorcode;
    DWORD waitcode;

    RExecPipes p={0};
    if(analysis) p.analysis=analysis;
    else p.analysis=logpipe_exec;
    p.length=1024*5;
    p.output_buffer=(char*)calloc(sizeof(char)*p.length,1);
    if(p.output_buffer==NULL) {
        printf("申请内存失败.\n");
        goto EXEC_EXIT;
    }
    
    if(0!=(cmd_line?initialsubproc(&p,cmd_line):initialsubproc(&p))) {
        printf("初始化失败.\n");
        goto EXEC_EXIT;
    }

    //关闭无关管道
    CloseHandle(p.subproc_in_rd);
    CloseHandle(p.subproc_out_wt);
    
    //向输入管道写入信息
    DWORD dwBytes;
    if(cmd_buffer) {
        strcat(p.input_buffer," ");
        p.input_len+=1;
        strcat(p.input_buffer,cmd_buffer);
        p.input_len+=strlen(cmd_buffer);

        strcpy(cmd,cmd_buffer);
        strcat(cmd,"\n");
        DWORD cmd_length=strlen(cmd); 
        if(!WriteFile(p.subproc_in_wt,cmd,cmd_length,&dwBytes,NULL)) {
            printf("向输入管道写入数据失败,错误代码：%d\n",GetLastError());
            goto EXEC_EXIT;
        } else {
            //printf(">>>Exec CMD: %s\n",cmd_buffer);
        }
    }
    FlushFileBuffers(p.subproc_in_wt);
    WriteFile(p.subproc_in_wt,"exit\n",5,&dwBytes,NULL);
    
    
    while(ReadFile(p.subproc_out_rd,p.output_buffer+offset,p.length-offset,&dwBytes,NULL)) {
        if(dwBytes>0) {
            offset+=dwBytes;
            assert(offset<p.length);
        }
        else break;
    }
    p.output_len=offset;

    errorcode=GetLastError();
    if(0!=errorcode) {
        LPVOID lpMsgBuf;

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorcode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf,
            0,NULL);
        if(ERROR_BROKEN_PIPE!=errorcode) printf("error(%d):%s\n",errorcode,(char*)lpMsgBuf);
        LocalFree(lpMsgBuf);
        
        if(ERROR_BROKEN_PIPE!=errorcode) goto EXEC_EXIT;    
    }
        
    //printf("%s\n",p.output_buffer);
    
    //等待子进程退出,5秒后如何还未退出，强行剿灭
    waitcode=WaitForSingleObject(p.subproc,5000);
    if(waitcode==WAIT_TIMEOUT) {
        printf("等待超时，强制终止子进程\n");
        TerminateProcess(p.subproc,1);
    } 
    p.end=time(NULL);
    // else {
    //    if(waitcode==WAIT_OBJECT_0) {
    //        printf("子进程正常退出.\n");
    //    }
    //}
    
    //判断进程状态
    DWORD exitcode;
    if(GetExitCodeProcess(p.subproc,&exitcode)) {
        p.ret=exitcode;
        if(exitcode==STILL_ACTIVE) {
            printf("子进程运行中...\n");
        } else {
            //printf("子进程已经退出:%d\n",exitcode);
            result=exitcode;    
            if(exitcode==0) {
                //printf("（YES-0）\t\t[%s]\t(命令成功执行。)\n",cmd_buffer);
            } else if(exitcode==1) {
                printf("（UNKNOWN-%d）\t\t[%s]\t(不明，操作可能存在错误。)\n",exitcode,cmd_buffer);
            } else {
                printf("（NO-%d）\t\t[%s]\t(命令未成功执行。)\n",exitcode,cmd_buffer);
            }
            
        }
    } else {
        printf("获取子进程状态码失败.\n");
    }
    

EXEC_EXIT:
    //执行回调
    if(p.analysis) p.analysis(exitcode,&p,cb_param);
    if(p.analysis!=logpipe_exec) logpipe_exec(exitcode,&p,cb_param);

    releasesubproc(&p);

    return result;
}

void subproc_test() {
    char log_file[256]="E:\\SRC_REF\\tesseract\\self-bulid\\test\\vs\\pipe_exec_cmd.log";
    char cmd2[256]=" ls -l";
    if(0!=subproc_execmd(NULL,cmd2,log_file)) printf("Command Execution failed.\n");
    else printf("subproc test done.\n");
  
    char cmd3[256]="adb exec-out screencap -p > ../pics/x.png";
    if(0!=subproc_execmd((char*)"cmd",(char*)cmd3,log_file)) printf("Command Execution failed.\n");
    else printf("subproc test done.\n");
    
    char cmd[256]=" ip addr show wlan0 | grep 'inet'| grep '192' | awk '{print $2}' | awk -F/ '{print $1}' ";
    if(0!=subproc_execmd((char*)"adb shell ",cmd,log_file)) printf("Command Execution failed.\n");
    else printf("subproc test done.");

    char cmd4[256]="dir ";
    if(0!=subproc_execmd((char*)"cmd",(char*)cmd4,log_file)) printf("Command Execution failed.\n");
    else printf("subproc test done.\n");
  
    int cel_count=10;
    pCEL cel=(pCEL)calloc(sizeof(CEL)*cel_count,1);
    if(cel) {
        int fetched=0;
        char guid_up[50]="94C65315-9E69-4DCB-9A0D-E6BDEB1AC3B8";//"F44663F6-DFCD-47A4-B868-F58F9B84EC5X";//
        int ret=FetchCELRecord(log_file,cel,&fetched,cel_count,guid_up);
        printf("fetched %d record(s) \n",fetched);


        for(int idx=0;idx<fetched;idx++) {
            printf("RETURN:%d\n"
                   "INPUT:%s\n"
                   "OUTPUT:%s\n",(cel+idx)->header.ret,
                   (cel+idx)->input,
                   (cel+idx)->output);

            printf("------------------------------------------------------\n");
        }
        free(cel);
    }
}

/*
 * 从文件中指定guid 寻找前向 count 条记录
 * 正常情况下，文件从上到下，按时间顺序记录每条命令的执行日志
 * 根据指定的guid，向前倒回记录，共计 list 行
 * 处理逻辑如下
 * 全面解析文件，生成 guid 列表 及 对应日志 offset 列表
 * 
 */
int FetchCELRecord(char* filename,pCEL list,int* pfetch_count,int count,const char* guid) {
    int ret=-1;
    char* buffer=NULL;
    DWORD filesizehigh;
    DWORD filesize=0;
    char guid_list[5000][33]={0};
    int offset_list[5000]={0};
    int rcd_idx=0;
    const char* plt=NULL;

    HANDLE file=CreateFile(filename,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(file==INVALID_HANDLE_VALUE) {
        goto FETCHECELRECORD_EXIT;
    }

    filesize=GetFileSize(file,&filesizehigh);

    buffer=(char*)calloc(sizeof(char)*(filesize+1),1);
    if(!buffer) goto FETCHECELRECORD_EXIT;

    DWORD bytesread;
    ReadFile(file,buffer,filesize,&bytesread,NULL);
    CloseHandle(file);
    file=INVALID_HANDLE_VALUE;

    /*
     * 15 分钟 100条，一小时 400 条
     */
    plt=buffer;
    while(*plt!='\0'&&plt-buffer<bytesread) {
        CELHeader header;
        int offset=plt-buffer;
        plt=CELHeaderParse(plt,&header);

        strcpy(guid_list[rcd_idx],header.guid);
        offset_list[rcd_idx]=offset;
        rcd_idx++;

        if(header.content_length>0) plt+=header.content_length;

        if(strcmp(guid,header.guid)==0) break;
    }

    if(rcd_idx>0&&strcmp(guid,guid_list[rcd_idx-1])==0) {
        *pfetch_count=count;
        int start_idx=rcd_idx-count;
        if(start_idx<0) {
            start_idx=0;
            *pfetch_count=rcd_idx;
        }
        
        for(int idx=0;idx<*pfetch_count;idx++) {
            const char* content_plt=CELParse(buffer+offset_list[idx+start_idx],list+idx);
        }
        ret=0;
    }
FETCHECELRECORD_EXIT:
    if(buffer) { free(buffer);}
    
    if(file!=INVALID_HANDLE_VALUE) CloseHandle(file);
    return ret;
}

const char* CELHeaderParse(const char* plt,pCELHeader header) {
    strcpy(header->guid,plt);
    plt+=41;

    strcpy(header->begin,plt);
    plt+=20;

    strcpy(header->end,plt);
    plt+=20;

    sscanf(plt,"%x",&header->ret);
    plt+=9;
    sscanf(plt,"%x",&header->content_length);
    plt+=9;

    return plt;
}

const char* CELParse(const char* plt,pCEL cel) {
    const char* content_plt=plt;

    content_plt=CELHeaderParse(content_plt,&cel->header);
    
    int intput_length,output_length;
    sscanf(content_plt,"%x",&intput_length);
    content_plt+=9;
    sscanf(content_plt,"%x",&output_length);
    content_plt+=9;

    if(intput_length>255) {
        sprintf(cel->input,"%.252s...",content_plt);
    } else strcpy(cel->input,content_plt);
    content_plt+=intput_length+1;

    if(output_length>255) {
        sprintf(cel->output,"%.252s...",content_plt);
    } else strcpy(cel->output,content_plt);
    content_plt+=output_length+1;

    return content_plt; 
}

/*
 * 写入的文件名
 */
void logpipe_exec(int exitcode,void* pEPParam,void* param) {
    pExecPipes pep=(pExecPipes)pEPParam;
    if(!pep) return;

    CEL cel={0};
    GUID guid;
    char guid_string[41]={0};
    pwfpj_param wfpj=(pwfpj_param)(f->extra);

    if(CoCreateGuid(&guid)==S_OK) {
        sprintf(guid_string,"%08lX-%04X-%04X-%04X-%012llX",
                guid.Data1, guid.Data2, guid.Data3,
                (guid.Data4[0] << 8) | guid.Data4[1],
                ((unsigned long long)guid.Data4[2] << 40) |
                ((unsigned long long)guid.Data4[3] << 32) |
                ((unsigned long long)guid.Data4[4] << 24) |
                ((unsigned long long)guid.Data4[5] << 16) |
                ((unsigned long long)guid.Data4[6] << 8) |
                (unsigned long long)guid.Data4[7]);
    } else {
        printf("写入文件失败\n");
        if(wfpj) MessageBox(wfpj->frame,"写入文件失败","异常",MB_OK|MB_ICONERROR);
    }
    strncpy(cel.header.guid,guid_string,36);

    char filename[MAX_PATH]={0};
    if(param) strcpy(filename,(char*)param);
    HANDLE file=CreateFile(filename,FILE_APPEND_DATA,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(file==INVALID_HANDLE_VALUE) {
        printf("打开文件失败。\n");
        return;
    }

    DWORD dwWritten;
    /*
     * 写入 guid
     */
    WriteFile(file,guid_string,sizeof(guid_string),&dwWritten,NULL);
    
    char begin_time_str[20]={0},end_time_str[20]={0};
    struct tm* tminfo;
    /*
     * 写入开始时间
     * 
     */
    tminfo=localtime(&pep->begin);
    strftime(begin_time_str,sizeof(begin_time_str),"%Y-%m-%d %H:%M:%S",tminfo);
    WriteFile(file,begin_time_str,sizeof(begin_time_str),&dwWritten,NULL);
    strcpy(cel.header.begin,begin_time_str);

    /*
     * 写入结束时间
     * 
     */
    tminfo=localtime(&pep->end);
    strftime(end_time_str,sizeof(end_time_str),"%Y-%m-%d %H:%M:%S",tminfo);
    WriteFile(file,end_time_str,sizeof(end_time_str),&dwWritten,NULL);
    strcpy(cel.header.end,end_time_str);

    /*
     * 写入执行返回值
     */
    char ret_string[9]={0};
    sprintf(ret_string,"%x",pep->ret);
    WriteFile(file,ret_string,sizeof(ret_string),&dwWritten,NULL);
    cel.header.ret=pep->ret;

    /*
     * 计算 content_length
     */
    char content_length_string[9]={0};
    int content_length=pep->input_len+1+pep->output_len+1+9*2;
    sprintf(content_length_string,"%x",content_length);
    WriteFile(file,content_length_string,sizeof(content_length_string),&dwWritten,NULL);
    cel.header.content_length=content_length;

    /*
     * 写入input_length
     */
    char input_length_string[9]={0};
    sprintf(input_length_string,"%x",pep->input_len);
    WriteFile(file,input_length_string,sizeof(input_length_string),&dwWritten,NULL);
    if(pep->input_len>255) sprintf(cel.input,"%.252s...",pep->input_buffer);
    else strcpy(cel.input,pep->input_buffer);

    /*
     * 写入output_length
     */
    char output_length_string[9]={0};
    sprintf(output_length_string,"%x",pep->output_len);
    WriteFile(file,output_length_string,sizeof(output_length_string),&dwWritten,NULL);
    if(pep->output_len>255) sprintf(cel.output,"%.252s...",pep->output_buffer);
    else strcpy(cel.output,pep->output_buffer);

    /*
     * 写入input buffer , input_length 不包括 '\0'
     */
    WriteFile(file,pep->input_buffer,pep->input_len+1,&dwWritten,NULL);

    /*
     * 写入output buffer
     */
    WriteFile(file,pep->output_buffer,pep->output_len+1,&dwWritten,NULL);

    CloseHandle(file);

    /*
     * 向CmdView 发送消息
     *
     */
    if(wfpj) {
        CmdView_AttachCmdNode(wfpj->cmdview,&cel);

        if(cel.header.ret==0&&strstr(cel.input,"screencap")) {
            /*
             * 截图发给 scratchview
             */
            HWND scratch_hwnd=wfpj->imgsratch;
            pScratchView psv=ScratchView_GetSettings(scratch_hwnd);
            char curr_file[256]={0};
            strcpy(curr_file,psv->img_path);
            ScratchView_SetImage(scratch_hwnd,curr_file);
        }
    }
    
    return ;
}

#ifdef PIPE_EXEC_UNIT_TEST
int main() {
    subproc_test();
    
    printf("Exit Normally.\n");
    return 0;
}

#endif