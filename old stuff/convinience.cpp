#include "convinience.hpp"


void list_from_file(list< string >* mylist, string dir )
{
    mylist->clear();
    ifstream read;
    int n=256;
    char  buffer[n];
    read.open( dir.c_str()  );
    if(!read.is_open()) {
        cout<< "vektor_from_file file not found: "<<dir<<endl;
        read.close();
        return;
    }
    while(read.getline(buffer,n,'\n'))
    {
        mylist->push_back (buffer);
    }
    read.close();
}
void map_unorderd___from_file(unordered_map<string, string>* settings_map, string dir) {

    settings_map->clear();
    int i;
    int t;
    string temp;
    ifstream read;
    int n=256;
    char  buffer[n];
    read.open( dir.c_str()  );
    if(!read.is_open()) {
        cout<< "map_unorderd__from_file file not found: "<<dir<<endl;
        read.close();
        return;
    }
    while(read.getline(buffer,n,'\n'))
    {
      i=-1;
        temp=buffer;

        while ( (( t=temp.find("=",i+1) )) !=-1 ) {
            i=t;
        };

        if(i!=-1) {
            (*settings_map)[temp.substr(0,i)]=temp.substr(i+1);
        }//   "volume=64"      settings_map["volume"]="64";

    }
    read.close();
}


void file_from_list(list< string >* mylist, string dir)
{
    ofstream write;
    write.open(dir.c_str(),ios::trunc);
    for (auto ml: (*mylist) ) {
        write.write (ml.c_str(),ml.size() );
        write.write ("\n",1);
    }
    write.close();
}
void file_from___map_unorderd(unordered_map<string, string>* settings_map, string dir) {
    ofstream write;
    string temp;
    write.open(dir.c_str(),ios::trunc);
    for (auto ml: (*settings_map) ) {
        temp=ml.first+"="+ml.second;
        write.write (temp.c_str(),temp.size() );
        write.write ("\n",1);
    }
    write.close();
}



void args_to_list(string *dir,list <string> *an, int argc,char **argv) {

    string a;
    int n=0;
    (*dir)=argv[0];
    for(int i=1; i<argc; i++) {
        n=0;
        a="";
        while(argv[i][n]) {
            a+=argv[i][n];
            n++;
        }
        an->push_back(a);
    }
}







