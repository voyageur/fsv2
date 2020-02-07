#include "ColorCellEditable.h"

#include <gtkmm.h>

#include <sstream>

#include <iostream>

ColorCellEditable::ColorCellEditable(const Glib::ustring& path) :
	Glib::ObjectBase( typeid(ColorCellEditable) ),
	Gtk::EventBox(),
	Gtk::CellEditable(),
	path_( path ),
	color_area_ptr_( 0 ),
	entry_ptr_( 0 ),
	button_ptr_( 0 ),
	editing_cancelled_( false )
{
	Gtk::HBox *const hbox = new Gtk::HBox(false, 0);
	add (*Gtk::manage( hbox ));

	color_area_ptr_ = new ColorArea();
	// TODO: expose color_area size for get_size_vfunc
	color_area_ptr_->set_size_request (16, 16);
	hbox->pack_start (*Gtk::manage( color_area_ptr_ ), Gtk::PACK_SHRINK, 2);
  
	entry_ptr_ = new Gtk::Entry();
	hbox->pack_start (*Gtk::manage(entry_ptr_), Gtk::PACK_EXPAND_WIDGET);
	entry_ptr_->set_has_frame (false);
	entry_ptr_->gobj()->is_cell_renderer = true; // XXX
  
	button_ptr_ = new Gtk::Button();
	hbox->pack_start (*Gtk::manage( button_ptr_ ), Gtk::PACK_SHRINK);
	button_ptr_->add (*Gtk::manage(new Gtk::Arrow(Gtk::ARROW_DOWN, Gtk::SHADOW_OUT)));
  
	set_flags(Gtk::CAN_FOCUS);
  
	show_all_children();
}

ColorCellEditable::~ColorCellEditable()
{
}

Glib::ustring ColorCellEditable::get_path() const
{
	return path_;
}

void ColorCellEditable::set_text(const Glib::ustring& text)
{
	int r = 0;
	int g = 0;
	int b = 0;

	entry_ptr_->set_text (text);
  
	std::stringstream ss;
	ss << text;
	ss >> r;
	ss >> g;
	ss >> b;
  
	color_.set_rgb_p (r/255.0, g/255.0, b/255.0);
	
	color_area_ptr_->set_color (color_);
}

Glib::ustring ColorCellEditable::get_text() const
{
	std::stringstream ss;
	ss << int(color_.get_red_p() * 255) << " ";
	ss << int(color_.get_green_p() * 255) << " "; 
	ss << int(color_.get_blue_p() * 255);
	
	return ss.str();
}

int ColorCellEditable::get_button_width()
{
	Gtk::Window window (Gtk::WINDOW_POPUP);

	Gtk::Button *const button = new Gtk::Button();
	window.add(*Gtk::manage(button));

	button->add(*Gtk::manage(new Gtk::Arrow(Gtk::ARROW_DOWN, Gtk::SHADOW_OUT)));

	// Urgh.  Hackish :/
	window.move(-500, -500);
	window.show_all();

	Gtk::Requisition requisition = window.size_request();

	return requisition.width;
}

/* static */ int ColorCellEditable::get_color_area_width()
{
	return 16;
}

void ColorCellEditable::start_editing_vfunc(GdkEvent*)
{
	entry_ptr_->select_region(0, -1);
	entry_ptr_->signal_activate().connect(sigc::mem_fun(*this, &ColorCellEditable::on_entry_activate));
	entry_ptr_->signal_key_press_event().connect(sigc::mem_fun(*this, &ColorCellEditable::on_entry_key_press_event));
	
	button_ptr_->signal_clicked().connect (sigc::mem_fun( *this, &ColorCellEditable::on_button_clicked ));
}

void ColorCellEditable::on_editing_done()
{
	std::cout << "on_editing_done " << editing_cancelled_ << std::endl; 
	
	if (!editing_cancelled_)
	{
		int r = 0;
		int g = 0;
		int b = 0;
	
		std::stringstream ss;
		ss << entry_ptr_->get_text();
		ss >> r;
		ss >> g;
		ss >> b;
	  
		color_.set_rgb_p (r/255.0, g/255.0, b/255.0);
	}
	
	signal_editing_done_.emit();
}

void ColorCellEditable::on_button_clicked()
{
	Gtk::ColorSelectionDialog dialog( "Changing color" );

	dialog.set_transient_for ((Gtk::Window&)(*this->get_toplevel()));

	Gtk::ColorSelection* colorsel = dialog.get_colorsel();
	colorsel->set_previous_color (color_);
	colorsel->set_current_color (color_);
	colorsel->set_has_palette (true);

	if (dialog.run() == Gtk::RESPONSE_OK)
	{
		color_ = colorsel->get_current_color();
		
		signal_editing_done_.emit();
	}
	
}

bool ColorCellEditable::on_entry_key_press_event(GdkEventKey* event)
{
	if (event->keyval == GDK_Escape)
	{
		std::cout << "Press ESCAPE" << std::endl;
		
		editing_cancelled_ = true;
		
	    editing_done();
	    remove_widget();
		return true;
	}
	
	return false;
}

void ColorCellEditable::on_entry_activate()
{
	editing_done();
}
