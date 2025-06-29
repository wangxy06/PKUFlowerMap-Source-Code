#include "albumwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QFileInfo>
#include <QImageReader>
#include <QDir>
#include <QFrame>
#include <QApplication>

AlbumWidget::AlbumWidget(QWidget *parent)
    : QDialog(parent)
    , currentPage(0)
{
    setWindowTitle("燕园花笺");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(600, 800);
    setupUI();

    connect(flipButton, &QPushButton::clicked, this, &AlbumWidget::onFlipButtonClicked);
    connect(closeButton, &QPushButton::clicked, this, &AlbumWidget::onCloseButtonClicked);
    connect(nextPageButton, &QPushButton::clicked, this, &AlbumWidget::onNextPageButtonClicked);
    connect(previousPageButton, &QPushButton::clicked, this, &AlbumWidget::onPreviousPageButtonClicked);
}

void AlbumWidget::setupUI() {
    // 设置主窗口样式
    setStyleSheet("QDialog { background-color: #fff0f5; border: 1px solid #ffb6c1; }");

    pageStack = new QStackedWidget(this);
    pageStack->setStyleSheet("QStackedWidget { background: transparent; }");

    // 创建封面、扉页和内容页
    setupCoverPage();
    setupFirstPage();

    // 翻页按钮区域（用于内容页）
    QHBoxLayout *navLayout = new QHBoxLayout();
    previousPageButton = new QPushButton("◀", this);
    nextPageButton = new QPushButton("▶", this);
    pageLabel = new QLabel("", this);

    // 设置按钮样式
    QString buttonStyle = "QPushButton { background-color: #ffb6c1; border: none; color: white; "
                          "padding: 5px 10px; border-radius: 12px; min-width: 40px; }"
                          "QPushButton:hover { background-color: #ff69b4; }"
                          "QPushButton:disabled { background-color: #d3d3d3; }";

    previousPageButton->setStyleSheet(buttonStyle);
    nextPageButton->setStyleSheet(buttonStyle);
    previousPageButton->setFixedSize(40, 40);
    nextPageButton->setFixedSize(40, 40);
    pageLabel->setStyleSheet("QLabel { color: #ff69b4; font-weight: bold; }");

    navLayout->addStretch();
    navLayout->addWidget(previousPageButton);
    navLayout->addSpacing(20);
    navLayout->addWidget(pageLabel);
    navLayout->addSpacing(20);
    navLayout->addWidget(nextPageButton);
    navLayout->addStretch();

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(pageStack);
    mainLayout->addLayout(navLayout);
    mainLayout->setContentsMargins(0, 0, 0, 20);

    setLayout(mainLayout);

    // 初始状态
    updatePageDisplay(0);
    previousPageButton->hide();
    nextPageButton->hide();
    pageLabel->hide();
}

