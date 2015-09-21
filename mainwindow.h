#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "podcast_strukts.hpp"
#include "playlist_tree_wg.h"
#include "communiction_node.h"
#include "podlist_item.hpp"


#include <list>
#include <map>
#include <unordered_map>

#include <QSqlDatabase>
#include <QSqlQuery>

#include <QPixmap>
#include <QImage>
#include <QLabel>
#include <QCloseEvent>

#include <QSystemTrayIcon>
#include <QMainWindow>
#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QSystemTrayIcon>
#include <QFileDialog>
#include <QTime>
#include <QTimer>
#include <QTreeWidgetItem>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;    // so we can redifine the window close button action

signals:
   void pause();
   void play();
   void play_pause();
   void seeking(qint64 pos);
   void current_wg_update( Playlist_tree_wg* who );

public:
    explicit MainWindow(QStringList args_i,QWidget *parent = 0);
    ~MainWindow();
   bool singel_app();




private slots:

   void connect_playlist(Playlist_tree_wg* playlist_wg);
   void disconnect_playlist(Playlist_tree_wg* playlist_wg);

   void apply_remote_commands(int comand);
   void add_new_on_plus_click(int index);               //TAB
   void scroll_bounds(int i);                       //TAB
   void on_tabWidget_tabCloseRequested(int index);  //TAB

   void tray_icon_clicked(QSystemTrayIcon::ActivationReason reason);    // min_max

   void switch_play_pause();
   void seek_sl_setValue(qint64 pos);
   void seek_sl_setLenght(qint64 lenght);
   void on_seek_sl_sliderReleased();
   void save_volume();

   void add_tab(QStringList locations= QStringList(), QString tab_name = QString());

 //  void add_media(Episode *epi);// gets a podcast with raiting and other things from the subsystem and gives the media to the playlist and the info to the treewidget
 //  void add_podcast(Podcast *pod_i);
//   void on_podcast_list_tw_itemDoubleClicked(QTreeWidgetItem *item, int column);    //user wants to change podcast raiting
  // void save_position();// periodicaly called to save the current playlist position and mark item as listened
 //  void mark_epi_as_listened();
 //  void reset_playtime_slot();
//void on_podcast_list_tw_itemChanged(QTreeWidgetItem *item_i, int column);   //user or (and other prozesses) edits a podcast Qtreeitem ie change the raiting or (sets a podcast pointer)

    void generate_ordered_playlist();

//void on_playlist_tw_itemDoubleClicked(QTreeWidgetItem *item, int column);   //user wants to play item from playlist

void on_seek_sl_valueChanged(int value);


private:

//unordered_map< string, string > settings_map;   //holds settings like podcast dir, last volume, ...
//list<string> locations;
//string local_pod_dir;   //name of the settingsmap[ key ] for the podcast dir string value
    communiction_node *comm_interface;   ///my own class that responsible for the comuniation with other instances of this application over shared memory
    Podcast_manager * manager;  ///my own class that walks through the filesystem and indexes the podcast and their episodes
    QTimer *saver;
    QDir cwdir;             //path to the exe
    QDir settings_location; //path to the settings (~.config)
    Ui::MainWindow *ui;     //default thing containing all the ui elements like qtreewiget and so on
    Playlist_tree_wg * current_playlist;
    QMediaPlayer *player;       //the player
    QSystemTrayIcon *tray_icon; //system tray icon, bug when initaili set gets pixel noise, fix set icon later again after initail show.

};

#endif // MAINWINDOW_H
