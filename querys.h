#ifndef QUERYS_H
#define QUERYS_H
#include "podcast_strukts.hpp"

#include <QDebug>

#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>         // needed for the addBind of querys, no idea why

#include <QSql>
#include <QDir>
#include <QStandardPaths>



class Querys
{
        QSqlQuery*  sq;
        QSqlQuery*  settings_GetQuery;
        QSqlQuery*  settings_SetQuery;
        QSqlQuery*  settings_AddQuery;
/*
        QSqlQuery  dirLoc_SetQuery;
        QSqlQuery  dirLoc_GetQuery;
        QSqlQuery  dirLoc_AddQuery;*/

        QSqlQuery*  raiting_GetQuery;
        QSqlQuery*  raiting_SetQuery;
        QSqlQuery*  raiting_AddQuery;

        QSqlQuery*  posTime_GetQuery;
        QSqlQuery*  posTime_SetQuery;
        QSqlQuery*  posTime_AddQuery;


        void print_query_error( QSqlQuery *from_this, QString info );


public:
    Querys();
    void get_time_from_DB(Episode* for_this);       // querys the DB for the Episode values and sets them in the Episode. If DB does not know the Episode the episode is added to the DB with its current values.
    void set_DB_time_from(Episode* from_this);      // atemptts to set the DB values to the current values of the Episode. Does nothing if the Episode is not found.
    void add_DB_time_from(Episode* from_this);      // Adds the requested item to the DB. No error / duplicate checking yet.

    void get_raiting_from_DB(Podcast* for_this);    // querys the DB for the Podcast values and sets them in the Podcast. If DB does not know the Podcast the episode is added to the DB with its current values.
    void set_DB_raiting_from(Podcast* from_this);   // attempts to set the DB values to the current values of the Podcast. Does nothing if the Podcast is not found.
    void add_DB_raiting_from(Podcast* from_this);   // Adds the requested item to the DB. No error / duplicate checking yet.

    void add_DB_setting_from(QString setting_name, QString value="");
    void set_DB_setting_from(QString setting_name, QString value);
 QString get_setting_from_DB(QString setting_name);
};

#endif // QUERYS_H
