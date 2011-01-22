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

#if defined HAVE_FTGL
#include <FTGL/ftgl.h>
//extern FTGLfont *font;
#elif defined HAVE_GL_GLC_H
#include <GL/glc.h>
extern GLint textFont;
#endif

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
  FsvFileList();
  static FsvFileList* file_tree;
  void switch_scan(); 
  void switch_list(); 
};

class FsvWindow : public Gtk::Window {
#if defined HAVE_FTGL
public:
  FTFont *font;
private:
#elif defined HAVE_GL_GLC_H
  GLint ctx;
#endif
  static Gtk::StockID BIRDEYE;
  static Gtk::StockItem BirdEye;
  Gtk::VBox bx_main;
  Gtk::VBox bx_left;
  Gtk::VPaned pn_left;
  Gtk::HPaned pn_main;
  Gtk::Table  tbl_right;
  Gtk::HScrollbar x_scroll;
  Gtk::VScrollbar y_scroll;
  Gtk::ScrolledWindow scr_tree;
  Gtk::ScrolledWindow scr_list;
  FsvDirTree tr_dirs;
  FsvFileList tr_files;
public:
  Gtk::Statusbar sbar,rsbar;//??
protected:
  void on_change_root();
  void on_exit();
  void on_about();
  void on_fsv_mode(FsvMode mode);
  void on_birdseye_view();
public:
  Glib::RefPtr<Gtk::ToggleAction> birdeye;
  Glib::RefPtr<Gtk::ActionGroup> ag_unsetsitive;
  Glib::RefPtr<Gtk::ActionGroup> ag_allways;
  Glib::RefPtr<Gtk::UIManager > ui_man;	
  Gtk::Menu *popa;
protected:
  void on_fullscreen(){fullscreen();}
  void on_collapse();
  void on_expand();
  void on_expand_all();
  void on_look_at();
  void on_properties();
  
  void on_color_type(ColorMode);
  void on_color_setup();
  void on_cd_root();
  void on_cd_back();
  void on_cd_up();
public:
  GNode* popa_node;
  FsvWindow();
  ~FsvWindow();
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

void text_init( void );
void text_pre( void );
void text_post( void );
void text_draw_straight( const char *text, const XYZvec *text_pos, const XYvec *text_max_dims );
void text_draw_straight_rotated( const char *text, const RTZvec *text_pos, const XYvec *text_max_dims );
void text_draw_curved( const char *text, const RTZvec *text_pos, const RTvec *text_max_dims );

#ifdef __cplusplus
};
#endif

