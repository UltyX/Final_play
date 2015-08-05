#include "playlist_tree_wg.h"

Playlist_tree_wg::Playlist_tree_wg(QMediaPlayer* the_player, QStackedWidget *stack_wg, QTimer* save_timer,QString location,Playlist_tree_wg** current_playlist_wg_mem_p):QTreeWidget()
{
    timer = save_timer;
    player = the_player;                                        // the only player in this programm, we share it between tabs and use its signals to set position, also we give it our playlist
    corresponing_stack_wg = stack_wg;                           // the stack WG, we put our Raiting WG on it, and display it from there
    connected_to_player = false;
    raiting_stack = new my_stack_tw();                          // create our very own Raiting WG
    raiting_stack->setColumnCount(2);                           // Lable all the things
    QStringList raiting_wg_labels={"Name","Raiting"};
    raiting_stack->setHeaderLabels(raiting_wg_labels);
    this->setColumnCount(2);
    QStringList playlist_wg_labels={"Name","Last Position"};
    this->setHeaderLabels(playlist_wg_labels);                   // Lable all the things

    corresponing_stack_wg->addWidget(raiting_stack);   //add our raintings widget to the stack wg

    if(location==""){
        podcast_dir = QFileDialog::getExistingDirectory(this, tr("Open Podcast Dir"),"",QFileDialog::ShowDirsOnly);  // use this to ask the user for the dir of the podcasts :D
    }else{
        podcast_dir=location;
    }

    connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this, SLOT(item_dubble_clck(QTreeWidgetItem*,int)  ) );    // this widget has been clicked. it will tell the player to play the item and change the current playlist if needed
    connect(player,SIGNAL(currentMediaChanged(QMediaContent)),this,SLOT(current_media_changed(QMediaContent)) );    // detect when playlist has changed so we don't call all the funktinos of all the widgets all the time
    //connect(player,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),this, SLOT() );    // in case i want to replace playlist with manual media changes
    //connect(player,SIGNAL(stateChanged(QMediaPlayer::State)),this,SLOT( playerstate_changed(QMediaPlayer::State)) );  //was only a work around, to be deleted if no other use found for it
    ///connect(playlist,SIGNAL(currentIndexChanged(int)),this,SLOT(update_TW_playlist(int)) );         TODO             //Playlist and Treewidget link, to show grafikly which one is playing now

    manager = new Podcast_manager( podcast_dir.toStdString());              // create an instance of the manager, it will save and load position and rating, and index the files
    generate_ordered_playlist(false);                                       // use the manager to run through the filesystem and add 1lvl of folders and their contents to the mix

    connect(raiting_stack,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(on_podcast_list_tw_itemChanged(QTreeWidgetItem*,int)) );

    // begin context menu setup                        puting things into context :D (Menu)
    setContextMenuPolicy(Qt::CustomContextMenu);                                                // gives me in mainwondow the event when right clicked

    QAction *mark_as_listened = new QAction( "soft mark as listened", &playlist_contextMenu );          // create  action
    connect(mark_as_listened,SIGNAL(triggered()),this,SIGNAL(listend_soft()) );                         // connect  action to a signal with false value
    playlist_contextMenu.addAction(mark_as_listened);                                                   // add first action to the menu
    connect(this,SIGNAL(listend_soft(bool)),this,SLOT(mark_epi_as_listened(bool)));                     // connect 2. signal to solt

    QAction *mark_as_listened_remove = new QAction( "hard mark as listened", &playlist_contextMenu );    // create  action
    connect(mark_as_listened_remove,SIGNAL(triggered()),this,SIGNAL(listend_hard()) );                   // connect  action to a signal with true value
    playlist_contextMenu.addAction(mark_as_listened_remove);                                             // add first action to the menu
    connect(this,SIGNAL(listend_hard(bool)),this,SLOT(mark_epi_as_listened(bool)));                      // connect 2. signal to solt

    QAction *reset_play_time = new QAction( "Reset playtime", &playlist_contextMenu );
    connect(reset_play_time,SIGNAL(triggered()),this,SLOT( reset_playtime_slot() ) );
    playlist_contextMenu.addAction(reset_play_time);
    connect(this,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT( on_Playlist_tree_wg_customContextMenuRequested(QPoint)) );
    // end context menu setup

    this->setAcceptDrops(true);                                     // make drag and drop work -begin
    this->setDragEnabled(true);                                     //
    //this->setDragDropMode(QAbstractItemView::InternalMove);       // -forgot why I had this at one point
    this->setDragDropMode(QAbstractItemView::DragDrop);             //
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);   // make drag and drop work -- end
    if(player->playlist()==NULL){                                   // give player an initail playlist
        player->setPlaylist(&playlist);
    }
    current_wg_mem_p=current_playlist_wg_mem_p;      //works ! good now we can start to replace if(playlist is current) with *current_playlist_wg_mem_p
    (*current_wg_mem_p)=this;
}
// Planing:
// Goal: one central DB for time many small DB/Tabels for playlists. || Maybee get away from the premate playlist and only use the tree_WG, so we don't have to update the playlist when user
// reoders or drag and dropps. this would propaply only require me to check for end of playback ie QMediaPlayer::MediaStatus QMediaPlayer::EndOfMedia or media change SIGNALS
// but then how do we know which playlist is current when we can't get_playlist == our_playlist for saving and such ?
// add a attribut to mainwindow which has the current playlist_wg in it, will be set when user dubbleclicks an item of one of the playlists

