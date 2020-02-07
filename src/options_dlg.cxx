#include "options_dlg.h"
#include <iostream>
#include <sstream>

OptionsDialog::WildTree::WildTree(OptionsDialog* dlg):colcolor(_("Color"),colrend)
                                   ,colpattern(_("Patterns"),records.name){
  model = Gtk::TreeStore::create(records);
  set_model(model);
  append_column(colcolor);
  colcolor.add_attribute(colrend.property_text(),records.color);
  colrend.signal_edited().connect(
    sigc::mem_fun(*dlg,&OptionsDialog::on_color_edited));
  Gtk::CellRendererText* rwp = dynamic_cast<Gtk::CellRendererText*>(colpattern.get_first_cell_renderer());
  if(rwp){
    rwp->property_editable() = true;
    rwp->signal_edited().connect(
      sigc::mem_fun(*dlg,&OptionsDialog::on_wp_edited));
  }
  //colcolor.add_attribute(colrend.property_visible(),records.visible);
  append_column(colpattern);
}

void OptionsDialog::WildTree::fill_tree(const ColorConfig& cfg){
  model->clear();
  for(std::vector<WPatternGroup>::const_iterator wpgit = 
    cfg.by_wpattern.wpgroup_list.begin();
    wpgit < cfg.by_wpattern.wpgroup_list.end(); ++wpgit){
	  Gtk::TreeIter treegr = model->append();
	  Gtk::TreeModel::Row rowgr = *(treegr);
    std::stringstream str;
    str<<(int)(wpgit->color.r*255.)<<" "<<(int)(wpgit->color.g*255.)<<" "<<(int)(wpgit->color.b*255.);
    rowgr[records.color] = str.str();
    str.str("");
    for(std::vector<std::string>::const_iterator it = wpgit->wp_list.begin();
        it < wpgit->wp_list.end();++it){
	      /*Gtk::TreeIter treewc = model->append(treegr->children());
	      Gtk::TreeModel::Row rowwc = *(treewc);
        rowwc[records.name] = *it;*/
        if(it!=wpgit->wp_list.begin())str<<";";
        str<<*it;
//        rowgr[records.visible] = false;
		}
    rowgr[records.name] = str.str();
  }
}
void OptionsDialog::on_color_edited(const Glib::ustring& path,const Glib::ustring& newval){
  Gtk::TreeIter it = tree_wild.model->get_iter(path);
  std::stringstream str(newval);
  RGBcolor color;
  str>>color.r;
  str>>color.g;
  str>>color.b;
  color.r/=255.;
  color.g/=255.;
  color.b/=255.;
  ccfg.by_wpattern.wpgroup_list[Gtk::TreePath(it).front()].color = color;
  (*it)[tree_wild.records.color] = newval;
}
void OptionsDialog::on_wp_edited(const Glib::ustring& path,const Glib::ustring& newval){
  Gtk::TreeIter it = tree_wild.model->get_iter(path);
  std::stringstream str(newval);
  std::vector<std::string> patterns;
  while(!str.eof() && ! str.fail()){
    std::string tstr;
    std::getline(str,tstr,';');
    std::stringstream ts(tstr);ts>>tstr;
    if(!tstr.empty())patterns.push_back(tstr);
  }
  ccfg.by_wpattern.wpgroup_list[Gtk::TreePath(it).front()].wp_list = patterns;
  (*it)[tree_wild.records.name] = newval;
}

void OptionsDialog::on_bt_add(){
  ccfg.by_wpattern.wpgroup_list.resize(ccfg.by_wpattern.wpgroup_list.size()+1);
  tree_wild.fill_tree(ccfg);
}
void OptionsDialog::on_bt_remove(){
  Glib::RefPtr<Gtk::TreeSelection> sel=tree_wild.get_selection();
  Glib::RefPtr<Gtk::TreeModel> md = Glib::RefPtr<Gtk::TreeModel>::cast_static(tree_wild.model);
  Gtk::TreeIter it = sel->get_selected(md);
  if(it){
    Gtk::TreePath path(it);
    ccfg.by_wpattern.wpgroup_list.erase(ccfg.by_wpattern.wpgroup_list.begin()+path.front());
    tree_wild.fill_tree(ccfg);
  }
}

