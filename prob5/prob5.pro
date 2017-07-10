#-------------------------------------------------
#
# Project created by QtCreator 2017-07-09T06:52:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Prob5
TEMPLATE = app

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    search.cpp \
    soundex.cpp

HEADERS  += mainwindow.h \
    search.h \
    soundex.h

FORMS    += mainwindow.ui

mac {
    BIN_QML_FILES.files = words.bin
    BIN_QML_FILES.path = Contents/MacOS
    QMAKE_BUNDLE_DATA += BIN_QML_FILES
}
