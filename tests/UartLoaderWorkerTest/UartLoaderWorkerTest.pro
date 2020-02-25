QT += testlib
QT -= gui

TARGET = tst_uartloaderworkertest
CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += tst_uartloaderworkertest.cpp \
    mcumemorysingleton.cpp \
    qserialport.cpp
SOURCES += ../../uartloaderworker.cpp

HEADERS += ../../uartloaderworker.h \
    mcumemorysingleton.h \
    qserialport.h

INCLUDEPATH += ../../

RESOURCES += ../../hex_files.qrc \
    testFiles.qrc
