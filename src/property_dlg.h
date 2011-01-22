#ifndef _property_dlg_h_
#define _property_dlg_h_

#include <gtkmm.h>
#include "common.h"

class PropertyDialog : public Gtk::Dialog{
  GNode *node;
  Gtk::Notebook nbk;
  Gtk::Table tbl_gen;
  Gtk::Label lbl_type,lbl_loc,lbl_size,lbl_alloc,
  lbl_owner,lbl_group,
  lbl_modif,lbl_attrib,lbl_access;
  Gtk::Image ico;
public:
  PropertyDialog(GNode *nd );
};


#endif //_property_dlg_h_

