#include "podcast_manager.hpp"





void Podcast_manager::set_values() {
    int t=myTimer.elapsed();
    for ( auto pod : pc) {
            get_raiting_from_DB(pod);
    }
    qDebug()<< myTimer.elapsed()-t<< " ms to check and add the raitings";
}

void Podcast_manager::update_values(Podcast* podc)
{        
        set_DB_raiting_from(podc);
}





void Podcast_manager::main_dir () {

    string sub;
    if( !Podcast_Dir.exists() ) {
        cout << "main_dir, did not find dir: "<< Podcast_Dir.absolutePath().toStdString()<< endl;
        return;
    }
    for ( auto temp:Podcast_Dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries) ) {

        if(temp.isDir()) {		//current dir is folder?
            sub= temp.absoluteFilePath().toStdString();	//use foldername as pocast name
            pc.push_front(new Podcast);
            pc.front()->name=temp.baseName().toStdString();
            pc.front()->dir=sub;
            pc.front()->raiting=-2;
            sub_dir(pc.front());	// go through the folder and add all its episodes
        }
    }
    if(pc.empty()){ // case user has only choosen a single folder as target /// not good with raiting yet.. will create useless .value in folder ! TODO
        sub= Podcast_Dir.absolutePath().toStdString();	//use foldername as pocast name
        pc.push_front(new Podcast);
        pc.front()->name=Podcast_Dir.dirName().toStdString();
        pc.front()->dir=sub;
        pc.front()->raiting=-1;
        sub_dir(pc.front());	// go through the folder and add all its episodes
    }
}

void Podcast_manager::sub_dir(Podcast *parent_i) {

    QDir folders;
    QStringList filters;
    folders.cd(parent_i->dir.c_str());

    if(!folders.exists()) {
        cout << "sub_dir, did not find dir: "<< parent_i->dir<< endl;
        return;
    }
    filters <<"*.mp*"<<"*.ogg"<<"*.ogv"<<"*.flac"<<"*.wav" <<"*.oga" <<"*.ogx" <<"*.ogm" <<"*.spx"<< "*.opus";
    for (auto temp:folders.entryInfoList(filters,QDir::Files)) {    //can also add sorting Here

        parent_i->episodes.push_front(new Episode);
        parent_i->episodes.front()->name=temp.fileName().toStdString();
        parent_i->episodes.front()->parent=parent_i;
        parent_i->episodes.front()->dir=temp.absoluteFilePath().toStdString();
        parent_i->episodes.front()->last_position=0;                            // sain default value
        parent_i->episodes.front()->listend=false;                              // sain default value
    }
int t=myTimer.elapsed();
    for (auto epi : (*parent_i).episodes) {
        get_time_from_DB(epi);                  // if already in DB get time else add with default
    }    
    qDebug()<< myTimer.elapsed()-t<< " ms to check and add the time, for "<<(*parent_i).episodes.size() ;
}






list<Podcast*> Podcast_manager::load_list() {  
    int t=myTimer.elapsed();
  for(auto xxp  : pc){              // clean up any old items first -begin
    for (auto xxe : xxp->episodes){
      delete xxe;
    }
    delete xxp;
  }
    pc.clear();                     // clean up any old items first -- end
QSqlDatabase::database().transaction();     //WARNING THIS IS NOT HOW IT SHOULD BE USED IN RELATION TO PREPARED QUERYS ! BUT HELL IT FAST NOW WEEEEEEE
    main_dir();		// find podcasts and use sub_dir to add episodes, set time and listened !!!



qDebug()<< myTimer.elapsed()-t<< " ms aaa)";
    set_values();	// set Rankings

qDebug()<< myTimer.elapsed()-t<< " ms to index the given dir (pod raiting and epsiode positions...)";
QSqlDatabase::database().commit();          //WARNING THIS IS NOT HOW IT SHOULD BE USED IN RELATION TO PREPARED QUERYS ! EEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
    return pc;
}


void Podcast_manager::save_position(Episode *epi)
{
    set_DB_time_from(epi);
}


