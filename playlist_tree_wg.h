#ifndef PLAYLIST_TREE_WG_H
#define PLAYLIST_TREE_WG_H
#include "podcast_manager.hpp"
#include "epi_list_item.hpp"
#include "my_stack_tw.h"
#include "podlist_item.hpp"

#include <QMimeData>
#include <QDropEvent>
#include <Qt>
#include <QTimer>
#include <QAction>
#include <QFileDialog>
#include <QStackedWidget>
#include <QMenu>
#include <QTreeWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <iostream>
#include "epi_list_item.hpp"

#include <set>
//#include "mainwindow.h"

class Playlist_tree_wg : public QTreeWidget
{
    Q_OBJECT  // without this slots will never be found "no such slot runtime error"
    QMediaPlayer * player;
    QMenu playlist_contextMenu;
    Raiting_tree_wg *raiting_stack;
    Podcast_manager *manager;
    QStackedWidget* corresponing_stack_wg;
    QStringList dirs;
    QString tab_name;
    QTimer* timer;
    bool connected_to_player;
    std::set<QString> myset_epi;
    std::set<QString> myset_pod;
    Epi_list_item* current_item;

    list <Podcast*> pc; // TODO a MAP <String, Podcast*> would be better

//int playerstate_old;

private slots:
void add_podcast(Podcast *pod_i);                                           //add foldername als overarching description
void add_media(Episode *epi);                                               //add media to playlist_tw & playlist
void on_podcast_list_tw_itemChanged(QTreeWidgetItem *item_i, int column);   //save raiting changes
void on_Playlist_tree_wg_customContextMenuRequested(const QPoint &pos);
void skipp_to_last_pos();

void save_position();
void item_dubble_clck(QTreeWidgetItem* item_i, int column);
void reset_playtime_slot();

void mark_epi_as_listened(bool hard);
void player_mediastatus_changed(QMediaPlayer::MediaStatus);
void resume();
void paint_item(Epi_list_item* item_i,bool norm);


//void current_media_changed(QMediaContent current_media_p);
//void playerstate_changed(QMediaPlayer::State state);

void play_item( Epi_list_item* item_i );
void play_prev();
void play_next();

protected:
/*
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
*/
signals:
    void listend_soft(bool = false);
    void listend_hard(bool = true);
    void disconnect_me(Playlist_tree_wg * );
    void connect_me(Playlist_tree_wg * );
    void current_wg_update(Playlist_tree_wg*);

public slots:
    void generate_ordered_playlist();
    void set_current( Playlist_tree_wg * who );
    void disconnect_temp();
    void connect_signals();
    void set_duration(QString time_formated_i);

public:

    Playlist_tree_wg(QMediaPlayer* the_player, Podcast_manager* Podcast_manager_i, QStackedWidget *stack_wg, QTimer* save_timer, QStringList locations, QString tab_name_i);
    ~Playlist_tree_wg();

    void remove_from_stack_wg();        // caled by main window when deconstruction is wanted

    void set_dirs( QStringList dirs );  // ??? nothing here yet
    QString get_name(){return tab_name;}
};


    inline bool compareison(const Podcast* first, const Podcast* second) {
    return ( first->raiting > second->raiting ); //compare the int raitings
}
#endif // PLAYLIST_TREE_WG_H
