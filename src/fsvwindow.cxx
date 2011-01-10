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
#include "fsvwindow.h"
#include "ogl.h"
#include "viewport.h"
#include "color.h"
#include "dirtree.h"
#include "about.h"
#include "camera.h"
#include "colexp.h"
#include "fsv.h"
#include "options_dlg.h"
#include "property_dlg.h"

#include "xmaps/birdseye_view.xpm"
// will_be_removed;
void gui_update(){
	while (gtk_events_pending( ) > 0)
		gtk_main_iteration( );
}
void gui_cursor( GtkWidget *widget, int glyph ){
  if(glyph<0)glyph=0;
  /*Gdk::Window::*///Glib::wrap(widget)->get_window()->set_cursor(Gdk::Cursor(Gdk::CursorType(glyph)));
  //gdk_window_set_cursor( , cursor );
}
/* This checks if the widget associated with the given adjustment is
 * currently busy redrawing/reconfiguring itself, or is in steady state
 * (this is used when animating widgets to avoid changing the adjustment
 * too often, otherwise the widget can't keep up and things slow down) */
boolean
gui_adjustment_widget_busy( GtkAdjustment *adj )
{
	static const double threshold = (1.0 / 18.0);
	double t_prev;
	double t_now;
	double *tp;

	/* ---- HACK ALERT ----
	 * This doesn't actually check GTK+ internals-- I'm not sure which
	 * ones are relevant here. This just checks the amount of time that
	 * has passed since the last time the function was called with the
	 * same adjustment and returned FALSE, and if it's below a certain
	 * threshold, the object is considered "busy" (returning TRUE) */

	t_now = xgettime( );

	tp = (double*)g_object_get_data( G_OBJECT(adj), "t_prev" );
	if (tp == NULL) {
		tp = NEW(double);
		*tp = t_now;
		g_object_set_data_full( G_OBJECT(adj), "t_prev", tp, _xfree );
		return FALSE;
	}

	t_prev = *tp;

	if ((t_now - t_prev) > threshold) {
		*tp = t_now;
		return FALSE;
	}

	return TRUE;
}

// old C functions
void window_set_access( boolean enabled ){
}
void window_set_color_mode( ColorMode mode ){}
void window_birdseye_view_off( void ){
  FsvWindow::current->birdeye->set_active(false);
  FsvWindow::current->tb_birdeye.set_active(false);
}
void window_statusbar( StatusBarID sb_id, const char *message ){
  if(sb_id == SB_RIGHT){
  FsvWindow::current->rsbar.pop(sb_id);
  FsvWindow::current->rsbar.push(message,sb_id);
    return;
  }
  FsvWindow::current->sbar.pop(sb_id);
  FsvWindow::current->sbar.push(message,sb_id);
}
// old C functions
void context_menu( GNode *node, GdkEventButton *ev_button ){
  FsvWindow::current->popa_node = node;
	/* Check for the special case in which the menu has only one item */
  Gtk::Menu::MenuList& items = FsvWindow::current->popa.items();
  for(Gtk::Menu::MenuList::iterator it = items.begin();it!=items.end();++it){
    it->hide();
  }
	/*if (!NODE_IS_DIR(node) && (node == globals.current_node)) {
		items[4].show();//dialog_node_properties( node );
		//return;
	}*/
	if (NODE_IS_DIR(node)) {
		if (dirtree_entry_expanded( node ))
			items[0].show();//gui_menu_item_add( popup_menu_w, _("Collapse"), collapse_cb, node );
		else {
			items[1].show();//gui_menu_item_add( popup_menu_w, _("Expand"), expand_cb, node );
			if (DIR_NODE_DESC(node)->subtree.counts[NODE_DIRECTORY] > 0)
				items[2].show();//gui_menu_item_add( popup_menu_w, _("Expand all"), expand_recursive_cb, node );
		}
	}
	if (node != globals.current_node)
		items[3].show();//gui_menu_item_add( popup_menu_w, _("Look at"), look_at_cb, node );
	//gui_menu_item_add( popup_menu_w, _("Properties"), properties_cb, node );
	  items[4].show();
	//gtk_menu_popup( GTK_MENU(popup_menu_w), NULL, NULL, NULL, NULL, ev_button->button, ev_button->time );
  FsvWindow::current->popa.popup(ev_button->button,ev_button->time);
}

//
void FsvWindow::on_change_root(){
	Glib::ustring root_name = node_absname( root_dnode );
  Gtk::FileChooserDialog dialog(_("Change Root Directory"), Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER );
  dialog.set_transient_for(*this);
  dialog.set_filename(root_name);
  dialog.set_create_folders(false);
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
  int result = dialog.run();

  if (result == Gtk::RESPONSE_OK){
    std::string filename = dialog.get_filename();
    dialog.hide();
    get_window()->set_cursor(Gdk::Cursor(Gdk::WATCH));
	  fsv_load( filename.c_str() );
    get_window()->set_cursor(Gdk::Cursor(Gdk::LEFT_PTR));
  }
}

