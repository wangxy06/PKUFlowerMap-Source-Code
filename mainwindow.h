#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mapwidget.h"
#include "addcheckindialog.h"
#include "albumwidget.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    MapWidget *mapWidget;
    Flower flower;  // 新增成员变量
    void onNavigationButtonClicked();//槽函数，实现导航
    void onQuizButtonClicked();
    // CheckinWidget *checkinWidget; // 打卡界面
    AddCheckinDialog *addcheckindialog;
    AlbumWidget *albumWidget;     // 图册界面
    QVector<CheckinData> checkinRecords; // 打卡记录数组

    // 加载打卡记录
    void loadCheckinRecords();
    // 保存打卡记录
    void saveCheckinRecords();
    void onCheckinButtonClicked(); // 打卡按钮槽函数
    void onAlbumButtonClicked();   // 图册按钮槽函数
    void handleAddedCheckin(const CheckinData& data); //不知道干什么的先放着

signals:
    // 发送添加的打卡记录
    void addCheckin(const CheckinData& data);

};

#endif // MAINWINDOW_H