void Playlist_tree_wg::item_dubble_clck(QTreeWidgetItem* item_i, int column){

    int index = this->indexOfTopLevelItem(item_i);

   std::cout<<"doblle cklick"<<std::endl;
   if(player->playlist()!= &playlist){  //   will emit a player current media changed
       player->setPlaylist(&playlist);  /// WARNING will reset playlist index back to 0  when set

       std::cout<<"set_playlist_from_this_tab"<<std::endl;
   }

   if(index != playlist.currentIndex()){
        playlist.setCurrentIndex( index );
        std::cout<<"set_index"<<std::endl;
  }
   player->play();
    //std::cout<<"row:"<<column<<"  index:"<< index<<std::endl;
}

void Playlist_tree_wg::skipp_to_last_pos(){     // called when mediaplayer media changes and if our playlist is in use skipp to the last pos of the item

    std::cout<<"seeking to last position "<<std::endl;

    if(player->playlist()!= &playlist){
        std::cout<< "skipp called but playlist is not current ! "<<std::endl;
        return;
    }
    if( (player->mediaStatus() != QMediaPlayer::LoadedMedia )&&(player->mediaStatus() != QMediaPlayer::BufferedMedia ) ){// can't seek if media is not loaded/buffered yet
        std::cout<<" seek to position ignored "<<player->mediaStatus()<<std::endl; // also gets called on exit for some reasone
        return;
    } // if true not our battle :D

    int index=playlist.currentIndex();
    Episode *epi;
    if((index!=-1)&&(this->topLevelItemCount()>0)){ // catch bad states
        epi = ((Epi_list_item*) this->topLevelItem(index))->getEpisode();

        if(epi->listend){
            playlist.next();    ///TODO good on forward bad on reverse !!
        }
        else{
            std::cout<< index<<"setting playback position to "<< ((Epi_list_item*)this->topLevelItem(index))->getEpisode()->last_position /1000<< "s"<<std::endl;       //debug
            player->setPosition(epi->last_position);
        }
    }
}

/*
void Playlist_tree_wg::playerstate_changed(QMediaPlayer::State state){
    int temp_state  = playerstate_old;
    playerstate_old = state;

    if(player->playlist()!= &playlist){     /// replace this with a propper disconect from the player when tabs change, so we don't have to actively ignor signals
        std::cout<<" ignore state change "<<std::endl; // also gets called on exit for some reasone
        return;
    } // if true not our battle :D

    if( temp_state == QMediaPlayer::StoppedState){   /// workaround for initial first item load, see set pos failed for cause
        skipp_to_last_pos();
    }    
}*/



void Playlist_tree_wg::on_Playlist_tree_wg_customContextMenuRequested(const QPoint &pos)// playlist tw context menu
{
    QModelIndex index = this->indexAt(pos);
    if (index.isValid() ){
        playlist_contextMenu.exec(this->mapToGlobal(pos));
        std::cout<<"yy"<<std::endl;
    }
    std::cout<<"called"<<std::endl;
}


