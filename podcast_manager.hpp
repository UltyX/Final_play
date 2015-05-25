#ifndef podcast_manager_w_w
#define podcast_manager_w_w

#include "podcast_strukts.hpp"
#include <string>

#include <list>
#include <iostream>


#include <QFile>
#include <QFileInfo>
#include <QDir>

#include "convinience.hpp"

using namespace std;

class Podcast_manager {
  
    list<Podcast*> pc;    
    QDir Podcast_Dir;
    string position_file_name;
    string value_file_name;


    void sub_dir(Podcast *parent_i);
    void main_dir ();
    void set_values();    

    
public:
    Podcast_manager(string dir);
    
    list<Podcast*> load_list();		//goes thourgh the filesystem and makes a list of all the podcasts including the episodes. calling it again will destroy all previes objekts and replace them... Warning invalid püointers posible!
    void save_position(Episode *epi);	//still a good idea to let this class handle the saving process
    void update_values();
    
};



#endif
