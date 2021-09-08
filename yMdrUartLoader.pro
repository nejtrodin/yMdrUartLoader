QT += quick
QT += quickcontrols2
QT += serialport
QT += widgets
QT += svg

TARGET = yMdrUartLoader

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# for Windows
VERSION = 0.0.0.1
QMAKE_TARGET_COMPANY = svdSoft
QMAKE_TARGET_PRODUCT = $${TARGET}
QMAKE_TARGET_DESCRIPTION = $${TARGET}
QMAKE_TARGET_COPYRIGHT = (c) Vyasheslav Sinishenko
RC_ICONS = images/$${TARGET}.ico

SOURCES += main.cpp \
    uartloader.cpp \
    uartloaderworker.cpp \
    fileiohelper.cpp

RESOURCES += qml.qrc \
    hex_files.qrc \
    images.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    uartloader.h \
    uartloaderworker.h \
    fileiohelper.h

DISTFILES += \
    README.md \
    TODO.md

