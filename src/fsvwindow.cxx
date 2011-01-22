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

#include <iostream>

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
  Gtk::Menu::MenuList& items = FsvWindow::current->popa->items();
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
  FsvWindow::current->popa->popup(ev_button->button,ev_button->time);
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
  hide();
#ifdef HAVE_GL_GLC_H
  glcDeleteGLObjects();
  //glcDeleteFont(textFont);
  //glcDeleteContext(glcGetCurrentContext());
  //glcContext(0);
  exit(0);
#endif
}
void FsvWindow::on_about(){
  about( ABOUT_BEGIN );
}
void FsvWindow::on_fsv_mode(FsvMode mode){
	if (globals.fsv_mode != mode)
		fsv_set_mode( mode );
}
void FsvWindow::on_birdseye_view(){
  camera_birdseye_view(birdeye->get_active() );
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
Gtk::StockID FsvWindow::BIRDEYE("bird-eye");
Gtk::StockItem FsvWindow::BirdEye(FsvWindow::BIRDEYE,_("Top View"));

#ifdef HAVE_GL_GLC_H
GLint textFont=-1;
#endif

FsvWindow::~FsvWindow(){
#ifdef HAVE_GL_GLC_H
  g_print("begin %s\n",__FUNCTION__);
  glcDeleteGLObjects();
  ////glcDeleteFont(textFont);
  //glcDeleteContext(glcGetCurrentContext());
  //glcContext(0);
  glcDeleteContext(ctx);
  g_print("end   %s\n",__FUNCTION__);
#endif
}


FsvWindow::FsvWindow() : Gtk::Window(){
/*  int wd,hg;
  Gtk::IconSize::lookup(Gtk::ICON_SIZE_LARGE_TOOLBAR,wd,hg);
  g_print("is:%d %d\n",wd,hg);*/
  Glib::RefPtr<Gtk::IconFactory> icfa=Gtk::IconFactory::create();
  icfa->add(BIRDEYE,Gtk::IconSet(Gdk::Pixbuf::create_from_xpm_data(birdseye_view_xpm)));
  /*icfa->add(BIRDEYE,Gtk::IconSet(Gdk::Pixbuf::create_from_xpm_data(birdseye_view_xpm)
  ->scale_simple(18,18,Gdk::INTERP_NEAREST )));
  icfa->add(BIRDEYE,Gtk::IconSet(Gdk::Pixbuf::create_from_xpm_data(birdseye_view_xpm)
  ->scale_simple(32,32,Gdk::INTERP_NEAREST )));*/
  Gtk::Stock::add(BirdEye);
  icfa->add_default();
  
  add(bx_main);
  bx_main.set_homogeneous(false);
  ag_unsetsitive = Gtk::ActionGroup::create("unsens");
  ag_unsetsitive->add(Gtk::Action::create("Chroot",_("_Change Root")),
                  sigc::mem_fun(*this,&FsvWindow::on_change_root) );
  
  Gtk::RadioAction::Group group_mode;
  ag_unsetsitive->add(Gtk::RadioAction::create(group_mode,"MapV",_("_Map View")),
                  sigc::bind(sigc::mem_fun(*this,&FsvWindow::on_fsv_mode),FSV_MAPV) );
  ag_unsetsitive->add(Gtk::RadioAction::create(group_mode,"TreeV",_("_Tree View")),
                  sigc::bind(sigc::mem_fun(*this,&FsvWindow::on_fsv_mode),FSV_TREEV) );
  ag_unsetsitive->add(Gtk::RadioAction::create(group_mode,"DiskV",_("_Disc View")),
                  sigc::bind(sigc::mem_fun(*this,&FsvWindow::on_fsv_mode),FSV_DISCV) );
  
  Gtk::RadioAction::Group group_color;
  ag_unsetsitive->add(Gtk::RadioAction::create(group_color,"ColType",_("By n_odetype")),
                  sigc::bind(sigc::mem_fun(*this,&FsvWindow::on_color_type),COLOR_BY_NODETYPE) );
  ag_unsetsitive->add(Gtk::RadioAction::create(group_color,"ColTime",_("By t_imestamp")),
                  sigc::bind(sigc::mem_fun(*this,&FsvWindow::on_color_type),COLOR_BY_TIMESTAMP) );
  ag_unsetsitive->add(Gtk::RadioAction::create(group_color,"ColWild",_("By _wildcard")),
                  sigc::bind(sigc::mem_fun(*this,&FsvWindow::on_color_type),COLOR_BY_WPATTERN) );
  birdeye = Gtk::ToggleAction::create("Beye",BIRDEYE);
  ag_unsetsitive->add(birdeye,sigc::mem_fun(*this,&FsvWindow::on_birdseye_view));
  //Gtk
  ag_unsetsitive->add(Gtk::Action::create("CDRoot",Gtk::Stock::GOTO_FIRST),
                      sigc::mem_fun(*this,&FsvWindow::on_cd_root));
  ag_unsetsitive->add(Gtk::Action::create("CDBack",Gtk::Stock::GO_BACK),
                      sigc::mem_fun(*this,&FsvWindow::on_cd_back));
  ag_unsetsitive->add(Gtk::Action::create("CDUp",Gtk::Stock::GO_UP),
                      sigc::mem_fun(*this,&FsvWindow::on_cd_up));
  
  ag_unsetsitive->add(Gtk::Action::create("Collapse",_("Collapse")),
                      sigc::mem_fun(*this,&FsvWindow::on_collapse));
  ag_unsetsitive->add(Gtk::Action::create("Expand",_("Expand")),
                      sigc::mem_fun(*this,&FsvWindow::on_expand));
  ag_unsetsitive->add(Gtk::Action::create("Expandall",_("Expand all")),
                      sigc::mem_fun(*this,&FsvWindow::on_expand_all));
  ag_unsetsitive->add(Gtk::Action::create("Lookat",_("Look at")),
                      sigc::mem_fun(*this,&FsvWindow::on_look_at));
  ag_unsetsitive->add(Gtk::Action::create("Properties",_("Properties")),
                      sigc::mem_fun(*this,&FsvWindow::on_properties));

  ag_allways = Gtk::ActionGroup::create("main");
  ag_allways->add(Gtk::Action::create("File",_("_File")));
  ag_allways->add(Gtk::Action::create("View",_("_View")));
  ag_allways->add(Gtk::ToggleAction::create("FullScreen",_("_Full Screen")),Gtk::AccelKey("F11"),
                  sigc::mem_fun(*this,&FsvWindow::on_fullscreen));
  ag_allways->add(Gtk::Action::create("Quit",Gtk::Stock::QUIT),
                  sigc::mem_fun(*this,&FsvWindow::on_exit));
  ag_allways->add(Gtk::Action::create("Help",_("_Help")));
  ag_allways->add(Gtk::Action::create("About",_("_About")),
                  sigc::mem_fun(*this,&FsvWindow::on_about));
  ag_allways->add(Gtk::Action::create("Pref",Gtk::Stock::PREFERENCES),
                  sigc::mem_fun(*this,&FsvWindow::on_color_setup) );

  ui_man = Gtk::UIManager::create();
  ui_man->insert_action_group(ag_unsetsitive);
  ui_man->insert_action_group(ag_allways);
  add_accel_group(ui_man->get_accel_group());
  
  Glib::ustring ui_info = 
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='File'>"
        "      <menuitem action='Chroot'/>"
        "      <separator/>"
        "      <menuitem action='Quit'/>"
        "    </menu>"
        "    <menu action='View'>"
        "      <menuitem action='FullScreen'/>"
        "      <separator/>"
        "      <menuitem action='MapV'/>"
        "      <menuitem action='TreeV'/>"
        "      <separator/>"
        "      <menuitem action='Beye'/>"
        "      <separator/>"
        "      <menuitem action='ColType'/>"
        "      <menuitem action='ColTime'/>"
        "      <menuitem action='ColWild'/>"
        "      <separator/>"
        "      <menuitem action='Pref'/>"
        "    </menu>"
        "    <menu action='Help'>"
        "      <menuitem action='About'/>"
        "    </menu>"
        "  </menubar>"
        "  <toolbar  name='ToolBar'>"
        "    <toolitem action='CDRoot'/>"
        "    <toolitem action='CDBack'/>"
        "    <toolitem action='CDUp'/>"
        //"    <separator action='Sep1'/>"
        "    <toolitem action='Beye'/>"
        "  </toolbar>"
        "  <popup name='Popa'>"
        "      <menuitem action='Collapse'/>"
        "      <menuitem action='Expand'/>"
        "      <menuitem action='Expandall'/>"
        "      <menuitem action='Lookat'/>"
        "      <menuitem action='Properties'/>"
        "  </popup>"
        "</ui>";

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {  
    ui_man->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }
  #else
  std::auto_ptr<Glib::Error> error;
  ui_man->add_ui_from_string(ui_info, error);
  if(error.get())
  {
    std::cerr << "building menus failed: " <<  error->what();
  }
  #endif //GLIBMM_EXCEPTIONS_ENABLED

  Gtk::Widget* pMenuBar = ui_man->get_widget("/MenuBar") ;
  bx_left.pack_start(*pMenuBar, Gtk::PACK_SHRINK);
  //
  bx_main.pack_start(pn_main,true,true);

  Gtk::Widget* pToolBar = ui_man->get_widget("/ToolBar") ;

  bx_left.pack_start(*pToolBar,false,false);
  
  popa = dynamic_cast<Gtk::Menu*>(ui_man->get_widget("/Popa"));
  
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
  
#if defined HAVE_FTGL
  font = new FTTextureFont("./DejaVuSans.ttf");
  font->CharMap(ft_encoding_unicode);
  font->FaceSize(24.);
  font->Outset(10.);
#elif defined HAVE_GL_GLC_H
  ctx = glcGenContext();
  glcContext(ctx);
  //glcAppendCatalog("/usr/lib/X11/fonts/Type1");

  /* Create a font "Sans Bold" */
  textFont = glcGenFontID();
  g_print("GLC_MASTER_COUNT %d\n",glcGeti(GLC_MASTER_COUNT));
  //
//  glcNewFontFromFamily(textFont, "Sans");
      static const char* master_name="Sans";
      static const char* face_name="Normal";
      int master_count = glcGeti(GLC_MASTER_COUNT);
      int master = 0;
      int i; 
      for (i = 0; i < master_count; i++) {
         if (!strcmp((const char*)glcGetMasterc(i, GLC_FAMILY), master_name)) {
            int face_count = glcGetMasteri(i, GLC_FACE_COUNT);
            for (int j = 0; j < face_count; j++) {
               if (!strcmp((const char*)glcGetMasterListc(i, GLC_FACE_LIST, j), face_name)) {
                  master = i;
               }
            }
         }
      }


      if (i < master_count) {
        /* Access the font family. */
        GLint result = glcNewFontFromFamily(textFont,glcGetMasterc(master, GLC_FAMILY));
        if (result == textFont) {
          /* Use the font. */
          glcFont(textFont);

          /* Use the face. */
          glcFontFace(textFont, face_name);
        }
      }
/*  glcNewFontFromFamily(textFont, "Monospace");
  glcFontFace(textFont, "Normal");
  glcFont(textFont);*/
  glcStringType(GLC_UTF8_QSO);
  glcRenderStyle(GLC_LINE);
  ////glcRenderStyle(GLC_TRIANGLE);
  //glcRenderStyle(GLC_TEXTURE);
#endif  
  show_all_children();
};

#include <GL/gl.h>


void text_init( void ){}
void text_pre( void ){
	glDisable( GL_LIGHTING );
	glDisable( GL_POLYGON_OFFSET_FILL );
	glEnable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
}
void text_post( void ){
	glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_POLYGON_OFFSET_FILL );
	glEnable( GL_LIGHTING );
}


