#-------------------------------------------------
#
# Project created by QtCreator 2014-10-06T01:11:37
#
#-------------------------------------------------

contains(QMAKE_TARGET.arch, x86_64):{
TARGET = tmp64launcher
SOURCES += main.cpp
TEMPLATE = app
} else {
QT = ""
RC_FILE = launcher.rc
TARGET = ../../../softodrom_launcher
TEMPLATE = app
SOURCES += main.cpp
HEADERS  +=

Release:QMAKE_EXTRA_TARGETS += makeCleanLauncher dummy
Release:dummy.target = first
Release:dummy.depends = makeCleanLauncher
Release:makeCleanLauncher.depends = all
Release:makeCleanLauncher.CONFIG = phony
Release:makeCleanLauncher.commands = g++ -static-libgcc -Wl,-subsystem,windows -o ./../../softodrom_launcher ./release/launcher_res.o ./release/main.o

}
OTHER_FILES += launcher.rc
