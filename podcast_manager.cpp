#include "podcast_manager.hpp"



// perform the middelman duties between HDD Databas and the playlist
// go over HDD and writ it into DB
// make querys to DB and give the results to the playlist that asked

// usage: Ask it to scann a folder for items / subfolder_items
// it shall add these to the DB
// ask it to give you all items in the DB by certen critiria


// so we have a DB with 2 Tabels so far.
// Tabels: Raitings and Times
// Raitings stores the raitings of the given Podcasts / Folders
// Times stores the time where we left off and if the episode was marked as listened
// in the constructor we prepare the querys that we use to access the DB

Podcast_manager::Podcast_manager()        // Constructor
{
    myTimer.start();
    qDebug()<< "manager";
    open_DB();
    filters <<"*.mp*"<<"*.ogg"<<"*.ogv"<<"*.flac"<<"*.wav" <<"*.oga" <<"*.ogx" <<"*.ogm" <<"*.spx"<< "*.opus";
    qDebug()<< "manager";
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

void Podcast_manager::main_dir (QString path) { // expand to handle single files

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
        cout << "main_dir, did not find dir: "<< dir.absolutePath().toStdString()<< endl;
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



void Podcast_manager::add_podcast_to_list(QString name,QString path){
    Podcast* pod_new;

    if(myset_pod->find(name) == myset_pod->end() ){
        pod_new = new Podcast;
        pod_new->name     = name.toStdString();
        pod_new->dir      = path.toStdString();
        pod_new->raiting  = -2;   // default value

        querys->get_raiting_from_DB(pod_new);     // ask DB for raiting
        current_list->push_front(pod_new);
    }
    for(auto podi: (*current_list) ) { // easyer MAP  <Name,Podcast*> it would have been with dedicated update function TODO
        if(QString::fromStdString(podi->name) == name ){
            add_episodes_to_(podi);// go through the folder and add all its episodes
        }
    }
}

void Podcast_manager::add_episodes_to_(Podcast *parent_i) {

    Episode * epi_new;
    QDir folders;
    QString path = QString::fromStdString(parent_i->dir);
    folders.cd(path);

    if(!folders.exists()) {
        cout << "sub_dir, did not find dir: "<< parent_i->dir<< endl;
        return;
    }

    for (auto temp:folders.entryInfoList(filters,QDir::Files | QDir::NoDotAndDotDot)) {    //can also add sorting Here BY DATE
        if(myset_epi->find(temp.fileName()) == myset_epi->end() ){ // if in set
            epi_new = new Episode;
            cout << temp.fileName().toStdString()<<endl;
            epi_new->name               =temp.fileName().toStdString();
            epi_new->parent             =parent_i;
            epi_new->dir                =temp.absoluteFilePath().toStdString();
            epi_new->last_position      =0;                                     // sain default value
             epi_new->listend            =false;                                 // sain default value
            querys->get_time_from_DB(epi_new);
            parent_i->episodes.push_front(epi_new);
        }
    }       
}







void Podcast_manager::update_value(Podcast* podc)// update the values of a given podcast in the DB
{
    querys->set_DB_raiting_from(podc);
}

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

// TODO remove old entrys HERE
// Locations one entry for all tabnames saved in the DB and for each name one entry with their locations
QStringList Podcast_manager::get_saved_tabs_names(){
    return querys->get_setting_from_DB("tab_names").split("\n", QString::SkipEmptyParts);
}
void Podcast_manager::save_locations(QString tab_name, QStringList locations){

    QStringList  temp_list=get_saved_tabs_names();
    if(!temp_list.contains( tab_name ) ){
        temp_list.append(tab_name);
        querys->set_DB_setting_from("tab_names",temp_list.join("\n"));
    }
    QString value = locations.join("\n");
    querys->set_DB_setting_from(tab_name,value);
    qDebug()<< "atempt to save "<<tab_name<<value;
}
QStringList Podcast_manager::get_locations(QString tab_name){
    qDebug()<< "atempt to load "<<querys->get_setting_from_DB(tab_name).split("\n", QString::SkipEmptyParts);
    return querys->get_setting_from_DB(tab_name).split("\n", QString::SkipEmptyParts);
}
void Podcast_manager::delete_locations(QString tab_name){
    QStringList  temp_list=get_saved_tabs_names();
    if(temp_list.contains( tab_name ) ){
        qDebug()<<  temp_list;
        temp_list.removeAt( temp_list.lastIndexOf(tab_name) );
        qDebug()<<  temp_list;
        querys->set_DB_setting_from("tab_names",temp_list.join("\n"));
    }
    else{
        qDebug()<< "delete_locations, bad call not item found";
    }
    qDebug()<< "delete_locations TODO manager";
}
// Locations


void Podcast_manager::open_DB(){// kindly only call this once, no mem leek prevention here

    foreach (QString driv, QSqlDatabase::drivers() ){
        qDebug()<< QSqlDatabase::isDriverAvailable(driv)<<driv;
    }
    //Problem querys are created bevore DB is opended so we need to open db first or create qerys after it
    QDir settings_location = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setHostName("acidalia");
    db.setDatabaseName(settings_location.filePath("FinalPlayDB_test") );
    db.setUserName("mojito");
    db.setPassword("J0a1m8");
    if( ! db.open()){std::cout<<"opening Database failed"<<db.lastError().text().toStdString()<<std::endl;}

    querys = new Querys(&db); // must be done after DB opened

}




/*
void Podcast_manager::add_from_url(QUrl where){

    if(where.path()!=where.toLocalFile()){
        qDebug()<<"warning path my missbehave Podcast_manager::add_from_url";
        qDebug()<<where.path();
        qDebug()<<where.toLocalFile();
    }
    QFileInfo info(where.toLocalFile());


    if(info.isDir()){
        main_dir(info.absoluteFilePath());
        qDebug()<<"dir";
    }else if(info.isFile()){

        qDebug()<<"file/s not yet supported";

    }
    else{qDebug()<<"Podcast_manager::add_from_url unknown filetype or other error: is no file and is no dir...";}
}

*/

// DB - I/O Functions --------------------------------------------------------------------------------------------- V


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

/* cleaning out a list
    for(auto xxp  : pc){              // clean up any old items first -begin
        for (auto xxe : xxp->episodes){
            delete xxe;
    }
    delete xxp;
    }
    pc.clear();
*/

/*
void Podcast_manager::main_dir () {


    if( !Podcast_Dir.exists() ) {
        cout << "main_dir, did not find dir: "<< Podcast_Dir.absolutePath().toStdString()<< endl;
        return;
    }

    for ( auto temp:Podcast_Dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries) ) {    //get all folders in location
        if(temp.isDir()) {                                                                  //current dir is folder?

               add_podcast_to_list(temp.baseName(),temp.absoluteFilePath());
        }
    }
    if(pc.empty()){                                                                 // no subfolders found, asume this is the only relevant folder
        add_podcast_to_list( Podcast_Dir.dirName() , Podcast_Dir.absolutePath() );  //use foldername as pocast name
    }
}
*/

