#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlError>
#include <QSplitter>  // alow user at some point to resize the ratio between widgetsizes TODO

MainWindow::MainWindow(QStringList args_i, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    comm_interface = new communiction_node(args_i);                  // create instance of the shared memory communication interface
    if(comm_interface->was_other_instance_detected()){      // nothing more we need to do, commands if there have been written to s.mem
        return;                                             // return without creating the UI
    }
    connect(comm_interface,SIGNAL(message_found(int)),this,SLOT(apply_remote_commands(int)) );  // interpret recived commands HERE
    manager = new Podcast_manager();
    current_playlist = NULL;

    cwdir= QCoreApplication::applicationDirPath();

    // Tabwidget config -begin
    QLabel *l =new QLabel;   
    l->setPixmap(QPixmap::fromImage(*(new QImage(cwdir.filePath("p2.png")))));
    l->setAlignment( Qt::AlignCenter);
    //l->setScaledContents(true); //useless if aspect ratio is not maintained, we got have order
    l->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    ui->tabWidget->addTab( l, "");
    ui->tabWidget->tabBar()->tabButton( 0,QTabBar::RightSide)->resize(0,0);  //    hide closebutton on new_tab tab ->hide() leaves big empty space, will crash if closeable tabs not enabeled
    ui->tabWidget->tabBar()->tabButton( 0,QTabBar::RightSide)->hide();       //
    ui->tabWidget->tabBar()->setTabIcon(0,QIcon::fromTheme("list-add"));
    connect(ui->tabWidget, SIGNAL(tabBarClicked(int)),this,SLOT(add_new_on_plus_click(int)) );
    connect(ui->tabWidget->tabBar(), SIGNAL(currentChanged(int)),this,SLOT(scroll_bounds(int)) );
    // Tabwidget config -- end

                                         // location of the executable, and icons



    saver = new QTimer(this);                                        //save QTimer, will be connected to the Playlist_tree_wg's.
    saver->start(2000);                                              //save signal ever x ms

    player = new QMediaPlayer(this);    // THE MEDIA PLAYER BACKEND HERE



    connect(this,SIGNAL(play())         ,player ,SLOT(play()));       //play  button hooked up over main window signal emit for play/pause switching
    connect(this,SIGNAL(pause())        ,player ,SLOT(pause()));      //pause button hooked up over main window signal emit for play/pause switching
    connect(this,SIGNAL(play_pause())   ,this   ,SLOT(switch_play_pause()));    //switch between playing and pause, had to implement it myselfe. not in player as funktion available


    ui->previos_b->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));    // give it the nice icon
    ui->next_b->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));        // give it the nice icon

    connect(ui->play_b   ,SIGNAL(clicked()), this, SIGNAL(play_pause()));   // play/pause button pressed, emits just a signal
    ui->play_b->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));       // give it the nice play icon

    ui->vol_sl->setRange(0,100); //set range 0-100%
    connect(ui->vol_sl,SIGNAL(valueChanged(int)),player,SLOT(setVolume(int)));  //volume slider moved
    connect(ui->vol_sl,SIGNAL(sliderReleased()),this,SLOT(save_volume()) );
    connect(ui->vol_sl,SIGNAL(valueChanged(int)),this,SLOT(save_volume()) );
    ui->vol_sl->setValue(manager->get_volume());   // decent iniatail value

    connect(  this, SIGNAL(seeking(qint64))        ,player,SLOT(setPosition(qint64))       );   //seek slider moved
    connect(player, SIGNAL(durationChanged(qint64)),  this,SLOT(seek_sl_setLenght(qint64)) );   //update seek slider over mainwindow slot
    connect(player, SIGNAL(positionChanged(qint64)),  this,SLOT(seek_sl_setValue (qint64)) );   //update seek slider over mainwindow slot







    // Icon begin
    QIcon icon_temp(cwdir.filePath("xx.png"));          // generate a temporary icon
    setWindowIcon(icon_temp);                           // set windows icon to same as tray/temp icon
    tray_icon =  new QSystemTrayIcon(icon_temp, this);  // generate the system tray objekt withe the temp icon

    connect( tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(tray_icon_clicked( QSystemTrayIcon::ActivationReason  )) );  // on left-,dubble-,middel- click

    QAction *quit_action = new QAction( "Exit", tray_icon );            // tray menu exit option
    connect( quit_action, SIGNAL(triggered()), qApp, SLOT(quit()));     //

    QAction *up_action = new QAction( "update", tray_icon );                                // tray menu re- generate playlist
    connect( up_action, SIGNAL(triggered()), this, SLOT(generate_ordered_playlist()) );     //

    QMenu *tray_icon_menu = new QMenu("File");              // make the menu
    tray_icon_menu->addAction( up_action );                 // add the option
    tray_icon_menu->addAction( quit_action );               // add the option
    tray_icon->setContextMenu( tray_icon_menu );            /// link menu with tray objekt
    ui->menuBar->addMenu( tray_icon_menu );
    tray_icon->show();
    // Icon end


    setWindowTitle("Final Play");   // in windo decoration set app name
    show();                         // display main window

    foreach ( QString tab_name, manager->get_saved_tabs_names() ){  //resotre tabs from DB
        add_tab( manager->get_locations(tab_name),tab_name );
    }



}
/*
*  TODO:
*
*
* make settings store at ~/.config/final_play/   voluem  podcast_dir   maybe last positions...  and set to previos settings Volume!...
* bool setup done bad workaround shold just diconect and conect signals if posible
*
*   http://stackoverflow.com/questions/23775424/sort-qtreewidget-toplevel-item-base-on-a-child-data
*
*  /home/ulty/projects/qt_create/Examples/Qt-5.4/widgets/dialogs/tabdialog/
*
*/


