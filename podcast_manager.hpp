#ifndef podcast_manager_w_w
#define podcast_manager_w_w

#include "podcast_strukts.hpp"
#include <string>

#include <list>
#include <iostream>

#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include "convinience.hpp"

using namespace std;

class Podcast_manager {
  
    list<Podcast*> pc;    
    QDir Podcast_Dir;

    QSqlQuery  raiting_GetQuery;
    QSqlQuery  posTime_GetQuery;

    QSqlQuery  raiting_SetQuery;
    QSqlQuery  posTime_SetQuery;

    QSqlQuery  raiting_AddQuery;
    QSqlQuery  posTime_AddQuery;

    void sub_dir(Podcast *parent_i);
    void main_dir ();
    void set_values();    

    void get_time_from_DB(Episode* for_this);
    void set_DB_time_from(Episode* from_this);

    void get_raiting_from_DB(Podcast* for_this);
    void set_DB_raiting_from(Podcast* from_this);

    
public:
    Podcast_manager(string dir);
    
    list<Podcast*> load_list();		//goes thourgh the filesystem and makes a list of all the podcasts including the episodes. calling it again will destroy all previes objekts and replace them... Warning invalid püointers posible!
    void save_position(Episode *epi);	//still a good idea to let this class handle the saving process
    void update_values(Podcast *podc);
    
};



#endif
