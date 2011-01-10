/* fsv - 3D File System Visualizer
 *
 * Copyright (C)2009-2011 Yury P. Fedorchenko <yuryfdr@users.sf.net>
 */

/* This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
#include "common.h"
#include <color.h>

#ifdef __cplusplus
#include <gtkmm.h>


class FsvDirTree : public Gtk::TreeView{
public:
  struct FsvDirColumns : public Gtk::TreeModelColumnRecord{
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<GNode*> dnode;
    FsvDirColumns(){
      add(icon);
      add(name);
      add(dnode);
    }
  } records;
protected:
  virtual bool on_button_press_event(GdkEventButton* event); 
//  virtual void on_row_activated	(const Gtk::TreeModel::Path&	path,Gtk::TreeViewColumn* column);
  virtual void on_row_collapsed	(const Gtk::TreeModel::iterator& iter,const Gtk::TreeModel::Path&	path);
	virtual void on_row_expanded 	(const Gtk::TreeModel::iterator& iter,const Gtk::TreeModel::Path&	path);
public:
  Glib::RefPtr<Gtk::TreeStore> model;
  Glib::RefPtr<Gdk::Pixbuf> folder_closed;
  Glib::RefPtr<Gdk::Pixbuf> folder_opened;
  FsvDirTree();
  static FsvDirTree* dir_tree;
};

class FsvFileList : public Gtk::TreeView{
public:
  struct FsvFLColumns : public Gtk::TreeModelColumnRecord{
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<GNode*> node;
    FsvFLColumns(){
      add(icon);
      add(name);
      add(node);
    }
  } records;
  struct PropColumns : public Gtk::TreeModelColumnRecord{
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<unsigned int> count;
    Gtk::TreeModelColumn<unsigned int> size;
    PropColumns(){
      add(icon);
      add(name);
      add(count);
      add(size);
    }
  } prop_cols;
protected:
/*  virtual void on_row_activated	(const Gtk::TreeModel::Path&	path,Gtk::TreeViewColumn* column);
  virtual void on_row_collapsed	(const Gtk::TreeModel::iterator& iter,const Gtk::TreeModel::Path&	path);
	virtual void on_row_expanded 	(const Gtk::TreeModel::iterator& iter,const Gtk::TreeModel::Path&	path);
	*/
  virtual bool on_button_press_event(GdkEventButton* event); 
	Gtk::TreeView::Column nm;
	Gtk::TreeView::Column nm_s,count_s,size_s;
public:
  Glib::RefPtr<Gtk::ListStore> model;
  Glib::RefPtr<Gtk::ListStore> scan_model;
  /*Glib::RefPtr<Gdk::Pixbuf> folder_closed;
  Glib::RefPtr<Gdk::Pixbuf> folder_opened;*/
  FsvFileList();
  static FsvFileList* file_tree;
  void switch_scan(); 
  void switch_list(); 
};

class FsvWindow : public Gtk::Window {
  Gtk::VBox bx_main;
  Gtk::VBox bx_left;
  Gtk::VPaned pn_left;
  Gtk::HPaned pn_main;
  Gtk::Table  tbl_right;
  Gtk::HScrollbar x_scroll;
  Gtk::VScrollbar y_scroll;
  Gtk::MenuBar menu_bar;
  Gtk::ScrolledWindow scr_tree;
  Gtk::ScrolledWindow scr_list;
  FsvDirTree tr_dirs;
  FsvFileList tr_files;
public:
  Gtk::Menu popa;
  Gtk::Statusbar sbar,rsbar;//??
protected:
  void on_change_root();
  void on_exit();
  void on_about();
  void on_map_view();
  void on_tree_view();
  void on_birdseye_view(int);
public:
  Gtk::CheckMenuItem* birdeye;

protected:
  void on_collapse();
  void on_expand();
  void on_expand_all();
  void on_look_at();
  void on_properties();
  
  void on_color_type(ColorMode);
  void on_color_setup();
  Gtk::Toolbar tool_bar;
  Gtk::ToolButton tb_root,tb_back,tb_up;
  void on_cd_root();
  void on_cd_back();
  void on_cd_up();
  Gtk::Image birdeye_image;
public:
  Gtk::ToggleToolButton tb_birdeye;
  GNode* popa_node;
  FsvWindow();
  static FsvWindow* current;
};

extern "C" {
#else
#include <gtk/gtk.h>
#endif

typedef enum {
	SB_LEFT,
	SB_RIGHT
} StatusBarID;

void window_statusbar( StatusBarID sb_id, const char *message );
void gui_update();
void gui_cursor( GtkWidget *widget, int glyph );
void window_birdseye_view_off( void );
void window_set_access( boolean enabled );
void window_set_color_mode( ColorMode mode );
boolean gui_adjustment_widget_busy( GtkAdjustment *adj );
void context_menu( GNode *node, GdkEventButton *ev_button );
#ifdef __cplusplus
};
#endif

