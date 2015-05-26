#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QStringList args_i, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    comm_interface = new pod_play(args_i);                  // create instance of the shared memory communication interface
    if(comm_interface->was_other_instance_detected()){      // nothing more we need to do, commands if there have been written to s.mem
        return;                                             // return without creating the UI
    }
    connect(comm_interface,SIGNAL(message_found(int)),this,SLOT(apply_remote_commands(int)) );  // interpret recived commands HERE

cwdir= QCoreApplication::applicationDirPath();

    // Tabwidget config -begin
    QLabel *l =new QLabel;   
    l->setPixmap(QPixmap::fromImage(*(new QImage(cwdir.filePath("p2.png")))));
    l->setAlignment( Qt::AlignCenter);
  //  l->setScaledContents(true); //useless if aspect ratio is not maintained, we got have order
    l->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    ui->tabWidget->addTab( l, "");
    ui->tabWidget->tabBar()->tabButton( 0,QTabBar::RightSide)->resize(0,0);  //    hide closebutton on new_tab tab ->hide() leaves big empty space, will crash if closeable tabs not enabeled
    ui->tabWidget->tabBar()->tabButton( 0,QTabBar::RightSide)->hide();       //
    ui->tabWidget->tabBar()->setTabIcon(0,QIcon::fromTheme("list-add"));
    connect(ui->tabWidget, SIGNAL(tabBarClicked(int)),this,SLOT(add_new_on_plus_click(int)) );
    connect(ui->tabWidget->tabBar(), SIGNAL(currentChanged(int)),this,SLOT(scroll_bounds(int)) );
    // Tabwidget config -- end

                                         // location of the executable, and icons
settings_location = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);   // local of the settings

saver = new QTimer(this);                                   //save QTimer
saver->start(2000);                                                 //save signal ever x ms

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
ui->vol_sl->setValue(25);   // decent iniatail value

connect(  this, SIGNAL(seeking(qint64))        ,player,SLOT(setPosition(qint64))       );   //seek slider moved
connect(player, SIGNAL(durationChanged(qint64)),  this,SLOT(seek_sl_setLenght(qint64)) );   //update seek slider over mainwindow slot
connect(player, SIGNAL(positionChanged(qint64)),  this,SLOT(seek_sl_setValue (qint64)) );   //update seek slider over mainwindow slot


connect(ui->order_ckbox,SIGNAL(toggled(bool)),this,SLOT(generate_ordered_playlist()));


list_from_file(&locations , settings_location.filePath(".smap_fp").toStdString());  // Remember from last time what tabs where open -begin
int index_tab;
for(auto x:locations){
    index_tab = ui->tabWidget->count()-1;
    ui->tabWidget->insertTab(index_tab,new Playlist_tree_wg(player,ui->stackedWidget,saver,QString::fromStdString(x)), QString::number(index_tab));
}                                                                                   // Remember from last time what tabs where open -- end


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
    show();                     // display main window
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
        case next_e:        this->on_next_b_clicked();                          break;
        case prev_e:        this->on_previos_b_clicked();                       break;
        case save_pos_e:    std::cout<< "??? why1 "<<std::endl;                 break;
        case empty_e:       std::cout<< "??? why2 "<<std::endl;                 break;
        case test_ping_e:   std::cout<< "pinged by other instance "<<std::endl; break;
    }
}





void MainWindow::switch_play_pause()    //play / pause with one button. this slot..signal player
{   if(player->mediaStatus()== QMediaPlayer::NoMedia){
    player->playlist()->setCurrentIndex(0);                 //can't play with no media...
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





void MainWindow::closeEvent(QCloseEvent *event) // just minimize and don't close the player
{
    if (tray_icon->isVisible()) {
        hide();
        event->ignore();
    }
}

MainWindow::~MainWindow()
{
    locations.clear();                                                                                 // save tabs to disk -begin
    for(int i=0; i< (ui->tabWidget->count()-1); i++){
       locations.push_back(((Playlist_tree_wg*)ui->tabWidget->widget(i))->get_dir().toStdString());
    }
    file_from_list(&locations,settings_location.filePath(".smap_fp").toStdString() );                   // save tabs to disk -- end

    delete comm_interface;  //will result in sharedmem. not being detached if skipped !
    delete ui;
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
            raise();
            setFocus();
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





void MainWindow::on_tabWidget_tabCloseRequested(int index)  //TAB
{
    ((Playlist_tree_wg*)ui->tabWidget->widget(index))->remove_from_stack_wg();
   delete ui->tabWidget->widget(index); //also removes the tabed page YAY  ~of the wiget informs the stack wg
}

void MainWindow::scroll_bounds(int i){                      //TAB don't lest user scroll into plus tab again & stackwidget set to same index
std::cout<<i<<std::endl;
ui->stackedWidget->setCurrentIndex(i);
int number_of_tabs=ui->tabWidget->count()-1;
if(i==number_of_tabs){
    ui->tabWidget->setCurrentIndex(number_of_tabs-1);
}
}

void MainWindow::add_new_on_plus_click(int index){              //TAB


    std::cout<<index<<std::endl;
    int number_of_items=ui->tabWidget->count()-1;
    if(index==number_of_items){
        ui->tabWidget->insertTab(number_of_items,new Playlist_tree_wg(player,ui->stackedWidget,saver,""), QString::number(number_of_items));
        ui->tabWidget->setCurrentIndex(number_of_items);
        }
}

void MainWindow::generate_ordered_playlist(){
   int number_of_tabs= ui->tabWidget->count()-1;
    for(int i=0;i<number_of_tabs;i++)
    {
        ((Playlist_tree_wg*)ui->tabWidget->widget(i))->generate_ordered_playlist(ui->order_ckbox->isChecked());
    }
}



void MainWindow::on_previos_b_clicked()
{
    player->playlist()->previous();
}

void MainWindow::on_next_b_clicked()
{
    player->playlist()->next();
}
