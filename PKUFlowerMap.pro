QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QT += axcontainer
QT += multimedia

SOURCES += \
    addcheckindialog.cpp \
    albumwidget.cpp \
    flowerdata.cpp \
    gamewidget.cpp \
    main.cpp \
    mainwindow.cpp \
    mapwidget.cpp \
    navigation.cpp \
    splashScreen.cpp

HEADERS += \
    addcheckindialog.h \
    albumwidget.h \
    flowerdata.h \
    gamewidget.h \
    mainwindow.h \
    mapwidget.h \
    navigation.h \
    splashScreen.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources/Icons.qrc \
    resources/flowerimage.qrc \
    resources/images.qrc \
    resources/mapData.qrc

DISTFILES += \
    data/images/pku_map.jpg \
    data/map/pku_map.jpg \
    logs/checkin_logs \
    logs/checkin_logs.txt \
    resources/images/coverpage.png
