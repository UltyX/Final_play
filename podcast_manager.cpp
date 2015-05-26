#include "podcast_manager.hpp"

Podcast_manager::Podcast_manager(string dir)
{
    Podcast_Dir=QString::fromStdString(dir);
    position_file_name=".pos.txt";
    value_file_name=".vale.txt";
}


void Podcast_manager::set_values() {

    std::string::size_type siz;   
    string dir=(Podcast_Dir.absoluteFilePath(QString::fromStdString(value_file_name)).toStdString());
    unordered_map<string,string> my_map;
    string key;
    string value;

    map_unorderd___from_file(&my_map,dir);

    for ( auto pod : pc) {

        key=pod->name;
        value=my_map[key];				//get value

        if(   value  == "" ) {				// check if it is still not initialisiert  (empty map when read creats std objekt which is an empty string)
            my_map[key]="0";				// add it to the file with =0 so it will be availabel in the file            
        }
        else {
            pod->raiting = stoi (value,&siz);		//Numeric Conversions string -> int
        }
    }
}

void Podcast_manager::update_values()
{    
    string dir=Podcast_Dir.absoluteFilePath(QString::fromStdString(value_file_name)).toStdString();
    unordered_map<string,string> my_map;
    for ( auto pod : pc) {
        my_map[pod->name]= std::to_string(pod->raiting);
    }
    file_from___map_unorderd(&my_map,dir);
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
            pc.front()->raiting=-1;
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
    std::string::size_type siz;
    unordered_map<string, string> pos_map; //from file
    unordered_map<string, string> pos_map_update; //from to_file if files nolonger exist
    string dir;
    string pos;
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
    dir=(folders.absoluteFilePath(QString::fromStdString(position_file_name))).toStdString();
    map_unorderd___from_file(&pos_map,dir);	// open the position file

    for (auto epi : (*parent_i).episodes) {
        pos=pos_map[epi->name];
        pos_map_update[epi->name]=pos;
        if(pos=="") {
            epi->last_position=0;
            epi->listend=false;
        }
        else if( (pos[0]!='d') && (pos[0]!='s') ) {
            epi->last_position = stoi (pos,&siz);
	    epi->listend=false;
        }
        else {
        epi->listend=true;
        }
    }
    if(pos_map.size() != pos_map_update.size() ){
        file_from___map_unorderd(&pos_map_update,dir);
        cout << " updated .pos file for "<< parent_i->dir<< endl<<"from: "<< pos_map.size()<<"to: "<<pos_map_update.size()<<"entrys"<<endl;
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
    unordered_map<string, string> pos_map;
    QDir folders=QString::fromStdString(epi->parent->dir);

    string dir=(folders.absoluteFilePath(QString::fromStdString(position_file_name))).toStdString();
    string pos_time;

    if(!epi->listend) {	//if it is still playing write the position, else mark it as done 'd'
        pos_time=to_string(epi->last_position);
    }
    else {
        pos_time="d";
    }
    map_unorderd___from_file(&pos_map,dir);	//generate map
    pos_map[epi->name]=pos_time;    	//add/overwrite position
    file_from___map_unorderd(&pos_map,dir);	//write to file
}

















