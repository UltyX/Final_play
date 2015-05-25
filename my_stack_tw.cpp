#include "my_stack_tw.h"

my_stack_tw::my_stack_tw()
{
    this->setSortingEnabled(true);
    connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(on_podcast_list_tw_itemDoubleClicked(QTreeWidgetItem*,int))  );
}


void my_stack_tw::on_podcast_list_tw_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    this->editItem(item, 1);
}

my_stack_tw::~my_stack_tw(){
std::cout <<"got removed"<<std::endl;
}
