#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mapwidget.h"
#include "navigation.h"
#include "gamewidget.h"
#include "addcheckindialog.h"
#include "albumwidget.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QVector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 设置窗口无边框
    setWindowFlags(Qt::FramelessWindowHint);

    // 创建主分割器（水平分割，左侧放按钮，右侧放地图）
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setHandleWidth(2);
    mainSplitter->setStyleSheet(
        "QSplitter::handle {"
        "   background-color: #9F4125;"
        "   width: 2px;"
        "}"
        );

    // 创建左侧按钮区域
    QWidget *leftPanel = new QWidget(mainSplitter);
    leftPanel->setStyleSheet(
        "QWidget {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #F8F4E8, stop:1 #F0E6D2);"
        "   border-right: 1px solid #9F4125;"
        "}"
        );

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(20, 20, 20, 20);
    leftLayout->setSpacing(20);

    // 添加月份选择
    QLabel *monthLabel = new QLabel("选择月份:", leftPanel);
    monthLabel->setStyleSheet("color: #9F4125; font-size: 16px; font-weight: bold;");

    QComboBox *monthComboBox = new QComboBox(leftPanel);
    monthComboBox->setFixedWidth(150);
    monthComboBox->setStyleSheet(
        "QComboBox {"
        "   background-color: white;"
        "   border: 1px solid #9F4125;"
        "   border-radius: 5px;"
        "   padding: 5px;"
        "   min-height: 30px;"
        "   font: 14px '楷体';"
        "}"
        "QComboBox::drop-down {"
        "   border: none;"
        "}"
        );

    monthComboBox->addItem("全部月份");
    for (int i = 1; i <= 12; ++i) {
        monthComboBox->addItem(QString::number(i) + "月");
    }

    leftLayout->addWidget(monthLabel);
    leftLayout->addWidget(monthComboBox);
    leftLayout->addSpacing(20);

    QLabel *titleLabel = new QLabel("燕园花册", leftPanel);
    titleLabel->setStyleSheet(
        "QLabel {"
        "   color: #9F4125;"
        "   font-size: 28px;"
        "   font-weight: bold;"
        "   font-family: '楷体';"
        "   padding-bottom: 10px;"
        "   border-bottom: 2px solid #9F4125;"
        "   margin-bottom: 20px;"
        "}"
        );
    leftLayout->insertWidget(0, titleLabel);
    // 创建功能按钮
    QString buttonStyle =
        "QPushButton {"
        "   background-color: #9F4125;"
        "   color: white;"
        "   border-radius: 8px;"
        "   padding: 12px;"
        "   font:bold 16px '楷体';"
        "   min-width: 150px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #B85C3D;"
        "}";

    QPushButton *navButton = new QPushButton("导航", leftPanel);
    QPushButton *checkinButton = new QPushButton("打卡", leftPanel);
    QPushButton *quizButton = new QPushButton("花卉问答", leftPanel);
    QPushButton *albumButton = new QPushButton("赏花相册", leftPanel);
    QPushButton *exitButton = new QPushButton("退出", leftPanel);

    navButton->setStyleSheet(buttonStyle);
    //flowerButton->setStyleSheet(buttonStyle);
    checkinButton->setStyleSheet(buttonStyle);
    quizButton->setStyleSheet(buttonStyle);
    albumButton->setStyleSheet(buttonStyle);
    exitButton->setStyleSheet(buttonStyle);

    leftLayout->addWidget(navButton);
    //leftLayout->addWidget(flowerButton);
    leftLayout->addWidget(checkinButton);
    leftLayout->addWidget(quizButton);
    leftLayout->addWidget(albumButton);
    leftLayout->addStretch();
    leftLayout->addWidget(exitButton);

    // 创建地图区域
    mapWidget = new MapWidget(mainSplitter);
    mapWidget->setStyleSheet(
        "QGraphicsView {"
        "   background: #F8F4E8;"
        "   border: 1px solid #D9C7B8;"
        "   border-radius: 5px;"
        "}"
        );

    // 加载地图数据
    mapWidget->loadMap(":/map/pku_map.jpg");
    mapWidget->loadMapFromTxt(":/mapData/map_data.txt");
    mapWidget->initStructureData();
    mapWidget->linkFlowersToLocations(flower.getAllFlowers());
    mapWidget->addFlowerIcons();

    // 设置分割比例
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 4);

    // 将分割器设置为主窗口的中心部件
    setCentralWidget(mainSplitter);

    // 连接信号与槽
    connect(monthComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            mapWidget, &MapWidget::setSelectedMonth);
    connect(navButton, &QPushButton::clicked, this, &MainWindow::onNavigationButtonClicked);
    connect(quizButton, &QPushButton::clicked, this, &MainWindow::onQuizButtonClicked);
    connect(checkinButton, &QPushButton::clicked, this, &MainWindow::onCheckinButtonClicked);
    connect(albumButton, &QPushButton::clicked, this, [this]() {
        AlbumWidget albumWidget(this);
        albumWidget.setCheckinRecords(checkinRecords);
        albumWidget.exec();
    });
    connect(exitButton, &QPushButton::clicked, qApp, &QApplication::quit);
    connect(this, &MainWindow::addCheckin, this, &MainWindow::handleAddedCheckin);


    // 加载打卡记录
    loadCheckinRecords();

    // 初始化打卡窗口
    // checkinWidget = nullptr;
    addcheckindialog =nullptr;
}

