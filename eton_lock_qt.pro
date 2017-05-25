#-------------------------------------------------
#
# Project created by QtCreator 2016-05-16T16:36:36
#
#-------------------------------------------------

QT       += core gui sql network webkitwidgets serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = eton_lock_qt
TEMPLATE = app
RC_FILE = myapp.rc

SOURCES += main.cpp\
        loginmainwindow.cpp \
    mainwindow.cpp \
    exportexcelobject.cpp \
    mysqloperation.cpp \
    qcomboboxdelegate.cpp \
    spcomm.cpp \
    frmmessagebox.cpp \
    iconhelper.cpp

HEADERS  += loginmainwindow.h \
    mainwindow.h \
    exportexcelobject.h \
    mysqloperation.h \
    qcomboboxdelegate.h \
    spcomm.h \
    frmmessagebox.h \
    iconhelper.h \
    myhelper.h \
    selfdefine.h \
    mycontroller.h \
    myworker.h \
    myworkerthread.h

FORMS    += loginmainwindow.ui \
    mainwindow.ui \
    frmmessagebox.ui

DISTFILES +=

RESOURCES += \
    resource.qrc
