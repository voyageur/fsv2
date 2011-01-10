/* dirtree.c */

/* Directory tree control */

/* fsv - 3D File System Visualizer
 * Copyright (C)1999 Daniel Richard G. <skunk@mit.edu>
 * Copyright (C)2009-2011 Yury P. Fedorchenko <yuryfdr@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or
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
#include "dirtree.h"
#include "fsvwindow.h"

#include <gtk/gtk.h>

#include "about.h"
#include "camera.h"
#include "colexp.h"
#include "filelist.h"
#include "geometry.h"

/* Time for the directory tree to scroll to a given entry (in seconds) */
#define DIRTREE_SCROLL_TIME 0.5

/* Current directory */
static GNode *dirtree_current_dnode;


/* Callback for button press in the directory tree area */
bool FsvDirTree::on_button_press_event(GdkEventButton* ev_button )
{
	/* If About presentation is up, end it */
	about( ABOUT_END );

	if (globals.fsv_mode == FSV_SPLASH)	return false;

	bool ret = Gtk::TreeView::on_button_press_event(ev_button);
  Glib::RefPtr<Gtk::TreeSelection> sel = FsvDirTree::dir_tree->get_selection();
  Gtk::TreeIter iter = sel->get_selected();
	if(!iter) return false;

  GNode *dnode = (*iter)[records.dnode];
	if (dnode == NULL) return false;

	// A single-click from button 1 highlights the node, shows the
	// name, and updates the file list if necessary. (and also selects
	// the row, but GTK+ does that automatically for us) 
	if ((ev_button->button == 1) && (ev_button->type == GDK_BUTTON_PRESS)) {
		geometry_highlight_node( dnode, FALSE );
		window_statusbar( SB_RIGHT, node_absname( dnode ) );
		if (dnode != dirtree_current_dnode)
			filelist_populate( dnode );
		dirtree_current_dnode = dnode;
		return false;
	}

	// A double-click from button 1 gets the camera moving 
	if ((ev_button->button == 1) && (ev_button->type == GDK_2BUTTON_PRESS)) {
		camera_look_at( dnode );
	}

	// A click from button 3 selects the row, highlights the node,
	// shows the name, updates the file list if necessary, and brings
	// up a context-sensitive menu 
	if (ev_button->button == 3) {
		//gtk_clist_select_row( GTK_CLIST(ctree_w), row, 0 );
		geometry_highlight_node( dnode, FALSE );
		window_statusbar( SB_RIGHT, node_absname( dnode ) );
		if (dnode != dirtree_current_dnode)
			filelist_populate( dnode );
		dirtree_current_dnode = dnode;
		context_menu( dnode, ev_button );
		return ret;
	}
	return ret;
}

/*void FsvDirTree::on_row_activated	(const Gtk::TreeModel::Path&	path,Gtk::TreeViewColumn* column){
	const Gtk::TreeModel::Row row = *model->get_iter(path);
	GNode *dnode = (GNode *)row[records.dnode];
	geometry_highlight_node( dnode, FALSE );
	window_statusbar( SB_RIGHT, node_absname( dnode ) );
	if (dnode != dirtree_current_dnode)
		filelist_populate( dnode );
	dirtree_current_dnode = dnode;
}*/
/* collapse of a directory tree entry */
void FsvDirTree::on_row_collapsed	(const Gtk::TreeModel::iterator& iter,const Gtk::TreeModel::Path&	path){
	if (globals.fsv_mode == FSV_SPLASH)
		return;
	const Gtk::TreeModel::Row row = *iter;
	GNode *dnode = (GNode *)row[records.dnode];
	colexp( dnode, COLEXP_COLLAPSE_RECURSIVE );
}
/* expand of a directory tree entry */
void FsvDirTree::on_row_expanded 	(const Gtk::TreeModel::iterator& iter,const Gtk::TreeModel::Path&	path){
	if (globals.fsv_mode == FSV_SPLASH)
		return;
	const Gtk::TreeModel::Row row = *iter;
	GNode *dnode = (GNode *)row[records.dnode];
	colexp( dnode, COLEXP_EXPAND );
}

