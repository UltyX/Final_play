#-------------------------------------------------
#
# Project created by QtCreator 2015-04-18T14:21:38
#
#-------------------------------------------------
CONFIG += c++11
QT       += core gui \
      multimedia \
      multimediawidgets \
widgets \
sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Final_play
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
podcast_manager.cpp\
convinience.cpp\
pod_play.cpp\
playlist_tree_wg.cpp \
    my_stack_tw.cpp \
    epi_list_item.cpp

HEADERS  += mainwindow.h\
podcast_strukts.hpp\
podcast_manager.hpp\
convinience.hpp\
pod_play.h \
    podlist_item.hpp \
epi_list_item.hpp\
playlist_tree_wg.h \
    my_stack_tw.h

FORMS    += mainwindow.ui
