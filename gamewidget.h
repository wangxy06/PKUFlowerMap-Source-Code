#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QVector>
#include <QLabel>
#include <QPushButton>
#include "flowerdata.h"
#include <QMediaPlayer>  // 音效支持

class MapWidget;
struct FlowerInfo;

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    GameWidget(QWidget *parent = nullptr);
    void setFlowers(const QVector<FlowerInfo> &flowers);

private slots:
    void startGame();
    void closeGame();
    void changeImage();
    void onFlowerClicked(int flowerId);

private:
    void setupUI();
    void showRandomFlower();

    MapWidget *mapWidget;
    QLabel *imageLabel;
    QPushButton *startButton;
    QPushButton *exitButton;
    QPushButton *changeButton;
    QPushButton *closeButton;
    int currentFlowerId;
    QVector<int> shownFlowerIds;
    QVector<FlowerInfo> flowers;

};

#endif // GAMEWIDGET_H