void Playlist_tree_wg::mark_epi_as_listened(bool hard=false){      // call from the context menu
    std::cout<< " marking  "<<hard<<std::endl;
    foreach (QTreeWidgetItem* item_i, this->selectedItems()) {

        Episode* epi =   ((Epi_list_item*)(item_i))->getEpisode();
        epi->listend=true;
        manager->save_position(epi);
        if(hard){
            playlist.removeMedia( this->indexOfTopLevelItem(item_i) );  // makes a little jump but works fine during play
            delete item_i;  // don't forget to delete item else index of playlist will be wrong for the next,
                            // unless we deleted from bottum to top, but the
        }
   }
}


void Playlist_tree_wg::reset_playtime_slot(){        // call from the context menu
    foreach (QTreeWidgetItem* item_i, this->selectedItems()) {

        Epi_list_item* epi_li   = (Epi_list_item*)item_i;
        Episode* epi            = (epi_li)->getEpisode();
        epi->last_position=0;
        epi_li->setText(1,"00:00:00");
        epi->listend=false;
        manager->save_position(epi);
        }
}





void Playlist_tree_wg::generate_ordered_playlist(bool new_first){// anoyingly will end playback when called...

playlist.clear();
this->clear();              //clear this widget (playlist_wg) out
raiting_stack->clear();


    list <Podcast*> pc= manager->load_list();   //generats the episodes and podcasts, calling it agin delets the previos objekts
    pc.sort(compareison);
    for(auto pods:pc){
        add_podcast(pods);
        if(!new_first) {pods->episodes.reverse();}   // old ones first then, reverse ie (old first) by default
        for(auto epi:pods->episodes){
            if(!epi->listend){
                add_media(epi);
            }
        }
    }
    this->resizeColumnToContents(0);
    //raiting_stack->resizeColumnToContents(0);   // resize
    raiting_stack->resizeColumnToContents(1);   // resize
}


void Playlist_tree_wg::save_position(){
    qint64 position;
    qint64 duration;
    qint64 position_sec;
    Epi_list_item* epi_list_item;
    Episode *epi;
    // std::cout<< "save!  "<<std::endl;
    if(player->playlist()!= &playlist){
        std::cout<< "save called but playlist is not current ! "<<std::endl;
        return;
    }
    //std::cout<< "save...  "<<std::endl;
    if( (player->state() == QMediaPlayer::PlayingState) && ( ((duration = player->duration())) !=-1) ){// duration may not be available when initial playback begins... ie player->duration() returns -1
        epi_list_item = (Epi_list_item*)(this->topLevelItem(playlist.currentIndex()));
        epi=epi_list_item->getEpisode();
        position = player->position();
        position_sec=position/1000;

        if((! epi->listend)&&( (((float)duration)*0.975) < ((float)position) )&&( duration!=-1 ) ){ // decide if marking as listened
             epi->listend=true;
             std::cout<< "marked as listened "<< epi->name <<std::endl;
             std::cout<< "current position  "<<position<<std::endl;
             std::cout<< "current duratoin "<<duration<<std::endl;
        }
        epi->last_position=position;
        manager->save_position(epi);

        QTime currentTime((position_sec/3600)%60, (position_sec/60)%60, position_sec%60);
        this->topLevelItem(playlist.currentIndex())->setText(1, currentTime.toString("hh:mm:ss")  ); //update ui
    }
}


void Playlist_tree_wg::on_podcast_list_tw_itemChanged(QTreeWidgetItem *item_i, int column)// user has changed the value of one of our podcasts, save change to DB ?
{
    Podlist_item *item=(Podlist_item*)item_i;
    if((item->getPodcast() !=NULL)&&(column!=0)&&(item->get_setup_done()) ){
        item->getPodcast()->raiting= item->text(1).toInt();
        manager->update_values(item->getPodcast());
        std::cout<< "updating values: "<< item->getPodcast()->name<< "\nwith Raitung:" <<item->getPodcast()->raiting<<" column: "  <<column <<std::endl;
    }
}

void Playlist_tree_wg::add_podcast(Podcast *pod_i)
{
    Podlist_item *new_podcast = new Podlist_item(raiting_stack);
    new_podcast->setFlags(new_podcast->flags() | Qt::ItemIsEditable);   //alow user to change raiting
    new_podcast->setPodcast(pod_i);
    raiting_stack->addTopLevelItem(new_podcast);
}