void FsvWindow::on_exit(){
  exit(0);
}
void FsvWindow::on_about(){
  about( ABOUT_BEGIN );
}
void FsvWindow::on_map_view(){
	if (globals.fsv_mode != FSV_MAPV)
		fsv_set_mode( FSV_MAPV );
}
void FsvWindow::on_tree_view(){
	if (globals.fsv_mode != FSV_TREEV)
		fsv_set_mode( FSV_TREEV );
}
void FsvWindow::on_birdseye_view(int tb){
	if(tb==0){
	  camera_birdseye_view(birdeye->get_active() );
	  tb_birdeye.set_active(birdeye->get_active() );
	}else{
	  camera_birdseye_view(tb_birdeye.get_active() );
	  birdeye->set_active(tb_birdeye.get_active() );
	}
}
//
void FsvWindow::on_collapse(){
	colexp( popa_node, COLEXP_COLLAPSE_RECURSIVE );
}
void FsvWindow::on_expand(){
	colexp( popa_node, COLEXP_EXPAND );
}
void FsvWindow::on_expand_all(){
	colexp( popa_node, COLEXP_EXPAND_RECURSIVE );
}
void FsvWindow::on_look_at(){
	camera_look_at( popa_node );
}
void FsvWindow::on_properties(){
  PropertyDialog dlg(popa_node);
  dlg.run();
}

void FsvWindow::on_color_type(ColorMode col){
	color_set_mode( col );
}
void FsvWindow::on_color_setup(){
  OptionsDialog dlg;
  dlg.run();
}

void FsvWindow::on_cd_root(){
	camera_look_at( root_dnode );
}
void FsvWindow::on_cd_back(){
	camera_look_at_previous();
}
void FsvWindow::on_cd_up(){
	if (NODE_IS_DIR(globals.current_node->parent))
		camera_look_at( globals.current_node->parent );
}

FsvWindow* FsvWindow::current;

