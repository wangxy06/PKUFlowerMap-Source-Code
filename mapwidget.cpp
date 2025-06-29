#include "mapwidget.h"
#include "flowerdata.h"
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
// 为QPointF定义哈希函数
inline uint qHash(const QPointF &key, uint seed = 0)
{
    // 使用qHash组合x和y坐标的哈希值
    return qHash(key.x(), seed) ^ (qHash(key.y(), seed) << 1);
}


MapWidget::MapWidget(QWidget *parent) : QGraphicsView(parent)
{
    scene = new QGraphicsScene(this);
    this->setScene(scene);
    initStructureData();       // 初始化建筑类数据
    this->setRenderHint(QPainter::Antialiasing);
    this->setTransformationAnchor(AnchorUnderMouse);  // 缩放时以鼠标为中心
    //this->setDragMode(ScrollHandDrag);  // 允许拖拽移动地图
    this->setDragMode(NoDrag); // 默认禁用拖拽模式，允许点击事件
    checkinRecords = QVector<CheckinData>();
}
void MapWidget::loadMap(const QString &imagePath) //加载地图
{


    // 加载地图
    QPixmap mapPixmap(imagePath);
    if(mapPixmap.isNull()) {
        qDebug() << "Failed to load map image:" << imagePath;
        return;
    }

    mapItem = scene->addPixmap(mapPixmap);
    scene->setSceneRect(mapPixmap.rect());
    this->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    currentScale = 1.0;  // 重置缩放比例
    scale(36,36); //wxy:这行用来放大到合适的大小，否则它自适应显示的图片太小，不信删了试试
    addFlowerIcons(); // 初始化时加载全部花卉图标
}


