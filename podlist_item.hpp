#ifndef Pod_list_item_W_w
#define Pod_list_item_W_w
#include <QTreeWidgetItem>
#include "podcast_strukts.hpp"
#include <iostream>


class Pod_list_item : public QTreeWidgetItem {

public:
    Pod_list_item(QTreeWidget* parent):QTreeWidgetItem(parent){podcast=NULL;setup_done=false;}
    // ~Podlist_item(){std::cout<<"been killed p"<<std::endl;}


    Podcast * getPodcast(){return podcast;}
    bool get_setup_done(){return setup_done;}

    void setPodcast(Podcast * pod_i){
        podcast=pod_i;
        setText(0,QString::fromStdString(pod_i->name) );
        setText(1,QString::number(pod_i->raiting) );
        setup_done=true;
    }

private:
    bool setup_done;
    Podcast *podcast;

    bool operator<(const QTreeWidgetItem &other)const {   //needs to be QTreeWidgetItem else it will not replace the virtual parent funktion and never be called

        int column = treeWidget()->sortColumn();
        if(column==1){
            return text(column).toInt() < other.text(column).toInt();
        }
        else{
            return text(column) < other.text(column);
        }
    }

};

#endif
