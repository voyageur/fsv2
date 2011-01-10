#include "property_dlg.h"

PropertyDialog::PropertyDialog(GNode *nd ): Gtk::Dialog(_("Properties"),true),node(nd){
  get_vbox()->add(nbk);
  nbk.append_page(tbl_gen,_("General"));
  
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CANCEL);
  show_all_children();
}

