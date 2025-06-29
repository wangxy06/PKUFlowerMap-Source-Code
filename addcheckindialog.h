
#ifndef ADDCHECKINDIALOG_H
#define ADDCHECKINDIALOG_H

#include <QDialog>
#include "mapwidget.h"
#include <QDateEdit>
#include <QComboBox>
#include <QLabel>
#include <QTextEdit>

class AddCheckinDialog : public QDialog
{
    Q_OBJECT

public:
    AddCheckinDialog(MapWidget* map = nullptr, QWidget *parent = nullptr);
    ~AddCheckinDialog();

    CheckinData getCheckinData() const;

private slots:
    void onDateSelected(int year, int month, int day);
    void onLocationSelected(const QString& location);
    void onImageSelected(const QString& path);
    void onFinishButtonClicked();

private:
    void setupUI();
    bool saveCheckinDataToFile(const CheckinData& data);
    QString copyImageToResources(const QString& sourcePath);

    MapWidget *mapWidget;
    CheckinData checkinData;

    // UI Components
    QDateEdit *dateEdit;
    QComboBox *locationComboBox;
    QLineEdit *flowerNameEdit;
    QPushButton *imageButton;
    QLabel *imagePathLabel;
    QTextEdit *logEdit;
    QPushButton *finishButton;
};

#endif // ADDCHECKINDIALOG_H
