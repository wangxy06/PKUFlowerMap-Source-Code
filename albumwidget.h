#ifndef ALBUMWIDGET_H
#define ALBUMWIDGET_H

#include <QDialog>
#include <QVector>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <mapwidget.h>

class AlbumWidget : public QDialog
{
    Q_OBJECT

public:
    AlbumWidget(QWidget *parent = nullptr);
    ~AlbumWidget();

    void setCheckinRecords(const QVector<CheckinData>& records);

private slots:
    void onFlipButtonClicked();
    void onCloseButtonClicked();
    void onNextPageButtonClicked();
    void onPreviousPageButtonClicked();

private:
    void setupUI();
    void updatePageDisplay(int pageIndex);
    void setupCoverPage();
    void setupFirstPage();
    void setupContentPages();

    QVector<CheckinData> localRecords;
    QStackedWidget *pageStack;
    QPushButton *flipButton;
    QPushButton *closeButton;
    QPushButton *nextPageButton;
    QPushButton *previousPageButton;
    QLabel *pageLabel;
    int currentPage;
};

#endif // ALBUMWIDGET_H
