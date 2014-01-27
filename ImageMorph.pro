#-------------------------------------------------
#
# Project created by QtCreator 2014-01-01T21:43:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageMorph
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    imagepreviewdialog.cpp \
    imagewrapper.cpp \
    imagehighlight.cpp \
    imageutil.cpp

HEADERS  += mainwindow.h \
    imagepreviewdialog.h \
    imagewrapper.h \
    imagehighlight.h \
    imageutil.h

FORMS    += mainwindow.ui \
    imagepreviewdialog.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    default.style
