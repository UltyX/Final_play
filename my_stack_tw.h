#ifndef MY_STACK_TW_H
#define MY_STACK_TW_H

#include <QTreeWidget>
#include <iostream>
class my_stack_tw : public QTreeWidget
{
    Q_OBJECT

private slots:
    void on_podcast_list_tw_itemDoubleClicked(QTreeWidgetItem *item, int column);


public:
    my_stack_tw();
    ~my_stack_tw();
};

#endif // MY_STACK_TW_H
