 
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
};

#endif