static int
get_char_dims(const char *text, const XYvec *max_dims, XYvec *cdims ,double *scale, int reallen){
    int len;
    if(g_utf8_validate(text,-1,NULL)){
	    len = g_utf8_strlen(text,-1);
	    FTBBox box =  FsvWindow::current->font->BBox(text);
	    cdims->x=box.Upper().X()-box.Lower().X();
	    cdims->y=box.Upper().Y()-box.Lower().Y();
	    //g_print("%s %e %e -  %e %e\n",text,cdims->x,cdims->y,max_dims->x,max_dims->y);
	    if(cdims->x*max_dims->y/24. > max_dims->x){
	      *scale= max_dims->x *24./ cdims->x;
	      cdims->x=max_dims->x/ *scale *24.;
	    }else
    	  *scale=max_dims->y;
    }else{
	    g_print("vot :%s invalid\n",text);
    }
    *scale;
  return (reallen==0)?1:len;
}


/* Draws a straight line of text centered at the given position,
 * fitting within the dimensions specified */
void
text_draw_straight( const char *text, const XYZvec *text_pos, const XYvec *text_max_dims )
{
	XYvec cdims;
	XYvec c0;
	int len, i;
	double scale=1.;
	len = get_char_dims( text, text_max_dims, &cdims ,&scale,0);
	/* Corners of first character */
	c0.x = text_pos->x - 0.5 * cdims.x *scale/24.;
	c0.y = text_pos->y - 0.5 * cdims.y *scale/24.;
	//c1.x = c0.x + cdims.x;
	//c1.y = c0.y + cdims.y;

    glPushMatrix();
    glTranslated(c0.x,c0.y,text_pos->z);
    glScaled(scale/24.,text_max_dims->y/24.,1.);//text_max_dims->y);
    FsvWindow::current->font->Render(text);
    glPopMatrix();
}


