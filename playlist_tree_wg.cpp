#include "playlist_tree_wg.h"

Playlist_tree_wg::Playlist_tree_wg(QMediaPlayer* the_player, QStackedWidget *stack_wg, QTimer* save_timer,QString location):QTreeWidget()
{
    player = the_player;  
    corresponing_stack_wg = stack_wg;

    raiting_stack = new my_stack_tw();
    raiting_stack->setColumnCount(2);    // Lable all the things
    QStringList a={"Name","Raiting"};
    raiting_stack->setHeaderLabels(a);
    this->setColumnCount(2);
    a={"Name","Last Position"};
    this->setHeaderLabels(a);           // Lable all the things

    corresponing_stack_wg->addWidget(raiting_stack);   //add our raintings widget to the stack wg

    if(location==""){
        podcast_dir = QFileDialog::getExistingDirectory(this, tr("Open Podcast Dir"),"",QFileDialog::ShowDirsOnly);  // use this to ask the user for the dir of the podcasts :D
    }else{
        podcast_dir=location;
    }

    connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this, SLOT(item_dubble_clck(QTreeWidgetItem*,int)  ) );
    connect(player,SIGNAL(currentMediaChanged(QMediaContent)),this,SLOT(skipp_to_last_pos()) );     //SKIPP to POSITION, WOULD HAVE liked to do it over playlist current media changed but race condition (emits signal(position skipping to) then changes track)
    //connect(playlist,SIGNAL(currentIndexChanged(int)),this,SLOT(update_TW_playlist(int)) );       //Playlist and Treewidget link, to show grafikly which one is playing now

    manager = new Podcast_manager( podcast_dir.toStdString());              // create an instance of the manager, it will save and load position and rating, and index the files
    generate_ordered_playlist(false);                                       // use the manager to run through the filesystem and add 1lvl of folders and their contents to the mix

    connect(raiting_stack,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(on_podcast_list_tw_itemChanged(QTreeWidgetItem*,int)) );

    // begin context menu setup                        puting things into context :D (Menu)
    setContextMenuPolicy(Qt::CustomContextMenu);                                                // gives me in mainwondow the event when right clicked
    QAction *mark_as_listened = new QAction( "mark as listened", &playlist_contextMenu );       // create first action
    connect(mark_as_listened,SIGNAL(triggered()),this,SLOT(mark_epi_as_listened()) );           // connect first action to the slot where its behavior is defined
    playlist_contextMenu.addAction(mark_as_listened);                                           // add first action to the menu
    QAction *reset_play_time = new QAction( "Reset playtime", &playlist_contextMenu );
    connect(reset_play_time,SIGNAL(triggered()),this,SLOT( reset_playtime_slot() ) );
    playlist_contextMenu.addAction(reset_play_time);
    connect(this,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT( on_Playlist_tree_wg_customContextMenuRequested(QPoint)) );
    // end context menu setup

    connect(save_timer,SIGNAL(timeout()),this,SLOT(save_position()));   //saving people, hunting things, the family business
    // periodicly save playback position and kill deamons Sammy


    this->setAcceptDrops(true);                                     // make drag and drop work -begin
    this->setDragEnabled(true);
    //this->setDragDropMode(QAbstractItemView::InternalMove);
    this->setDragDropMode(QAbstractItemView::DragDrop);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);   // make drag and drop work -- end
    if(player->playlist()==NULL){
        player->setPlaylist(&playlist);
}


}

void Playlist_tree_wg::item_dubble_clck(QTreeWidgetItem* item_i, int column){

    int index = this->indexOfTopLevelItem(item_i);

   std::cout<<"set_pl"<<std::endl;
   if(player->playlist()!= &playlist){  //   will emit a player current media changed
       player->setPlaylist(&playlist);  /// WARNING will reset playlist index back to 0  when set
   }
   std::cout<<"set_index"<<std::endl;
   if(index != playlist.currentIndex()){
      playlist.setCurrentIndex( index );
  }
   player->play();
    std::cout<<"row:"<<column<<"  index:"<< index<<std::endl;
}

void Playlist_tree_wg::skipp_to_last_pos(){     // called when mediaplayer media changes and if our playlist is in use skipp to the last pos of the item
    if(player->playlist()!= &playlist){return;} // if true not our battle :D
    int index=playlist.currentIndex();
    if((index!=-1)&&(this->topLevelItemCount()>0)){ // catch bad states
        std::cout<< index<<"setting playback position to "<< ((Epi_list_item*)this->topLevelItem(index))->getEpisode()->last_position /1000<< "s"<<std::endl;       //debug
        player->setPosition(((Epi_list_item*)this->topLevelItem(index))->getEpisode()->last_position);
    }
}





