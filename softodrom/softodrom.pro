#-------------------------------------------------
#
# Project created by QtCreator 2014-10-06T01:11:37
#
#-------------------------------------------------


contains(QMAKE_TARGET.arch, x86_64):{
TARGET = ../../../softodrom/softodrom64
RC_FILE = softodrom64.rc
} else {
TARGET = ../../../softodrom/softodrom
RC_FILE = softodrom.rc
}

QT       += core gui
CONFIG   += console
TEMPLATE = app console
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    appbox.cpp \
    scaner.cpp \
    settings.cpp \
    kitmenu.cpp \
    installer.cpp \
    softodromglobal.cpp \
    errcodes.cpp \
    aboutbox.cpp \
    easteregg.cpp
HEADERS  += \
    mainwindow.h \
    appbox.h \
    scaner.h \
    settings.h \
    kitmenu.h \
    installer.h \
    softodromglobal.h \
    errcodes.h \
    aboutbox.h
FORMS += \
    mainwindow.ui \
    appbox.ui \
    settings.ui
RESOURCES += \
    resources.qrc

LIBS += \
-L version.lib -lversion \
-L winmm.lib -lwinmm
OTHER_FILES += softodrom.rc \
    softodrom64.rc \
    ../help/info.txt
