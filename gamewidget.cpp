#include "gamewidget.h"
#include "mapwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFile>
#include <QPixmap>
#include <QRandomGenerator>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QMediaPlayer>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent),
    mapWidget(nullptr),
    currentFlowerId(-1)
{
    setWindowTitle("花卉识别游戏");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint); // 全屏无边框

    // 获取屏幕尺寸并设置全屏
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        setGeometry(screen->geometry());
    }

    setupUI();
}

void GameWidget::setFlowers(const QVector<FlowerInfo> &flowers) {
    this->flowers = flowers;
    if (mapWidget) {
        mapWidget->linkFlowersToLocations(flowers);
        mapWidget->addFlowerIcons();
    }
}

void GameWidget::setupUI()
{
    // 设置复古风格背景
    setStyleSheet(
        "GameWidget {"
        "   background-color: #2E1E0F;"  // 深棕色背景
        "   border: 3px solid #6B4423;"  // 木质边框
        "}"
        );

    // 创建主分割器（水平分割）
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setHandleWidth(2);
    mainSplitter->setStyleSheet(
        "QSplitter::handle {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "       stop:0 #6B4423, stop:0.5 #8B5A2B, stop:1 #6B4423);"
        "}"
        );

    // 创建左侧控制面板
    QWidget *leftPanel = new QWidget(mainSplitter);
    leftPanel->setMinimumWidth(350);
    leftPanel->setStyleSheet(
        "QWidget {"
        "   background-color: #3A2515;"  // 深棕色面板
        "   border-right: 2px solid #6B4423;"  // 右侧边框
        "}"
        );

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(30, 30, 30, 30);
    leftLayout->setSpacing(20);

    // 游戏标题
    QLabel *titleLabel = new QLabel("花卉识别挑战", leftPanel);
    titleLabel->setStyleSheet(
        "QLabel {"
        "   color: #E8C9A7;"  // 米黄色文字
        "   font: bold 28px '楷体';"  // 加大标题字号
        "   qproperty-alignment: AlignCenter;"
        "   border-bottom: 2px solid #6B4423;"
        "   padding-bottom: 15px;"  // 增加底部间距
        "   margin-bottom: 20px;"  // 增加下边距
        "}"
        );

    // 欢迎引导语
    QLabel *welcomeLabel = new QLabel(leftPanel);
    welcomeLabel->setWordWrap(true);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet(
        "QLabel {"
        "   color: #E8C9A7;"  // 米黄色文字
        "   font: italic 18px '楷体';"  // 斜体
        "   padding: 10px;"
        "   margin-bottom: 10px;"  // 底部间距
        "}"
        );
    welcomeLabel->setText("欢迎来到花卉识别挑战！\n让我们一起探索校园花卉的奥秘");

    // 游戏规则说明
    QLabel *ruleLabel = new QLabel(leftPanel);
    ruleLabel->setWordWrap(true);
    ruleLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ruleLabel->setStyleSheet(
        "QLabel {"
        "   color: #E8C9A7;"  // 米黄色文字
        "   font: 16px '楷体';"
        "   padding: 15px;"  // 增加内边距
        "   background-color: rgba(107, 68, 35, 0.3);"  // 半透明背景
        "   border-radius: 10px;"  // 圆角边框
        "   margin-top: 10px;"  // 上边距
        "}"
        );

    QString gameRules =
        "游戏规则：\n\n"
        "1. 点击「开始游戏」后，左侧将显示随机花卉图片\n"
        "2. 仔细观察花卉特征，判断其品种\n"
        "3. 在右侧地图上找到该花卉对应的位置\n"
        "4. 点击地图上的花卉图标\n"
        "5. 从列表中选择正确的花卉名称\n\n"
        "✔ 答对将获得积分\n"
        "✘ 答错会显示正确答案\n"
        "🔄 点击「切换花卉」可获取新题目";

    ruleLabel->setText(gameRules);

    // 创建复古风格按钮（垂直排列，加大样式）
    startButton = new QPushButton("开 始 游 戏", leftPanel);
    exitButton = new QPushButton("退 出 游 戏", leftPanel);

    QString buttonStyle =
        "QPushButton {"
        "   background-color: #6B4423;"  // 木质按钮
        "   color: #E8C9A7;"             // 米黄色文字
        "   border: 2px solid #8B5A2B;"  // 边框
        "   border-radius: 10px;"        // 更大圆角
        "   font: bold 20px '楷体';"     // 加大字体
        "   padding: 12px 0;"            // 垂直内边距
        "   min-width: 170px;"           // 适当宽度
        "   min-height: 25px;"           // 更高按钮
        "}"
        "QPushButton:hover {"
        "   background-color: #8B5A2B;"  // 悬停颜色
        "}"
        "QPushButton:pressed {"
        "   background-color: #4A3310;"  // 按下颜色
        "}";

    startButton->setStyleSheet(buttonStyle);
    exitButton->setStyleSheet(buttonStyle);

    // 按钮容器（垂直布局）
    QWidget *buttonContainer = new QWidget(leftPanel);
    QVBoxLayout *buttonLayout = new QVBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(0, 20, 0, 0);  // 上边距
    buttonLayout->setSpacing(30);  // 按钮间距

    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(exitButton);
    buttonLayout->addStretch();

    // 添加组件到左侧布局
    leftLayout->addWidget(titleLabel);
    leftLayout->addWidget(welcomeLabel);
    leftLayout->addStretch(1);  // 弹性空间
    leftLayout->addWidget(ruleLabel);
    leftLayout->addStretch(1);  // 弹性空间
    leftLayout->addWidget(buttonContainer, 0, Qt::AlignHCenter);  // 水平居中

    // 创建地图部件
    mapWidget = new MapWidget(mainSplitter);
    mapWidget->loadMap(":/map/pku_map.jpg");
    mapWidget->loadMapFromTxt(":/mapData/map_data.txt");
    mapWidget->initStructureData();
    mapWidget->setStyleSheet("background-color: #F5E6D3;");  // 浅米色地图背景

    // 如果有花卉数据，加载花卉图标
    if (!flowers.isEmpty()) {
        mapWidget->linkFlowersToLocations(flowers);
        mapWidget->addFlowerIcons();
    }

    // 将左右部件添加到分割器
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(mapWidget);
    mainSplitter->setStretchFactor(1, 1);  // 地图区域可拉伸

    // 设置主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(mainSplitter);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    // 连接信号槽
    connect(startButton, &QPushButton::clicked, this, &GameWidget::startGame);
    connect(exitButton, &QPushButton::clicked, this, &GameWidget::closeGame);
    connect(mapWidget, &MapWidget::flowerSelectedInQuiz, this, &GameWidget::onFlowerClicked);
}

