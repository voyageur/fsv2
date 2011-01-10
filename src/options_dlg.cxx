#include "options_dlg.h"
#include "common.h"
#include "color.h"

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
  struct ColorConfig ccfg;
  color_get_config(&ccfg);

  for(int i=0;i<8;++i){
    btn[i].set_title(node_type_names[i+1]);
    Gdk::Color rgb;
    rgb.set_rgb_p(ccfg.by_nodetype.colors[i+1].r,ccfg.by_nodetype.colors[i+1].g,ccfg.by_nodetype.colors[i+1].b);
    btn[i].set_color(rgb);
    lbls[i].set_text(node_type_names[i+1]);
//    bxs[i].pack_start(
    bxs[i].pack_end(lbls[i]);
  }
  ntb.append_page(tbl_timestamp,_("By timestamp"));
  tbl_timestamp.attach(lbl_colorby,0,1,      0,1);
  tbl_timestamp.attach(cbx_timestamp_type,1,5,0,1);
  
  cbx_timestamp_type.append_text(_("time of last access"));
  cbx_timestamp_type.append_text(_("time of last modification"));
  cbx_timestamp_type.append_text(_("time of last attribute change"));
  cbx_timestamp_type.set_active((int)ccfg.by_timestamp.timestamp_type);
  
  tbl_timestamp.attach(btn_older,0,1,        1,2);
  tbl_timestamp.attach(lbl_older,1,2,        1,2);
  tbl_timestamp.attach(cbx_spectrum_type,2,3,1,2);
  tbl_timestamp.attach(lbl_newer,3,4,        1,2);
  tbl_timestamp.attach(btn_newer,4,5,        1,2);
  cbx_spectrum_type.append_text(_("Rainbow"));
  cbx_spectrum_type.append_text(_("Heat"));
  cbx_spectrum_type.append_text(_("Gradient"));
  cbx_spectrum_type.set_active((int)ccfg.by_timestamp.spectrum_type);
  
  ntb.append_page(tbl_wildcard,_("By wildcard"));

  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CANCEL);
  show_all_children();
}