FsvDirTree* FsvDirTree::dir_tree;

FsvDirTree::FsvDirTree(){
  model = Gtk::TreeStore::create(records);
  set_model(model);
  dir_tree = this;
	set_headers_visible(false);
	Gtk::TreeView::Column* pcol = Gtk::manage( new Gtk::TreeView::Column() );
	pcol->pack_start(records.icon,false);
	pcol->pack_start(records.name);
	append_column(*pcol);
	folder_closed = render_icon(Gtk::Stock::DIRECTORY,Gtk::IconSize(Gtk::ICON_SIZE_MENU));//Gdk::Pixbuf::create_from_xpm_data(mini_folder_closed_xpm);
	folder_opened = render_icon(Gtk::Stock::OPEN     ,Gtk::IconSize(Gtk::ICON_SIZE_MENU));//Gdk::Pixbuf::create_from_xpm_data(mini_folder_open_xpm);
}

/* Clears out all entries from the directory tree */
void
dirtree_clear( void )
{
  FsvDirTree::dir_tree->model->clear();
	dirtree_current_dnode = NULL;
}


/* Adds a new entry to the directory tree */
void
dirtree_entry_new( GNode *dnode )
{
	char *parent_ctnode = NULL;
	const char *name;
	boolean expanded;

	g_assert( NODE_IS_DIR(dnode) );

	parent_ctnode = DIR_NODE_DESC(dnode->parent)->ctnode;
	if (strlen( NODE_DESC(dnode)->name ) > 0)
		name = NODE_DESC(dnode)->name;
	else
		name = _("/. (root)");
	expanded = g_node_depth( dnode ) <= 2;

	if (parent_ctnode == NULL) {
	  Gtk::TreeIter iter = FsvDirTree::dir_tree->model->append();
	  Gtk::TreeModel::Row row = *(iter);
    row[FsvDirTree::dir_tree->records.name]=name;
    row[FsvDirTree::dir_tree->records.icon]=FsvDirTree::dir_tree->folder_opened;
    row[FsvDirTree::dir_tree->records.dnode]=dnode;
		DIR_NODE_DESC(dnode)->ctnode = strdup(FsvDirTree::dir_tree->model->get_string(iter).c_str());
		FsvDirTree::dir_tree->expand_row(Gtk::TreePath(iter),false);
	}else{
	  Gtk::TreeIter piter = FsvDirTree::dir_tree->model->get_iter(parent_ctnode);
	  Gtk::TreeIter iter = FsvDirTree::dir_tree->model->append(piter->children());
	  Gtk::TreeModel::Row row = *(iter);
    row[FsvDirTree::dir_tree->records.name]=name;
    row[FsvDirTree::dir_tree->records.icon]=expanded?FsvDirTree::dir_tree->folder_opened:FsvDirTree::dir_tree->folder_closed;	  
    row[FsvDirTree::dir_tree->records.dnode]=dnode;
		DIR_NODE_DESC(dnode)->ctnode = strdup(FsvDirTree::dir_tree->model->get_string(iter).c_str());
		if(expanded)FsvDirTree::dir_tree->expand_row(Gtk::TreePath(iter),false);
	}
	if(parent_ctnode && FsvDirTree::dir_tree->row_expanded(Gtk::TreePath(parent_ctnode))) {
		/* Pre-update (allow ctree to register new row) */
		//gtk_clist_thaw( GTK_CLIST(dir_ctree_w) );
		gui_update( );
		//gtk_clist_freeze( GTK_CLIST(dir_ctree_w) );
		/* Select last row */
		//gtk_ctree_select( GTK_CTREE(dir_ctree_w), DIR_NODE_DESC(dnode)->ctnode );
    Glib::RefPtr<Gtk::TreeSelection> sel = FsvDirTree::dir_tree->get_selection();
    Gtk::TreePath path = Gtk::TreePath(DIR_NODE_DESC(dnode)->ctnode);
    sel->select(path);
		/* Scroll directory tree down to last row */
		//gui_clist_moveto_row( dir_ctree_w, -1, 0.0 );
		FsvDirTree::dir_tree->expand_row(path,false);
		FsvDirTree::dir_tree->scroll_to_row(path);
		/* Post-update (allow ctree to perform select/scroll) */
		//gtk_clist_thaw( GTK_CLIST(dir_ctree_w) );
		gui_update( );
		//gtk_clist_freeze( GTK_CLIST(dir_ctree_w) );
	}
}