Podcast_manager::Podcast_manager(string dir)
{
    Podcast_Dir=QString::fromStdString(dir);


    myTimer.start();  //debug because very slow when adding new entrys about 13ms per entry. maybe timeout for failed select call ? TODO test with no select and only add 220 times and see if still slow

   if(! raiting_GetQuery.prepare("SELECT raiting FROM Raitings WHERE name = ?")){qDebug() <<raiting_GetQuery.lastError().text();}
   if(! posTime_GetQuery.prepare("SELECT playtime, done FROM Times WHERE name = ?")){qDebug() <<posTime_GetQuery.lastError().text();}

   if(! raiting_SetQuery.prepare("UPDATE Raitings SET raiting = ? where name= ?")){qDebug() <<raiting_SetQuery.lastError().text();}
   if(! posTime_SetQuery.prepare("UPDATE Times SET playtime = ?,done = ? where name= ?")){qDebug() <<posTime_SetQuery.lastError().text();}


   if(!  raiting_AddQuery.prepare("INSERT INTO Raitings(name, location, raiting) "
                              "VALUES (:name, :local, :raiting)")){
       qDebug() <<raiting_AddQuery.lastError().text();
   }

    if(! posTime_AddQuery.prepare("INSERT INTO Times(name, location, playtime, done) "
                              "VALUES (:name, :local, :playtime, :done)")){
        qDebug() <<posTime_AddQuery.lastError().text();
    }

}



void Podcast_manager::get_time_from_DB(Episode* for_this){

    posTime_GetQuery.addBindValue(QString::fromStdString(for_this->name));
    posTime_GetQuery.exec();
    if(posTime_GetQuery.next()){
        for_this->last_position = posTime_GetQuery.value(0).toInt();
        for_this->listend       = posTime_GetQuery.value(1).toBool();   //TODO does not save and recall raiting...
    }
    else{
        qDebug()<< posTime_GetQuery.lastError().text()<<"no time for you, adding new timetable entry";
        posTime_AddQuery.bindValue(":name", QString::fromStdString(for_this->name));
        posTime_AddQuery.bindValue(":location", QString::fromStdString(for_this->dir)); //TODO make a extra funktion for Adding episodes
        posTime_AddQuery.bindValue(":playtime", for_this->last_position);
        posTime_AddQuery.bindValue(":done", for_this->listend);
        posTime_AddQuery.exec();
    }
}
void Podcast_manager::set_DB_time_from(Episode* from_this){

    posTime_SetQuery.addBindValue(from_this->last_position);
    posTime_SetQuery.addBindValue(from_this->listend);
    posTime_SetQuery.addBindValue(QString::fromStdString(from_this->name));
    posTime_SetQuery.exec();
}

void Podcast_manager::get_raiting_from_DB(Podcast* for_this){

    raiting_GetQuery.addBindValue(QString::fromStdString(for_this->name));
    raiting_GetQuery.exec();
    if(raiting_GetQuery.next())
    {for_this->raiting= raiting_GetQuery.value(0).toInt();}
    else{
        qDebug()<< raiting_GetQuery.lastError().text()<<"no raitingtable for you, adding new timetable entry";
        raiting_AddQuery.bindValue(":name", QString::fromStdString(for_this->name));
        raiting_AddQuery.bindValue(":location", QString::fromStdString(for_this->dir)); //TODOmake a extra funktion for Adding podcasts
        raiting_AddQuery.bindValue(":raiting", for_this->raiting);
        raiting_AddQuery.exec();
    }

}
void Podcast_manager::set_DB_raiting_from(Podcast* from_this){

    raiting_SetQuery.addBindValue(from_this->raiting);
    raiting_SetQuery.addBindValue(QString::fromStdString(from_this->name));
  if(!  raiting_SetQuery.exec()){qDebug() <<raiting_SetQuery.lastError().text();}
  else{qDebug()<< from_this->raiting <<"should be now in table";}
}
/*

  http://stackoverflow.com/questions/3852068/sqlite-insert-very-slow

    INSERT is really slow - I can only do few dozen INSERTs per second

    Actually, SQLite will easily do 50,000 or more INSERT statements per second on an average desktop computer. But it will only do a few dozen transactions per second.

    Transaction speed is limited by disk drive speed because (by default) SQLite actually waits until the data really is safely stored on the disk surface before the transaction is complete. That way, if you suddenly lose power or if your OS crashes, your data is still safe. For details, read about atomic commit in SQLite..

    By default, each INSERT statement is its own transaction. But if you surround multiple INSERT statements with BEGIN...COMMIT then all the inserts are grouped into a single transaction. The time needed to commit the transaction is amortized over all the enclosed insert statements and so the time per insert statement is greatly reduced.

*/




/*
QString sqlQuery = QString("SELECT name FROM sqlite_master WHERE type =:table AND name = :tablename ");
query.prepare(sqlQuery);
query.bindValue(":table", "table");
query.bindValue(":tablename", tableName);
query.exec();
*/




