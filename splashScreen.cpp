#include "SplashScreen.h"
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QMediaPlayer>
#include <QUrl>
#include <QApplication>
#include <QGraphicsPixmapItem>
#include <QRandomGenerator>
#include <QPropertyAnimation>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QDialog>
#include <QTextEdit>
#include <QScrollArea>
#include "mainwindow.h"
#include <QKeyEvent>
#include <QDebug>
#include <QPaintEvent>

SplashScreen::SplashScreen(QWidget *parent) : QWidget(parent)
{
    // 设置窗口无边框
    setWindowFlags(Qt::FramelessWindowHint);

    // 创建花瓣场景
    petalScene = new QGraphicsScene(this);
    petalView = new QGraphicsView(petalScene, this);
    petalView->setStyleSheet("background: transparent; border: none;");
    petalView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    petalView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    petalView->setRenderHint(QPainter::Antialiasing);

    // 加载背景图片
    if (!background.load(":/image/background1.png")) {
        qDebug() << "Failed to load background image: :/image/background1.png";
    } else {
        qDebug() << "Background image loaded successfully.";
    }
    if (!background.isNull()) {
        background = background.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    } else {
        qDebug() << "Background pixmap is null. Cannot scale.";
    }

    // 设置窗口为全屏
    showFullScreen();

    setupUI();
    setupAnimations();

    // 花瓣飘落定时器
    petalTimer = new QTimer(this);
    connect(petalTimer, &QTimer::timeout, this, &SplashScreen::createPetal);
    petalTimer->start(800); // 每800毫秒创建一个花瓣
}

SplashScreen::~SplashScreen()
{
    // 停止定时器
    if (petalTimer) {
        petalTimer->stop();
        delete petalTimer;
    }

    // 清除所有花瓣
    qDeleteAll(petals);
    petals.clear();

    // 清理其他对象
    delete petalScene;
    delete petalView;
    delete titleLabel;
    delete startButton;
    delete introButton;
}

#include <QPainter>

void SplashScreen::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawPixmap(0, 0, background);
}

void SplashScreen::setupUI()
{
    startButton = new QPushButton("开始探索", this);
    startButton->setFixedSize(150, 50); // 修改为和导言按钮相同的大小
    startButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #9F4125;"
        "   color: white;"
        "   border-radius: 8px;"
        "   font-family: 楷体;font-size: 22px;"
        "   padding: 5px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #B85C3D;"
        "}"
        );

    // 创建导言按钮
    introButton = new QPushButton("导言", this);
    introButton->setFixedSize(150, 50);
    introButton->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(159, 65, 37, 0.7);"
        "   color: white;"
        "   border-radius: 8px;"
        "   font-family: 楷体;font-size: 22px;"
        "   padding: 5px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(184, 92, 61, 0.9);"
        "}"
        );

    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch(2);
    // mainLayout->addWidget(titleLabel, 0, Qt::AlignCenter); 注释掉添加标题的代码
    mainLayout->addStretch(1);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(introButton);
    buttonLayout->addSpacing(100);
    buttonLayout->addWidget(startButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch(1);

    // 连接信号槽
    connect(startButton, &QPushButton::clicked, this, &SplashScreen::onStartClicked);
    connect(introButton, &QPushButton::clicked, this, &SplashScreen::onIntroClicked);
}

void SplashScreen::setupAnimations()
{
    // 设置花瓣飘落效果
    petalView->setSceneRect(0, 0, width(), height());
    petalView->setGeometry(0, 0, width(), height());
}


AnimatedPetal::AnimatedPetal(const QPixmap &pixmap, QGraphicsScene *scene, QObject *parent)
    : QObject(parent), QGraphicsPixmapItem(pixmap) {
    scene->addItem(this);

    // 随机初始位置和属性
    setPos(QRandomGenerator::global()->bounded(scene->width()), -50);
    setRotation(QRandomGenerator::global()->bounded(360));
    setOpacity(0.7 + QRandomGenerator::global()->bounded(0.3));

    // 创建下落动画
    fallAnimation = new QPropertyAnimation(this, "pos", this);
    fallAnimation->setDuration(3000 + QRandomGenerator::global()->bounded(3000));
    fallAnimation->setStartValue(pos());
    fallAnimation->setEndValue(QPointF(
        pos().x() + QRandomGenerator::global()->bounded(-100, 100),
        scene->height() + 50
        ));
    fallAnimation->setEasingCurve(QEasingCurve::InQuad);

    // 创建旋转动画
    rotateAnimation = new QPropertyAnimation(this, "rotation", this);
    rotateAnimation->setDuration(3000 + QRandomGenerator::global()->bounded(3000));
    rotateAnimation->setStartValue(rotation());
    rotateAnimation->setEndValue(rotation() + QRandomGenerator::global()->bounded(-180, 180));

    // 连接动画结束信号
    connect(fallAnimation, &QPropertyAnimation::finished, this, &AnimatedPetal::onAnimationFinished);

    // 同时启动两个动画
    fallAnimation->start();
    rotateAnimation->start();
}

AnimatedPetal::~AnimatedPetal() {
    fallAnimation->stop();
    rotateAnimation->stop();
}

void AnimatedPetal::onAnimationFinished() {
    emit animationFinished(this);
}

