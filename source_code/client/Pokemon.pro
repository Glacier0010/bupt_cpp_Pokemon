#-------------------------------------------------
#
# Project created by QtCreator 2020-07-22T21:23:22
#
#-------------------------------------------------

QT       += core gui
QT       += sql
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = Pokemon
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    dialog.cpp \
    rdialog.cpp \
    userlist.cpp \
    pokemonlist.cpp \
    cfdialog.cpp \
    smpdialog.cpp \
    fffdialog.cpp

HEADERS += \
        mainwindow.h \
    dialog.h \
    rdialog.h \
    parameter.h \
    userlist.h \
    pokemonlist.h \
    cfdialog.h \
    smpdialog.h \
    fffdialog.h

FORMS += \
        mainwindow.ui \
    dialog.ui \
    rdialog.ui \
    userlist.ui \
    pokemonlist.ui \
    cfdialog.ui \
    smpdialog.ui \
    fffdialog.ui

DISTFILES += \
    C:/Users/Glacier/Desktop/p0.jpg
