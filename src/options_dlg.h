#include <gtkmm.h>
#include "common.h"
#include "color.h"

/*class TimeEntry : public Gtk::Entry{
  public:
  NumEntry(){}
  NumEntry(GtkEntry* cobject,const Glib::RefPtr<Gnome::Glade::Xml>& refGlade) : Gtk::Entry(cobject){}
  void set_value(double);
  void set_value(int);
  double get_value()const;
  int get_value_as_int()const;
  protected: 
  virtual bool on_key_press_event(GdkEventKey* event);
};
bool NumEntry::on_key_press_event(GdkEventKey* event){
  if(event->string && Glib::Unicode::isprint(*event->string)){
    if(!Glib::Unicode::isdigit(*event->string)){
      char a = Glib::Unicode::toupper(*event->string);
      if(!(a=='E' || a=='+' || a=='-' || a=='.'))return false;
      else std::cerr<<"Sym:"<<a<<std::endl;
    }
  }
  return Gtk::Entry::on_key_press_event(event);
}
void NumEntry::set_value(double val){
  //char str[64];
  //sprintf(str,"%g",val);
  set_text( Glib::Ascii::dtostr(val));
}
void NumEntry::set_value(int val){
  char str[64];
  sprintf(str,"%d",val);
  set_text(str);
}
double NumEntry::get_value()const{
  return atof(get_text().c_str());
}
int NumEntry::get_value_as_int()const{
  return atoi(get_text().c_str());
}

class CalendarEntry : public Gtk::HBox{
  Gtk::Label lab;
  Gtk::Entry ent;
  Gtk::Button but;
  Gtk::Window win;
  Gtk::Calendar cal;
};*/

class OptionsDialog : public Gtk::Dialog {
  struct ColorConfig ccfg;
  Gtk::Notebook ntb;
  Gtk::Table tbl_file_type,tbl_timestamp,tbl_wildcard;
  Gtk::ColorButton btn[8];
  void on_file_type_color(int i);
  Gtk::HBox bxs[8];
  Gtk::Label lbls[8];
  
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
  void on_response(int response_id);
public:
  OptionsDialog();
};

