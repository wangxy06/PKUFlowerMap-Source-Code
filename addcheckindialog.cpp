#include "addcheckindialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDateEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QTextEdit>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>
#include <QUuid>
#include <QScreen>
#include <QApplication>
#include <QSpacerItem>
#include <QMessageBox>

AddCheckinDialog::AddCheckinDialog(MapWidget* map, QWidget *parent)
    : QDialog(parent)
    , mapWidget(map)
{
    setWindowTitle("添加打卡记录");

    // 设置为全屏窗口
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        setGeometry(screen->availableGeometry());
    }

    setupUI();

    // 连接信号与槽
    connect(dateEdit, &QDateEdit::dateChanged, this, [this](const QDate& date) {
        checkinData.date = date.toString("yyyy-MM-dd");
    });

    connect(locationComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                if (index >= 0) {
                    checkinData.location = locationComboBox->itemText(index);
                }
            });

    connect(imageButton, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, tr("选择图片"), "", tr("Images (*.png *.jpg *.jpeg)"));
        if (!filePath.isEmpty()) {
            QString relativePath = copyImageToResources(filePath);
            if (!relativePath.isEmpty()) {
                checkinData.imagePath = relativePath;
                imagePathLabel->setText(QFileInfo(relativePath).fileName());
                imagePathLabel->setStyleSheet("color: #5A2A1D;");
            }
        }
    });

    connect(finishButton, &QPushButton::clicked, this, &AddCheckinDialog::onFinishButtonClicked);
}

AddCheckinDialog::~AddCheckinDialog()
{
}

void AddCheckinDialog::setupUI()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    // 设置背景色
    setStyleSheet("QDialog { background-color: #F8F1E9; }");

    // 表单布局
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setContentsMargins(20, 20, 20, 20);
    formLayout->setSpacing(15);
    formLayout->setLabelAlignment(Qt::AlignRight);

    // 日期选择
    dateEdit = new QDateEdit(this);
    dateEdit->setCalendarPopup(true);
    dateEdit->setDate(QDate::currentDate());
    dateEdit->setStyleSheet("QDateEdit { background: white; border: 1px solid #9F4125; border-radius: 4px; padding: 5px; }");
    formLayout->addRow("日期:", dateEdit);

    // 地点选择
    locationComboBox = new QComboBox(this);
    locationComboBox->setStyleSheet("QComboBox { background: white; border: 1px solid #9F4125; border-radius: 4px; padding: 5px; }");
    if (mapWidget) {
        for (const auto& location : mapWidget->getAllLocations()) {
            locationComboBox->addItem(location);
        }
    }
    formLayout->addRow("地点:", locationComboBox);

    // 花名输入
    flowerNameEdit = new QLineEdit(this);
    flowerNameEdit->setStyleSheet("QLineEdit { background: white; border: 1px solid #9F4125; border-radius: 4px; padding: 5px; }");
    formLayout->addRow("花名:", flowerNameEdit);

    // 图片选择
    imageButton = new QPushButton("选择图片", this);
    imageButton->setStyleSheet(
        "QPushButton { background-color: #9F4125; color: white; border: none; "
        "border-radius: 15px; padding: 8px 16px; min-width: 100px; }"
        "QPushButton:hover { background-color: #B85C3B; }"
        );

    imagePathLabel = new QLabel("未选择图片", this);
    imagePathLabel->setStyleSheet("color: #9F4125;");
    imagePathLabel->setWordWrap(true);

    QHBoxLayout *imageLayout = new QHBoxLayout();
    imageLayout->addWidget(imageButton);
    imageLayout->addWidget(imagePathLabel, 1);
    formLayout->addRow("图片:", imageLayout);

    // 日志输入
    logEdit = new QTextEdit(this);
    logEdit->setPlaceholderText("请输入打卡日志...");
    logEdit->setStyleSheet(
        "QTextEdit { background: white; border: 1px solid #9F4125; border-radius: 4px; padding: 5px; }"
        );
    formLayout->addRow("日志:", logEdit);

    // 添加表单到主布局
    mainLayout->addLayout(formLayout, 1);

    // 按钮区域
    finishButton = new QPushButton("完成打卡", this);
    finishButton->setStyleSheet(
        "QPushButton { background-color: #9F4125; color: white; border: none; "
        "border-radius: 20px; padding: 12px 24px; font-size: 16px; min-width: 150px; }"
        "QPushButton:hover { background-color: #B85C3B; }"
        );

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(finishButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(20);

    // 设置标签样式
    QList<QLabel*> labels = findChildren<QLabel*>();
    for (QLabel* label : labels) {
        label->setStyleSheet("color: #5A2A1D; font-weight: bold;");
    }

    setLayout(mainLayout);
}