void SplashScreen::createPetal() {
    // 限制花瓣数量
    if (petals.size() > 30) {
        AnimatedPetal *oldestPetal = petals.takeFirst();
        delete oldestPetal;
    }

    // 随机选择花瓣图片
    int petalType = QRandomGenerator::global()->bounded(1, 6);
    QPixmap petalPix;
    QString petalPath = QString(":/flowericon/petal%1.png").arg(petalType);
    if (!petalPix.load(petalPath)) {
        qDebug() << "Failed to load petal image: " << petalPath;
    } else {
        qDebug() << "Petal image loaded successfully: " << petalPath;
    }
    if (!petalPix.isNull()) {
        petalPix = petalPix.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        qDebug() << "Petal pixmap is null. Cannot scale.";
        return;
    }

    // 创建自定义花瓣
    AnimatedPetal *petal = new AnimatedPetal(petalPix, petalScene, this);
    petals.append(petal);

    // 连接信号以处理花瓣动画结束
    connect(petal, &AnimatedPetal::animationFinished, this, &SplashScreen::handlePetalFinished);
}

void SplashScreen::handlePetalFinished(AnimatedPetal *petal) {
    petals.removeOne(petal);
    petal->deleteLater();
}

void SplashScreen::onStartClicked() {
    // 停止花瓣定时器
    petalTimer->stop();

    // 清除所有花瓣
    qDeleteAll(petals);
    petals.clear();

    // 创建并显示主窗口
    MainWindow *mainWindow = new MainWindow();
    mainWindow->showFullScreen();

    // 关闭启动界面
    close();
}
void SplashScreen::showIntroLetter()
{
    QDialog *letterDialog = new QDialog(this);
    letterDialog->setWindowTitle("燕园花册导言");
    letterDialog->setFixedSize(600, 400);
    letterDialog->setStyleSheet(
        "QDialog {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F8F4E8, stop:1 #F0E6D2);"
        "   border: 2px solid #9F4125;"
        "   border-radius: 10px;"
        "}"
        );

    QVBoxLayout *layout = new QVBoxLayout(letterDialog);

    // 添加信纸背景
    QLabel *paperBg = new QLabel(letterDialog);
    paperBg->setStyleSheet(
        "QLabel {"
        "   background-color: #FFFEF9;"
        "   border: 1px solid #D9C7B8;"
        "   border-radius: 5px;"
        "}"
        );

    QVBoxLayout *paperLayout = new QVBoxLayout(paperBg);
    paperLayout->setContentsMargins(30, 20, 30, 20);

    // 添加标题
    QLabel *title = new QLabel("燕园花册", paperBg);
    title->setStyleSheet("font-size: 24px; color: #9F4125; font-weight: bold;");
    title->setAlignment(Qt::AlignCenter);
    paperLayout->addWidget(title);

    // 添加分隔线
    QFrame *line = new QFrame(paperBg);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("color: #D9C7B8;");
    paperLayout->addWidget(line);

    // 添加滚动区域
    QScrollArea *scrollArea = new QScrollArea(paperBg);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("border: none; background: transparent;");

    QTextEdit *content = new QTextEdit();
    content->setReadOnly(true);
    content->setStyleSheet(
        "QTextEdit {"
        "   border: none;"
        "   background: transparent;"
        "   font-family: '楷体';"
        "   font-size: 16px;"
        "   color: #5A4A42;"
        "}"
        );

    // 设置导言内容
    content->setHtml("<p>亲爱的花友：</p>"
                     "<p style='text-indent: 2em;'>欢迎使用《燕园花册》，这是一款专为北京大学校园花卉爱好者设计的应用程序。</p>"
                     "<p style='text-indent: 2em;'>燕园四季，花开花落，每一朵花都承载着校园的记忆与故事。本程序收录了校园内常见的花卉信息，包括花期、分布位置和详细介绍，帮助您更好地了解和欣赏这些美丽的植物。</p>"
                     "<p style='text-indent: 2em;'>您可以通过地图浏览花卉分布，定制您的专属赏花足迹，参与花卉知识问答，还能创建个人赏花相册。</p>"
                     "<p style='text-indent: 2em;'>愿您在使用本程序的过程中，发现更多燕园花卉之美！</p>"
                     "<p align='right'>不想起名队</p>");

    scrollArea->setWidget(content);
    paperLayout->addWidget(scrollArea);

    // 添加关闭按钮
    QPushButton *closeButton = new QPushButton("关闭", paperBg);
    closeButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #9F4125;"
        "   color: white;"
        "   border-radius: 5px;"
        "   padding: 5px 15px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #B85C3D;"
        "}"
        );
    connect(closeButton, &QPushButton::clicked, letterDialog, &QDialog::accept);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    paperLayout->addLayout(buttonLayout);

    layout->addWidget(paperBg);

    // 设置淡入效果
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(letterDialog);
    letterDialog->setGraphicsEffect(effect);
    QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0);
    fadeIn->setEndValue(1);
    fadeIn->start();

    letterDialog->exec();
}



void SplashScreen::onIntroClicked()
{
    showIntroLetter();
}

void SplashScreen::resizeEvent(QResizeEvent *event)
{
    if (!background.isNull()) {
        background = background.scaled(size(), Qt::KeepAspectRatioByExpanding);
    }
    petalView->setGeometry(0, 0, width(), height());
    petalScene->setSceneRect(0, 0, width(), height());
    QWidget::resizeEvent(event);
}
void SplashScreen::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        qApp->quit();
    }
    QWidget::keyPressEvent(event);
}
