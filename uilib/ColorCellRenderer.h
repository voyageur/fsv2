#ifndef COLORCELLRENDERER2_H_
#define COLORCELLRENDERER2_H_

#include <gdkmm.h>
#include <gtkmm/cellrenderer.h>

#include "ColorCellEditable.h"

class ColorCellRenderer : public Gtk::CellRenderer
{
public:
	ColorCellRenderer();
	virtual ~ColorCellRenderer();

	// Properties
	Glib::PropertyProxy< Glib::ustring > property_text();
	Glib::PropertyProxy< bool > property_editable();
	
	// Edited signal
	typedef sigc::signal<void, const Glib::ustring&, const Glib::ustring& > signal_edited_t;
	signal_edited_t& signal_edited() { return signal_edited_; };
	
protected:
	Glib::Property< Glib::ustring > property_text_;
	Glib::Property< bool > property_editable_;
	signal_edited_t signal_edited_;
	ColorCellEditable* color_cell_edit_ptr_;
	mutable int button_width_; //mutable because it is just a cache for get_size_vfunc

	// Raise the edited event
	void edited(const Glib::ustring& path, const Glib::ustring& new_text);
	
	/* override */virtual void get_size_vfunc (Gtk::Widget& widget, const Gdk::Rectangle* cell_area, int* x_offset, int* y_offset, int* width, int* height) const;
	/* override */virtual void render_vfunc (const Glib::RefPtr<Gdk::Drawable>& window, Gtk::Widget& widget, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, const Gdk::Rectangle& expose_area, Gtk::CellRendererState flags);
	/* override */virtual bool activate_vfunc (GdkEvent*, Gtk::Widget&, const Glib::ustring& path, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, Gtk::CellRendererState flags);
	/* override */virtual Gtk::CellEditable* start_editing_vfunc(GdkEvent* event, Gtk::Widget& widget, const Glib::ustring& path, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, Gtk::CellRendererState flags);
	
	// Manage editing_done event for color_cell_edit_ptr_
	void on_editing_done();
};

#endif /*COLORCELLRENDErer_H_*/
