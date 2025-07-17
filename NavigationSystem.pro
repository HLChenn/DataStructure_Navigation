QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    graphicdraw.cpp \
    graphsystem.cpp \
    main.cpp \
    map.cpp \
    pathdraw.cpp \
    vertex.cpp \
    vertexdraw.cpp \
    widget.cpp

HEADERS += \
    Qhead.h \
    graphicdraw.h \
    graphsystem.h \
    map.h \
    pathdraw.h \
    vertex.h \
    vertexdraw.h \
    widget.h

FORMS += \
    widget.ui

TRANSLATIONS += \
    NavigationSystem_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