OptionsDialog::OptionsDialog() : Gtk::Dialog(_("Color Setup"),true),tbl_file_type(4,4),
  tbl_timestamp(5,4),tbl_wildcard(2,7),
  lbl_oldest(_("Oldest:")),lbl_newest(_("Newest:")),
  lbl_colorby(_("Color by:")),lbl_older(_("Older")),lbl_newer(_("Newer")),
  bt_add(_("Add")),bt_remove(_("Remove")),tree_wild(this){
  get_vbox()->add(ntb);
  ntb.append_page(tbl_file_type,_("By nodetype"));
  tbl_file_type.set_spacings(5);
  tbl_file_type.set_border_width(10);
  for(int i=0;i<4;++i){
    tbl_file_type.attach(btn[i],  0,1,i,i+1);
    tbl_file_type.attach(bxs[i],  1,2,i,i+1);
    tbl_file_type.attach(btn[i+4],2,3,i,i+1);
    tbl_file_type.attach(bxs[i+4],3,4,i,i+1);
  }
  color_get_config(&ccfg);

  for(int i=0;i<8;++i){
    btn[i].set_title(_(node_type_names[i+1]));
    Gdk::Color rgb;
    rgb.set_rgb_p(ccfg.by_nodetype.colors[i+1].r,ccfg.by_nodetype.colors[i+1].g,ccfg.by_nodetype.colors[i+1].b);
    btn[i].set_color(rgb);
    btn[i].signal_color_set().connect(
                      sigc::bind(sigc::mem_fun(*this,&OptionsDialog::on_file_type_color),i) );
    lbls[i].set_text(_(node_type_names[i+1]));
    bxs[i].pack_end(lbls[i]);
  }
  ntb.append_page(tbl_timestamp,_("By timestamp"));
  tbl_timestamp.set_spacings(5);
  tbl_timestamp.set_border_width(10);
  
  tbl_timestamp.attach(lbl_oldest  ,0,1,0,1);
  tbl_timestamp.attach(ent_old_date,2,3,0,1);
  ent_old_date.set_max_length(10);
  ent_old_date.set_width_chars(10);
  ent_old_date.signal_changed().connect(sigc::mem_fun(*this,&OptionsDialog::on_old_editing_done));
  tbl_timestamp.attach(ent_old_time,4,5,0,1);
  ent_old_time.set_max_length(8);
  ent_old_time.set_width_chars(8);
  ent_old_time.signal_changed().connect(sigc::mem_fun(*this,&OptionsDialog::on_old_editing_done));
  set_old_time(ccfg.by_timestamp.old_time);
  

  tbl_timestamp.attach(lbl_newest  ,0,1,1,2);
  tbl_timestamp.attach(ent_new_date,2,3,1,2);
  ent_new_date.set_max_length(10);
  ent_new_date.set_width_chars(10);
  ent_new_date.signal_changed().connect(sigc::mem_fun(*this,&OptionsDialog::on_new_editing_done));
  tbl_timestamp.attach(ent_new_time,4,5,1,2);
  ent_new_time.set_max_length(8);
  ent_new_time.set_width_chars(8);
  ent_new_time.signal_changed().connect(sigc::mem_fun(*this,&OptionsDialog::on_new_editing_done));
  set_new_time(ccfg.by_timestamp.new_time);
  
  tbl_timestamp.attach(lbl_colorby,0,1,      2,3);
  tbl_timestamp.attach(cbx_timestamp_type,1,5,2,3);
  
  cbx_timestamp_type.append_text(_("time of last access"));
  cbx_timestamp_type.append_text(_("time of last modification"));
  cbx_timestamp_type.append_text(_("time of last attribute change"));
  cbx_timestamp_type.set_active((int)ccfg.by_timestamp.timestamp_type);
  cbx_timestamp_type.signal_changed().connect(sigc::mem_fun(*this,&OptionsDialog::on_timestamp_type_changed));
  
  tbl_timestamp.attach(btn_older,0,1,        3,4);
  tbl_timestamp.attach(lbl_older,1,2,        3,4);
  tbl_timestamp.attach(cbx_spectrum_type,2,3,3,4);
  tbl_timestamp.attach(lbl_newer,3,4,        3,4);
  tbl_timestamp.attach(btn_newer,4,5,        3,4);
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
  tbl_wildcard.attach(scr_wild,0,2,0,1);
  scr_wild.add(tree_wild);
  tree_wild.fill_tree(ccfg);
  
  tbl_wildcard.attach(bt_add,0,1,1,2,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK);
  tbl_wildcard.attach(bt_remove,1,2,1,2,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK);

  bt_add.signal_clicked().connect(sigc::mem_fun(*this,&OptionsDialog::on_bt_add));
  bt_remove.signal_clicked().connect(sigc::mem_fun(*this,&OptionsDialog::on_bt_remove));
    
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
  //add_button(Gtk::Stock::APPLY,Gtk::RESPONSE_NONE);
  show_all_children();
}

void OptionsDialog::set_old_time(time_t tmv){
  Glib::TimeVal tv(tmv,0);
  Glib::ustring utime(tv.as_iso8601());
  ent_old_date.set_text(utime.substr(0,10));
  ent_old_time.set_text(utime.substr(11,19));
}
void OptionsDialog::set_new_time(time_t tmv){
  Glib::TimeVal tv(tmv,0);
  Glib::ustring utime(tv.as_iso8601());
  ent_new_date.set_text(utime.substr(0,10));
  ent_new_time.set_text(utime.substr(11,19));
}

void OptionsDialog::on_old_editing_done(){
  std::stringstream str;
  str<<ent_old_date.get_text()<<"T"<<ent_old_time.get_text()<<"Z";
  Glib::TimeVal tv;
  if(tv.assign_from_iso8601(str.str())){
    ccfg.by_timestamp.old_time = (time_t)tv.as_double();
  }else
    set_old_time(ccfg.by_timestamp.old_time);
}
void OptionsDialog::on_new_editing_done(){
  std::stringstream str;
  str<<ent_new_date.get_text()<<"T"<<ent_new_time.get_text()<<"Z";
  Glib::TimeVal tv;
  if(tv.assign_from_iso8601(str.str())){
    ccfg.by_timestamp.new_time = (time_t)tv.as_double();
  }else
    set_new_time(ccfg.by_timestamp.new_time);
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

void OptionsDialog::on_apply(){
  color_set_config(&ccfg,(ColorMode)ntb.get_current_page());
}

void OptionsDialog::on_response(int response_id){
  if(response_id == Gtk::RESPONSE_OK){
    color_set_config(&ccfg,(ColorMode)ntb.get_current_page());
    return;
  }
  Gtk::Dialog::on_response(response_id);
}