/* Draws a straight line of text centered at the given position, rotated
 * to be tangent to a circle around the origin, and fitting within the
 * dimensions specified (which are also rotated) */
void
text_draw_straight_rotated( const char *text, const RTZvec *text_pos, const XYvec *text_max_dims )
{
	XYvec cdims;
	XYvec t_c0, t_c1, c0, c1;
	XYvec hdelta, vdelta;
	double sin_theta, cos_theta,scale;
	int len, i;

	len = get_char_dims( text, text_max_dims, &cdims ,&scale,0);

	sin_theta = sin( RAD(text_pos->theta) );
	cos_theta = cos( RAD(text_pos->theta) );

	/* Vector to move from one character to the next */
	hdelta.x = sin_theta * cdims.x/24.;
	hdelta.y = - cos_theta * cdims.x/24.;
	/* Vector to move from bottom of character to top */
	vdelta.x = cos_theta * cdims.y/24.;
	vdelta.y = sin_theta * cdims.y/24.;

	/* Corners of first character */
	c0.x = cos_theta * text_pos->r - 0.5 * ((double)len * hdelta.x + vdelta.x);
	c0.y = sin_theta * text_pos->r - 0.5 * ((double)len * hdelta.y + vdelta.y);
	c1.x = c0.x + hdelta.x + vdelta.x;
	c1.y = c0.y + hdelta.y + vdelta.y;

   glPushMatrix();
    glTranslated(c0.x,c0.y,text_pos->z);
    glRotated(-90.+text_pos->theta,0.,0.,1.);
    glScalef(scale/24.,text_max_dims->y/24.,text_max_dims->y/24.);
    FsvWindow::current->font->Render(text);
    glPopMatrix();
}
/* Draws a curved arc of text, occupying no more than the depth and arc
 * width specified. text_pos indicates outer edge (not center) of text */
