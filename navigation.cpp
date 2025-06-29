#include "navigation.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QPointF>
#include <QSplitter>
#include <QMessageBox>
#include <QVector>
#include <QRadioButton>
#include <QButtonGroup>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <QGraphicsScene>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsPathItem>
bool operator<(const QPoint& a, const QPoint& b) {
    return (a.x() != b.x()) ? (a.x() < b.x()) : (a.y() < b.y());
}


// 在NavigationWindow构造函数中修改样式设置
NavigationWindow::NavigationWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setWindowFlags(Qt::FramelessWindowHint);
    showFullScreen();

    // 设置整体背景风格
    this->setStyleSheet(
        "QWidget {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #F8F4E8, stop:1 #F0E6D2);"
        "}"
        );
}

void NavigationWindow::setupUI()
{
    // 创建主分割器（水平分割，左侧放按钮，右侧放地图）
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);

    // 创建控制面板区域的容器
    QWidget *controlPanel = new QWidget(mainSplitter);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);
    controlLayout->setSpacing(15); // 增加控件间距
    controlLayout->setContentsMargins(20, 20, 20, 20); // 增加边距

    // 标题样式
    QLabel *titleLabel = new QLabel("校园导航", this);
    titleLabel->setStyleSheet(
        "QLabel {"
        "   font-family: '楷体';"
        "   font-size: 32px;"
        "   font-weight: bold;"
        "   color: #6D3B1E;"
        "   padding: 10px 0;"
        "   border-bottom: 2px solid #D9C7B8;"
        "}"
        );
    controlLayout->addWidget(titleLabel, 0, Qt::AlignCenter);

    // 设置控制面板背景色
    controlPanel->setStyleSheet(
        "QWidget {"
        "   background: rgba(248, 244, 232, 0.9);" // 更透明的背景
        "   border-radius: 15px;"
        "   border: 1px solid #D9C7B8;"
        "}"
        );

    // 输入框样式
    QLabel *startLabel = new QLabel("起点:", controlPanel);
    startLabel->setStyleSheet("font-family: '微软雅黑'; font-size: 14px; color: #5A4A42;");
    QLineEdit *startEdit = new QLineEdit(controlPanel);
    startEdit->setPlaceholderText("请输入起点位置");
    startEdit->setStyleSheet(
        "QLineEdit {"
        "   font-family: '微软雅黑';"
        "   font-size: 14px;"
        "   padding: 8px;"
        "   border: 1px solid #D9C7B8;"
        "   border-radius: 8px;"
        "   background: white;"
        "}"
        "QLineEdit:focus {"
        "   border: 1px solid #9F4125;"
        "}"
        );

    QLabel *endLabel = new QLabel("终点:", controlPanel);
    endLabel->setStyleSheet("font-family: '微软雅黑'; font-size: 14px; color: #5A4A42;");
    QLineEdit *endEdit = new QLineEdit(controlPanel);
    endEdit->setPlaceholderText("请输入终点位置");
    endEdit->setStyleSheet(startEdit->styleSheet());

    // 单选按钮样式
    QLabel *modeLabel = new QLabel("导航模式:", controlPanel);
    modeLabel->setStyleSheet("font-family: '微软雅黑'; font-size: 14px; color: #5A4A42;");

    QButtonGroup *modeGroup = new QButtonGroup(controlPanel);
    QRadioButton *flowerPathRadio = new QRadioButton("赏花路径", controlPanel);
    QRadioButton *avoidPollenRadio = new QRadioButton("躲避花粉", controlPanel);

    QString radioStyle =
        "QRadioButton {"
        "   font-family: '微软雅黑';"
        "   font-size: 14px;"
        "   color: #5A4A42;"
        "   spacing: 8px;"
        "}"
        "QRadioButton::indicator {"
        "   width: 16px;"
        "   height: 16px;"
        "}"
        "QRadioButton::indicator::unchecked {"
        "   border: 1px solid #9F4125;"
        "   border-radius: 8px;"
        "}"
        "QRadioButton::indicator::checked {"
        "   border: 1px solid #9F4125;"
        "   border-radius: 8px;"
        "   background-color: #9F4125;"
        "}";

    flowerPathRadio->setStyleSheet(radioStyle);
    avoidPollenRadio->setStyleSheet(radioStyle);
    modeGroup->addButton(flowerPathRadio);
    modeGroup->addButton(avoidPollenRadio);
    flowerPathRadio->setChecked(true);

    // 月份选择样式
    QLabel *monthLabel = new QLabel("选择月份:", controlPanel);
    monthLabel->setStyleSheet("font-family: '微软雅黑'; font-size: 14px; color: #5A4A42;");
    QComboBox *monthComboBox = new QComboBox(controlPanel);
    monthComboBox->addItem("全部");
    for (int i = 1; i <= 12; ++i) {
        monthComboBox->addItem(QString::number(i) + "月");
    }
    monthComboBox->setStyleSheet(
        "QComboBox {"
        "   font-family: '微软雅黑';"
        "   font-size: 14px;"
        "   padding: 6px;"
        "   border: 1px solid #D9C7B8;"
        "   border-radius: 8px;"
        "   background: white;"
        "}"
        "QComboBox::drop-down {"
        "   subcontrol-origin: padding;"
        "   subcontrol-position: top right;"
        "   width: 20px;"
        "   border-left: 1px solid #D9C7B8;"
        "}"
        "QComboBox::down-arrow {"
        "   image: url(:/icons/down_arrow.png);"
        "   width: 12px;"
        "   height: 12px;"
        "}"
        );

    // 按钮样式
    QPushButton *navigateButton = new QPushButton("开始导航", controlPanel);
    QString buttonStyle =
        "QPushButton {"
        "   font-family: '楷体';"
        "   font-size: 18px;"
        "   color: white;"
        "   background-color: #9F4125;"
        "   border-radius: 10px;"
        "   padding: 10px 20px;"
        "   border: none;"
        "   min-width: 120px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #B85C3D;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #8A3015;"
        "}";

    navigateButton->setStyleSheet(buttonStyle);

    // 添加浮动效果
    QGraphicsDropShadowEffect *buttonEffect = new QGraphicsDropShadowEffect(navigateButton);
    buttonEffect->setBlurRadius(10);
    buttonEffect->setColor(QColor(159, 65, 37, 100));
    buttonEffect->setOffset(0, 2);
    navigateButton->setGraphicsEffect(buttonEffect);

    QPushButton *exitButton = new QPushButton("退出", controlPanel);
    exitButton->setStyleSheet(
        "QPushButton {"
        "   font-family: '楷体';"
        "   font-size: 16px;"
        "   color: #9F4125;"
        "   background-color: rgba(159, 65, 37, 0.1);"
        "   border-radius: 8px;"
        "   padding: 8px 16px;"
        "   border: 1px solid #9F4125;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(159, 65, 37, 0.2);"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(159, 65, 37, 0.3);"
        "}");

    // 添加浮动效果
    QGraphicsDropShadowEffect *exitEffect = new QGraphicsDropShadowEffect(exitButton);
    exitEffect->setBlurRadius(8);
    exitEffect->setColor(QColor(159, 65, 37, 80));
    exitEffect->setOffset(0, 1);
    exitButton->setGraphicsEffect(exitEffect);

    // 将控件添加到布局中
    controlLayout->addWidget(startLabel);
    controlLayout->addWidget(startEdit);
    controlLayout->addWidget(endLabel);
    controlLayout->addWidget(endEdit);
    controlLayout->addWidget(modeLabel);
    controlLayout->addWidget(flowerPathRadio);
    controlLayout->addWidget(avoidPollenRadio);
    controlLayout->addWidget(monthLabel);
    controlLayout->addWidget(monthComboBox);

    // 添加弹簧将按钮推到顶部
    controlLayout->addStretch();

    // 创建按钮容器，使按钮居中
    QWidget *buttonContainer = new QWidget(controlPanel);
    QVBoxLayout *buttonLayout = new QVBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(15);
    buttonLayout->addWidget(navigateButton, 0, Qt::AlignCenter);
    buttonLayout->addWidget(exitButton, 0, Qt::AlignCenter);

    controlLayout->addWidget(buttonContainer);

    // 设置控制面板的最小宽度
    controlPanel->setMinimumWidth(250);

    // 创建地图视图并添加到分割器
    MapWidget *mapWidget = new MapWidget(mainSplitter);
    mapWidget->loadMap(":/map/pku_map.jpg");
    mapWidget->loadMapFromTxt(":/mapData/map_data.txt");
    mapWidget->initStructureData();
    mapWidget->addFlowerIcons();
    mapWidget->linkFlowersToLocations(flower.getAllFlowers());

    // 将控制面板和地图视图添加到分割器
    mainSplitter->addWidget(controlPanel);
    mainSplitter->addWidget(mapWidget);

    // 设置分割器的初始大小比例
    mainSplitter->setSizes({300, 600});

    // 将分割器设置为主窗口的中心部件
    setCentralWidget(mainSplitter);

    // 连接信号槽
    connect(monthComboBox, QOverload<int>::of(&QComboBox::activated),
            mapWidget, &MapWidget::setSelectedMonth);
    connect(navigateButton, &QPushButton::clicked,
            this, &NavigationWindow::onNavigateClicked);
    connect(exitButton, &QPushButton::clicked,
            this, &NavigationWindow::onExitClicked);

    // 保存控件指针到成员变量
    this->startEdit = startEdit;
    this->endEdit = endEdit;
    this->flowerPathRadio = flowerPathRadio;
    this->avoidPollenRadio = avoidPollenRadio;
    this->monthComboBox = monthComboBox;
    this->mapWidget = mapWidget;
}

