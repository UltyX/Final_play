#include "pod_play.h"


pod_play::pod_play(QStringList args)
{
    other_instance_detected=false;
    smem = new QSharedMemory("pod_play");	//create    sharedmemory
    smem->attach();				//attach to sharedmemory
    smem->create(sizeof(int));			//tell it to be big enough for a std int




    string arg;
    if(args.count()>1) {
        for(auto x: args.toStdList() ) {// write valid arguments into sharedmemory as int's then exit
            arg=x.toStdString();
cout<<arg<<endl;
            if(arg=="vol_+") {
                write_sm(volume_up_e);
            }
            if(arg=="vol_-") {
                write_sm(volume_down_e);
            }
            if(arg=="seek+") {
                write_sm(seek_forw_e);
            }
            if(arg=="seek-") {
                write_sm(seek_back_e);
            }
            if(arg=="pause") {
                write_sm(play_pause_e);
            }
            if(arg=="skipp") {
                write_sm(skipp_e);// marks it as listened
            }
            if(arg=="next") {
                write_sm(next_e);// jump but not mark as done
            }
            if(arg=="prev") {
                write_sm(prev_e);// jump but not mark as done
            }
            if(arg=="quit") {
                write_sm(quit_e);
            }
            if((arg=="--help") || (arg=="help") ||(arg=="-h")) {
                cout<<"The following comands are implemented in (pod_play.cpp):\nvol_+\nvol_-\nseek+\nseek-\npause\nquit\n--help"<<endl;
            }
            QThread::msleep(55);    // give other instance time to read and process in case of more than one event
        }
        other_instance_detected=true;
        return;
    }

    // test if the prozess is already running----------------------------------------------------------------------------------------------------------------V
    int test;
    read__sm(test);
    if(test==empty_e){
        cout <<"sharedmemory is already set_ checking if other prozess is running..."<<endl;
        write_sm(test_ping_e);        
        QThread::msleep(55);
        read__sm(test);
        if(test==empty_e) {
            write_sm(play_pause_e);     // since it has been called lets change play/pause state bevor we exit
            cout <<"very sure the prozess is already running, telling it to switch play/pause"<<endl;
            other_instance_detected=true;
            return;
        }
        cout <<"nope, it is not responding to my sharedmemory tinkering"<<endl;
    }
    // test if the prozess is already running----------------------------------------------------------------------------------------------------------------A

    write_sm(empty_e);// set sharedmemory into defined state

    QTimer *check = new QTimer(this);                                       //check QTimer
    connect(check, SIGNAL(timeout()), this, SLOT( checking()) );	//check periodicly
    check->start(50);
    
//    std::string::size_type siz;
//stoi ( settings_map["volume"] ,&siz);//here volume   

}

/*
void pod_play::save_settings()
{
file_from___map_unorderd( &settings_map, (current_dir+"settings.txt") ); 
}
*/

bool pod_play::was_other_instance_detected()
{
return other_instance_detected;
}


void pod_play::checking()
{
    int info=empty_e;

    read__sm(info);
    if(info!=empty_e)    {
        write_sm(empty_e);
        emit message_found(info);
    }    
}





void pod_play::read__sm(int &a)//maybe i can do it with a single copy of buffer and stream, if i open and close to clear them out
{

    QBuffer buffer;
    QDataStream in(&buffer);

    smem->lock();
    buffer.setData((char*)smem->constData(), smem->size());
    buffer.open(QBuffer::ReadOnly);
    in >> a;
    smem->unlock();

}

void pod_play::write_sm(int a)// do you realy need to construct a Buffer and a DataStream every time a_new, can't i just put them in the corner somewhere ?
{
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream out(&buffer);
    out << a;
    int size = buffer.size();

    smem->lock();
    char *to = (char*)smem->data();
    const char *from = buffer.data().data();
    memcpy(to, from, qMin(smem->size(), size));
    smem->unlock();
}

pod_play::~pod_play()
{
    smem->detach();	//detache from sharedmemory, we are done here
}
