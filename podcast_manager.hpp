#ifndef podcast_manager_w_w
#define podcast_manager_w_w

#include "podcast_strukts.hpp"
#include "querys.h"
#include <string>

#include <list>
//#include <iostream>



#include <QStandardPaths>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <set>

#include<QTime> //debug

using namespace std;    // std::list

class Podcast_manager {
    QTime myTimer;//debug
  
    QStringList filters;
    Querys* querys;
    list<Podcast*> * current_list;
    std::set<QString> *myset_epi;
    std::set<QString> *myset_pod;

    void main_dir (QString path);
    void add_podcast_to_list(QString name,QString path);    //creates a new podcast in the pc list with the given path
    void add_episodes_to_(Podcast *parent_i);



public:
    // the next step rewrite it so that playlist needs to provide a Dir for an update and the regular calles to add to the list
    //
    Podcast_manager(); // TODO < V
    void apend_to_list(QDir Podcast_Dir, list<Podcast*> * the_list , std::set<QString> *myset_epi_i, std::set<QString> *myset_pod_i);    //goes thourgh the filesystem and makes a list of all the podcasts including the episode
    void save_position(Episode *epi);                                   // launch a save_pos_update query
    void update_value(Podcast *podc);                                   // launch a raiting__update query
    void save_volume(int volume);
    int  get_volume();



    QStringList get_locations(QString tab_name);
    void save_locations(QString tab_name, QStringList locations);
    QStringList get_saved_tabs_names();
    void delete_locations(QString tab_name);


  //  void add_from_url(QUrl where); // can propably be replaced by the apend to list func now ! TODO
};

#endif
