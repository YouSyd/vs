##------------------------------------------------------##
##     WorkFlow proj test                               ##
##------------------------------------------------------##
##  该程序将实现一个类似流程节点的控件，通过配置流程节      ##
##  点前驱后继、处置函数，自动完成状态切换和任务处理        ##
##                                                      ##
##------------------------------------------------------##
## 生成调试信息的程序
CC = cl /Zi /EHsc /D"SCRATCHVIEW_DBGTEST"
### $(CCDBG) ......
LINKDBG = /DEBUG
### LINK ... $(LINKDBG)

##CC = cl
CFLAGS = user32.lib gdi32.lib kernel32.lib Shell32.lib Shlwapi.lib Comctl32.lib comdlg32.lib container.lib frame.lib dialog.lib button.lib Ole32.lib

## yDark 控件模块
LIBC=E:\SRC_REF\dsc_test\dsctrls\lib
INCC=E:\SRC_REF\dsc_test\dsctrls\include
LIBDBGC=E:\SRC_REF\dsc_test\dsctrls\DBGlib

## opencv 、tesseract 、tesseract-ocr 模块 (vcpkg中已安装)
LIBO=E:\SRC_REF\vcpkg\vcpkg\installed\x64-windows\lib
INCO=E:\SRC_REF\vcpkg\vcpkg\installed\x64-windows\include

all: workflow_proj.exe scratchview.dll cmdview.dll host.dll projcfg.dll
    @echo.Done.

target: workflow_proj.exe
    @call workflow_proj.exe

clean:
    @if exist *.obj @DEL -S -Q *.obj
    @if exist *.lib @DEL -S -Q *.lib
    @if exist *.exp @DEL -S -Q *.exp
    @if exist *.dll @DEL -S -Q *.dll
    @if exist *.bak @DEL -S -Q *.bak
    @if exist workflow_proj.exe @DEL -S -Q workflow_proj.exe
    
tesseract_test.obj:tesseract_test.cpp
    $(CC) -c tesseract_test.cpp /I $(INCO)

tesseract_test.lib:tesseract_test.obj
    LIB tesseract_test.obj /LIBPATH:$(LIBO) /OUT:tesseract_test.lib
    
pipe_exec_test.obj: pipe_exec_test.cpp
    $(CC) -c pipe_exec_test.cpp 

pipe_exec_test.lib: pipe_exec_test.obj
    LIB pipe_exec_test.obj /OUT:pipe_exec_test.lib

workflow_test.obj:workflow_test.cpp
    $(CC) -c workflow_test.cpp
    
workflow_test.lib: workflow_test.obj
    LIB workflow_test.obj /OUT:workflow_test.lib

workflow_ctrl.obj:workflow_ctrl.cpp
    $(CC) -c workflow_ctrl.cpp

workflow_ctrl.lib:workflow_ctrl.obj
    LIB workflow_ctrl.obj /OUT:workflow_ctrl.lib

PopupMenu.obj:PopupMenu.cpp
    $(CC) -c PopupMenu.cpp

PopupMenu.lib: PopupMenu.obj
    LIB PopupMenu.obj /OUT:PopupMenu.lib

img_scratch.obj:img_scratch.cpp
    $(CC) -c img_scratch.cpp

img_scratch.lib:img_scratch.obj trzcfg.obj PopupMenu.obj trzcfg.res
    LIB img_scratch.obj trzcfg.obj PopupMenu.obj trzcfg.res /OUT:img_scratch.lib
    
workflow_proj.obj:workflow_proj.cpp
    $(CC) -c workflow_proj.cpp
    
workflow_proj.exe:workflow_proj.obj workflow_test.lib pipe_exec_test.lib tesseract_test.lib workflow_ctrl.lib trzcfg.lib trzcfg.res img_scratch.lib PopupMenu.lib CmdConsole.lib workflow_proj.res 
    link workflow_proj.obj workflow_test.lib pipe_exec_test.lib tesseract_test.lib workflow_ctrl.lib trzcfg.lib trzcfg.res img_scratch.lib PopupMenu.lib CmdConsole.lib workflow_proj.res $(LIBDBGC)\container.obj $(CFLAGS) /out:$@ /LIBPATH:$(LIBC) /LIBPATH:$(LIBO) /MANIFEST:EMBED $(LINKDBG) 

rc_ref.obj: rc_ref.cpp
    $(CC) -c rc_ref.cpp 

scratchview.res: scratchview.rc
    @RC scratchview.rc

scratchview.dll: scratchview.res rc_ref.obj
    link -DLL rc_ref.obj scratchview.res /out:scratchview.dll

cmdview.dll: CmdConsole.res rc_ref.obj
    link -DLL rc_ref.obj CmdConsole.res /out:cmdview.dll

CmdConsole.res: CmdConsole.rc
    @RC CmdConsole.rc

CmdConsole.obj: CmdConsole.cpp
    $(CC) -c CmdConsole.cpp

CmdConsole.lib: CmdConsole.obj 
    LIB CmdConsole.obj

trzcfg.res: trzcfg.rc
    @RC trzcfg.rc

trzcfg.obj: trzcfg.cpp
    $(CC) -c trzcfg.cpp

trzcfg.lib: trzcfg.obj trzcfg.res
    LIB trzcfg.obj trzcfg.res /OUT:trzcfg.lib

host.res: host.rc
    @RC host.rc 

host.obj: host.cpp
    $(CC) -c host.cpp

host.dll: host.obj host.res
    link -DLL host.obj host.res $(CFLAGS) /out:host.dll

projcfg.res: projcfg.rc
    @RC projcfg.rc 

projcfg.obj: projcfg.cpp
    $(CC) -c projcfg.cpp

projcfg.dll: projcfg.obj projcfg.res
    link -DLL projcfg.obj projcfg.res $(CFLAGS) /out:projcfg.dll


workflow_proj.res: workflow_proj.rc
    @RC workflow_proj.rc

test: all
    @ECHO.WorkFlow Proj assumble.
    @if exist workflow_proj.exe call workflow_proj.exe