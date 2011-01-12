/* filelist.c */

/* File list control */

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
#include "filelist.h"
#include "fsvwindow.h"

#include <gtk/gtk.h>

#include "about.h"
#include "camera.h"
#include "dirtree.h"
#include "geometry.h"


/* Time for the filelist to scroll to a given entry (in seconds) */
#define FILELIST_SCROLL_TIME 0.5

static GNode *filelist_current_dnode;

/* Mini node type icons */
static Glib::RefPtr<Gdk::Pixbuf> node_type_mini_icons[NUM_NODE_TYPES];


/* Loads the mini node type icons (from XPM data) */
static void
filelist_icons_init( void )
{
/*	GtkStyle *style;
	GdkColor *trans_color;
	GdkWindow *window;
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	int i;

	style = gtk_widget_get_style( file_clist_w );
	trans_color = &style->bg[GTK_STATE_NORMAL];
	gtk_widget_realize( file_clist_w );
	window = file_clist_w->window;

	// Make mini node type icons 
	for (i = 1; i < NUM_NODE_TYPES; i++) {
		pixmap = gdk_pixmap_create_from_xpm_d( window, &mask, trans_color, node_type_mini_xpms[i] );
		node_type_mini_icons[i].pixmap = pixmap;
		node_type_mini_icons[i].mask = mask;
	}*/
}

FsvFileList* FsvFileList::file_tree;

FsvFileList::FsvFileList():nm_s(_("Type")),count_s(_("Found"),prop_cols.count),size_s(_("Bytes"),prop_cols.size){
  nm.pack_start(records.icon,false);
  nm.pack_end(records.name);
  nm_s.pack_start(prop_cols.icon,false);
  nm_s.pack_end(prop_cols.name);
  model = Gtk::ListStore::create(records);
  scan_model = Gtk::ListStore::create(prop_cols);
	for (int i = 1; i <= NUM_NODE_TYPES; i++) {
	  Gtk::TreeRow row = *(scan_model->append());
		if (i < NUM_NODE_TYPES) {
			row[prop_cols.icon] = node_type_mini_icons[i];
			row[prop_cols.name] = _(node_type_plural_names[i]);
		}
		else
		  row[prop_cols.name] = _("TOTAL");
		//gtk_clist_set_selectable( GTK_CLIST(file_clist_w), i - 1, FALSE );
	}
	node_type_mini_icons[NODE_DIRECTORY] = render_icon(Gtk::Stock::DIRECTORY,Gtk::IconSize(Gtk::ICON_SIZE_MENU));
	node_type_mini_icons[NODE_REGFILE] = render_icon(Gtk::Stock::FILE,Gtk::IconSize(Gtk::ICON_SIZE_MENU));

  FsvFileList::file_tree = this;
}

void FsvFileList::switch_scan(){
  remove_all_columns();
  set_model(scan_model);
  append_column(nm_s);
  append_column(count_s);
  append_column(size_s);
	set_headers_visible(true);
}
void FsvFileList::switch_list(){
  remove_all_columns();
  set_model(model);
  append_column(nm);
	set_headers_visible(false);
}

/* This makes entries in the file list selectable or unselectable,
 * depending on whether the directory they are in is expanded or not */
void filelist_reset_access( void )
{
	boolean enabled;

  enabled = dirtree_entry_expanded( filelist_current_dnode );
  FsvFileList::file_tree->set_sensitive(enabled);
	/* Extra fluff for interface niceness */
	if (enabled)
		gui_cursor( GTK_WIDGET(FsvFileList::file_tree->gobj()), -1 );
	else {
		FsvFileList::file_tree->get_selection()->unselect_all();
		gui_cursor( GTK_WIDGET(FsvFileList::file_tree->gobj()), GDK_X_CURSOR );
	}
}


/* Compare function for sorting nodes alphabetically */
static int compare_node( GNode *a, GNode *b )
{
	return strcmp( NODE_DESC(a)->name, NODE_DESC(b)->name );
}


/* Displays contents of a directory in the file list */
void
filelist_populate( GNode *dnode )
{
	GNode *node;
	GList *node_list = NULL, *node_llink;
	int count = 0;
	char strbuf[64];

	g_assert( NODE_IS_DIR(dnode) );

  /* Get an alphabetized list of directory's immediate children */
	node = dnode->children;
	while (node != NULL) {
		G_LIST_PREPEND(node_list, node);
		node = node->next;
	}
	G_LIST_SORT(node_list, compare_node);

	/* Update file clist */
	FsvFileList::file_tree->model->clear();
	node_llink = node_list;
	while (node_llink != NULL) {
		node = (GNode *)node_llink->data;
    Gtk::TreeRow row = *(FsvFileList::file_tree->model->append());
		row[FsvFileList::file_tree->records.icon] = node_type_mini_icons[NODE_DESC(node)->type];
		row[FsvFileList::file_tree->records.name] = NODE_DESC(node)->name;
		row[FsvFileList::file_tree->records.node] = node;
		++count;
		node_llink = node_llink->next;
	}
	g_list_free( node_list );
	/* Set node count message in the left statusbar */
	switch (count) {
		case 0:
		strcpy( strbuf, "" );
		break;

		case 1:
		strcpy( strbuf, _("1 node") );
		break;

		default:
		sprintf( strbuf, _("%d nodes"), count );
		break;
	}
	window_statusbar( SB_LEFT, strbuf );

	filelist_current_dnode = dnode;
	filelist_reset_access( );
}

/* This updates the file list to show (and select) a particular node
 * entry. The directory tree is also updated appropriately */