FsvWindow::FsvWindow() : Gtk::Window(),tb_root(Gtk::Stock::GOTO_FIRST),tb_back(Gtk::Stock::GO_BACK),tb_up(Gtk::Stock::GO_UP){
  add(bx_main);
  bx_main.set_homogeneous(false);
  bx_main.pack_start(menu_bar,Gtk::PACK_SHRINK);
  //
  {
    using namespace Gtk::Menu_Helpers;
    MenuList items = menu_bar.items();

    items.push_back(MenuElem(_("File")));
    Gtk::MenuItem* pMenuItem = &items.back();
    Gtk::Menu* pMenu = Gtk::manage(new Gtk::Menu());
    {
      MenuList items = pMenu->items();
      items.push_back(MenuElem(_("Change Root"),sigc::mem_fun(*this,&FsvWindow::on_change_root)));
      items.push_back(SeparatorElem());
      items.push_back(MenuElem(_("Exit"),Gtk::AccelKey(GDK_Q, Gdk::CONTROL_MASK ) ) );
      Gtk::MenuItem* pMenuItem = &items.back();
      pMenuItem->signal_activate().connect(sigc::mem_fun(*this,&FsvWindow::on_exit));
    }
    pMenuItem->set_submenu(*pMenu);

    items.push_back(MenuElem(_("View")));
    pMenuItem = &items.back();
    pMenu = Gtk::manage(new Gtk::Menu());
    { 
      Gtk::RadioMenuItem::Group radiogroup;
      MenuList items = pMenu->items();
      items.push_back(RadioMenuElem(radiogroup,_("MapV"),sigc::mem_fun(*this,&FsvWindow::on_map_view) ) );
      items.push_back(RadioMenuElem(radiogroup,_("TreeV"),sigc::mem_fun(*this,&FsvWindow::on_tree_view) ) );
      items.push_back(SeparatorElem());
      items.push_back(CheckMenuElem(_("Top View")));
      Gtk::MenuItem* pMenuItem = &items.back();
      birdeye = dynamic_cast<Gtk::CheckMenuItem*>(pMenuItem);
      if(!birdeye)g_print("UPS!!!!!!!!!!!!!!!!!!!!!!!\n");
      pMenuItem->signal_activate().connect(
            sigc::bind(sigc::mem_fun(*this,&FsvWindow::on_birdseye_view),0));
    }
    pMenuItem->set_submenu(*pMenu);

    items.push_back(MenuElem("Colors"));
    pMenuItem = &items.back();
    pMenu = Gtk::manage(new Gtk::Menu());
    { 
      Gtk::RadioMenuItem::Group radiogroup;
      MenuList items = pMenu->items();
      items.push_back(RadioMenuElem(radiogroup,_("By nodetype")
                      ,sigc::bind(sigc::mem_fun(*this,&FsvWindow::on_color_type),COLOR_BY_NODETYPE) ) );
      items.push_back(RadioMenuElem(radiogroup,_("By timestamp")
                      ,sigc::bind(sigc::mem_fun(*this,&FsvWindow::on_color_type),COLOR_BY_TIMESTAMP) ) );
      items.push_back(RadioMenuElem(radiogroup,_("By wildcard")
                      ,sigc::bind(sigc::mem_fun(*this,&FsvWindow::on_color_type),COLOR_BY_WPATTERN) ) );
      items.push_back(SeparatorElem());
      items.push_back(StockMenuElem(Gtk::Stock::PREFERENCES
                      ,sigc::mem_fun(*this,&FsvWindow::on_color_setup) ) );
    }
    pMenuItem->set_submenu(*pMenu);
    
    items.push_back(MenuElem(_("Help")));
    pMenuItem = &items.back();
    pMenu = Gtk::manage(new Gtk::Menu());
    { 
      Gtk::RadioMenuItem::Group radiogroup;
      MenuList items = pMenu->items();
      items.push_back(MenuElem(_("About"),Gtk::AccelKey(GDK_F1,Gdk::ModifierType(0))
                      ,sigc::mem_fun(*this,&FsvWindow::on_about)));
    }
    pMenuItem->set_submenu(*pMenu);
    pMenuItem->set_right_justified();
    //
    items = popa.items();
    items.push_back(MenuElem(_("Collapse"),sigc::mem_fun(*this,&FsvWindow::on_collapse)));
    items.push_back(MenuElem(_("Expand"),sigc::mem_fun(*this,&FsvWindow::on_expand)));
    items.push_back(MenuElem(_("Expand all"),sigc::mem_fun(*this,&FsvWindow::on_expand_all)));
    items.push_back(MenuElem(_("Look at"),sigc::mem_fun(*this,&FsvWindow::on_look_at)));
    items.push_back(MenuElem(_("Properties"),sigc::mem_fun(*this,&FsvWindow::on_properties)));
    
  }
  //
  bx_main.pack_start(pn_main,true,true);
  bx_left.pack_start(tool_bar,false,false);
  tool_bar.append(tb_root);
  tb_root.signal_clicked().connect(sigc::mem_fun(*this,&FsvWindow::on_cd_root));
  tool_bar.append(tb_back);
  tb_back.signal_clicked().connect(sigc::mem_fun(*this,&FsvWindow::on_cd_back));
  tool_bar.append(tb_up);
  tb_up.signal_clicked().connect(sigc::mem_fun(*this,&FsvWindow::on_cd_up));
  tool_bar.append(tb_birdeye);
  birdeye_image.set(Gdk::Pixbuf::create_from_xpm_data(birdseye_view_xpm));
  tb_birdeye.set_icon_widget(birdeye_image);
  tb_birdeye.signal_toggled().connect(
     sigc::bind(sigc::mem_fun(*this,&FsvWindow::on_birdseye_view),1));
  
  
  bx_left.pack_end(pn_left);
  pn_left.set_size_request(150);
  pn_main.add1(bx_left);
  
  pn_left.add1(scr_tree);
  scr_tree.add(tr_dirs);
  
  pn_left.add2(scr_list);
  scr_list.add(tr_files);
  
	pn_main.add2(tbl_right);
	
  {
    
	GtkWidget *gl_area_w;
	int bitmask = 0;

	gl_area_w = ogl_widget_new( );
	bitmask |= GDK_EXPOSURE_MASK;
	bitmask |= GDK_POINTER_MOTION_MASK;
	bitmask |= GDK_BUTTON_MOTION_MASK;
	bitmask |= GDK_BUTTON1_MOTION_MASK;
	bitmask |= GDK_BUTTON2_MOTION_MASK;
	bitmask |= GDK_BUTTON3_MOTION_MASK;
	bitmask |= GDK_BUTTON_PRESS_MASK;
	bitmask |= GDK_BUTTON_RELEASE_MASK;
	bitmask |= GDK_LEAVE_NOTIFY_MASK;
	gtk_widget_set_events( GTK_WIDGET(gl_area_w), bitmask );
	gtk_widget_set_size_request( GTK_WIDGET(gl_area_w), 600, 480);
	//parent_child_full( parent_w, gl_area_w, EXPAND, FILL );
	g_signal_connect( GTK_OBJECT(gl_area_w), "event", G_CALLBACK(viewport_cb), NULL );
	Gtk::Widget* w=Glib::wrap(gl_area_w);
	tbl_right.attach(*w,0,1,0,1);
	tbl_right.attach(x_scroll,0,1,1,2,Gtk::FILL|Gtk::EXPAND,Gtk::SHRINK);
	tbl_right.attach(y_scroll,1,2,0,1,Gtk::SHRINK,Gtk::FILL|Gtk::EXPAND);
	
  }
	camera_pass_scrollbar_widgets( GTK_WIDGET(x_scroll.gobj()), GTK_WIDGET(y_scroll.gobj()) );

  bx_main.pack_end(sbar,Gtk::PACK_SHRINK);
  rsbar.set_size_request(150);
  rsbar.set_has_resize_grip(false);
  sbar.add(rsbar);
  
  show_all_children();
};

