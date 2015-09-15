#ifndef conv_w_W
#define conv_w_W

#include <string>
#include <list>
#include <unordered_map>
#include <unistd.h>//P-ID
#include <fstream>
#include <iostream>
#include "podcast_strukts.hpp"
//#define NDEBUG  // uncomment this if you want it to be produktion ready TODO
#include <assert.h>

using namespace std;

void args_to_list  (string *dir,list <string> *an, int argc,char **argv);
void list_from_file(list< string >* mylist, string dir );
void file_from_list(list< string >* mylist, string dir);

void file_from___map_unorderd(unordered_map< string, string >* settings_map, string dir);// goes through the map and writs it to the file at dir like this:  <test, 213> --> test=123\n ...
void map_unorderd___from_file(unordered_map< string, string >* settings_map, string dir);// turns a file with test=123 into a map with  key[ test ] and value[ 213 ] ...  <string, string> and <test, 213>

//string get_dir_of_exe(string argv_0);				//turn the first argv into a string like /home/ulty/.../pod_play/build/

#endif
