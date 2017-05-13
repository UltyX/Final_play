#include "epi_list_item.hpp"
/*
This class extends Q Tree Widget Item and add some Attributes and methodes to the item.

*/


 /*QTreeWidgetItem *Epi_list_item::clone()const{

     //needed for drag and drop, else it will only make a QTreeWidgetItem


     Epi_list_item * the_clone = new  Epi_list_item(this);   //and episode pointer = null


    std::cout<<"cloned"<<std::endl;
    return the_clone;
    }*/

/* Epi_list_item *Epi_list_item::clone()const{

     //needed for drag and drop, else it will only make a QTreeWidgetItem


     Epi_list_item * the_clone = new  Epi_list_item(this);   //and episode pointer = null


    std::cout<<"cloned"<<std::endl;
    return the_clone;
    }*/





//  Normal constructor
//                              parameter          parameter given to parent class constructor first
Epi_list_item::Epi_list_item(QTreeWidget* parent):QTreeWidgetItem(parent){
    episode=NULL;
    setup_done=false;
    main_w = parent->parent();
}

// Copy Constructor
Epi_list_item::Epi_list_item(const Epi_list_item* other):QTreeWidgetItem(other->parent()){
    setEpisode(other->episode);
    main_w = other->main_w;


    std::cout<<"copy from other"<<std::endl;
}

// Assign an episode to this Episode List item. Sets the text fields so it shows what episode it has loaded.
void Epi_list_item::setEpisode(Episode * epi_i){
      episode=epi_i;
      setText(0,QString::fromStdString(epi_i->name) );

      qint64 pos = epi_i->last_position/1000;
      QTime currentTime((pos/3600)%60, (pos/60)%60, pos%60);
      setText(1, currentTime.toString("hh:mm:ss") );
      setText(2, QString::number(epi_i->parent->raiting ) );

      setData(1,Qt::ToolTipRole,"shows where you last left off");
      setup_done=true;
  }