void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) &&
        (event->modifiers() & Qt::ControlModifier)) {
        if (dragMode() != ScrollHandDrag) {
            this->setDragMode(ScrollHandDrag);
            // 重新触发鼠标按下事件以启用拖拽
            QMouseEvent fakePress(QEvent::MouseButtonPress,
                                  event->pos(), Qt::LeftButton,
                                  Qt::LeftButton, event->modifiers());
            QGraphicsView::mousePressEvent(&fakePress);
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}
void MapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF scenePos = this->mapToScene(event->pos());

        if (isQuizMode) {
            // 游戏模式下的处理
            handleQuizClick(scenePos);
        } else {
            // 原有主页面处理逻辑
            handleMapClick(scenePos);
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void MapWidget::handleQuizClick(const QPointF& clickPos)
{
    int nearestLocationNum = -1;
    qreal minDistance = std::numeric_limits<qreal>::max();

    // 找到最近的建筑位置
    for (auto it = mapData.locations.begin(); it != mapData.locations.end(); ++it) {
        qreal distance = ((*it).displayPos - clickPos).manhattanLength();
        if (distance < minDistance) {
            minDistance = distance;
            nearestLocationNum = it - mapData.locations.begin();
        }
    }

    if (nearestLocationNum == -1) return;

    const auto& flowers = mapData.locations[nearestLocationNum].flowers;
    if (flowers.isEmpty()) return;

    if (flowers.size() == 1) {
        // 只有一种花，直接判断
        emit flowerSelectedInQuiz(flowers[0]->id);
    } else {
        // 多种花，弹出选择对话框
        QDialog dialog(this);
        dialog.setWindowTitle("选择花卉");
        dialog.setFixedSize(200, 300); // 固定对话框大小

        // 设置复古风格
        dialog.setStyleSheet(
            "QDialog {"
            "   background-color: #3A2515;" // 深棕色背景
            "   border: 3px solid #6B4423;" // 木质边框
            "}"
            "QListWidget {"
            "   background-color: #2E1E0F;"
            "   color: #E8C9A7;"
            "   border: 1px solid #6B4423;"
            "   font: 16px '楷体';"
            "   padding: 5px;"
            "   border-radius: 5px;"
            "}"
            "QListWidget::item {"
            "   padding: 10px;"
            "   border-bottom: 1px solid #6B4423;"
            "}"
            "QListWidget::item:hover {"
            "   background-color: #4A3310;"
            "}"
            "QListWidget::item:selected {"
            "   background-color: #6B4423;"
            "   color: #E8C9A7;"
            "}"
            );

        QVBoxLayout layout(&dialog);
        layout.setContentsMargins(20, 20, 20, 20);
        layout.setSpacing(20);

        QListWidget listWidget(&dialog);
        listWidget.setSelectionMode(QAbstractItemView::SingleSelection);

        for (const auto* flower : flowers) {
            QListWidgetItem *item = new QListWidgetItem(flower->name);
            item->setTextAlignment(Qt::AlignCenter);
            listWidget.addItem(item);
        }

        layout.addWidget(&listWidget);

        QPushButton okButton("确 定", &dialog);
        okButton.setStyleSheet(
            "QPushButton {"
            "   background-color: #6B4423;"
            "   color: #E8C9A7;"
            "   border: 2px solid #8B5A2B;"
            "   border-radius: 8px;"
            "   padding: 10px 20px;"
            "   font: bold 16px '楷体';"
            "   min-width: 120px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #8B5A2B;"
            "}"
            );

        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        buttonLayout->addWidget(&okButton);
        buttonLayout->addStretch();

        layout.addLayout(buttonLayout);

        connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

        if (dialog.exec() == QDialog::Accepted) {
            int selectedRow = listWidget.currentRow();
            if (selectedRow >= 0) {
                emit flowerSelectedInQuiz(flowers[selectedRow]->id);
            }
        }
    }
}

void MapWidget::wheelEvent(QWheelEvent *event)// 用滚轮缩放
{

    qreal scaleFactor = 1.1;  // 缩放步长

    // 根据滚轮方向决定放大还是缩小
    if (event->angleDelta().y() > 0) {
        // 放大
        if (currentScale * scaleFactor <= maxScale) {
            scaleView(scaleFactor);
        }
    } else {
        // 缩小
        if (currentScale / scaleFactor >= minScale) {
            scaleView(1.0 / scaleFactor);
        }
    }

    event->accept();  // 阻止事件继续传递
}

void MapWidget::scaleView(qreal scaleFactor) //放缩功能实现
{
    scale(scaleFactor, scaleFactor);
    currentScale *= scaleFactor;
    qDebug() << "Current scale:" << currentScale;
    // for (auto& itemPair : flowerIcons) {
    //     itemPair.second->setScale(currentScale);
    // }
}

void MapWidget::initStructureData() { //初始化建筑数据
    mapData.locations.append({"西门",QPointF(51,4)});
    mapData.locations.append({"西侧门",QPointF(71,4)});
    mapData.locations.append({"朗润园",QPointF(21,55)});
    mapData.locations.append({"朗润湖",QPointF(22,66)});
    mapData.locations.append({"镜春园",QPointF(32,59)});
    mapData.locations.append({"成府园",QPointF(44,98)});
    mapData.locations.append({"荷花池",QPointF(38,30)});
    mapData.locations.append({"民主楼",QPointF(43,30)});
    mapData.locations.append({"华表",QPointF(51,18)});
    mapData.locations.append({"红楼",QPointF(44,42)});
    mapData.locations.append({"一体",QPointF(48,71)});
    mapData.locations.append({"未名湖",QPointF(45,51)});
    mapData.locations.append({"东操场",QPointF(39,81)});
    mapData.locations.append({"廖凯原楼",QPointF(43,84)});
    mapData.locations.append({"东侧门",QPointF(70,99)});
    mapData.locations.append({"档案馆",QPointF(58,31)});
    mapData.locations.append({"蔡元培像",QPointF(57,35)});
    mapData.locations.append({"校史馆",QPointF(67,20)});
    mapData.locations.append({"勺海",QPointF(78,14)});
    mapData.locations.append({"勺园宾馆",QPointF(88,19)});
    mapData.locations.append({"正太国际中心",QPointF(93,18)});
    mapData.locations.append({"西南门",QPointF(120,7)});
    mapData.locations.append({"学五食堂",QPointF(108,23)});
    mapData.locations.append({"临湖轩",QPointF(61,51)});
    mapData.locations.append({"俄文楼",QPointF(70,44)});
    mapData.locations.append({"一教",QPointF(74,60)});
    mapData.locations.append({"老生物楼",QPointF(74,75)});
    mapData.locations.append({"国关大楼",QPointF(82,21)});
    mapData.locations.append({"一院",QPointF(78,27)});
    mapData.locations.append({"二院",QPointF(82,27)});
    mapData.locations.append({"三院",QPointF(86,27)});
    mapData.locations.append({"静园",QPointF(84,35)});
    mapData.locations.append({"四院",QPointF(78,43)});
    mapData.locations.append({"五院",QPointF(82,43)});
    mapData.locations.append({"六院",QPointF(86,43)});
    mapData.locations.append({"图书馆",QPointF(81,62)});
    mapData.locations.append({"网球场",QPointF(93,27)});
    mapData.locations.append({"燕南园",QPointF(106,36)});
    mapData.locations.append({"第一食堂",QPointF(119,33)});
    mapData.locations.append({"哲学楼",QPointF(90,62)});
    mapData.locations.append({"百讲",QPointF(101,56)});
    mapData.locations.append({"南门",QPointF(130,62)});
    mapData.locations.append({"地学楼",QPointF(83,72)});
    mapData.locations.append({"文史楼",QPointF(79,72)});
    mapData.locations.append({"东门",QPointF(80,99)});
    mapData.locations.append({"金光生命科学楼",QPointF(79,90)});
    mapData.locations.append({"老化学楼",QPointF(88,70)});
    mapData.locations.append({"光华楼",QPointF(95,70)});
    mapData.locations.append({"东南门",QPointF(98,100)});
    mapData.locations.append({"理教",QPointF(89,77)});
    mapData.locations.append({"理科一号楼",QPointF(92,76)});
    mapData.locations.append({"英杰交流中心",QPointF(97,86)});
    mapData.locations.append({"博雅塔",QPointF(59,74)});
    mapData.locations.append({"二教",QPointF(103,76)});
    mapData.locations.append({"邱德拔体育馆",QPointF(106,94)});
    mapData.locations.append({"五四操场",QPointF(118,70)});
    mapData.locations.append({"遥感楼",QPointF(69,84)});
    mapData.locations.append({"塞万提斯像",QPointF(72,21)});
    mapData.locations.append({"北阁",QPointF(68,32)});
    mapData.locations.append({"南阁",QPointF(74,30)});
    mapData.locations.append({"鸣鹤园",QPointF(45,16)});
    for (auto& location : mapData.locations) {//将每个点从数组坐标转换到像素坐标
        location.displayPos.setX((location.position.y() + 1) * 36);
        location.displayPos.setY((location.position.x() + 1) * 36);
    }
}

void MapWidget::loadMapFromTxt(const QString &filePath) {//读入txt地图
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Error: Cannot open file" << filePath;
        return;
    }

    QTextStream in(&file);
    int row = 0;

    // 按行读取，直到填满固定大小或文件结束
    while (row < 138 && !in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        // 确保每行长度不超过138
        for (int col = 0; col < 104; ++col) {
            mapData.charMap[row][col] = line[col].toLatin1();
        }
        row++;
    }


    file.close();
}

QPointF MapWidget::getLocationForFlower(const QString& locationName)
{
    for (const auto& location : mapData.locations) {
        if (location.name == locationName) {
            return location.position;
        }
    }
    return QPointF();
}

void MapWidget::setSelectedMonth(int comboBoxIndex) {
    // 转换索引为月份值
    selectedMonth = comboBoxIndex;  // 1-12 月
    // 触发图标更新
    addFlowerIcons();
}
void MapWidget::addFlowerIcons()
{
    clearFlowerIcons();
    for(const auto& location:mapData.locations){
        bool flag=false;
        if(selectedMonth>0){
            for(auto flower:location.flowers){
                if(flower->florescence.contains(QString::number(selectedMonth))){
                    flag=true;
                    break;
                }
            }
        }
        else if(selectedMonth==0){
            if(!location.flowers.isEmpty())
                flag=true;
        }
        if(flag){
            QPixmap pixmap(":/flowericon/Icon.png");
            auto iconItem = new QGraphicsPixmapItem(pixmap);
            iconItem->setPos(location.displayPos);
            scene->addItem(iconItem);
            flowerIcons.append(iconItem);  // 存储指针
        }
    }
}

// 删除所有花的图标
void MapWidget::clearFlowerIcons() {
    for (auto item : flowerIcons) {
        scene->removeItem(item);  // 从场景移除（可选）
        delete item;              // 删除对象
    }
    flowerIcons.clear();         // 清空指针列表
}
void MapWidget::handleMapClick(const QPointF& clickPos)
{
    int nearestlocationNum=-1;
    qreal minDistance = std::numeric_limits<qreal>::max();
    for (auto it=mapData.locations.begin();it!=mapData.locations.end();++it) {
        qreal distance = ((*it).displayPos - clickPos).manhattanLength();
        if (distance < minDistance) {
            minDistance = distance;
            //nearestLocation = &loc;
            nearestlocationNum=it-mapData.locations.begin();
        }
    }
    int num=mapData.locations[nearestlocationNum].flowers.size();
    if (num==0) {
        return;
    }
    else if (num==1) {
        showFlowerInfoDialog(nearestlocationNum,0);
    }
    else {
        showFlowerSelectionDialog(nearestlocationNum);
    }
}

QVector<const FlowerInfo*> MapWidget::getFlowersAtLocation(const QString& locationName) const
{
    QVector<const FlowerInfo*> result;
    for (const auto& loc : mapData.locations) {
        if (loc.name == locationName) {
            result = loc.flowers;
            break;
        }
    }
    return result;
}



void MapWidget::showFlowerInfoDialog(int locationindex, int flowerindex) {
    QDialog dialog(this);
    dialog.setWindowTitle(mapData.locations[locationindex].flowers[flowerindex]->name);
    dialog.setFixedSize(400, 500); // 固定对话框大小

    // 设置复古风格
    dialog.setStyleSheet(
        "QDialog {"
        "   background-color: #3A2515;" // 深棕色背景
        "   border: 3px solid #6B4423;" // 木质边框
        "}"
        "QLabel {"
        "   color: #E8C9A7;" // 米黄色文字
        "   font: 14px '楷体';"
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    // 1. 图片区域
    int flowerId = mapData.locations[locationindex].flowers[flowerindex]->id;
    QString imagePath = QString(":/flowers/flowerimage/%1.png").arg(flowerId);
    QPixmap pixmap(imagePath);

    QLabel *photoLabel = new QLabel(&dialog);
    if (pixmap.isNull()) {
        photoLabel->setText("图片加载失败");
        photoLabel->setStyleSheet(
            "QLabel {"
            "   background-color: #2E1E0F;"
            "   border: 1px solid #6B4423;"
            "   padding: 20px;"
            "   qproperty-alignment: AlignCenter;"
            "}"
        );
    } else {
        photoLabel->setPixmap(pixmap.scaled(300, 300, Qt::KeepAspectRatio));
        photoLabel->setStyleSheet(
            "QLabel {"
            "   background-color: #2E1E0F;"
            "   border: 2px solid #6B4423;"
            "   padding: 5px;"
            "}"
        );
    }
    photoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(photoLabel);

    // 2. 信息区域
    QLabel *nameLabel = new QLabel(&dialog);
    nameLabel->setText(QString("🌸 花名: %1").arg(mapData.locations[locationindex].flowers[flowerindex]->name));
    nameLabel->setStyleSheet("font: bold 18px; qproperty-alignment: AlignCenter;");

    QLabel *florescenceLabel = new QLabel(&dialog);
    florescenceLabel->setText(QString("📅 花期: %1月").arg(mapData.locations[locationindex].flowers[flowerindex]->florescence.join(", ")));

    QLabel *introLabel = new QLabel(&dialog);
    introLabel->setText(QString("📖 介绍:\n%1").arg(mapData.locations[locationindex].flowers[flowerindex]->introduction));
    introLabel->setWordWrap(true);
    introLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #2E1E0F;"
        "   border: 1px solid #6B4423;"
        "   padding: 10px;"
        "   border-radius: 5px;"
        "}"
    );

    layout->addWidget(nameLabel);
    layout->addWidget(florescenceLabel);
    layout->addWidget(introLabel);

    // 3. 关闭按钮
    QPushButton *closeButton = new QPushButton("关闭", &dialog);
    closeButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #6B4423;"
        "   color: #E8C9A7;"
        "   border: 2px solid #8B5A2B;"
        "   border-radius: 5px;"
        "   padding: 8px 16px;"
        "   font: bold 14px '楷体';"
        "   min-width: 100px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #8B5A2B;"
        "}"
    );
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    dialog.exec();
}

