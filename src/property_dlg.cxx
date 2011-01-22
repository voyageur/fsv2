#include "property_dlg.h"

PropertyDialog::PropertyDialog(GNode *nd ): Gtk::Dialog(_("Properties"),true),node(nd)
  ,tbl_gen(6,2)
  ,lbl_type(_("Type:"),0.9,0.5)
  ,lbl_loc(_("Location:"),0.9,0.5)
  ,lbl_alloc(_("Allocation:"),0.9,0.5)
  ,lbl_owner(_("Owner:"),0.9,0.5)
  ,lbl_group(_("Group:"),0.9,0.5)
  ,lbl_modif(_("Modified:"),0.9,0.5)
  ,lbl_attrib(_("AttribCh:"),0.9,0.5)
  ,lbl_access( _("Accessed:"),0.9,0.5)
{
  get_vbox()->add(nbk);
  nbk.append_page(tbl_gen,_("General"));
  tbl_gen.set_spacings(5);
  tbl_gen.set_border_width(10);
  const struct NodeInfo* node_info = get_node_info( node );
  if(NODE_DESC(node)->type==NODE_DIRECTORY){
    ico.set(Gtk::Stock::DIRECTORY,Gtk::IconSize(Gtk::ICON_SIZE_DIALOG) );
  } else if(NODE_DESC(node)->type==NODE_REGFILE){
    ico.set(Gtk::Stock::FILE,Gtk::IconSize(Gtk::ICON_SIZE_DIALOG) );
  } else {
    ico.set(Gdk::Pixbuf::create_from_xpm_data(node_type_xpms[NODE_DESC(node)->type]));
  }
  int pos=0; 
  tbl_gen.attach(ico,0,1,pos,pos+1);
  Gtk::Label* lbl = Gtk::manage(new Gtk::Label(node_info->name,0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
  tbl_gen.attach(lbl_type,0,1,pos,pos+1);
  lbl = Gtk::manage(new Gtk::Label(_(node_type_names[NODE_DESC(node)->type]),0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
  tbl_gen.attach(lbl_loc,0,1,pos,pos+1);
  lbl = Gtk::manage(new Gtk::Label(node_info->prefix,0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
	if (NODE_IS_DIR(node)){
    lbl_size.set_label(_("Total size:"));
    tbl_gen.attach(lbl_size,0,1,pos,pos+1);
    Glib::ustring lbsz(node_info->subtree_size);
    lbsz+=" bytes";
    if (DIR_NODE_DESC(node)->subtree.size >= 1024) {
			lbsz+=Glib::ustring(" (")+node_info->subtree_size_abbr+")";
		}
    lbl = Gtk::manage(new Gtk::Label(lbsz,0.,0.5));
    tbl_gen.attach(*lbl,1,2,pos,pos+1);
	} else {
    lbl_size.set_label(_("Size:"));
    tbl_gen.attach(lbl_size,0,1,pos,pos+1);
    lbl = Gtk::manage(new Gtk::Label(Glib::ustring(node_info->size)+" bytes ("+
                                     node_info->size_abbr+")",0.,0.5));
    tbl_gen.attach(*lbl,1,2,pos,pos+1);
    ++pos;
    tbl_gen.attach(lbl_alloc,0,1,pos,pos+1);
    lbl = Gtk::manage(new Gtk::Label(node_info->size_alloc,0.,0.5));
    tbl_gen.attach(*lbl,1,2,pos,pos+1);
	}
	++pos;
  tbl_gen.attach(lbl_owner,0,1,pos,pos+1);
  std::stringstream lbow;
  lbow<<node_info->user_name<<" (uid "<<NODE_DESC(node)->user_id<<")";
  lbl = Gtk::manage(new Gtk::Label(lbow.str(),0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
  tbl_gen.attach(lbl_group,0,1,pos,pos+1);
  std::stringstream lbgr;
  lbgr<<node_info->group_name<<" (gid "<<NODE_DESC(node)->group_id<<")";
  lbl = Gtk::manage(new Gtk::Label(lbgr.str(),0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
  
  tbl_gen.attach(lbl_modif,0,1,pos,pos+1);
  lbl = Gtk::manage(new Gtk::Label(node_info->mtime,0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
  tbl_gen.attach(lbl_attrib,0,1,pos,pos+1);
  lbl = Gtk::manage(new Gtk::Label(node_info->ctime,0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
  tbl_gen.attach(lbl_access,0,1,pos,pos+1);
  lbl = Gtk::manage(new Gtk::Label(node_info->atime,0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CANCEL);
  show_all_children();
}