void GameWidget::startGame()
{
    startButton->hide();
    exitButton->hide();

    // 创建游戏区域
    QWidget *gamePanel = qobject_cast<QWidget*>(startButton->parent());
    QVBoxLayout *leftLayout = qobject_cast<QVBoxLayout*>(gamePanel->layout());

    // 花卉图片显示区域
    QWidget *imageContainer = new QWidget(gamePanel);
    imageContainer->setStyleSheet(
        "QWidget {"
        "   background-color: #3A2515;"
        "   border: 2px solid #6B4423;"
        "   border-radius: 10px;"
        "}"
        );

    QVBoxLayout *imageLayout = new QVBoxLayout(imageContainer);
    imageLayout->setContentsMargins(15, 15, 15, 15);
    imageLayout->setSpacing(15);

    // 花卉图片标签
    imageLabel = new QLabel(imageContainer);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(200, 200);
    imageLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #F5E6D3;"  // 浅米色背景
        "   border: 2px solid #6B4423;"
        "   border-radius: 8px;"
        "   padding: 5px;"
        "}"
        );

    // 操作按钮
    QWidget *buttonContainer = new QWidget(imageContainer);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(15);

    changeButton = new QPushButton("切换花卉", buttonContainer);
    closeButton = new QPushButton("结束游戏", buttonContainer);

    QString gameButtonStyle =
        "QPushButton {"
        "   background-color: #6B4423;"
        "   color: #E8C9A7;"
        "   border: 2px solid #8B5A2B;"
        "   border-radius: 6px;"
        "   font: bold 16px '楷体';"
        "   padding: 8px;"
        "   min-width: 100px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #8B5A2B;"
        "}";

    changeButton->setStyleSheet(gameButtonStyle);
    closeButton->setStyleSheet(gameButtonStyle);

    buttonLayout->addWidget(changeButton);
    buttonLayout->addWidget(closeButton);

    imageLayout->addWidget(imageLabel);
    imageLayout->addWidget(buttonContainer);

    leftLayout->insertWidget(1, imageContainer, 0, Qt::AlignTop);

    // 连接按钮信号
    connect(changeButton, &QPushButton::clicked, this, &GameWidget::changeImage);
    connect(closeButton, &QPushButton::clicked, this, &GameWidget::closeGame);

    // 设置游戏模式
    mapWidget->setQuizMode(true);
    shownFlowerIds.clear();
    showRandomFlower();
}

