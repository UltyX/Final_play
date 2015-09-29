 
#ifndef Epi_list_item_W_w
#define Epi_list_item_W_w
#include <QTreeWidgetItem>
#include <QTime>
#include "podcast_strukts.hpp"
#include <iostream>

class Epi_list_item : public QTreeWidgetItem {

    bool setup_done;
    Episode *episode;

    public:
    Epi_list_item(QTreeWidget* parent);

    Epi_list_item(const Epi_list_item* other);


  // ~Epi_list_item(){std::cout<<"been killed e"<<std::endl;}// carefull if cloned do not delete the episode pointer
  //virtual QTreeWidgetItem *clone() const;
  //virtual Epi_list_item *clone() const;


    Episode * getEpisode(){return episode;}
    bool get_setup_done(){return setup_done;}

    void setEpisode(Episode * epi_i);

private:
    bool operator<(const QTreeWidgetItem &other)const {   //needs to be QTreeWidgetItem else it will not replace the virtual parent funktion and never be called

        /// todo ask mainwindow for sort rules like 1: importans [row 2 asending ] 2: if 1. rule had some that are ==  [3. row desending]   ...

    int column = treeWidget()->sortColumn();
    if(column==2){
        if(episode->parent->raiting == ((Epi_list_item*)(&other))->episode->parent->raiting){
            return (episode->name > ((Epi_list_item*)(&other))->episode->name);                         // by name
        }
        else{
            return(  episode->parent->raiting < ((Epi_list_item*)(&other))->episode->parent->raiting);  // by raiting
        }
    }
    else{
        return text(column) < other.text(column);
        }
    }
};

#endif
