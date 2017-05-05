#include "playlist_tree_wg.h"
#include <Qt>
Playlist_tree_wg::Playlist_tree_wg(QMediaPlayer* the_player,Podcast_manager* Podcast_manager_i,
                                   QStackedWidget *stack_wg, QTimer* save_timer,QStringList locations,QString tab_name_i):QTreeWidget()
{
    timer                   = save_timer;                       // timeout used for save pos, when this playlist is current
    tab_name                = tab_name_i;                       // say my name. For saving of locations
    player                  = the_player;                       // the only player in this programm, we share it between tabs and use its signals to set position, also we give it our playlist
    manager                 = Podcast_manager_i;                // the manager, DB access, HDD indexing
    corresponing_stack_wg   = stack_wg;                         // the stack WG, we put our Raiting WG on it, and display it from there
    connected_to_player     = false;
    current_item            = NULL;
    raiting_stack           = new Raiting_tree_wg();            // create our very own Raiting WG


    this->setColumnCount(4);                                    // 2 Collums Playlist WG
    row_names=QStringList({"Name","Last Position","Raiting","Duration"});
    this->setHeaderLabels(row_names);                  // Lable all the things

    corresponing_stack_wg->addWidget(raiting_stack);   //add our raintings widget to the stack wg

    if( locations.isEmpty() ){
        dirs.append( QFileDialog::getExistingDirectory(this, tr("Open Podcast Dir"),"",QFileDialog::ShowDirsOnly) );  // ask the user for the dir of the podcasts
        manager->save_locations(tab_name,dirs);
    }else{
        dirs=locations;
    }

    my_timer.setSingleShot(false);
    my_timer.setInterval(5);
    connect(&my_timer,SIGNAL(timeout()),this,SLOT(timer_stop()) );

    connect(this  ,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this, SLOT(item_dubble_clck(QTreeWidgetItem*,int)  ) );              //
    connect(raiting_stack,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(on_podcast_list_tw_itemChanged(QTreeWidgetItem*,int)) );  // here because manager


    setSortingEnabled(true);            // sorting enable
    sortItems(2,Qt::DescendingOrder);    // sort initial

    generate_ordered_playlist();           // use the manager to run through the filesystem and add 1lvl of folders and their contents to the mix

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

    this->setSelectionMode(QAbstractItemView::ExtendedSelection);   // multi select
    /*
    //this->setDragDropMode(QAbstractItemView::InternalMove);       // -forgot why I had this at one point
    this->setAcceptDrops(true);                                     // make drag and drop work -begin
    this->setDragEnabled(true);                                     //
    this->setDragDropMode(QAbstractItemView::DragDrop);             // make drag and drop work -end
    // see the bottom of this page for some old drag and drop things
    */

}
// Planing:
// Re enable drag and drop ordering
// Fix playbutton pres on new window crash
// update no work

void Playlist_tree_wg::item_dubble_clck(QTreeWidgetItem* item_i, int column){
    emit current_wg_update(this);           //inform other
    play_item((Epi_list_item*) item_i);
}


void Playlist_tree_wg::skipp_to_last_pos(){     // called when mediaplayer media changes and if our playlist is in use skipp to the last pos of the item
    cout <<"skipp to last pos"<<endl;
    Episode *epi;




    if( (current_item!=NULL)&&(this->topLevelItemCount()>0)){ // catch bad states
        epi = current_item->getEpisode();

        if(epi->listend){
            play_next(); // or previos if user wants to go up
            //play_prev();
        }
        else{            

            player->setPosition(epi->last_position);

        }
    }
}


void Playlist_tree_wg::player_mediastatus_changed(QMediaPlayer::MediaStatus state){

    if( state == QMediaPlayer::MediaStatus::EndOfMedia){
        play_next();
    }
    else if( (state == QMediaPlayer::LoadedMedia )||(state == QMediaPlayer::BufferedMedia ) )
    {
        timer_start();
        skipp_to_last_pos();
    }
}