void GameWidget::onFlowerClicked(int flowerId)
{
    if (currentFlowerId == -1) return;

    // 查找花卉名称
    QString selectedName, correctName;
    for (const FlowerInfo &flower : flowers) {
        if (flower.id == flowerId) selectedName = flower.name;
        if (flower.id == currentFlowerId) correctName = flower.name;
    }

    // 创建复古风格消息框
    QMessageBox msgBox;
    msgBox.setStyleSheet(
        "QMessageBox {"
        "   background-color: #3A2515;"
        "   color: #E8C9A7;"
        "}"
        "QMessageBox QLabel {"
        "   color: #E8C9A7;"
        "   font: 16px '楷体';"
        "}"
        "QMessageBox QPushButton {"
        "   background-color: #6B4423;"
        "   color: #E8C9A7;"
        "   border: 1px solid #8B5A2B;"
        "   padding: 5px 15px;"
        "   min-width: 80px;"
        "}"
        );

    if (flowerId == currentFlowerId) {
        msgBox.setWindowTitle("恭喜！");
        msgBox.setText(QString("✓ 回答正确！\n\n你找到了: %1").arg(selectedName));
        msgBox.setIconPixmap(QPixmap(":/icons/correct.png").scaled(64, 64));
    } else {
        msgBox.setWindowTitle("再接再厉");
        msgBox.setText(QString("✗ 回答错误\n\n你选择了: %1\n正确答案: %2")
                           .arg(selectedName)
                           .arg(correctName));
        msgBox.setIconPixmap(QPixmap(":/icons/wrong.png").scaled(64, 64));
    }

    QPushButton *continueButton = msgBox.addButton("继续", QMessageBox::AcceptRole);
    continueButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #8B5A2B;"
        "   font-weight: bold;"
        "}"
        );

    msgBox.exec();
    showRandomFlower();
}
void GameWidget::showRandomFlower() {
    if (flowers.isEmpty()) {
        QMessageBox::warning(this, "错误", "没有可用的花卉数据！");
        return;
    }

    // 如果已经显示过所有花卉，重置记录
    if (shownFlowerIds.size() >= flowers.size()) {
        shownFlowerIds.clear();
    }

    // 随机选择一个未显示过的花卉
    int index;
    do {
        index = QRandomGenerator::global()->bounded(flowers.size());
    } while (shownFlowerIds.contains(flowers[index].id) && shownFlowerIds.size() < flowers.size());

    currentFlowerId = flowers[index].id;
    shownFlowerIds.append(currentFlowerId);

    // 加载并显示花卉图片
    QString imagePath = QString(":/flowers/flowerimage/%1.png").arg(currentFlowerId);
    if (QFile::exists(imagePath)) {
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            imageLabel->setPixmap(pixmap.scaled(280, 280, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            qWarning() << "Failed to load pixmap from:" << imagePath;
            imageLabel->setText("图片加载失败");
        }
    } else {
        qWarning() << "Image file not found:" << imagePath;
        imageLabel->setText("图片未找到");
    }
}

void GameWidget::changeImage() {
    showRandomFlower();
}

void GameWidget::closeGame() {
    mapWidget->setQuizMode(false);  // 退出游戏模式
    this->close();
}
