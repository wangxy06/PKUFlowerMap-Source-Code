#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>
#include <QPixmap>
#include <QMediaPlayer>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPropertyAnimation>
#include <QTimer>
#include <QObject>
#include <QPaintEvent>


class QPushButton;
class QLabel;
class QGraphicsOpacityEffect;


class AnimatedPetal : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)

public:
    AnimatedPetal(const QPixmap &pixmap, QGraphicsScene *scene, QObject *parent = nullptr);
    ~AnimatedPetal();

signals:
    void animationFinished(AnimatedPetal *petal);

private slots:
    void onAnimationFinished();

private:
    QPropertyAnimation *fallAnimation;
    QPropertyAnimation *rotateAnimation;
};

// SplashScreen.h
class SplashScreen : public QWidget {
    Q_OBJECT

public:
    explicit SplashScreen(QWidget *parent = nullptr);
    ~SplashScreen();

private slots:
    void createPetal();
    void showIntroLetter();
    void onStartClicked();
    void onIntroClicked();
    void handlePetalFinished(AnimatedPetal *petal);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupUI();
    void setupAnimations();

    QGraphicsScene *petalScene;
    QGraphicsView *petalView;
    QPixmap background;
    QLabel *titleLabel;
    QPushButton *startButton;
    QPushButton *introButton;
    QTimer *petalTimer;
    QList<AnimatedPetal*> petals; // 存储所有花瓣
    void paintEvent(QPaintEvent *event);
};
#endif // SPLASHSCREEN_H