bool MainWindow::singel_app(){
// if args >1 send args to shared memory and quit, else check if no other process is running (quit: this and play_pause: other. if True)
// else scann the shared memory every x ms for commands from other instances
return comm_interface->was_other_instance_detected();
}

void MainWindow::apply_remote_commands(int comand){ //comm_interface looks for comand and gives it to this slot if it found one

    switch(comand){
        case volume_up_e:   ui->vol_sl->setValue(ui->vol_sl->value()+10 );      break;
        case volume_down_e: ui->vol_sl->setValue(ui->vol_sl->value()-10 );      break;
        case seek_forw_e:   player->setPosition(  player->position()+40000 );   break;
        case seek_back_e:   player->setPosition(  player->position()-40000 );   break;
        case play_pause_e:  emit play_pause();                                  break;
        case quit_e:                                                            break;
        case skipp_e:                                                           break;
        case next_e:        ui->next_b->pressed();                              break;
        case prev_e:        ui->previos_b->pressed();                           break;
        case save_pos_e:    std::cout<< "??? why1 "<<std::endl;                 break;
        case empty_e:       std::cout<< "??? why2 "<<std::endl;                 break;
        case test_ping_e:   std::cout<< "pinged by other instance "<<std::endl; break;
    }
}





void MainWindow::switch_play_pause()    //play / pause with one button. this slot..signal player
{
    if(player->mediaStatus()== QMediaPlayer::NoMedia){
        return;              //can't play with no media...
    }
    if(player->state() != QMediaPlayer::PlayingState){      //ask if playing or not (paused)
        emit play();                                                        // tell it over the singal/slot connection to play
        ui->play_b->setIcon(style()->standardIcon(QStyle::SP_MediaPause));  // set the pause icon on the button
    }
    else{
        emit pause();                                                       // tell it over the singal/slot connection to pause
        ui->play_b->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));   // set the play icon on the button
    }
}


void MainWindow::seek_sl_setLenght(qint64 lenght)//since the slider got no fitting slot for set lenght compatible with player signals I wrote this over main window.  this slot..signal player
{
    ui->seek_sl->setMaximum(lenght);
    lenght=lenght/1000;
    QTime totalTime((lenght/3600)%60, (lenght/60)%60, lenght%60);
    QString tStr = totalTime.toString("hh:mm:ss");
    ui->lcdNumber_2->display(tStr);
    if(current_playlist!=NULL){
        current_playlist->set_duration(tStr);
    }
}

void MainWindow::seek_sl_setValue(qint64 pos)//since the slider got no fitting slot for set value compatible with player signals I wrote this over main window. this slot..signal player
{
    if (!ui->seek_sl->isSliderDown()) {
        ui->seek_sl->setValue(pos);
    }    
}

void MainWindow::on_seek_sl_sliderReleased()// since the slider has no on release signal I wrote my own, so i don't get the jitter nois when seeking
{
    emit seeking(ui->seek_sl->value());
}

void MainWindow::save_volume(){
    if(!ui->vol_sl->isSliderDown()){
        manager->save_volume( ui->vol_sl->value() );
    }
}



void MainWindow::tray_icon_clicked(QSystemTrayIcon::ActivationReason reason){   //ICON RIGHTCLICK MENU DISPLAY
    if(reason==QSystemTrayIcon::MiddleClick){
    emit play_pause();
    }
    else if(reason==QSystemTrayIcon::Trigger  ){    //noramal leftclick hide_unhide takes about 1,5sec... no idea why ?...
        if(isVisible()){
            hide();
        }
        else
        {
            show();
            setFocus();
            activateWindow();
        }
    }
}