/* This updates the directory tree to show (and select) a particular
 * directory entry, repopulating the file list with the contents of the
 * directory if not already listed */
void
dirtree_entry_show( GNode *dnode )
{
	g_assert( NODE_IS_DIR(dnode) );

	/* Repopulate file list if directory is different */
	if (dnode != dirtree_current_dnode) {
		filelist_populate( dnode );
/* TODO: try removing this update from here */
		//gui_update( );
	}

	/* Scroll directory tree to proper entry */
	Gtk::TreeModel::Path path = Gtk::TreeModel::Path(DIR_NODE_DESC(dnode)->ctnode);
  Glib::RefPtr<Gtk::TreeSelection> sel = FsvDirTree::dir_tree->get_selection();
	if(FsvDirTree::dir_tree->model->get_iter(path)){
	  sel->select(path);
	  FsvDirTree::dir_tree->scroll_to_row(path);
	}else{
	  sel->unselect_all();
	}
	dirtree_current_dnode = dnode;
}

/* Returns TRUE if the entry for the given directory is expanded */
boolean
dirtree_entry_expanded( GNode *dnode )
{
	g_assert( NODE_IS_DIR(dnode) );
	//return GTK_CTREE_ROW(DIR_NODE_DESC(dnode)->ctnode)->expanded;
  return FsvDirTree::dir_tree->row_expanded(Gtk::TreeModel::Path(DIR_NODE_DESC(dnode)->ctnode));
}

/* Recursively collapses the directory tree entry of the given directory */
void
dirtree_entry_collapse_recursive( GNode *dnode )
{
	g_assert( NODE_IS_DIR(dnode) );
  FsvDirTree::dir_tree->collapse_row(Gtk::TreeModel::Path(DIR_NODE_DESC(dnode)->ctnode));
}


/* Expands the directory tree entry of the given directory. If any of its
 * ancestor directory entries are not expanded, then they are expanded
 * as well */
void
dirtree_entry_expand( GNode *dnode )
{
//	GNode *up_node;

	g_assert( NODE_IS_DIR(dnode) );

/*	block_colexp_handlers( );
	up_node = dnode;
	while (NODE_IS_DIR(up_node)) {
		if (!dirtree_entry_expanded( up_node ))
			gtk_ctree_expand( GTK_CTREE(dir_ctree_w), DIR_NODE_DESC(up_node)->ctnode );
		up_node = up_node->parent;
	}
	unblock_colexp_handlers( );*/
  FsvDirTree::dir_tree->expand_to_path(Gtk::TreeModel::Path(DIR_NODE_DESC(dnode)->ctnode));
}


/* Recursively expands the entire directory tree subtree of the given
 * directory */
void
dirtree_entry_expand_recursive( GNode *dnode )
{
	g_assert( NODE_IS_DIR(dnode) );

#if DEBUG
	/* Guard against expansions inside collapsed subtrees */
	/** NOTE: This function may be upgraded to behave similarly to
	 ** dirtree_entry_expand( ) w.r.t. collapsed parent directories.
	 ** This has been avoided thus far since such a behavior would
	 ** not be used by the program. */
	if (NODE_IS_DIR(dnode->parent))
		g_assert( dirtree_entry_expanded( dnode->parent ) );
#endif

	/*block_colexp_handlers( );
	gtk_ctree_expand_recursive( GTK_CTREE(dir_ctree_w), DIR_NODE_DESC(dnode)->ctnode );
	unblock_colexp_handlers( );*/
  FsvDirTree::dir_tree->expand_row(Gtk::TreeModel::Path(DIR_NODE_DESC(dnode)->ctnode),true);

}


/* end dirtree.c */

