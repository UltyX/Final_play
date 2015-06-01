#include "podcast_manager.hpp"





void Podcast_manager::set_values() {
    for ( auto pod : pc) {
            get_raiting_from_DB(pod);
    }
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
    }

//set left off positions
    for (auto epi : (*parent_i).episodes) {
        get_time_from_DB(epi);
    }    
}






list<Podcast*> Podcast_manager::load_list() {  
  for(auto xxp  : pc){              // clean up any old items first -begin
    for (auto xxe : xxp->episodes){
      delete xxe;
    }
    delete xxp;
  }
    pc.clear();                     // clean up any old items first -- end

    main_dir();		// find podcasts and use sub_dir to add episodes
    set_values();	// set Rankings


    return pc;
}


void Podcast_manager::save_position(Episode *epi)
{
    set_DB_time_from(epi);
}


Podcast_manager::Podcast_manager(string dir)
{
    Podcast_Dir=QString::fromStdString(dir);

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
        posTime_AddQuery.bindValue(":location", QString::fromStdString(for_this->dir));
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
    raiting_GetQuery.exec();//
    if(raiting_GetQuery.next())
    {for_this->raiting= raiting_GetQuery.value(0).toInt();}
    else{
        qDebug()<< raiting_GetQuery.lastError().text()<<"no raitingtable for you, adding new timetable entry";
        raiting_AddQuery.bindValue(":name", QString::fromStdString(for_this->name));
        raiting_AddQuery.bindValue(":location", QString::fromStdString(for_this->dir));
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