// 美化错误弹窗
void NavigationWindow::onNavigateClicked()
{
    QString start = startEdit->text().trimmed();
    QString end = endEdit->text().trimmed();
    bool avoidFlowers = avoidPollenRadio->isChecked();
    int month = monthComboBox->currentIndex();

    if(start.isEmpty() || end.isEmpty()) {
        // 创建自定义消息框
        QDialog *errorDialog = new QDialog(this);
        errorDialog->setWindowTitle("提示");
        errorDialog->setFixedSize(300, 150);
        errorDialog->setStyleSheet(
            "QDialog {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F8F4E8, stop:1 #F0E6D2);"
            "   border: 2px solid #9F4125;"
            "   border-radius: 10px;"
            "}"
            );

        QVBoxLayout *layout = new QVBoxLayout(errorDialog);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(15);

        QLabel *iconLabel = new QLabel(errorDialog);
        iconLabel->setPixmap(QPixmap(":/icons/warning.png").scaled(40, 40));
        iconLabel->setAlignment(Qt::AlignCenter);

        QLabel *messageLabel = new QLabel("请输入起点和终点", errorDialog);
        messageLabel->setStyleSheet(
            "QLabel {"
            "   font-family: '微软雅黑';"
            "   font-size: 16px;"
            "   color: #5A4A42;"
            "}"
            );
        messageLabel->setAlignment(Qt::AlignCenter);

        QPushButton *okButton = new QPushButton("确定", errorDialog);
        okButton->setStyleSheet(
            "QPushButton {"
            "   font-family: '楷体';"
            "   font-size: 16px;"
            "   color: white;"
            "   background-color: #9F4125;"
            "   border-radius: 8px;"
            "   padding: 8px 16px;"
            "   min-width: 80px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #B85C3D;"
            "}"
            );
        okButton->setFixedWidth(100);

        connect(okButton, &QPushButton::clicked, errorDialog, &QDialog::accept);

        layout->addWidget(iconLabel);
        layout->addWidget(messageLabel);
        layout->addWidget(okButton, 0, Qt::AlignCenter);

        errorDialog->exec();
        return;
    }

    QPointF startPos = getLocationByName(start);
    QPointF endPos = getLocationByName(end);

    if (startPos.isNull()) {
        showCustomMessageBox("错误", QString("找不到起点: %1").arg(start));
        return;
    }
    if (endPos.isNull()) {
        showCustomMessageBox("错误", QString("找不到终点: %1").arg(end));
        return;
    }

    QList<QPointF> path = findPath(startPos, endPos, avoidFlowers, month);
    if(path.isEmpty()) {
        showCustomMessageBox("提示", "无法找到可行路径");
    } else {
        drawPath(path);
    }
}