void Playlist_tree_wg::timer_start(){
    Episode *epi;
    if( (current_item!=NULL)&&(this->topLevelItemCount()>0)){ // catch bad states
        epi = current_item->getEpisode();
        soll_time = epi->last_position;

        my_timer.start();
    }
}

void Playlist_tree_wg::timer_stop(){

    if (soll_time-2 < player->position()){
        my_timer.stop();
    }
    else{
        player->setPosition(soll_time);
    }
}

void Playlist_tree_wg::on_Playlist_tree_wg_customContextMenuRequested(const QPoint &pos)// rigthclick context menu
{
    QModelIndex index = this->indexAt(pos);
    if (index.isValid() ){
        playlist_contextMenu.exec(this->mapToGlobal(pos));
    }
}


void Playlist_tree_wg::mark_epi_as_listened(bool hard=false){      // call from the context menu

    Episode* epi;
    foreach (QTreeWidgetItem* item_i, this->selectedItems()) {

        epi =   ((Epi_list_item*)(item_i))->getEpisode();
        epi->listend=true;
        manager->save_position(epi);
        if(hard){
            if(item_i == current_item){play_next();}
            delete item_i;  // don't forget to delete item else index of playlist will be wrong for the next,
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




// TODO
void Playlist_tree_wg::generate_ordered_playlist(){// anoyingly will end playback when called...


    foreach (QString directory, dirs){
        manager->apend_to_list(directory, &pc,&myset_epi,&myset_pod);   //generats the episodes and podcasts, calling it agin delets the previos objekts
    }

    for(auto pods:pc){
        if(myset_pod.find(QString::fromStdString(pods->name) ) == myset_pod.end() ){
            add_podcast(pods);
            myset_pod.insert(QString::fromStdString(pods->name) );
        }
        // still may have new epis
        for(auto epi:pods->episodes){
            if(myset_epi.find(QString::fromStdString(epi->name)) == myset_epi.end() ){
                myset_epi.insert(QString::fromStdString(epi->name) );
                if(!epi->listend){
                    add_media(epi);
                }
            }
        }
    }
    this->resizeColumnToContents(0);
    raiting_stack->resizeColumnToContents(0);   // resize
    raiting_stack->resizeColumnToContents(1);   // resize
}


void Playlist_tree_wg::save_position(){// called by the timer

    Episode *epi;
    qint64 position;
    qint64 position_sec;
    qint64 duration= player->duration();

    if( (player->state() == QMediaPlayer::PlayingState) && ( duration  !=-1) ){// catch player not ready / not playing
        epi=current_item->getEpisode();
        position = player->position();
        position_sec=position/1000;

        if((! epi->listend)&&( (((float)duration)*0.975) < ((float)position) )&&( duration!=-1 ) ){ // decide if marking as listened
             epi->listend=true;    
        }
        epi->last_position=position;
        manager->save_position(epi);

        QTime currentTime((position_sec/3600)%60, (position_sec/60)%60, position_sec%60);
        current_item->setText(1, currentTime.toString("hh:mm:ss")  ); //update ui
    }
}


void Playlist_tree_wg::on_podcast_list_tw_itemChanged(QTreeWidgetItem *item_i, int column)// user has changed the value of one of our podcasts, save change to DB ?
{
    Pod_list_item *item=(Pod_list_item*)item_i;
    if((item->getPodcast() !=NULL)&&(column!=0)&&(item->get_setup_done()) ){
        item->getPodcast()->raiting= item->text(1).toInt();
        manager->update_value(item->getPodcast());

        sortItems(sortColumn(),Qt::DescendingOrder );   // resort
    }
}

// Creates the Podcast | Raiting  line in the podcast stack wg
void Playlist_tree_wg::add_podcast(Podcast *pod_i)
{
    Pod_list_item *new_podcast = new Pod_list_item(raiting_stack);
    new_podcast->setFlags(new_podcast->flags() | Qt::ItemIsEditable);   //alow user to change raiting
    new_podcast->setPodcast(pod_i);
    raiting_stack->addTopLevelItem(new_podcast);
}

// Creates the Episode line in the playlist
void Playlist_tree_wg::add_media(Episode *epi)
{
    Epi_list_item *new_episode = new Epi_list_item(this);
    new_episode->setEpisode(epi);
    new_episode->setFlags(Qt::ItemIsSelectable |  Qt::ItemIsEnabled);                           // Qt::ItemIsDragEnabled
    this->addTopLevelItem(new_episode);                                                         // add to playlist WG
}





//  ##  Connect to player Events if current

void Playlist_tree_wg::disconnect_temp(){   //we nolonger need to care for all the signals as long as another playlist is active

    disconnect(timer,SIGNAL(timeout()),this,SLOT(save_position()));                                                 //periodicly save playback position
    disconnect(player,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),this, SLOT(player_mediastatus_changed(QMediaPlayer::MediaStatus)) );
}
void Playlist_tree_wg::connect_signals(){   //atach this widget to the timer and player

    connect(timer,SIGNAL(timeout()),this,SLOT(save_position()));                                                    //periodicly save playback position
    connect(player,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),this, SLOT(player_mediastatus_changed(QMediaPlayer::MediaStatus)) );
    resume();
}
void Playlist_tree_wg::set_current(Playlist_tree_wg *who){
    if( (who == this) && (!connected_to_player) ) {
        emit connect_me(this);
        connected_to_player = true;        
        std::cout<< "current now" << std::endl;
    }
    else if((who != this) && (connected_to_player)){
        emit disconnect_me(this);
        connected_to_player = false;
        std::cout<< "nolonger current now" << std::endl;
    }
}

//  ##  Connect to player Events if current




// play
void Playlist_tree_wg::resume(){
    if(current_item == NULL){
        current_item = (Epi_list_item*)topLevelItem(0);
    }
    play_item(current_item);
    player->pause();
}

void Playlist_tree_wg::play_next(){
    play_item( (Epi_list_item*) itemBelow(current_item) );
}
void Playlist_tree_wg::play_prev(){
    play_item( (Epi_list_item*) itemAbove(  current_item) );
}
void Playlist_tree_wg::play_item( Epi_list_item* item_i ){
    if(item_i == NULL)return;
    paint_item(current_item,true);
    paint_item(item_i,false);
    current_item = item_i;
    QFileInfo fileInfo( QString::fromStdString( current_item->getEpisode()->dir ) );                        //ask the Episode where it is in the filesystem and save it as Qstring

    player->setMedia( QUrl::fromLocalFile( fileInfo.absoluteFilePath() )  );
    player->play();
}
// play


void Playlist_tree_wg::paint_item(Epi_list_item* item_i,bool norm){
    QColor color_tx(0,0,0,0);   // transparent by default
    QColor color_bg(0,0,0,0);   // transparent by default
    if(current_item != NULL){
        if(norm){
            color_tx.setNamedColor("white");
        }
        else{
            color_tx.setNamedColor("white");  // http://www.w3.org/TR/SVG/types.html#ColorKeywords
            color_bg.setNamedColor("darkblue");
        }
        for(int i =0;i< columnCount();i++ ){
            item_i->setBackgroundColor(i,color_bg);
            item_i->setTextColor(i,color_tx);
        }
    }
}

void Playlist_tree_wg::set_duration(QString time_formated_i){

    if(current_item != NULL){
        current_item->setText(3,time_formated_i);
    }
}

QStringList Playlist_tree_wg::get_row_names(){
    return row_names;
}

void Playlist_tree_wg::remove_from_stack_wg(){
    corresponing_stack_wg->removeWidget(raiting_stack); //removes it from the wg but not delets it
    delete raiting_stack;                               // delete it
}



Playlist_tree_wg::~Playlist_tree_wg(){
// raiting_stack gets deletet by stackwidget in the end if i don't remove and delete it bevorehand
}
//if(!new_first) {pods->episodes.reverse();}   // old ones first then, reverse ie (old first) by default
//pc.sort(compareison);
/*

///  drag and drop down there V///  drag and drop down there V///  drag and drop down there V///  drag and drop down there V///  drag and drop down there V///  drag and drop down there V

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
*/
