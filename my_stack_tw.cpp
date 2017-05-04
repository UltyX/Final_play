#include "my_stack_tw.h"
/*
The Stack Widget.
It holds the podcast raitings.
Changing score is connected and recieved by playlist_tree_wg.
*/
Raiting_tree_wg::Raiting_tree_wg()
{

    QStringList raiting_wg_labels={"Name","Raiting"};           // Labels
    setColumnCount(2);                                          // 2 Collums Raiting WG
    setHeaderLabels(raiting_wg_labels);                         // Lable all the things
    setSortingEnabled(true);                                    // sorting enable
    sortItems(1,Qt::DescendingOrder);                           // sort initial
    connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(on_podcast_list_tw_itemDoubleClicked(QTreeWidgetItem*,int))  );
}


void Raiting_tree_wg::on_podcast_list_tw_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    this->editItem(item, 1);
    column++;                   // removes the warning "unused var"
}

Raiting_tree_wg::~Raiting_tree_wg(){
    std::cout <<"stack wg got removed"<<std::endl;
}
