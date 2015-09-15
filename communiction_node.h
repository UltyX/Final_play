#ifndef communiction_node_H
#define communiction_node_H



#include <QtCore/QObject>	//the default QT Objekt for pod_play, giving me all the QT goodness
#include <qthread.h>
#include <qtimer.h>

#include <QSharedMemory>	// comunication with
#include <QBuffer>		// othe instanzes of
#include <QDataStream>		// the aplication
#include <QStringList>
#include <iostream>
using namespace std;
class communiction_node : public QObject	// inherates from QObject
{
    Q_OBJECT			// no Idea what that macro does

    QSharedMemory *smem;	//comunication with othe instanzes of the aplication
    void write_sm(int a);	// lock  write unlock smem
    void read__sm(int& a);	// lock  read  unlock smem    

    string current_dir;     
    bool other_instance_detected;    

signals:
    void message_found(int msg);

private slots:
    void checking();		//call this to see if there has been a command send

public:
    communiction_node(QStringList args);
    bool was_other_instance_detected();
    virtual ~communiction_node();
};

enum settings {volume_up_e,volume_down_e,seek_forw_e,seek_back_e,play_pause_e,quit_e,skipp_e,next_e,prev_e,save_pos_e,empty_e,test_ping_e};

#endif // pod_play_H
