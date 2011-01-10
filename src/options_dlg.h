#include <gtkmm.h>

class OptionsDialog : public Gtk::Dialog {
  Gtk::Notebook ntb;
  Gtk::Table tbl_file_type,tbl_timestamp,tbl_wildcard;
  Gtk::ColorButton btn[8];
  Gtk::HBox bxs[8];
  Gtk::Label lbls[8];
  
  Gtk::ComboBoxText cbx_timestamp_type;
  Gtk::ColorButton btn_older,btn_newer;
  Gtk::Label lbl_colorby,lbl_older,lbl_newer;
  Gtk::ComboBoxText cbx_spectrum_type;
public:
  OptionsDialog();
};

