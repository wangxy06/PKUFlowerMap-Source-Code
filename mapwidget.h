#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QVector>
#include <QPointF>
#include <QString>
#include <QFile>
#include <QTextStream>
#include "flowerdata.h"
#include <QGraphicsScene>
// 打卡数据结构
struct CheckinData {
    QString date;
    QString location;
    QString flowerName;
    QString imagePath;
    QString log;
};
// 定义存储打卡日志的文件路径
const QString CHECKIN_LOG_FILE_PATH = "logs\checkin_logs.txt";
// 地点信息
struct Location {
    QString name;       // 地点名称
    QPointF position;   // 地点坐标
    QPointF displayPos;   // 显示坐标(像素)
    QVector<const FlowerInfo*> flowers;//存储这个地方对应的花的信息
    Location(const Location& other) {//复制构造函数
        name = other.name;
        // 浅拷贝QPointF类型的坐标（值语义类型，直接赋值即可）
        position = other.position;
        displayPos = other.displayPos;
        // 浅拷贝QVector<const FlowerInfo*>（复制指针，不复制对象）
        flowers = other.flowers;
    }
    Location(const QString& name, const QPointF& position)
        : name(name), position(position), displayPos(position), flowers() {
        // 初始化displayPos为与position相同的值
        // flowers向量会被默认初始化为空
    }
    Location(){}
};

// 地图数据
struct MapData {
    char charMap[138][104];// 字符矩阵地图，138*104
    QVector<Location> locations;  // 所有地点
};
class MapWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MapWidget(QWidget *parent = nullptr);
    ~MapWidget();
    void loadMap(const QString &imagePath);  // 加载地图图片
    void initStructureData();//初始化建筑类数据
    void loadMapFromTxt(const QString &filePath); //初始化地图
    void addFlowerIcons();//添加花的图标
    void setSelectedMonth(int month);//选择月份
    QPointF getLocationForFlower(const QString& locationName);//根据花的位置名称找到对应的坐标
    QVector<const FlowerInfo*> getFlowersAtLocation(const QString& locationName) const;//处理点击图标展示花信息
    void showFlowerInfoDialog(int locationindex,int flowerindex);
    void showFlowerSelectionDialog(int locationindex);
    FlowerInfo getFlowerInfoForIcon(QGraphicsPixmapItem *iconItem);
    void linkFlowersToLocations(const QVector<FlowerInfo>& flowers);//将花的指针存到地点对应的数组里面
    void clearFlowerIcons();
    MapData mapData;  // 字符地图数据 wxy:这里改成public了！！！不然外部无法访问
    QGraphicsScene* getScene() const { return QGraphicsView::scene(); } //wxy:这里也改成public了！！！
    void setQuizMode(bool isQuizMode);  // 设置游戏模式
    void setCorrectFlowerId(int id);    // 设置当前正确答案的花卉ID
    void handleQuizClick(const QPointF& clickPos);
    QVector<CheckinData> checkinRecords; // 打卡记录
    void clearCheckinMarkers();
    void addCheckinMarkers();
    void refreshCheckinMarkers();
    void setCheckinRecords(const QVector<CheckinData>& records);
    QVector<QString> getAllLocations() const;

protected:
    void mousePressEvent(QMouseEvent *event) override;  // 处理鼠标点击
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event) override;  // 添加滚轮缩放
    void handleMapClick(const QPointF& clickPos);
    void resizeEvent(QResizeEvent *event) override; // 新增用于处理窗口大小改变事件
private:
    QGraphicsScene* scene;
    QGraphicsPixmapItem *mapItem;
    void scaleView(qreal scaleFactor);
    qreal currentScale = 1.0;
    const qreal minScale = 0.3;  // 最小缩放比例
    const qreal maxScale = 3.0;  // 最大缩放比例
    QSet<QPair<double, double>> uniqueCoordinates;
    //QList<QPair<QPointF, QGraphicsPixmapItem*>> flowerIcons; // fym:这个原本是一个花对应一个图标的情况，看看后面想不想改
    int selectedMonth = 0; // 记录选择的月份，-1表示全部展示
    QVector<QGraphicsPixmapItem*> flowerIcons;  // 存储所有花的指针
    bool isQuizMode = false;          // 是否处于游戏模式
    int correctFlowerId = -1;         // 当前正确答案的花卉ID
    QVector<QGraphicsPixmapItem*> checkinMarkers; // 打卡标记列表

signals:
    void flowerSelectedInQuiz(int flowerId);  // 游戏模式下花卉被选中信号
};

#endif // MAPWIDGET_H