void MainWindow::on_seek_sl_valueChanged(int value) //on value change gives us always the currend slider pos even when user is dragging it around
{
   int pos=value/1000;  //ms-> sec
    if (pos) {
        QTime currentTime((pos/3600)%60, (pos/60)%60, pos%60);
         ui->lcdNumber->display(currentTime.toString("hh:mm:ss"));
    }else{
        ui->lcdNumber->display("00:00:00");
    }
}







void MainWindow::scroll_bounds(int i){                      //TAB don't lest user scroll into plus tab again & stackwidget set to same index

    ui->stackedWidget->setCurrentIndex(i);
    int number_of_tabs=ui->tabWidget->count()-1;
    if(i==number_of_tabs){
        ui->tabWidget->setCurrentIndex(number_of_tabs-1);
    }
}

void MainWindow::add_new_on_plus_click(int index){              //TAB ADD

    int number_of_items=ui->tabWidget->count()-1;
    if(index==number_of_items){
       add_tab();
    }
}
void MainWindow::add_tab(QStringList locations, QString tab_name){    // instance and connect new Tab
    int number_of_items=ui->tabWidget->count()-1;
    if(tab_name.isEmpty()){
        int i =0;
        QStringList names = manager->get_saved_tabs_names();
        do{
            tab_name=QString::number(i);
            i++;
        }while(names.contains(tab_name));
    }
    Playlist_tree_wg* new_playlist = new Playlist_tree_wg(player,manager,ui->stackedWidget,saver,locations,tab_name);

    connect( new_playlist,SIGNAL( connect_me(Playlist_tree_wg*) ),this,SLOT( connect_playlist(Playlist_tree_wg*))   ); // playlist has been set current
    connect( new_playlist,SIGNAL( disconnect_me(Playlist_tree_wg*)),this,SLOT(disconnect_playlist(Playlist_tree_wg*)));

    connect( new_playlist,SIGNAL( current_wg_update(Playlist_tree_wg*)  ),this,SIGNAL( current_wg_update(Playlist_tree_wg*)  )); // tell others to disconect
    connect( this,SIGNAL( current_wg_update(Playlist_tree_wg*)  ),new_playlist,SLOT(set_current(Playlist_tree_wg*))  );         // tell others to disconect

    ui->tabWidget->insertTab(number_of_items, new_playlist  , tab_name);
    ui->tabWidget->setCurrentIndex(number_of_items);
    if(current_playlist == NULL){   // set a ini playlist
        new_playlist->set_current( new_playlist );
        current_playlist = new_playlist;
        ui->row_sort0_b->addItems(  current_playlist->get_row_names() ); // get _row for button
    }
}

void MainWindow::connect_playlist(Playlist_tree_wg* playlist_wg){ // Connect TAB
    //emit current_wg_update(playlist_wg);
    connect(ui->previos_b,SIGNAL(pressed()),playlist_wg,SLOT(play_prev()) );
    connect(ui->next_b,SIGNAL(pressed()),playlist_wg,SLOT(play_next()) );
    playlist_wg->connect_signals();
}
void MainWindow::disconnect_playlist(Playlist_tree_wg* playlist_wg){  // DISConnect TAB
    disconnect(ui->previos_b,SIGNAL(pressed()),playlist_wg,SLOT(play_prev()) );
    disconnect(ui->next_b,SIGNAL(pressed()),playlist_wg,SLOT(play_next()) );
    playlist_wg->disconnect_temp();
}



void MainWindow::generate_ordered_playlist(){   // regenerate playlists
   int number_of_tabs= ui->tabWidget->count()-1;
    for(int i=0;i<number_of_tabs;i++)
    {
        ((Playlist_tree_wg*)ui->tabWidget->widget(i))->generate_ordered_playlist();
    }
}




// delete functions -------------------------------------------------------------------------------------------------V

void MainWindow::on_tabWidget_tabCloseRequested(int index)  //close TAB
{
    manager->delete_locations(((Playlist_tree_wg*)ui->tabWidget->widget(index))->get_name());

    ((Playlist_tree_wg*)ui->tabWidget->widget(index))->remove_from_stack_wg();  // remove stack WG
    delete ui->tabWidget->widget(index);                                         // remove playlist tab WG
}

void MainWindow::closeEvent(QCloseEvent *event) // just minimize and don't close the player
{
    if (tray_icon->isVisible()) {
        hide();
        event->ignore();
    }
}

MainWindow::~MainWindow()
{
    delete comm_interface;  //will result in sharedmem. not being detached if skipped !
    delete ui;
}
