#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <QMainWindow>
#include "mapwidget.h"
#include <QLineEdit>
#include <QCheckBox>
#include <QSpacerItem>
#include <QComboBox>
#include <QRadioButton>
// 自定义节点结构，用于找花路径的优先队列
struct WeightedNode {
    QPoint pos;
    int g;  // 实际成本
    int h;  // 启发式估值
    int score;  // 综合得分 (g + h - 3的奖励)
    int threeCount;  // 经过的3的数量
    std::shared_ptr<WeightedNode> parent;  // 改为智能指针

    // 修改WeightedNode构造函数
    WeightedNode(QPoint p, int g = 0, int h = 0, int three = 0,
                 std::shared_ptr<WeightedNode> parent = nullptr)
        : pos(p), g(g), h(h),
        threeCount(std::min(three, 20)),  // 限制最大计数20
        parent(parent)
    {
        // 调整后的非线性奖励
        int flowerReward = std::min(threeCount * 20 + threeCount * threeCount, 1000);
        score = g + h - flowerReward;
    }

    bool operator<(const WeightedNode& other) const {
        return score > other.score; // 小顶堆
    }
};

// 自定义节点结构，用于避花路径的优先队列
struct AvoidanceNode {
    QPoint pos;
    int g;          // 实际路径成本
    int h;          // 启发式估值
    int penalty;    // 经过3的惩罚值
    int score;      // 综合得分 (g + h + penalty)
    std::shared_ptr<AvoidanceNode> parent;  // 改为智能指针

    AvoidanceNode(QPoint p, int g = 0, int h = 0, int penalty = 0, std::shared_ptr<AvoidanceNode> parent = nullptr)
        : pos(p), g(g), h(h), penalty(penalty), parent(parent) {
        score = g + h + penalty; // 分数包含路径成本和3的惩罚
    }

    bool operator<(const AvoidanceNode& other) const {
        return score > other.score; // 小顶堆
    }
};
class NavigationWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit NavigationWindow(QWidget *parent = nullptr);
    // 可以添加其他自定义成员函数和变量

private:
    void setupUI(); // 初始化界面
    Flower flower;  // 新增成员变量
    QList<QPointF> findPath(const QPointF &start, const QPointF &end, bool avoidFlowers, int month);
    // UI 成员
    QLineEdit *startEdit;//起点
    QLineEdit *endEdit;//终点
    QRadioButton *flowerPathRadio;  // 赏花路径
    QRadioButton *avoidPollenRadio; // 避花路径
    QComboBox *monthComboBox;//月份选择
    MapWidget *mapWidget;
    QPointF getLocationByName(const QString &name) const;//输入的地点匹配
    bool isFlowering(const FlowerInfo* flower, int month) const;//判断花是否在花期
    // 路径查找函数
    QList<QPointF> flowerPath(const QPointF& start, const QPointF& end, const MapData& pathMap);
    QList<QPointF> avoidPath(const QPointF& start, const QPointF& end, const MapData& pathMap);
    //启发函数
    int heuristic(const QPoint& a, const QPoint& b);
    QList<QPointF> currentPath;  // 存储当前路径
    QGraphicsPathItem* pathItem = nullptr;  // 用于绘制路径的图形项
    QGraphicsEllipseItem* startMarker = nullptr;  // 起点标记
    QGraphicsEllipseItem* endMarker = nullptr;    // 终点标记
    QPointF convertToPixelCoordinates(const QPointF& gridPoint);//坐标转换
    void addMarker(const QPointF& pixelPos, const QColor& color);//辅助函数

    void drawPath(const QList<QPointF>& path);  // 绘制路径
    void clearPath();  // 清除路径

private slots:
    void onNavigateClicked();  // 导航按钮点击槽函数
    void onExitClicked();      // 退出按钮点击槽函数
    void showCustomMessageBox(const QString &title, const QString &message);
};

#endif