MainWindow::~MainWindow()
{
    // 保存打卡记录
    saveCheckinRecords();
    delete ui;
}

void MainWindow::onNavigationButtonClicked()//导航窗口
{
    // 创建并显示导航窗口
    NavigationWindow *navWindow = new NavigationWindow();
    navWindow->show();

    // 如果想要窗口关闭时自动删除，可以加上：
    navWindow->setAttribute(Qt::WA_DeleteOnClose);
}

void MainWindow::onQuizButtonClicked()//游戏窗口
{
    GameWidget*gameWidget = new GameWidget();
    gameWidget->setFlowers(flower.getAllFlowers());
    gameWidget->show();
    gameWidget->setAttribute(Qt::WA_DeleteOnClose);
}

void MainWindow::onCheckinButtonClicked()
{
    AddCheckinDialog dialog(mapWidget, this);
    if (dialog.exec() == QDialog::Accepted) {
        CheckinData data = dialog.getCheckinData();
        emit addCheckin(data);
    }
}
void MainWindow::handleAddedCheckin(const CheckinData& data)
{
    checkinRecords.append(data);
    if (mapWidget) {
        mapWidget->setCheckinRecords(checkinRecords);
    }
}
void MainWindow::onAlbumButtonClicked()
{
    // 显示图册窗口
    albumWidget->setCheckinRecords(checkinRecords);
    albumWidget->show();
}
// 移除void MainWindow::onAddCheckin(const CheckinData& data)函数定义
void MainWindow::loadCheckinRecords()
{
    // 从文件加载打卡记录
    QFile file(CHECKIN_LOG_FILE_PATH);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray array = doc.array();

        checkinRecords.clear();
        for (const QJsonValue& val : array) {
            QJsonObject obj = val.toObject();
            CheckinData record;
            record.date = obj["date"].toString();
            record.location = obj["location"].toString();
            record.flowerName = obj["flowerName"].toString();
            record.imagePath = obj["imagePath"].toString();
            record.log = obj["log"].toString();
            checkinRecords.append(record);
        }
        file.close();
    }
}

void MainWindow::saveCheckinRecords()
{
    // 保存打卡记录到文件
    QJsonArray array;
    for (const CheckinData& record : checkinRecords) {
        QJsonObject obj;
        obj["date"] = record.date;
        obj["location"] = record.location;
        obj["flowerName"] = record.flowerName;
        obj["imagePath"] = record.imagePath;
        obj["log"] = record.log;
        array.append(obj);
    }

    QJsonDocument doc(array);
    QFile file(CHECKIN_LOG_FILE_PATH);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson());
        file.close();
    }
}
