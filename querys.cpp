#include "querys.h"

Querys::Querys(QSqlDatabase *db)// TODO deletion, we only add and never delete, this may posibly fill the DB up a tiny bit.
{

    QSqlQuery sq((*db));  //recreate in case it got lost

    // The raitings table folder and its raiting
    if( ! sq.prepare( "CREATE TABLE Raitings(name   TEXT PRIMARY KEY, location   TEXT ,raiting   INT )" )){print_query_error(&sq ,"preparing query failed ");}
    if( ! sq.exec()){print_query_error(&sq, "proppably already exsits. creating table failed" ); }

    // The Time table, all files time played and if done
    if( ! sq.prepare( "CREATE TABLE Times(name   TEXT PRIMARY KEY, location TEXT ,playtime   INT, done BOOL )" )){print_query_error(&sq ,"preparing query failed ");}
    if( ! sq.exec()){print_query_error(&sq, "proppably already exsits. creating table failed" ); }

    // The settings table Here I will store Stuff like Volume
    if( ! sq.prepare( "CREATE TABLE Settings(name   TEXT PRIMARY KEY, value_string TEXT )" )){print_query_error(&sq ,"preparing query failed ");}
    if( ! sq.exec()){print_query_error(&sq, "proppably already exsits. creating table failed" ); }


    if(! raiting_GetQuery.prepare("SELECT raiting FROM Raitings WHERE name = ?"))         {print_query_error(&raiting_GetQuery,"preparing query failed ");}
    if(! posTime_GetQuery.prepare("SELECT playtime, done FROM Times WHERE name = ?"))     {print_query_error(&posTime_GetQuery,"preparing query failed ");}
    if(!settings_GetQuery.prepare("SELECT value_string FROM Settings WHERE name = ?" ))    {print_query_error(&settings_GetQuery,"preparing query failed ");}

    if(! raiting_SetQuery.prepare("UPDATE Raitings SET raiting = ? where name= ?"))         {print_query_error(&raiting_SetQuery,"preparing query failed ");}
    if(! posTime_SetQuery.prepare("UPDATE Times SET playtime = ?,done = ? where name= ?"))  {print_query_error(&posTime_SetQuery,"preparing query failed ");}
    if(!settings_SetQuery.prepare("UPDATE Settings SET value_string = ? where name= ?" ))   {print_query_error(&settings_SetQuery,"preparing query failed ");}

    if(!raiting_AddQuery.prepare("INSERT INTO Raitings(name, location, raiting) "
                               "VALUES (:name, :local, :raiting)")){
        print_query_error(&raiting_AddQuery,"preparing query failed ");
    }
    if(! posTime_AddQuery.prepare("INSERT INTO Times(name, location, playtime, done) "
                               "VALUES (:name, :local, :playtime, :done)")){
         print_query_error(&posTime_AddQuery,"preparing query failed ");
    }
    if(!settings_AddQuery.prepare("INSERT INTO Settings(name, value_string) "
                               "VALUES (:name, :value_string)")){
        print_query_error(&settings_AddQuery,"preparing query failed ");
    }

}




// Setting Querys
QString Querys::get_setting_from_DB(QString setting_name){

    settings_GetQuery.addBindValue(setting_name);
    settings_GetQuery.exec();
    if(settings_GetQuery.next()){
        return settings_GetQuery.value(0).toString() ;
    }
    else{
        print_query_error( &settings_GetQuery,"no setting for you, adding new Settings entry "+setting_name );
        add_DB_setting_from(setting_name);
    }
    return "";
}
void Querys::set_DB_setting_from(QString setting_name,QString value){

    get_setting_from_DB(setting_name);  // makes sure we have an entry and if not make one
    settings_SetQuery.addBindValue(value);
    settings_SetQuery.addBindValue(setting_name);
    settings_SetQuery.exec();   // can't use this, it does not error when no entry found to update
}

void Querys::add_DB_setting_from(QString setting_name, QString value){

    settings_AddQuery.bindValue(":name"          , setting_name);
    settings_AddQuery.bindValue(":value_string"  , value);
    settings_AddQuery.exec();
}
// Setting Querys


// Time Querys
void Querys::get_time_from_DB(Episode* for_this){

    posTime_GetQuery.addBindValue(QString::fromStdString(for_this->name));
    posTime_GetQuery.exec();
    if(posTime_GetQuery.next()){
        for_this->last_position = posTime_GetQuery.value(0).toInt();
        for_this->listend       = posTime_GetQuery.value(1).toBool();
    }
    else{
        print_query_error( &posTime_GetQuery,"no time for you, adding new timetable entry"+QString::fromStdString(for_this->name) );
        add_DB_time_from(for_this);
    }
}
void Querys::set_DB_time_from(Episode* from_this){

    posTime_SetQuery.addBindValue(from_this->last_position);
    posTime_SetQuery.addBindValue(from_this->listend);
    posTime_SetQuery.addBindValue(QString::fromStdString(from_this->name));
    posTime_SetQuery.exec();
}
void Querys::add_DB_time_from(Episode* from_this){

    posTime_AddQuery.bindValue(":name"      , QString::fromStdString(from_this->name));
    posTime_AddQuery.bindValue(":location"  , QString::fromStdString(from_this->dir)); //TODO make a extra funktion for Adding episodes
    posTime_AddQuery.bindValue(":playtime"  , from_this->last_position);
    posTime_AddQuery.bindValue(":done"      , from_this->listend);
    posTime_AddQuery.exec();
}
// Time Querys


// Raiting Querys
void Querys::get_raiting_from_DB(Podcast* for_this){

    qDebug() <<QString::fromStdString(for_this->name);
    raiting_GetQuery.addBindValue(QString::fromStdString(for_this->name));
    raiting_GetQuery.exec();
    if(raiting_GetQuery.next())
    {
        for_this->raiting= raiting_GetQuery.value(0).toInt();
    }
    else{
        print_query_error(&raiting_GetQuery,"no raitingtable for you, adding new timetable entry") ;
        add_DB_raiting_from(for_this);
    }
}

void Querys::set_DB_raiting_from(Podcast* from_this){

    raiting_SetQuery.addBindValue(from_this->raiting);
    raiting_SetQuery.addBindValue(QString::fromStdString(from_this->name));
    if(!  raiting_SetQuery.exec()){
        print_query_error(&raiting_SetQuery, "set_DB_raiting_from");
    }
    else{
        qDebug()<< from_this->raiting <<" should be now in table";
    }
}

void Querys::add_DB_raiting_from(Podcast* from_this){

    raiting_AddQuery.bindValue(":name"     , QString::fromStdString(from_this->name));
    raiting_AddQuery.bindValue(":location" , QString::fromStdString(from_this->dir)); //TODOmake a extra funktion for Adding podcasts
    raiting_AddQuery.bindValue(":raiting"  , from_this->raiting);
    raiting_AddQuery.exec();
}
// Raiting Querys

void Querys::print_query_error(QSqlQuery *from_this, QString info = ""){
    qDebug() <<from_this->lastError().text()<<" "<<info;
}