void MapWidget::showFlowerSelectionDialog(int locationindex) {
    QDialog dialog(this);
    dialog.setWindowTitle("选择花卉");
    dialog.setFixedSize(300, 400); // 固定对话框大小

    // 设置复古风格
    dialog.setStyleSheet(
        "QDialog {"
        "   background-color: #3A2515;" // 深棕色背景
        "   border: 3px solid #6B4423;" // 木质边框
        "}"
        "QListWidget {"
        "   background-color: #2E1E0F;"
        "   color: #E8C9A7;"
        "   border: 1px solid #6B4423;"
        "   font: 14px '楷体';"
        "   padding: 5px;"
        "}"
        "QListWidget::item {"
        "   padding: 8px;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: #4A3310;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #6B4423;"
        "}"
        );

    QVBoxLayout layout(&dialog);
    layout.setContentsMargins(15, 15, 15, 15);
    layout.setSpacing(15);

    // 花卉列表
    QListWidget listWidget(&dialog);
    for (const auto* flower : mapData.locations[locationindex].flowers) {
        QListWidgetItem *item = new QListWidgetItem(flower->name);
        item->setTextAlignment(Qt::AlignCenter);
        listWidget.addItem(item);
    }
    layout.addWidget(&listWidget);

    // 确定按钮
    QPushButton okButton("确定", &dialog);
    okButton.setStyleSheet(
        "QPushButton {"
        "   background-color: #6B4423;"
        "   color: #E8C9A7;"
        "   border: 2px solid #8B5A2B;"
        "   border-radius: 5px;"
        "   padding: 8px 16px;"
        "   font: bold 14px '楷体';"
        "   min-width: 100px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #8B5A2B;"
        "}"
        );

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(&okButton);
    buttonLayout->addStretch();

    layout.addLayout(buttonLayout);

    connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        int selectedRow = listWidget.currentRow();
        if (selectedRow >= 0) {
            showFlowerInfoDialog(locationindex, selectedRow);
        }
    }
}