void Playlist_tree_wg::add_media(Episode *epi)
{
    QFileInfo fileInfo( QString::fromStdString( epi->dir ) );                        //ask the Episode where it is in the filesystem and save it as Qstring
    playlist.addMedia( QUrl::fromLocalFile( fileInfo.absoluteFilePath() ) );                    // add to our playlist

    Epi_list_item *new_episode = new Epi_list_item(this);
    new_episode->setEpisode(epi);
    new_episode->setFlags(Qt::ItemIsDragEnabled |  Qt::ItemIsSelectable |  Qt::ItemIsEnabled);
    this->addTopLevelItem(new_episode);                                                         // add to playlist WG
}
void Playlist_tree_wg::remove_from_stack_wg(){
    corresponing_stack_wg->removeWidget(raiting_stack); //removes it from the wg but not delets it
    delete raiting_stack;                               // delete it
}



void Playlist_tree_wg::disconnect_temp(){   //we nolonger need to care for all the signals as long as another playlist is active
    std::cout<< " disconnecting player signals  "<<std::endl;
    disconnect(player,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),this,SLOT( skipp_to_last_pos() ) );     //done buffering and such
    disconnect(timer,SIGNAL(timeout()),this,SLOT(save_position()));                                                 //periodicly save playback position
    connected_to_player = false;
}
void Playlist_tree_wg::connect_signals(){   //atach this widget to the timer and player
    std::cout<< " connecting player signals  "<<std::endl;
    connect(player,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),this,SLOT( skipp_to_last_pos() ) );        //done buffering and such
    connect(timer,SIGNAL(timeout()),this,SLOT(save_position()));                                                    //periodicly save playback position
    connected_to_player = true;
}

void Playlist_tree_wg::current_media_changed(QMediaContent current_media_p){

    std::cout<< " current media changed WWWWWWWWWWWWWWWWWWWWWWWWWWW "<<std::endl;
    if( (player->playlist() != &playlist)&&(connected_to_player == true) ){
        disconnect_temp();
    }
    else if( (player->playlist() == &playlist)&&(connected_to_player == false) ){
        connect_signals();
    }
}


Playlist_tree_wg::~Playlist_tree_wg(){
// raiting_stack gets deletet by stackwidget in the end if i don't remove and delete it bevorehand
}
QString Playlist_tree_wg::get_dir(){
    return podcast_dir;
}





///  drag and drop down there V

void Playlist_tree_wg::dropEvent(QDropEvent * event)    //
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {            
            QList<QUrl> urlList = mimeData->urls();
            foreach (QUrl where, urlList ) {
                manager->add_from_url(where);  // not realy doable without database support
            }
            std::cout << "updating the ui not yet done 1"<<std::endl;
            return;
    }

//http://stackoverflow.com/questions/18738676/qt-drag-and-drop-treeview-what-am-i-missing

    QModelIndex dropIndex = indexAt(event->pos());  // event --> target position     /// wrong when to far below of one item
    int to = dropIndex.row();                       // target position --> int value ///
    QFileInfo fileInfo;

    list <QTreeWidgetItem*> my_list = this->selectedItems().toStdList();   // get moved times list
    my_list.reverse();                                                     // reverse list for bottom to top adding/removing so index does not change

    list <Epi_list_item*> my_list_epis;
    Epi_list_item *new_episode;         /// WARNING moving down and possible complex stuff is broken still !!
    foreach (QTreeWidgetItem* x, my_list ) {
        std::cout << "to: "<<to<<"    from: "<< this->indexOfTopLevelItem(x)<<std::endl;        // add    item in new place in  playlist
        fileInfo.setFile( QString::fromStdString( ((Epi_list_item*)x)->getEpisode()->dir ) );
        playlist.removeMedia(this->indexOfTopLevelItem(x));
        playlist.insertMedia(to,QUrl::fromLocalFile( fileInfo.absoluteFilePath() ));
        new_episode = new Epi_list_item((Epi_list_item*)x);
        delete x;                                               // automatikly tells tw to remove it, delete first so index is correct when moving downwards
        new_episode->setFlags(Qt::ItemIsDragEnabled |  Qt::ItemIsSelectable |  Qt::ItemIsEnabled);
        this->insertTopLevelItem(to,new_episode);
    }
}

void Playlist_tree_wg::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();  //nice green mouse icon
}

void Playlist_tree_wg::dragMoveEvent(QDragMoveEvent *event)
{
    QTreeWidget::dragMoveEvent(event);  // internal items generate highlitet destination place
    event->acceptProposedAction();      // order matters here   //nice green mouse icon
}

void Playlist_tree_wg::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();    //nice green mouse icon
}
