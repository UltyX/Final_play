#include <string>
#include <list>
#ifndef podcast_structs_W_w
#define podcast_structs_W_w
struct Podcast;
struct Episode {
    std::string name;
    std::string dir;
    Podcast* parent;
    int last_position;
    bool listend;

};

//#include <sqlite3.h>
//investigate sqllite

struct Podcast {
    std::string name;
    std::string dir;
    std::list< Episode* > episodes;
    int raiting;
    
};

#endif