void
text_draw_curved( const char *text, const RTZvec *text_pos, const RTvec *text_max_dims )
{
	XYvec straight_dims, cdims;
	XYvec char_pos, fwsl, bwsl;
	XYvec t_c0, t_c1;
	double char_arc_width, theta;
	double sin_theta, cos_theta;
	double text_r,scale;
	int len, i;

	/* Convert curved dimensions to straight equivalent */
	straight_dims.x = (PI / 180.0) * text_pos->r * text_max_dims->theta;
	straight_dims.y = text_max_dims->r;

	len = get_char_dims( text, &straight_dims, &cdims ,&scale,1);

	/* Radius of center of text line */
	text_r = text_pos->r - 0.5 * cdims.y/24.;

	/* Arc width occupied by each character */
	char_arc_width = (180.0 / PI) * cdims.x/24. / text_r;

    theta = text_pos->theta + 0.5 * char_arc_width;
    sin_theta = sin( RAD(theta) );
    cos_theta = cos( RAD(theta) );

    char_pos.x = cos_theta * text_r;
    char_pos.y = sin_theta * text_r;
    glPushMatrix();
    glTranslated(char_pos.x,char_pos.y,text_pos->z);
    glRotated(-90.,0,0,1);
    glTranslated(0,-straight_dims.y/3.,0);
    glScaled(scale/24.,straight_dims.y/24.,straight_dims.y/24.);
    FsvWindow::current->font->Render(text);
    glPopMatrix();
}


