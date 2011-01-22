#include "options_dlg.h"

OptionsDialog::OptionsDialog() : Gtk::Dialog(_("Color Setup"),true),tbl_file_type(4,4),
  tbl_timestamp(5,4),lbl_colorby(_("Color by:")),lbl_older(_("Older")),lbl_newer(_("Newer")){
  get_vbox()->add(ntb);
  ntb.append_page(tbl_file_type,_("By nodetype"));
  for(int i=0;i<4;++i){
    tbl_file_type.attach(btn[i],  0,1,i,i+1);
    tbl_file_type.attach(bxs[i],  1,2,i,i+1);
    tbl_file_type.attach(btn[i+4],2,3,i,i+1);
    tbl_file_type.attach(bxs[i+4],3,4,i,i+1);
  }
  color_get_config(&ccfg);

  for(int i=0;i<8;++i){
    btn[i].set_title(node_type_names[i+1]);
    Gdk::Color rgb;
    rgb.set_rgb_p(ccfg.by_nodetype.colors[i+1].r,ccfg.by_nodetype.colors[i+1].g,ccfg.by_nodetype.colors[i+1].b);
    btn[i].set_color(rgb);
    btn[i].signal_color_set().connect(
                      sigc::bind(sigc::mem_fun(*this,&OptionsDialog::on_file_type_color),i) );
    lbls[i].set_text(node_type_names[i+1]);
    bxs[i].pack_end(lbls[i]);
  }
  ntb.append_page(tbl_timestamp,_("By timestamp"));
  tbl_timestamp.attach(lbl_colorby,0,1,      0,1);
  tbl_timestamp.attach(cbx_timestamp_type,1,5,0,1);
  
  cbx_timestamp_type.append_text(_("time of last access"));
  cbx_timestamp_type.append_text(_("time of last modification"));
  cbx_timestamp_type.append_text(_("time of last attribute change"));
  cbx_timestamp_type.set_active((int)ccfg.by_timestamp.timestamp_type);
  cbx_timestamp_type.signal_changed().connect(sigc::mem_fun(*this,&OptionsDialog::on_timestamp_type_changed));
  
  tbl_timestamp.attach(btn_older,0,1,        1,2);
  tbl_timestamp.attach(lbl_older,1,2,        1,2);
  tbl_timestamp.attach(cbx_spectrum_type,2,3,1,2);
  tbl_timestamp.attach(lbl_newer,3,4,        1,2);
  tbl_timestamp.attach(btn_newer,4,5,        1,2);
  cbx_spectrum_type.append_text(_("Rainbow"));
  cbx_spectrum_type.append_text(_("Heat"));
  cbx_spectrum_type.append_text(_("Gradient"));
  cbx_spectrum_type.set_active((int)ccfg.by_timestamp.spectrum_type);
  cbx_spectrum_type.signal_changed().connect(sigc::mem_fun(*this,&OptionsDialog::on_spectrum_type_changed));
  btn_older.signal_color_set().connect(
                      sigc::bind(sigc::mem_fun(*this,&OptionsDialog::on_timestamp_color),0) );
  btn_newer.signal_color_set().connect(
                      sigc::bind(sigc::mem_fun(*this,&OptionsDialog::on_timestamp_color),1) );
  
  if(ccfg.by_timestamp.spectrum_type!=SPECTRUM_GRADIENT){
    btn_older.set_sensitive(false);
    btn_newer.set_sensitive(false);
  }
  ntb.append_page(tbl_wildcard,_("By wildcard"));

  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CANCEL);
  add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);
  show_all_children();
}

void OptionsDialog::on_file_type_color(int i){
  Gdk::Color rgb = btn[i].get_color();
  ccfg.by_nodetype.colors[i+1].r = rgb.get_red_p();
  ccfg.by_nodetype.colors[i+1].g = rgb.get_green_p();
  ccfg.by_nodetype.colors[i+1].b = rgb.get_blue_p();
}

void OptionsDialog::on_timestamp_type_changed(){
  ccfg.by_timestamp.timestamp_type = (TimeStampType)cbx_timestamp_type.get_active_row_number();
}

void OptionsDialog::on_spectrum_type_changed(){
  ccfg.by_timestamp.spectrum_type = (SpectrumType)cbx_spectrum_type.get_active_row_number();
  if(ccfg.by_timestamp.spectrum_type != SPECTRUM_GRADIENT){
    btn_older.set_sensitive(false);
    btn_newer.set_sensitive(false);
  }else{
    btn_older.set_sensitive(true);
    btn_newer.set_sensitive(true);
    Gdk::Color rgb;
    rgb.set_rgb_p(ccfg.by_timestamp.old_color.r,ccfg.by_timestamp.old_color.g,ccfg.by_timestamp.old_color.b);
    btn_older.set_color(rgb);
    rgb.set_rgb_p(ccfg.by_timestamp.new_color.r,ccfg.by_timestamp.new_color.g,ccfg.by_timestamp.new_color.b);
    btn_newer.set_color(rgb);
  }
}

void OptionsDialog::on_timestamp_color(int i){
  if(i==0){
    Gdk::Color rgb = btn_older.get_color();
    ccfg.by_timestamp.old_color.r = rgb.get_red_p();
    ccfg.by_timestamp.old_color.g = rgb.get_green_p();
    ccfg.by_timestamp.old_color.b = rgb.get_blue_p();
  }else{
    Gdk::Color rgb = btn_newer.get_color();
    ccfg.by_timestamp.new_color.r = rgb.get_red_p();
    ccfg.by_timestamp.new_color.g = rgb.get_green_p();
    ccfg.by_timestamp.new_color.b = rgb.get_blue_p();
  }
}

void OptionsDialog::on_response(int response_id){
  if(response_id == Gtk::RESPONSE_APPLY){
    color_set_config(&ccfg,(ColorMode)ntb.get_current_page());
    return;
  }
  Gtk::Dialog::on_response(response_id);
}

