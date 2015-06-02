#ifndef podcast_manager_w_w
#define podcast_manager_w_w

#include "podcast_strukts.hpp"
#include <string>

#include <list>
#include <iostream>

#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlDatabase>

#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include<QTime> //debug

#include "convinience.hpp"

using namespace std;

class Podcast_manager {QTime myTimer;//debug
  
    list<Podcast*> pc;    
    QDir Podcast_Dir;

    QSqlQuery  raiting_GetQuery;
    QSqlQuery  posTime_GetQuery;

    QSqlQuery  raiting_SetQuery;
    QSqlQuery  posTime_SetQuery;

    QSqlQuery  raiting_AddQuery;
    QSqlQuery  posTime_AddQuery;

    void sub_dir(Podcast *parent_i);
    void main_dir (QString path);
    void add_podcast_to_list(QString name,QString path);    //creates a new podcast in the pc list with the given path
    void set_values();    

    void get_time_from_DB(Episode* for_this);       // querys the DB for the Episode values and sets them in the Episode. If DB does not know the Episode the episode is added to the DB with its current values.
    void set_DB_time_from(Episode* from_this);      // atemptts to set the DB values to the current values of the Episode. Does nothing if the Episode is not found.
    void add_DB_time_from(Episode* from_this);      // Adds the requested item to the DB. No error / duplicate checking yet.

    void get_raiting_from_DB(Podcast* for_this);    // querys the DB for the Podcast values and sets them in the Podcast. If DB does not know the Podcast the episode is added to the DB with its current values.
    void set_DB_raiting_from(Podcast* from_this);   // attempts to set the DB values to the current values of the Podcast. Does nothing if the Podcast is not found.
    void add_DB_raiting_from(Podcast* from_this);   // Adds the requested item to the DB. No error / duplicate checking yet.
    
public:
    Podcast_manager(string dir);
    
    list<Podcast*> load_list();		//goes thourgh the filesystem and makes a list of all the podcasts including the episodes. calling it again will destroy all previes objekts and replace them... Warning invalid püointers posible!
    void save_position(Episode *epi);	//still a good idea to let this class handle the saving process
    void update_values(Podcast *podc);
    void add_from_url(QUrl where);
};



#endif
