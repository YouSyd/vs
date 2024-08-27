#!/bin/bash
#脚本参数 1 : 主机名
#    参数 2 ：锁屏密码

#脚本需要一个host参数
HOST=$1
#屏幕密码
SCREENPASSWD=$2

PACKAGE_NAME="com.ss.android.ugc.aweme"
#检查包是否已经安装
#PACKAGE_INSTALLED=0
if adb -s $HOST shell pm list packages | grep -q $PACKAGE_NAME; then
    echo "$PACKAGE_NAME is installed."
#    PACKAGE_INSTALLED=1
else 
    echo "$PACKAGE_NAME uninstalled."
    exit 1
fi

#1.检查屏幕是否已经熄灭
#检查屏幕是否已经关闭
SCREENSTATE=$(adb -s $HOST shell dumpsys power | grep "Display Power" | awk '{print $3}')

#2.如果熄灭，点亮屏幕
if [ "$SCREENSTATE"  = "state=OFF" ]; then
    echo "Screen light off now, script will light it on later."
    
    # 打开电源    
    adb -s $HOST shell input keyevent 26  # 点击电源键
    Sleep 0.5 

    # 如果设置了锁屏密码，需要输入密码
    echo "The screen is locked and may require a password."
    
    #3.检查是否需要输入密码
    adb -s $HOST shell input keyevent 224  # 唤醒屏幕
    sleep 0.5

    # 划屏，显示输入密码界面
    adb -s $HOST shell input swipe 300 1000 300 500 500
    sleep 0.5

    adb -s $HOST shell input text $SCREENPASSWD  # 输入密码
fi

#4.打开指定的包
PID=$(adb -s $HOST shell pidof $PACKAGE_NAME)
if [ -n "$PID" ]; then
    # kill $PID 需要 root 权限，改用 am force-stop 
    #adb -s $HOST shell kill $PID
    adb -s $HOST shell am force-stop $PACKAGE_NAME
    echo "目标包正在运行(进程ID $PID)，脚本会首先终止其执行，稍后重新启动"
fi

# 默认的Activity 获取脚本大概类似这种：
# adb shell cmd package resolve-activity --brief com.ss.android.ugc.aweme
MAINACTIVITY=".splash.SplashActivity"
adb -s $HOST shell am start -n "$PACKAGE_NAME/$MAINACTIVITY"
echo "启动应用需要一点时间，请稍适等待..."
sleep 5
PID=$(adb -s $HOST shell pidof $PACKAGE_NAME)
if [ -n "$PID" ]; then
    echo "目标包已经启动完成，脚本正常退出"
    exit 0
else
    echo "目标包未能正常启动，脚本退出"
    exit 1
fi



