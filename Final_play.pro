#-------------------------------------------------
#
# Project created by QtCreator 2015-04-18T14:21:38
#
#-------------------------------------------------
CONFIG += c++11
QT       += core gui \
        multimedia \            # sudo apt-get install qt5-default qtmultimedia5-dev libqt5multimediawidgets5 libqt5multimedia5-plugins libqt5multimedia5
        multimediawidgets \
        widgets \
        sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Final_play
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        podcast_manager.cpp\
        communiction_node.cpp\
        playlist_tree_wg.cpp \
        my_stack_tw.cpp \
        epi_list_item.cpp \
    querys.cpp

HEADERS  += mainwindow.h\
        podcast_strukts.hpp\
        podcast_manager.hpp\
        communiction_node.h \
        podlist_item.hpp \
        epi_list_item.hpp\
        playlist_tree_wg.h \
        my_stack_tw.h \
        querys.h

FORMS    += mainwindow.ui
