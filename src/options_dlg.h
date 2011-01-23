#include <gtkmm.h>
#include "common.h"
#include "color.h"

class OptionsDialog : public Gtk::Dialog {
  struct ColorConfig ccfg;
  Gtk::Notebook ntb;
  Gtk::Table tbl_file_type,tbl_timestamp,tbl_wildcard;
  Gtk::ColorButton btn[8];
  void on_file_type_color(int i);
  Gtk::HBox bxs[8];
  Gtk::Label lbls[8];
  
  Gtk::Label lbl_oldest,lbl_newest;
  Gtk::Entry ent_new_date,ent_old_date;
  Gtk::Entry ent_new_time,ent_old_time;
  //Gtk::Button
  Gtk::ComboBoxText cbx_timestamp_type;
  Gtk::ColorButton btn_older,btn_newer;
  Gtk::Label lbl_colorby,lbl_older,lbl_newer;
  Gtk::ComboBoxText cbx_spectrum_type;
  void on_timestamp_type_changed();
  void on_spectrum_type_changed();
  void on_timestamp_color(int i);
protected:
  void set_old_time(time_t);
  void set_new_time(time_t);
  void on_old_editing_done();
  void on_new_editing_done();
  void on_apply();
  void on_response(int response_id);
public:
  OptionsDialog();
};

