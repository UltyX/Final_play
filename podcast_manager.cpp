#include "podcast_manager.hpp"


/*
 This class performs the middelman duties between Databas and the playlist.
 It goes over the HDD and looks for episodes and podcasts.


 usage: Ask it to scann a folder for items / subfolder_items
 it shall add these to the DB
 ask it to give you all items in the DB by certen critiria


 so we have a DB with 2 Tabels so far.
 Tabels: Raitings and Times
 Raitings stores the raitings of the given Podcasts / Folders
 Times stores the time where we left off and if the episode was marked as listened
 in the constructor we prepare the querys that we use to access the DB
*/



// Constructor runns once and opens the DB and defines the file types.
Podcast_manager::Podcast_manager()
{
    myTimer.start();    
    querys = new Querys();                  // Opens the DB and prepares the querys

    filters <<"*.mp*"<<"*.ogg"<<"*.ogv"<<"*.flac"<<"*.wav" <<"*.oga" <<"*.ogx" <<"*.ogm" <<"*.spx"<< "*.opus"<< "*.m4a"<< "*.m4b";
}





void Podcast_manager::apend_to_list(QDir Podcast_Dir,list<Podcast*> * the_list,std::set<QString>* myset_epi_i,std::set<QString>* myset_pod_i ) {
    int t=myTimer.elapsed();
    current_list = the_list;
    myset_epi = myset_epi_i;
    myset_pod = myset_pod_i;

    QSqlDatabase::database().transaction();     //WARNING THIS IS NOT HOW IT SHOULD BE USED IN RELATION TO PREPARED QUERYS ! BUT HELL IT FAST NOW WEEEEEEE

    main_dir(Podcast_Dir.absolutePath());       // find podcasts and use sub_dir to add episodes, set time and listened !!!

    QSqlDatabase::database().commit();          //WARNING THIS IS NOT HOW IT SHOULD BE USED IN RELATION TO PREPARED QUERYS ! EEEEEEEEEEEEEEEEEEEEEEEEEEEEEE

    qDebug()<< myTimer.elapsed()-t<< " ms to index the given dir (pod raiting and epsiode positions...)";

}

// find podcasts and use sub_dir to add episodes, set time and listened
void Podcast_manager::main_dir (QString path) {

    QFileInfo info(path);
    QDir dir(path);
    bool subfolder_found=false;

    if(info.isDir()){
        ;//qDebug()<<"dir";
    }else if(info.isFile()){
        qDebug()<<"sigle file not yet supported, using intire top folder for now";
        dir.cdUp(); // TODO single file
    }

    if( !dir.exists() ) {
        qDebug()<< "main_dir, did not find dir: "<< dir.absolutePath();
        return;
    }

    for ( auto temp:dir.entryInfoList(QDir::NoDotAndDotDot|QDir::Dirs|QDir::NoSymLinks) ) {       //get all folders in location

               main_dir(temp.absoluteFilePath());                           // Recursivly call self
               subfolder_found=true;                                        //either you are a Parent Dir or you are a subdir no mixing of the two
    }
    if(!subfolder_found){                                                   // no subfolders found, asume this is the only relevant folder
        add_podcast_to_list( dir.dirName() , dir.absolutePath() );          // use foldername as pocast name
    }
}


// Take the path and the name of a folder and create a Podcast out of it. Also adds the Episodes in the folder to it from the add_episodes_to_() function.
void Podcast_manager::add_podcast_to_list(QString name, QString path){
    Podcast* pod_new;

    if(myset_pod->find(name) == myset_pod->end() ){
        pod_new = new Podcast;
        pod_new->name     = name.toStdString();
        pod_new->dir      = path.toStdString();
        pod_new->raiting  = -2;                             // default value

        querys->get_raiting_from_DB(pod_new);               // ask DB for raiting
        current_list->push_front(pod_new);
    }
    for(auto podi: (*current_list) ) {                      // easyer MAP  <Name,Podcast*> it would have been with dedicated update function TODO
        if(QString::fromStdString(podi->name) == name ){
            add_episodes_to_(podi);                         // go through the folder and add all its episodes
        }
    }
}