void Playlist_tree_wg::on_Playlist_tree_wg_customContextMenuRequested(const QPoint &pos)// playlist tw context menu
{
    QModelIndex index = this->indexAt(pos);
    if (index.isValid() ){
        playlist_contextMenu.exec(this->mapToGlobal(pos));
        std::cout<<"yy"<<std::endl;
    }
    std::cout<<"called"<<std::endl;
}


void Playlist_tree_wg::mark_epi_as_listened(){      // call from the context menu
    foreach (QTreeWidgetItem* item_i, this->selectedItems()) {

        Episode* epi =   ((Epi_list_item*)(item_i))->getEpisode();
        epi->listend=true;
        manager->save_position(epi);    //overkill, would only be necessary to call at end for each podcast which has one or more episodes changed
        }
}

void Playlist_tree_wg::reset_playtime_slot(){        // call from the context menu
    foreach (QTreeWidgetItem* item_i, this->selectedItems()) {

        Epi_list_item* epi_li   = (Epi_list_item*)item_i;
        Episode* epi            = (epi_li)->getEpisode();
        epi->last_position=0;
        epi_li->setText(1,"00:00:00");
        epi->listend=false;
        manager->save_position(epi);    //overkill, would only be necessary to call at end for each podcast which has one or more episodes changed
        }
}





void Playlist_tree_wg::generate_ordered_playlist(bool new_first){// anoyingly will end playback when called...

playlist.clear();
this->clear();
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
    raiting_stack->resizeColumnToContents(0);
    raiting_stack->resizeColumnToContents(1);
}


void Playlist_tree_wg::save_position(){
    qint64 position;
    qint64 duration;
    qint64 position_sec;
    Epi_list_item* epi_list_item;
    Episode *epi;
   // std::cout<< "save!  "<<std::endl;
if(player->playlist()!= &playlist)return;
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


void Playlist_tree_wg::on_podcast_list_tw_itemChanged(QTreeWidgetItem *item_i, int column)
{
    Podlist_item *item=(Podlist_item*)item_i;
    if((item->getPodcast() !=NULL)&&(column!=0)&&(item->get_setup_done()) ){
        item->getPodcast()->raiting= item->text(1).toInt();
        manager->update_values();
        std::cout<< "updating values: "<< item->getPodcast()->name<< "\nwith Raitung:" <<item->getPodcast()->raiting<<" column: "  <<column <<std::endl;
    }
}

void Playlist_tree_wg::add_podcast(Podcast *pod_i)
{
    Podlist_item *new_podcast = new Podlist_item(raiting_stack);
    new_podcast->setFlags(new_podcast->flags() | Qt::ItemIsEditable);
    new_podcast->setPodcast(pod_i);
    raiting_stack->addTopLevelItem(new_podcast);
}


void Playlist_tree_wg::add_media(Episode *epi)
{
    QFileInfo fileInfo( QString::fromStdString( epi->dir ) );                        //ask the Episode where it is in the filesystem and save it as Qstring
    playlist.addMedia( QUrl::fromLocalFile( fileInfo.absoluteFilePath() ) );

    Epi_list_item *new_episode = new Epi_list_item(this);
    new_episode->setEpisode(epi);
    new_episode->setFlags(Qt::ItemIsDragEnabled |  Qt::ItemIsSelectable |  Qt::ItemIsEnabled);
    this->addTopLevelItem(new_episode);
}
void Playlist_tree_wg::remove_from_stack_wg(){
    corresponing_stack_wg->removeWidget(raiting_stack); //removes it from the wg but not delets it
    delete raiting_stack;                               // delete it
}

Playlist_tree_wg::~Playlist_tree_wg(){
// raiting_stack gets deletet by stackwidget in the end if i don't remove and delete it bevorehand
}
QString Playlist_tree_wg::get_dir(){
    return podcast_dir;
}

void Playlist_tree_wg::dropEvent(QDropEvent * event)
{
//http://stackoverflow.com/questions/18738676/qt-drag-and-drop-treeview-what-am-i-missing
std::cout << "TODO for outside items... called dropping"<<std::endl;
    QModelIndex dropIndex = indexAt(event->pos());  // event --> target position
    int to = dropIndex.row();                       // target position --> int value
    QFileInfo fileInfo;

    list <QTreeWidgetItem*> my_list = this->selectedItems().toStdList();   // get moved times list
    my_list.reverse();                                                     // reverse list for bottom to top adding/removing so index does not change


    Epi_list_item *new_episode;/// WARNING moving down and possible complex stuff is broken still !!
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