// 自定义消息框函数
void NavigationWindow::showCustomMessageBox(const QString &title, const QString &message)
{
    QDialog *messageBox = new QDialog(this);
    messageBox->setWindowTitle(title);
    messageBox->setFixedSize(350, 180);
    messageBox->setStyleSheet(
        "QDialog {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F8F4E8, stop:1 #F0E6D2);"
        "   border: 2px solid #9F4125;"
        "   border-radius: 10px;"
        "}"
        );

    QVBoxLayout *layout = new QVBoxLayout(messageBox);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    QLabel *iconLabel = new QLabel(messageBox);
    iconLabel->setPixmap(QPixmap(":/icons/info.png").scaled(40, 40));
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel *messageLabel = new QLabel(message, messageBox);
    messageLabel->setStyleSheet(
        "QLabel {"
        "   font-family: '微软雅黑';"
        "   font-size: 16px;"
        "   color: #5A4A42;"
        "}"
        );
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setWordWrap(true);

    QPushButton *okButton = new QPushButton("确定", messageBox);
    okButton->setStyleSheet(
        "QPushButton {"
        "   font-family: '楷体';"
        "   font-size: 16px;"
        "   color: white;"
        "   background-color: #9F4125;"
        "   border-radius: 8px;"
        "   padding: 8px 16px;"
        "   min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #B85C3D;"
        "}"
        );
    okButton->setFixedWidth(100);

    connect(okButton, &QPushButton::clicked, messageBox, &QDialog::accept);

    layout->addWidget(iconLabel);
    layout->addWidget(messageLabel);
    layout->addWidget(okButton, 0, Qt::AlignCenter);

    messageBox->exec();
}