// Creates new Episodes and initializes them. Path is taken from the parent podcast.
void Podcast_manager::add_episodes_to_(Podcast *parent_i) {

    Episode *       epi_new;    // Temp Var for episode instance
    QDir            folders;    // The location of the episodes
    QString         path = QString::fromStdString(parent_i->dir);
    folders.cd(path);

    if(!folders.exists()) {
        qDebug()<< "sub_dir, did not find dir: "<< path;
        return;
    }

    for (auto temp:folders.entryInfoList(filters,QDir::Files | QDir::NoDotAndDotDot)) {     //can also add sorting Here BY DATE
        if(myset_epi->find(temp.fileName()) == myset_epi->end() ){                          // if in set
            epi_new = new Episode;
            qDebug() << temp.fileName();
            epi_new->name               = temp.fileName().toStdString();
            epi_new->parent             = parent_i;
            epi_new->dir                = temp.absoluteFilePath().toStdString();
            epi_new->last_position      = 0;                                     // sain default value
            epi_new->listend            = false;                                 // sain default value
            querys->get_time_from_DB(epi_new);
            parent_i->episodes.push_front(epi_new);
        }
    }       
}



// Simple DB access -------------------------------------------------------------V


// update the raiting of a given podcast in the DB
void Podcast_manager::update_value(Podcast* podc)
{
    querys->set_DB_raiting_from(podc);
}

// update the last position of a given episode in the DB
void Podcast_manager::save_position(Episode *epi)
{
    querys->set_DB_time_from(epi);
}



// volume
void Podcast_manager::save_volume(int volume){
    querys->set_DB_setting_from("volume", QString::number(volume) );
}
int Podcast_manager::get_volume(){
    return (querys->get_setting_from_DB("volume")).toInt();
}
// volume


// Simple DB access -------------------------------------------------------------A




// Locations one entry for all tabnames saved in the DB and for each name one entry with their locations
QStringList Podcast_manager::get_saved_tabs_names(){
    return querys->get_setting_from_DB("tab_names").split("\n", QString::SkipEmptyParts);
}

// Saves the location for a new tab. { Name of the tab : Location }
void Podcast_manager::save_locations(QString tab_name, QStringList locations){

    QStringList  temp_list = get_saved_tabs_names();
    if(!temp_list.contains( tab_name ) ){
        temp_list.append(tab_name);
        querys->set_DB_setting_from("tab_names",temp_list.join("\n"));
    }
    QString value = locations.join("\n");
    querys->set_DB_setting_from(tab_name, value);                           // Settings table: Tabname : Path
    qDebug()<< "atempt to save "<<tab_name<<value;
}

// Asks the DB for the stored locations that will be turened into open tabs.
QStringList Podcast_manager::get_locations(QString tab_name){

    qDebug()<< "atempt to load "<<querys->get_setting_from_DB(tab_name).split("\n", QString::SkipEmptyParts);

    return querys->get_setting_from_DB(tab_name).split("\n", QString::SkipEmptyParts);
}

// Removes the tab path from the DB - called from mainwindow when a tab is closed
void Podcast_manager::delete_locations(QString tab_name){

    QStringList  temp_list = get_saved_tabs_names();

    if(temp_list.contains( tab_name ) ){
        qDebug()<<  temp_list;
        temp_list.removeAt( temp_list.lastIndexOf(tab_name) );          // remove the given path from the temp list
        qDebug()<<  temp_list;
        querys->set_DB_setting_from("tab_names",temp_list.join("\n"));  // overwite the tab paths with the same minus one less entry
    }
    else{
        qDebug()<< "delete_locations, bad call not item found";
    }
}





// DB - I/O Functions --------------------------------------------------------------------------------------------- V


/*

  http://stackoverflow.com/questions/3852068/sqlite-insert-very-slow

    INSERT is really slow - I can only do few dozen INSERTs per second

    Actually, SQLite will easily do 50,000 or more INSERT statements per second on an average desktop computer. But it will only do a few dozen transactions per second.

    Transaction speed is limited by disk drive speed because (by default) SQLite actually waits until the data really is safely stored on the disk surface before the transaction
    is complete. That way, if you suddenly lose power or if your OS crashes, your data is still safe. For details, read about atomic commit in SQLite..

    By default, each INSERT statement is its own transaction. But if you surround multiple INSERT statements with BEGIN...COMMIT then all the inserts are grouped into a single transaction.
    The time needed to commit the transaction is amortized over all the enclosed insert statements and so the time per insert statement is greatly reduced.

*/


