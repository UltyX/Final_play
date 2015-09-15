#ifndef MY_STACK_TW_H
#define MY_STACK_TW_H

#include <QTreeWidget>
#include <iostream>
class Raiting_tree_wg : public QTreeWidget
{
    Q_OBJECT

private slots:
    void on_podcast_list_tw_itemDoubleClicked(QTreeWidgetItem *item, int column);


public:
    Raiting_tree_wg();
    ~Raiting_tree_wg();
};

#endif // MY_STACK_TW_H
