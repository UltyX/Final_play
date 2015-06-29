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

class Playlist_tree_wg : public QTreeWidget
{
    Q_OBJECT  // without this slots will never be found "no such slot runtime error"

QMediaPlaylist playlist;
QMediaPlayer * player;
QMenu playlist_contextMenu;
my_stack_tw *raiting_stack;
Podcast_manager *manager;
QStackedWidget* corresponing_stack_wg;
QString podcast_dir;
int playerstate_old;

private slots:
void add_podcast(Podcast *pod_i);                                           //add foldername als overarching description
void add_media(Episode *epi);                                               //add media to playlist_tw & playlist
void on_podcast_list_tw_itemChanged(QTreeWidgetItem *item_i, int column);   //save raiting changes
void on_Playlist_tree_wg_customContextMenuRequested(const QPoint &pos);
void skipp_to_last_pos();

void save_position();
void item_dubble_clck(QTreeWidgetItem* item_i, int column);
void reset_playtime_slot();
void mark_epi_as_listened();
void playerstate_changed(QMediaPlayer::State state);

protected:

void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
void dragLeaveEvent(QDragLeaveEvent *event) Q_DECL_OVERRIDE;
void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

public slots:
void generate_ordered_playlist(bool new_first);
public:

    Playlist_tree_wg(QMediaPlayer* the_player, QStackedWidget * stack_wg , QTimer *save_timer, QString location);
    ~Playlist_tree_wg();

void remove_from_stack_wg();
QString get_dir();

};
inline bool compareison(const Podcast* first, const Podcast* second) {

    return ( first->raiting > second->raiting ); //compare the int raitings
}
#endif // PLAYLIST_TREE_WG_H
