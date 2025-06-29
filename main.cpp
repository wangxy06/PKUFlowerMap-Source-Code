#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <crtdbg.h>  // Windows内存检测头文件
#include <splashScreen.h>

int main(int argc, char *argv[])
{
    // Windows内存泄漏检测（添加这行）
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz} %{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{file}:%{line} - %{message}");

    QApplication a(argc, argv);
    // 显示启动界面
    SplashScreen splash;
    splash.show();
    // MainWindow w;
    // w.show();
    return a.exec();
}