void AddCheckinDialog::onDateSelected(int year, int month, int day)
{
    checkinData.date = QString("%1-%2-%3").arg(year).arg(month, 2, 10, QChar('0'))
    .arg(day, 2, 10, QChar('0'));
}

void AddCheckinDialog::onLocationSelected(const QString& location)
{
    checkinData.location = location;
}

void AddCheckinDialog::onImageSelected(const QString& path)
{
    // 获取应用程序目录
    QString appDir = QCoreApplication::applicationDirPath();

    // 创建目标目录（如果不存在）
    QDir targetDir(appDir + "/resources/images/");
    if (!targetDir.exists()) {
        targetDir.mkpath(".");
    }

    // 生成唯一文件名
    QString newFileName = QUuid::createUuid().toString(QUuid::WithoutBraces) +
                          QFileInfo(path).suffix();

    // 目标路径
    QString destPath = targetDir.filePath(newFileName);

    // 复制文件
    if (QFile::copy(path, destPath)) {
        // 存储相对路径
        checkinData.imagePath = "resources/images/" + newFileName;
        imagePathLabel->setText(checkinData.imagePath);
    } else {
        qWarning() << "图片复制失败";
        imagePathLabel->setText("图片保存失败");
    }
}

QString AddCheckinDialog::copyImageToResources(const QString& sourcePath)
{
    QString appDir = QCoreApplication::applicationDirPath();
    QDir targetDir(appDir + "/resources/images/");

    if (!targetDir.exists()) {
        targetDir.mkpath(".");
    }

    QString newFileName = QUuid::createUuid().toString(QUuid::WithoutBraces) +
                          "." + QFileInfo(sourcePath).suffix();
    QString destPath = targetDir.filePath(newFileName);

    if (QFile::copy(sourcePath, destPath)) {
        return "resources/images/" + newFileName;

    } else {
        qWarning() << "图片复制失败:" << sourcePath << "->" << destPath;
        return QString();
    }
}

CheckinData AddCheckinDialog::getCheckinData() const
{
    CheckinData data;
    data.date = dateEdit->date().toString("yyyy-MM-dd");
    data.location = locationComboBox->currentText();
    data.flowerName = flowerNameEdit->text();
    data.imagePath = checkinData.imagePath; // 使用已存储的相对路径
    data.log = logEdit->toPlainText();
    return data;
}

bool AddCheckinDialog::saveCheckinDataToFile(const CheckinData& data)
{
    // 在保存前输出数据内容
    qDebug() << "准备保存打卡数据:";
    qDebug() << "日期:" << data.date;
    qDebug() << "地点:" << data.location;
    qDebug() << "花名:" << data.flowerName;
    qDebug() << "图片路径:" << data.imagePath;
    qDebug() << "日志:" << data.log;

    QFile file(CHECKIN_LOG_FILE_PATH);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "日期: " << data.date << "\n";
        stream << "地点: " << data.location << "\n";
        stream << "花名: " << data.flowerName << "\n";
        stream << "图片路径: " << data.imagePath << "\n";
        stream << "日志: " << data.log << "\n\n";

        // 显式刷新缓冲区
        stream.flush();

        // 检查文件大小确认写入成功
        qint64 fileSize = file.size();
        file.close();

        // 保存后再次输出确认
        qDebug() << "打卡数据已成功保存到文件:" << CHECKIN_LOG_FILE_PATH;
        qDebug() << "文件大小:" << fileSize << "字节";
        qDebug() << "-----------------------------";

        // 验证文件是否真的包含内容
        QFile testFile(CHECKIN_LOG_FILE_PATH);
        if (testFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream inStream(&testFile);
            QString content = inStream.readAll();
            qDebug() << "文件内容预览:" << content.left(100) << "...";
            testFile.close();
        }

        return true;

    }
    else {
        qWarning() << "无法打开文件进行写入:" << CHECKIN_LOG_FILE_PATH;
        qDebug() << "文件错误:" << file.errorString();
        return false;
    }
}

void AddCheckinDialog::onFinishButtonClicked()
{
    CheckinData data = getCheckinData();

    // 输出当前获取的数据
    qDebug() << "获取到的打卡数据:";
    qDebug() << "日期:" << data.date;
    qDebug() << "地点:" << data.location;
    qDebug() << "花名:" << data.flowerName;
    qDebug() << "图片路径:" << data.imagePath;
    qDebug() << "日志:" << data.log;

    if (saveCheckinDataToFile(data)) {
        accept();
    } else {
        QMessageBox::warning(this, "错误", "保存打卡记录失败！");
    }
}
