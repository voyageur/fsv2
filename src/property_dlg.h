#ifndef _property_dlg_h_
#define _property_dlg_h_

#include <gtkmm.h>
#include "common.h"

class PropertyDialog : public Gtk::Dialog{
  GNode *node;
  Gtk::Notebook nbk;
  Gtk::Table tbl_gen;
public:
  PropertyDialog(GNode *nd );
};


#endif //_property_dlg_h_