// 新增用于处理窗口大小改变事件
void MapWidget::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
}

void MapWidget::linkFlowersToLocations(const QVector<FlowerInfo>& flowers)
{
    // 清空所有地点原有的花卉关联（可选，根据需求决定是否保留）
    for (auto& location : mapData.locations) {
        location.flowers.clear();
    }

    // 为每朵花建立与地点的关联
    for (const auto& flower : flowers) {//对每一种花
        for (const QString& locationName : flower.locationNames) {//对花对应的地点
            // 查找对应的地点
            for (auto& location : mapData.locations) {
                if (location.name == locationName) {
                    // 将花的指针添加到地点的花卉列表中
                    location.flowers.append(&flower);
                    break;
                }
            }
        }
    }
}
void MapWidget::setQuizMode(bool isQuizMode) {
    this->isQuizMode = isQuizMode;
}

void MapWidget::setCorrectFlowerId(int id) {
    this->correctFlowerId = id;
}
// 添加设置打卡记录的方法
void MapWidget::setCheckinRecords(const QVector<CheckinData>& records) {
    checkinRecords = records;
    refreshCheckinMarkers(); // 调用刷新函数统一处理
}

// 添加打卡标记
void MapWidget::addCheckinMarkers() {
    clearCheckinMarkers();
    // 为每个打卡记录添加标记
    QHash<QString, bool> checkedLocations;
    for (const CheckinData& record : checkinRecords) {
        checkedLocations[record.location] = true;
    }
    // 加载原始图标
    QPixmap originalPixmap(":/flowericon/Icon2.png");

    // 缩放为36x36像素（保持比例，平滑缩放）
    QPixmap scaledPixmap = originalPixmap.scaled(
        36, 36,
        Qt::KeepAspectRatio,  // 保持原始比例
        Qt::SmoothTransformation  // 使用平滑算法（抗锯齿）
        );

    // 使用缩放后的图标
    for (const auto& location : mapData.locations) {
        if (checkedLocations.contains(location.name)) {
            auto markerItem = new QGraphicsPixmapItem(scaledPixmap);
            markerItem->setPos(location.displayPos);
            scene->addItem(markerItem);
            checkinMarkers.append(markerItem);
        }
    }
}

// 清除打卡标记
void MapWidget::clearCheckinMarkers() {
    for (auto item : checkinMarkers) {
        scene->removeItem(item);
        delete item;
    }
    checkinMarkers.clear();
}

// 刷新打卡标记
void MapWidget::refreshCheckinMarkers() {
    clearCheckinMarkers();
    addCheckinMarkers();
}

// 获取所有地点名称
QVector<QString> MapWidget::getAllLocations() const
{
    QVector<QString> locations;
    for (const auto& loc : mapData.locations) {
        locations.append(loc.name);
    }
    return locations;
}
MapWidget::~MapWidget()
{
    // 清理场景
    delete scene;
}