// 获取精确匹配的地点坐标
QPointF NavigationWindow::getLocationByName(const QString &name) const
{
    // 获取地图数据中的所有地点
    const QVector<Location>& locations = mapWidget->mapData.locations;

    // 精确匹配（完全匹配名称）
    for (const Location& loc : locations) {
        if (loc.name == name) {
            return loc.position;
        }
    }

    return QPointF(); // 返回无效坐标
}


// 检查是否在花期
bool NavigationWindow::isFlowering(const FlowerInfo* flower, int month) const
{
    if (month == 0) return true; // "全部"月份

    QString monthStr = QString::number(month);
    return flower->florescence.contains(monthStr);
}

QList<QPointF> NavigationWindow::findPath(const QPointF &start, const QPointF &end,
                                          bool avoidFlowers, int month)
{
    // 1. 获取地图数据副本
    MapData pathMap = mapWidget->mapData;

    for (const Location& loc : pathMap.locations) {
        for (const FlowerInfo* flower : loc.flowers) {
            // 检查是否在花期
            if (isFlowering(flower, month)) {
                // 将花的位置转换为地图坐标
                QPointF gridPos = loc.position;
                int x = gridPos.x();  // 获取 x 坐标
                int y = gridPos.y();  // 获取 y 坐标
                pathMap.charMap[x][y] = '3';
            }
        }
    }

    // 3. 使用路径查找算法 (这里使用A*算法示例)
    if(avoidFlowers){
        return avoidPath(start, end, pathMap); //避开花
    }
    else{
        return flowerPath(start, end, pathMap);  //赏花
    }
}
//启发函数
int NavigationWindow::heuristic(const QPoint& a, const QPoint& b)
{
    return 10 * (abs(a.x() - b.x()) + abs(a.y() - b.y()));
}
QList<QPointF> NavigationWindow::flowerPath(const QPointF& start, const QPointF& end, const MapData& mapData)
{
    QPoint startGrid = start.toPoint();
    QPoint endGrid = end.toPoint();

    // 定义8个移动方向
    const QList<QPoint> directions = {
        {-1,-1}, {-1,0}, {-1,1},
        {0,-1},          {0,1},
        {1,-1},  {1,0},  {1,1}
    };
    // 使用智能指针管理节点内存
    struct NodeWrapper {
        std::shared_ptr<WeightedNode> node;
        bool operator<(const NodeWrapper& other) const {
            return *node < *(other.node);
        }
    };

    // 优先队列现在存储NodeWrapper
    std::priority_queue<NodeWrapper> openSet;
    auto startNode = std::make_shared<WeightedNode>(startGrid, 0, heuristic(startGrid, endGrid));
    openSet.push({startNode});

    // 关闭列表记录最佳分数
    std::unordered_map<int, std::unordered_map<int, int>> bestScores;
    bestScores[startGrid.x()][startGrid.y()] = startNode->score;

    // 节点存储改用shared_ptr
    std::unordered_map<int, std::unordered_map<int, std::shared_ptr<WeightedNode>>> nodeMap;
    nodeMap[startGrid.x()][startGrid.y()] = startNode;

    while (!openSet.empty()) {
        auto currentWrapper = openSet.top();
        auto current = currentWrapper.node;
        openSet.pop();
        qDebug() << "Processing node:" << current->pos << "score:" << current->score;
        if (current->pos == endGrid) {
            QList<QPointF> path;
            auto node = current;
            while (node) {
                path.prepend(QPointF(node->pos));
                node = node->parent;
            }
            qDebug() << "Path found with" << current->threeCount << "flowers";
            return path;
        }

        if (current->score > bestScores[current->pos.x()][current->pos.y()]) {
            continue;
        }

        for (const QPoint& dir : directions) {
            QPoint neighbor = current->pos + dir;

            if (neighbor.x() < 0 || neighbor.x() >= 138 ||
                neighbor.y() < 0 || neighbor.y() >= 104) {
                continue;
            }

            char cell = mapData.charMap[neighbor.x()][neighbor.y()];
            if (cell == '0') continue;

            int newThreeCount = current->threeCount;
            std::set<QPoint> countedFlowers;

            for (const QPoint& d : directions) {
                QPoint around = neighbor + d;
                if (countedFlowers.count(around)) continue;

                if (around.x() >= 0 && around.x() < 138 &&
                    around.y() >= 0 && around.y() < 104)
                {
                    if (mapData.charMap[around.x()][around.y()] == '3') {
                        // 只有当花在路径前方时才计数
                        if (heuristic(around, endGrid) < heuristic(current->pos, endGrid)) {
                            newThreeCount++;
                            countedFlowers.insert(around);
                        }
                    }
                }
            }


            int moveCost = (dir.x() != 0 && dir.y() != 0) ? 14 : 10;
            int newG = current->g + moveCost;
            int newH = heuristic(neighbor, endGrid);

            // 确保分数不会过度偏离
            if (newG > 100000) {  // 防止g值过大
                continue;
            }

            // 非线性奖励计算
            int flowerReward = newThreeCount * 8 + newThreeCount * newThreeCount;
            int newScore = newG + newH - flowerReward;

            // 创建新节点（修改WeightedNode构造函数调用）
            auto newNode = std::make_shared<WeightedNode>(
                neighbor, newG, newH, newThreeCount, current);
            newNode->score = newScore; // 直接设置计算好的分数

            // 更新最佳路径
            if (!bestScores[neighbor.x()].count(neighbor.y()) ||
                newScore < bestScores[neighbor.x()][neighbor.y()])
            {
                bestScores[neighbor.x()][neighbor.y()] = newScore;
                nodeMap[neighbor.x()][neighbor.y()] = newNode;
                openSet.push({newNode});

                qDebug() << "Adding neighbor:" << neighbor
                         << "flowers:" << newThreeCount
                         << "reward:" << flowerReward
                         << "total score:" << newScore;
            }
        }
    }
    return QList<QPointF>();
}