void filelist_show_entry( GNode *node )
{
	GNode *dnode;
	/* Corresponding directory */
	if (NODE_IS_DIR(node))
		dnode = node;
	else
		dnode = node->parent;

	if (dnode != filelist_current_dnode) {
		/* Scroll directory tree to proper entry */
		dirtree_entry_show( dnode );
	}

	/* Scroll file list to proper entry */
  Glib::RefPtr<Gtk::TreeSelection> sel = FsvFileList::file_tree->get_selection();
	Gtk::TreeModel::Children children = FsvFileList::file_tree->model->children();
	Gtk::TreeIter it = children.begin();
	for(;it!=children.end();++it){
	  if((*it)[FsvFileList::file_tree->records.node]==node){
	    sel->select(it);
	    FsvFileList::file_tree->scroll_to_row(FsvFileList::file_tree->model->get_path(it));
	    return;
	  }
	}
  sel->unselect_all();
}


/* Callback for a click in the file list area */
bool FsvFileList::on_button_press_event(GdkEventButton* ev_button){
  bool ret = Gtk::TreeView::on_button_press_event(ev_button);
	// If About presentation is up, end it 
	about( ABOUT_END );
	if (globals.fsv_mode == FSV_SPLASH)	return false;

  
  Glib::RefPtr<Gtk::TreeSelection> sel = get_selection();
  Gtk::TreeIter iter = sel->get_selected();
	GNode *node = (*iter)[FsvFileList::file_tree->records.node];//(GNode *)gtk_clist_get_row_data( GTK_CLIST(clist_w), row );
	if (node == NULL)
		return FALSE;

	// A single-click from button 1 highlights the node and shows the
	// name (and also selects the row, but GTK+ does that for us) 
	if ((ev_button->button == 1) && (ev_button->type == GDK_BUTTON_PRESS)) {
		sel->select(iter);
		geometry_highlight_node( node, FALSE );
		window_statusbar( SB_RIGHT, node_absname( node ) );
		return ret;
	}

	// A double-click from button 1 gets the camera moving 
	if ((ev_button->button == 1) && (ev_button->type == GDK_2BUTTON_PRESS)) {
		camera_look_at( node );
		return ret;
	}

	// A click from button 3 selects the row, highlights the node,
	// shows the name, and pops up a context-sensitive menu 
	if (ev_button->button == 3) {
		sel->select(iter);
		geometry_highlight_node( node, FALSE );
		window_statusbar( SB_RIGHT, node_absname( node ) );
		context_menu( node, ev_button );
		return ret;
	}

	return FALSE;
}


/* Creates/initializes the file list widget */
void filelist_init( void )
{
  FsvFileList::file_tree->switch_list();
	filelist_populate( root_dnode );

	// Do this so that directory tree gets scrolled to the to at
	//  end of initial camera pan (right after filesystem scan) 
	filelist_current_dnode = NULL;
}


/* This replaces the file list widget with another one made specifically
 * to monitor the progress of an impending scan */
void filelist_scan_monitor_init( void ){
  FsvFileList::file_tree->switch_scan();
}

/* Updates the scan-monitoring file list with the given values */
void filelist_scan_monitor( int *node_counts, int64 *size_counts )
{
	//const char *str;
	int64 size_total = 0;
	int node_total = 0;
  Gtk::TreeModel::Children rows = FsvFileList::file_tree->scan_model->children();
	for (int i = 1; i <= NUM_NODE_TYPES; i++) {
		/* Column 2 */
		if (i < NUM_NODE_TYPES) {
			//str = i64toa( node_counts[i] );
      rows[i-1][FsvFileList::file_tree->prop_cols.count]= node_counts[i];
			node_total += node_counts[i];
		}
		else
      rows[i-1][FsvFileList::file_tree->prop_cols.count]= node_total;
			//str = i64toa( node_total );
		//gtk_clist_set_text( GTK_CLIST(file_clist_w), i - 1, 1, str );
		/* Column 3 */
		if (i < NUM_NODE_TYPES) {
			//str = i64toa( size_counts[i] );
      rows[i-1][FsvFileList::file_tree->prop_cols.size]= size_counts[i];
			size_total += size_counts[i];
		}
		else
      rows[i-1][FsvFileList::file_tree->prop_cols.size]= size_total;
		//	str = i64toa( size_total );
		//gtk_clist_set_text( GTK_CLIST(file_clist_w), i - 1, 2, str );
	}
	//gtk_clist_thaw( GTK_CLIST(file_clist_w) );
}

/* Creates the clist widget used in the "Contents" page of the Properties
 * dialog for a directory */
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
};

GtkWidget *
dir_contents_list( GNode *dnode )
{
	PropColumns prop_cols;
  Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(prop_cols);
	Gtk::TreeView *clist_w = Gtk::manage( new Gtk::TreeView(model));

	g_assert( NODE_IS_DIR(dnode) );

	Gtk::TreeView::Column* pcol = Gtk::manage( new Gtk::TreeView::Column(_("Node type") ) );
	pcol->pack_start(prop_cols.icon,false);
	pcol->pack_start(prop_cols.name);
	clist_w->append_column(*pcol);
	pcol = Gtk::manage( new Gtk::TreeView::Column(_("Quantity"),prop_cols.count ) );
  clist_w->append_column(*pcol);

	for (int i = 1; i < NUM_NODE_TYPES; i++) {
	  Gtk::TreeRow row = *(model->append());
	  row[prop_cols.icon] = node_type_mini_icons[i];
	  row[prop_cols.name] = _(node_type_plural_names[i]);
	  row[prop_cols.count] = DIR_NODE_DESC(dnode)->subtree.counts[i];
	}

	return GTK_WIDGET(clist_w->gobj());
}


/* end filelist.c */

