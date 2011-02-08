#include "property_dlg.h"
#include "fsvwindow.h"

#include <iostream>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

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

void PropertyDialog::make_dircontent(GNode *dnode)
{
	PropColumns prop_cols;
  Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(prop_cols);
	dircontent = Gtk::manage( new Gtk::TreeView(model));

	g_assert( NODE_IS_DIR(dnode) );

	Gtk::TreeView::Column* pcol = Gtk::manage( new Gtk::TreeView::Column(_("Node type") ) );
	pcol->pack_start(prop_cols.icon,false);
	pcol->pack_start(prop_cols.name);
	dircontent->append_column(*pcol);
	pcol = Gtk::manage( new Gtk::TreeView::Column(_("Quantity"),prop_cols.count ) );
  dircontent->append_column(*pcol);

	for (int i = 1; i < NUM_NODE_TYPES; i++) {
	  Gtk::TreeRow row = *(model->append());
	  row[prop_cols.icon] = FsvWindow::node_type_mini_icons[i];
	  row[prop_cols.name] = _(node_type_plural_names[i]);
	  row[prop_cols.count] = DIR_NODE_DESC(dnode)->subtree.counts[i];
	}
}



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
  //const struct NodeInfo* node_info = get_node_info( node );
  Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(node_absname(node));
  Glib::RefPtr<Gio::FileInfo> file_info = file->query_info();
  if(NODE_DESC(node)->type==NODE_DIRECTORY){
    ico.set(Gtk::Stock::DIRECTORY,Gtk::IconSize(Gtk::ICON_SIZE_DIALOG) );
  } else if(NODE_DESC(node)->type==NODE_REGFILE){
    ico.set(file_info->get_icon (),Gtk::IconSize(Gtk::ICON_SIZE_DIALOG) );
    //ico.set(Gtk::Stock::FILE,Gtk::IconSize(Gtk::ICON_SIZE_DIALOG) );
  } /*else if(NODE_DESC(node)->type==NODE_SYMLINK){
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(node_absname(node));
    ico.set(file->query_info ()->get_icon (),Gtk::IconSize(Gtk::ICON_SIZE_DIALOG) );
    Gtk::IconInfo ico_pb = Gtk::IconTheme::get_default()->lookup_icon(
                      file->query_info ()->get_icon (),
                      Gtk::IconSize(Gtk::ICON_SIZE_DIALOG),Gtk::ICON_LOOKUP_USE_BUILTIN);
    Glib::RefPtr<Gdk::Pixbuf> pb;
    Glib::RefPtr<Gdk::Pixbuf> pix_em;
    Gtk::IconInfo ico_em = Gtk::IconTheme::get_default()->lookup_icon(
                      "emblem-symbolic-link",
                      1/4.,Gtk::ICON_LOOKUP_USE_BUILTIN);
    if(ico_pb){
      pb = ico_pb.load_icon();
      pix_em = ico_em.load_icon();
    std::cerr<<"lic"<<std::endl;
    if(pb){
      int width = pb->get_width();
    std::cerr<<"gw"<<std::endl;
      if(pix_em){
        int wem = pix_em->get_width();
        std::cerr<<"wd"<<width<<"wem"<<wem<<std::endl;
        pix_em->composite(pb,0,0,wem,wem,0.,0.,1.,1.,Gdk::INTERP_NEAREST,100);
        ico.set(pb);
      }
    }
    }
    //ico.set(Gtk::Stock::FILE,Gtk::IconSize(Gtk::ICON_SIZE_DIALOG) );
  } */else {
    //ico.set(FsvWindow::get_file_icon(node,Gtk::IconSize(Gtk::ICON_SIZE_DIALOG)));
    ico.set(Gdk::Pixbuf::create_from_xpm_data(node_type_xpms[NODE_DESC(node)->type]));
  }
  int pos=0; 
  tbl_gen.attach(ico,0,1,pos,pos+1);
  Gtk::Label* lbl = Gtk::manage(new Gtk::Label(file->get_basename(),0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
  tbl_gen.attach(lbl_type,0,1,pos,pos+1);
  lbl = Gtk::manage(new Gtk::Label(_(node_type_names[NODE_DESC(node)->type]),0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
  tbl_gen.attach(lbl_loc,0,1,pos,pos+1);
  std::string prefix;
  if(file->has_parent()){
    prefix = file->get_parent()->get_path();
  }
  lbl = Gtk::manage(new Gtk::Label(prefix,0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
	if (NODE_IS_DIR(node)){
    lbl_size.set_label(_("Total size:"));
    tbl_gen.attach(lbl_size,0,1,pos,pos+1);
    std::stringstream lbsz;
    lbsz<<DIR_NODE_DESC(node)->subtree.size<<" bytes";
    if (DIR_NODE_DESC(node)->subtree.size >= 1024) {
			lbsz<<" ("<<DIR_NODE_DESC(node)->subtree.size<<")";
		}
    lbl = Gtk::manage(new Gtk::Label(lbsz.str(),0.,0.5));
    tbl_gen.attach(*lbl,1,2,pos,pos+1);
	} else {
    lbl_size.set_label(_("Size:"));
    tbl_gen.attach(lbl_size,0,1,pos,pos+1);
    {std::stringstream str;
    str<<NODE_DESC(node)->size<<" bytes ("<<abbrev_size(NODE_DESC(node)->size);
    lbl = Gtk::manage(new Gtk::Label(str.str(),0.,0.5));}
    tbl_gen.attach(*lbl,1,2,pos,pos+1);
    ++pos;
    tbl_gen.attach(lbl_alloc,0,1,pos,pos+1);
    std::stringstream str;
    {str<<NODE_DESC(node)->size_alloc;
    lbl = Gtk::manage(new Gtk::Label(str.str(),0.,0.5));}
    tbl_gen.attach(*lbl,1,2,pos,pos+1);
	}
	++pos;
  tbl_gen.attach(lbl_owner,0,1,pos,pos+1);
  std::stringstream lbow;
  struct passwd* pw = getpwuid(NODE_DESC(node)->user_id);
  lbow<<((pw)?(pw->pw_name):_("Unknown"));
  lbow<<" (uid "<<NODE_DESC(node)->user_id<<")";
  lbl = Gtk::manage(new Gtk::Label(lbow.str(),0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
  tbl_gen.attach(lbl_group,0,1,pos,pos+1);
  std::stringstream lbgr;
  struct group* gr = getgrgid( NODE_DESC(node)->group_id );
  lbgr<<((gr)?(gr->gr_name):("Unknown"))<<" (gid "<<NODE_DESC(node)->group_id<<")";
  lbl = Gtk::manage(new Gtk::Label(lbgr.str(),0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
  
  tbl_gen.attach(lbl_modif,0,1,pos,pos+1);
  lbl = Gtk::manage(new Gtk::Label(file_info->modification_time().as_iso8601(),0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
  tbl_gen.attach(lbl_attrib,0,1,pos,pos+1);
  lbl = Gtk::manage(new Gtk::Label(Glib::TimeVal(NODE_DESC(node)->ctime,0).as_iso8601(),0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
  tbl_gen.attach(lbl_access,0,1,pos,pos+1);
  lbl = Gtk::manage(new Gtk::Label(Glib::TimeVal(NODE_DESC(node)->atime,0).as_iso8601(),0.,0.5));
  tbl_gen.attach(*lbl,1,2,pos,pos+1);
  ++pos;
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CANCEL);
  
	switch (NODE_DESC(node)->type) {
		case NODE_DIRECTORY:{
      nbk.append_page(tbl_additional,_("Contents"));
      tbl_additional.set_spacings(5);
      tbl_additional.set_border_width(10);
		  Gtk::Label *lb = Gtk::manage(new Gtk::Label(_("This directory contains:")));
		  
	    tbl_additional.attach(*lb,0,1,0,1);
      make_dircontent(node);
	    tbl_additional.attach(*dircontent,0,1,1,2);
		}
    break;
    case NODE_REGFILE:{
      nbk.append_page(tbl_additional,_("File type"));
      tbl_additional.set_spacings(5);
      tbl_additional.set_border_width(10);
		  Gtk::Label *lb = Gtk::manage(new Gtk::Label(_("This file is recognized as:")));
	    tbl_additional.attach(*lb,0,1,0,1);
      Gtk::ScrolledWindow *scr = Gtk::manage(new Gtk::ScrolledWindow());
      Gtk::TextView * ent = Gtk::manage(new Gtk::TextView());
      //ent->get_buffer()->set_text(node_info->file_type_desc);
      ent->get_buffer()->set_text(file_info->get_content_type()
#ifdef HAVE_FILE_COMMAND
                                  +'\n'+get_file_type_desc(node_absname(node)) 
#endif //HAVE_FILE_COMMAND
                                  );
      ent->set_size_request(200,-1);
      ent->set_wrap_mode(Gtk::WRAP_WORD);
      scr->add(*ent);
	    tbl_additional.attach(*scr,0,1,1,2);
    }
    break;
    case NODE_SYMLINK:{
      nbk.append_page(tbl_additional,_("Target"));
      tbl_additional.set_spacings(5);
      tbl_additional.set_border_width(10);
		  Gtk::Label *lb = Gtk::manage(new Gtk::Label(_("This symlink points to:")));
	    tbl_additional.attach(*lb,0,1,0,1);
      lb = Gtk::manage(new Gtk::Label(file_info->get_symlink_target() ) );
	    tbl_additional.attach(*lb,0,1,1,2);
    }
    break;
    default:
      break;
  }
  show_all_children();
}