QList<QPointF> NavigationWindow::avoidPath(const QPointF& start, const QPointF& end, const MapData& mapData)
{
    // 转换起点和终点
    QPoint startGrid = start.toPoint();
    QPoint endGrid = end.toPoint();

    // 定义8个移动方向
    const QList<QPoint> directions = {
        {-1,-1}, {-1,0}, {-1,1},
        {0,-1},          {0,1},
        {1,-1},  {1,0},  {1,1}
    };

    // 使用智能指针管理节点内存
    struct NodeWrapper {
        std::shared_ptr<AvoidanceNode> node;
        bool operator<(const NodeWrapper& other) const {
            return *node < *(other.node);
        }
    };

    // 优先队列现在存储NodeWrapper
    std::priority_queue<NodeWrapper> openSet;
    auto startNode = std::make_shared<AvoidanceNode>(startGrid, 0, heuristic(startGrid, endGrid));
    openSet.push({startNode});

    // 关闭列表记录最佳分数
    std::unordered_map<int, std::unordered_map<int, int>> bestScores;
    bestScores[startGrid.x()][startGrid.y()] = startNode->score;

    // 节点存储改用shared_ptr
    std::unordered_map<int, std::unordered_map<int, std::shared_ptr<AvoidanceNode>>> nodeMap;
    nodeMap[startGrid.x()][startGrid.y()] = startNode;

    while (!openSet.empty()) {
        auto currentWrapper = openSet.top();
        auto current = currentWrapper.node;
        openSet.pop();
        qDebug() << "Processing node:" << current->pos << "score:" << current->score;
        if (current->pos == endGrid) {
            QList<QPointF> path;
            auto node = current;
            while (node) {
                qDebug()<<"here";
                path.prepend(QPointF(node->pos));
                node = node->parent; // 使用weak_ptr的lock()方法获取shared_ptr
            }
            qDebug()<<"finish";
            return path;
        }

        if (current->score > bestScores[current->pos.x()][current->pos.y()]) {
            continue;
        }

        for (const QPoint& dir : directions) {
            QPoint neighbor = current->pos + dir;

            if (neighbor.x() < 0 || neighbor.x() >= 138 ||
                neighbor.y() < 0 || neighbor.y() >= 104) {
                continue;
            }

            char cell = mapData.charMap[neighbor.x()][neighbor.y()];
            if (cell == '0') continue;

            int moveCost = (dir.x() != 0 && dir.y() != 0) ? 14 : 10;
            int newG = current->g + moveCost;
            int newH = heuristic(neighbor, endGrid);

            // 计算新的惩罚值 (检查周围8格是否有3)
            int newPenalty = current->penalty;
            for (const QPoint& d : directions) {
                QPoint around = neighbor + d;
                if (around.x() >= 0 && around.x() < 138 &&
                    around.y() >= 0 && around.y() < 104) {
                    if (mapData.charMap[around.x()][around.y()] == '3') {
                        newPenalty += 20; // 每个附近的3增加惩罚值
                        break; // 每个格子只计一次3的贡献
                    }
                }
            }

            auto newNode = std::make_shared<AvoidanceNode>(
                neighbor, newG, newH, newPenalty, current);

            if (!bestScores[neighbor.x()][neighbor.y()] ||
                newNode->score < bestScores[neighbor.x()][neighbor.y()]) {
                bestScores[neighbor.x()][neighbor.y()] = newNode->score;
                nodeMap[neighbor.x()][neighbor.y()] = newNode;
                openSet.push({newNode});
            }
        }
    }
    return QList<QPointF>(); // 没有找到路径
}
void NavigationWindow::drawPath(const QList<QPointF>& path) {
    // 清除旧路径
    clearPath();

    if (path.isEmpty()) {
        return;
    }

    // 获取场景和保存当前路径
    QGraphicsScene* scene = mapWidget->getScene();
    currentPath = path;

    // 创建平滑的贝塞尔曲线路径
    QPainterPath painterPath;

    // 转换并添加第一个点（起点）
    QPointF firstPixelPos = convertToPixelCoordinates(path.first());
    painterPath.moveTo(firstPixelPos);

    // 当路径只有1个点时特殊处理（只画起点）
    if (path.size() == 1) {
        addMarker(firstPixelPos, Qt::green); // 起点标记
        return;
    }

    // 使用贝塞尔曲线平滑连接路径点
    for (int i = 1; i < path.size(); ++i) {
        QPointF prevPixelPos = convertToPixelCoordinates(path[i-1]);
        QPointF currPixelPos = convertToPixelCoordinates(path[i]);

        // 计算控制点（取前后点之间的30%和70%位置）
        QPointF controlPoint1 = prevPixelPos + (currPixelPos - prevPixelPos) * 0.3;
        QPointF controlPoint2 = prevPixelPos + (currPixelPos - prevPixelPos) * 0.7;

        // 添加三次贝塞尔曲线段
        painterPath.cubicTo(controlPoint1, controlPoint2, currPixelPos);
    }

    // 创建并配置路径图形项
    pathItem = new QGraphicsPathItem(painterPath);
    pathItem->setCacheMode(QGraphicsItem::DeviceCoordinateCache); // 提高渲染性能

    // 设置路径样式
    QPen pathPen(QColor(0, 150, 255, 220), 5.0); // 半透明蓝色，5像素宽
    pathPen.setCapStyle(Qt::RoundCap);   // 圆角线帽
    pathPen.setJoinStyle(Qt::RoundJoin); // 圆角连接
    pathPen.setCosmetic(true);           // 不受视图缩放影响
    pathItem->setPen(pathPen);

    // 添加到场景
    scene->addItem(pathItem);

    // 添加起点标记（绿色）
    addMarker(firstPixelPos, Qt::green);

    // 添加终点标记（红色）
    QPointF lastPixelPos = convertToPixelCoordinates(path.last());
    addMarker(lastPixelPos, Qt::red);

    // 确保路径可见
    mapWidget->ensureVisible(pathItem);
}
// 添加标记的辅助函数
void NavigationWindow::addMarker(const QPointF& pixelPos, const QColor& color) {
    QGraphicsScene* scene = mapWidget->getScene();

    // 创建圆形标记
    QGraphicsEllipseItem* marker = new QGraphicsEllipseItem(-8, -8, 16, 16);
    marker->setPos(pixelPos);
    marker->setBrush(color);
    marker->setPen(Qt::NoPen);

    // 添加发光效果
    QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect();
    glow->setBlurRadius(10);
    glow->setColor(color);
    glow->setOffset(0);
    marker->setGraphicsEffect(glow);

    // 添加到场景并保存引用
    scene->addItem(marker);
    if (color == Qt::green) {
        startMarker = marker;
    } else {
        endMarker = marker;
    }
}
// 坐标转换辅助函数
QPointF NavigationWindow::convertToPixelCoordinates(const QPointF& gridPoint) {
    return QPointF(
        (gridPoint.y() + 1) * 36 - 18,  // 注意x/y交换
        (gridPoint.x() + 1) * 36 - 18
        );
}
void NavigationWindow::clearPath() {
    QGraphicsScene* scene = mapWidget->getScene();

    if (pathItem) {
        scene->removeItem(pathItem);
        delete pathItem;
        pathItem = nullptr;
    }

    if (startMarker) {
        scene->removeItem(startMarker);
        delete startMarker;
        startMarker = nullptr;
    }

    if (endMarker) {
        scene->removeItem(endMarker);
        delete endMarker;
        endMarker = nullptr;
    }

    currentPath.clear();
}
void NavigationWindow::onExitClicked()
{
    close();
}