void AlbumWidget::setupCoverPage() {
    QWidget *coverPage = new QWidget();
    QVBoxLayout *coverLayout = new QVBoxLayout(coverPage);
    coverLayout->setContentsMargins(0, 0, 0, 0);
    coverLayout->setSpacing(0);

    // 封面图片
    QLabel *coverImage = new QLabel(coverPage);
    QString coverPath = ":/image/coverpage.png";
    if (QPixmap(coverPath).isNull()) {
        coverImage->setText("封面图片未找到");
        coverImage->setAlignment(Qt::AlignCenter);
        qWarning() << "封面图片加载失败:" << coverPath;
    } else {
        coverImage->setPixmap(QPixmap(coverPath).scaled(600, 800, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
    coverImage->setStyleSheet("QLabel { background-color: #F8E8E8; }");

    // 按钮容器（放在图片内部）
    QWidget *buttonContainer = new QWidget(coverImage);
    buttonContainer->setStyleSheet("background: transparent;");
    QVBoxLayout *buttonLayout = new QVBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(200, 0, 200, 100);

    flipButton = new QPushButton("开 始 阅 读", buttonContainer);
    closeButton = new QPushButton("退 出 阅 读", buttonContainer);

    QString buttonStyle = "QPushButton {"
                          "   background-color: rgba(159, 65, 37, 0.7);"
                          "   color: #F8E8E8;"
                          "   border: 1px solid #B85C3B;"
                          "   border-radius: 15px;"
                          "   padding: 12px 0;"
                          "   font: bold 16px '楷体';"
                          "   min-width: 100px;"
                          "}"
                          "QPushButton:hover {"
                          "   background-color: rgba(184, 92, 59, 0.8);"
                          "}";

    flipButton->setStyleSheet(buttonStyle);
    closeButton->setStyleSheet(buttonStyle);

    buttonLayout->addStretch();
    buttonLayout->addWidget(flipButton);
    buttonLayout->addSpacing(15);
    buttonLayout->addWidget(closeButton);

    // 使用相对布局将按钮容器定位在图片上
    coverImage->setLayout(new QVBoxLayout());
    coverImage->layout()->addWidget(buttonContainer);

    coverLayout->addWidget(coverImage);
    pageStack->addWidget(coverPage);
}

void AlbumWidget::setupFirstPage() {
    QWidget *firstPage = new QWidget();
    firstPage->setStyleSheet("background: transparent;");
    QGridLayout *firstLayout = new QGridLayout(firstPage);
    firstLayout->setContentsMargins(0, 0, 0, 0);
    firstLayout->setSpacing(0);

    // 扉页图片
    QLabel *firstImage = new QLabel(firstPage);
    QString firstPath = ":/image/firstpage.png";
    if (QPixmap(firstPath).isNull()) {
        firstImage->setText("扉页图片未找到");
        firstImage->setAlignment(Qt::AlignCenter);
        qWarning() << "扉页图片加载失败:" << firstPath;
    } else {
        firstImage->setPixmap(QPixmap(firstPath).scaled(600, 800, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
    firstImage->setStyleSheet("QLabel { background-color: #F8E8E8; }");

    firstLayout->addWidget(firstImage,0,0,1,3);

    // 导航按钮容器（放在图片内部）
    QWidget *navContainer = new QWidget(firstImage);
    navContainer->setStyleSheet("background: transparent;");
    QGridLayout *navLayout = new QGridLayout(navContainer);
    navLayout->setContentsMargins(20, 0, 20, 50);

    // 左箭头按钮
    QPushButton *prevBtn = new QPushButton("◀", navContainer);
    prevBtn->setFixedSize(40, 40);
    prevBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(159, 65, 37, 0.7);"
        "   color: #F8E8E8;"
        "   border: none;"
        "   border-radius: 20px;"
        "   font: bold 18px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(184, 92, 59, 0.8);"
        "}"
        );

    // 右箭头按钮
    QPushButton *nextBtn = new QPushButton("▶", navContainer);
    nextBtn->setFixedSize(40, 40);
    nextBtn->setStyleSheet(prevBtn->styleSheet());

    navLayout->addWidget(prevBtn,0,0,Qt::AlignVCenter | Qt::AlignLeft);
    navLayout->addWidget(nextBtn,0,0,Qt::AlignVCenter | Qt::AlignRight);
    // 退出按钮（放在图片内部）
    QPushButton *backBtn = new QPushButton("返回封面", firstImage);
    backBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(159, 65, 37, 0.7);"
        "   color: #F8E8E8;"
        "   border: 1px solid #B85C3B;"
        "   border-radius: 15px;"
        "   padding: 8px 16px;"
        "   font: bold 14px '楷体';"
        "   min-width: 100px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(184, 92, 59, 0.8);"
        "}"
        );

    // 使用布局将按钮定位在图片上
    QVBoxLayout *imageLayout = new QVBoxLayout(firstImage);
    imageLayout->addStretch();
    imageLayout->addWidget(navContainer);
    imageLayout->addSpacing(250);
    imageLayout->addWidget(backBtn, 0, Qt::AlignCenter);
    imageLayout->setContentsMargins(0, 0, 0, 30);

    firstLayout->addWidget(firstImage);
    pageStack->addWidget(firstPage);

    // 连接信号
    connect(prevBtn, &QPushButton::clicked, this, &AlbumWidget::onPreviousPageButtonClicked);
    connect(nextBtn, &QPushButton::clicked, this, &AlbumWidget::onNextPageButtonClicked);
    connect(backBtn, &QPushButton::clicked, this, [this](){
        updatePageDisplay(0);
    });
}


void AlbumWidget::setupContentPages() {
    // 为每条记录创建内容页
    for (const CheckinData& record : localRecords) {
        QWidget *contentPage = new QWidget();
        contentPage->setStyleSheet("background: transparent;");  // 设置页面背景透明

        // 主布局 - 使用网格布局方便层叠
        QGridLayout *mainLayout = new QGridLayout(contentPage);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // 背景图片（缩放至页面大小）
        QLabel *bgImage = new QLabel(contentPage);
        QString bgPath = ":/image/background2.png";
        if (QPixmap(bgPath).isNull()) {
            bgImage->setText("背景图片未找到");
            bgImage->setAlignment(Qt::AlignCenter);
            qWarning() << "背景图片加载失败:" << bgPath;
        } else {
            bgImage->setPixmap(QPixmap(bgPath).scaled(600, 800, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
        bgImage->setStyleSheet("QLabel { background-color: #F8E8E8; }");

        // 将背景图片添加到布局，占据整个单元格
        mainLayout->addWidget(bgImage, 0, 0, 1, 3);  // 行0，列0，占1行3列

        // 内容容器（放在图片内部）
        QWidget *contentContainer = new QWidget(contentPage);  // 父对象改为contentPage
        contentContainer->setStyleSheet("background: transparent;");
        QVBoxLayout *containerLayout = new QVBoxLayout(contentContainer);
        containerLayout->setContentsMargins(60, 150, 60, 60);
        containerLayout->setSpacing(15);

        // 白色半透明内容框
        QFrame *contentFrame = new QFrame(contentContainer);
        contentFrame->setStyleSheet("QFrame { background-color: rgba(255, 255, 255, 0.85); "
                                    "border-radius: 15px; border: 1px solid #B85C3B; }"
                                    "padding: 15px;" );
        QVBoxLayout *frameLayout = new QVBoxLayout(contentFrame);
        frameLayout->setContentsMargins(20, 20, 20, 20);
        frameLayout->setSpacing(15);

        // 日期标签
        QLabel *dateLabel = new QLabel("📅 " + record.date, contentFrame);
        dateLabel->setStyleSheet("QLabel { color: #9F4125; font-weight: bold; font-size: 16px; "
                                 "font-family: '楷体'; }");

        // 地点标签
        QLabel *locationLabel = new QLabel("📍 " + record.location, contentFrame);
        locationLabel->setStyleSheet("QLabel { color: #B85C3B; font-size: 14px; font-family: '楷体'; }");

        // 花名标签
        QLabel *flowerLabel = new QLabel("🌸 " + record.flowerName, contentFrame);
        flowerLabel->setStyleSheet(locationLabel->styleSheet());

        // 图片显示
        QLabel *imageLabel = new QLabel(contentFrame);
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setMinimumSize(300, 300);
        imageLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid #B85C3B; "
                                  "border-radius: 10px; padding: 5px; }");

        QString absoluteImagePath = QCoreApplication::applicationDirPath() + "/" + record.imagePath;
        if (!record.imagePath.isEmpty() && QFile::exists(absoluteImagePath)) {
            QPixmap pixmap(absoluteImagePath);
            if (!pixmap.isNull()) {
                pixmap = pixmap.scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                imageLabel->setPixmap(pixmap);
            } else {
                imageLabel->setText("⚠️ 图片加载失败");
                imageLabel->setStyleSheet("QLabel { color: #9F4125; }");
                qWarning() << "内容图片加载失败:" << absoluteImagePath;
            }
        } else {
            imageLabel->setText("🖼️ 无有效图片");
            imageLabel->setStyleSheet("QLabel { color: #B85C3B; }");
            qWarning() << "内容图片文件不存在:" << absoluteImagePath;
        }

        // 日志显示
        QLabel *logLabel = new QLabel("📝 " + record.log, contentFrame);
        logLabel->setWordWrap(true);
        logLabel->setStyleSheet("QLabel { background-color: rgba(255, 255, 255, 0.7); "
                                "color: #555; padding: 10px; border-radius: 8px; "
                                "border: 1px solid #B85C3B; font-family: '楷体'; }");

        // 添加到内容框架
        frameLayout->addWidget(dateLabel);
        frameLayout->addWidget(locationLabel);
        frameLayout->addWidget(flowerLabel);
        frameLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
        frameLayout->addWidget(logLabel);
        frameLayout->addStretch();

        // 将内容框添加到容器
        containerLayout->addWidget(contentFrame);
        containerLayout->addStretch();

        // 使用布局将内容容器定位在背景图片上
        bgImage->setLayout(new QVBoxLayout());
        bgImage->layout()->addWidget(contentContainer);

        // 将内容容器添加到布局，与背景图片重叠
        mainLayout->addWidget(contentContainer, 0, 1);  // 放在中间列

        // 左侧翻页按钮
        QPushButton *leftBtn = new QPushButton("◀", contentPage);
        leftBtn->setFixedSize(40, 40);
        leftBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: rgba(159, 65, 37, 0.7);"
            "   color: #F8E8E8;"
            "   border: none;"
            "   border-radius:20px;"
            "   font: bold 18px;"
            "}"
            "QPushButton:hover {"
            "   background-color: rgba(184, 92, 59, 0.8);"
            "}"
            );
        connect(leftBtn, &QPushButton::clicked, this, &AlbumWidget::onPreviousPageButtonClicked);

        // 右侧翻页按钮
        QPushButton *rightBtn = new QPushButton("▶", contentPage);
        rightBtn->setFixedSize(40, 40);
        rightBtn->setStyleSheet(leftBtn->styleSheet());
        connect(rightBtn, &QPushButton::clicked, this, &AlbumWidget::onNextPageButtonClicked);

        // 将按钮添加到布局，与背景图片重叠
        mainLayout->addWidget(leftBtn, 0, 0, Qt::AlignVCenter | Qt::AlignLeft);
        mainLayout->addWidget(rightBtn, 0, 2, Qt::AlignVCenter | Qt::AlignRight);

        pageStack->addWidget(contentPage);
    }

    updatePageDisplay(0);
}

void AlbumWidget::setCheckinRecords(const QVector<CheckinData>& records)
{
    // 输出传入的记录数量
    qDebug() << "接收到打卡记录数量:" << records.size();

    // 清空现有内容页（保留封面和扉页）
    while (pageStack->count() > 2) {
        QWidget* widget = pageStack->widget(2);
        pageStack->removeWidget(widget);
        delete widget;
    }

    QFile file(CHECKIN_LOG_FILE_PATH);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        QString line;
        CheckinData data;
        while (!stream.atEnd()) {
            line = stream.readLine();
            if (line.startsWith("日期: ")) {
                data.date = line.mid(3).trimmed();
            } else if (line.startsWith("地点: ")) {
                data.location = line.mid(3).trimmed();
            } else if (line.startsWith("花名: ")) {
                data.flowerName = line.mid(3).trimmed();
            } else if (line.startsWith("图片路径: ")) {
                data.imagePath = line.mid(5).trimmed();
            } else if (line.startsWith("日志: ")) {
                data.log = line.mid(3).trimmed();
                localRecords.append(data);
                data = CheckinData(); // 重置
            }
        }
        file.close();
    } else {
        qWarning() << "无法打开打卡记录文件:" << file.errorString();
    }

    // 按日期降序排序
    std::sort(localRecords.begin(), localRecords.end(), [](const CheckinData& a, const CheckinData& b) {
        return a.date > b.date;
    });
    // 输出将要显示的内容
    qDebug() << "将要显示的打卡记录:";
    for (const CheckinData& record : localRecords) {
        qDebug() << "记录内容:";
        qDebug() << "日期:" << record.date;
        qDebug() << "地点:" << record.location;
        qDebug() << "花名:" << record.flowerName;
        qDebug() << "图片路径:" << record.imagePath;
        qDebug() << "日志:" << record.log;
        qDebug() << "-------------------";
    }

    setupContentPages();
}

void AlbumWidget::updatePageDisplay(int pageIndex) {
    if (pageIndex >= 0 && pageIndex < pageStack->count()) {
        currentPage = pageIndex;
        pageStack->setCurrentIndex(pageIndex);

        // 更新页面标签和按钮显示
        if (pageIndex == 0) { // 封面
            pageLabel->hide();
            previousPageButton->hide();
            nextPageButton->hide();
        }
        else if (pageIndex == 1) { // 扉页
            pageLabel->hide();
            previousPageButton->hide();
            nextPageButton->hide();
        }
        else { // 内容页
            pageLabel->setText(QString("%1 / %2").arg(currentPage - 1).arg(pageStack->count() - 2));
            pageLabel->hide();
            previousPageButton->hide();
            nextPageButton->hide();
        }

        //更新按钮状态
        previousPageButton->setEnabled(currentPage > 0);
        nextPageButton->setEnabled(currentPage < pageStack->count() - 1);
    }
}

void AlbumWidget::onFlipButtonClicked() {
    updatePageDisplay(1); // 翻到扉页
}

void AlbumWidget::onCloseButtonClicked() {
    close();
}

void AlbumWidget::onNextPageButtonClicked() {
    updatePageDisplay(currentPage + 1);
}

void AlbumWidget::onPreviousPageButtonClicked() {
    updatePageDisplay(currentPage - 1);
}

AlbumWidget::~AlbumWidget() {}
