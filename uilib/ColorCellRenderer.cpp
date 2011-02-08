#include "ColorCellRenderer.h"

#include <sstream>

#include <iostream>
#include <memory>

ColorCellRenderer::ColorCellRenderer() :
	Glib::ObjectBase( typeid(ColorCellRenderer) ),
	Gtk::CellRenderer(),
	property_text_( *this, "text", "" ),
	property_editable_( *this, "editable", true ),
	color_cell_edit_ptr_( 0 ),
	button_width_( -1 )
{
    property_mode() = Gtk::CELL_RENDERER_MODE_EDITABLE;
    property_xpad() = 2;
    property_ypad() = 2;
}

ColorCellRenderer::~ColorCellRenderer()
{
}

Glib::PropertyProxy< Glib::ustring > ColorCellRenderer::property_text()
{
	return property_text_.get_proxy();
}

Glib::PropertyProxy< bool > ColorCellRenderer::property_editable()
{
	return property_editable_.get_proxy();
}

/* override */void ColorCellRenderer::get_size_vfunc (Gtk::Widget& widget, const Gdk::Rectangle* cell_area, int* x_offset, int* y_offset, int* width, int* height) const
{
	// We cache this because it takes a really long time to get the width.
	if(button_width_ < 0)
		button_width_ = ColorCellEditable::get_button_width();
	
	// Compute text width
    Glib::RefPtr<Pango::Layout> layout_ptr = widget.create_pango_layout (property_text_);
    Pango::Rectangle rect = layout_ptr->get_pixel_logical_extents();
	
    const int calc_width  = property_xpad() * 4 + rect.get_width();
    const int calc_height = property_ypad() * 4 + rect.get_height();

    // Add button width and color area width
    if( width )
        *width = calc_width + button_width_ + ColorCellEditable::get_color_area_width();

    if( height )
        *height = calc_height;    
}

/* override */void ColorCellRenderer::render_vfunc (const Glib::RefPtr<Gdk::Drawable>& window, Gtk::Widget& widget, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, const Gdk::Rectangle& expose_area, Gtk::CellRendererState flags)
{
	// Get cell size
    int x_offset = 0, y_offset = 0, width = 0, height = 0;
    get_size (widget, cell_area, x_offset, y_offset, width, height);
    
    // Create the graphic context
    Glib::RefPtr< Gdk::GC > gc = Gdk::GC::create (window);
    
    // Get cell state
    Gtk::StateType state;
    Gtk::StateType text_state;
    if ((flags & Gtk::CELL_RENDERER_SELECTED) != 0)
    {
        state = Gtk::STATE_SELECTED;
        text_state = (widget.has_focus()) ? Gtk::STATE_SELECTED : Gtk::STATE_ACTIVE;
    }
    else
    {
        state = Gtk::STATE_NORMAL;
        text_state = (widget.is_sensitive()) ? Gtk::STATE_NORMAL : Gtk::STATE_INSENSITIVE;
    }

    // Get cell color
	int r = 0;
	int g = 0;
	int b = 0;

	std::stringstream ss;
	ss << property_text_;
	ss >> r;
	ss >> g;
	ss >> b;
  
	Gdk::Color color_value;
	color_value.set_rgb_p (r/255.0, g/255.0, b/255.0);
	
	// Draw color area
    gc->set_rgb_fg_color( color_value );
    window->draw_rectangle(gc, 
    		true, 
    		cell_area.get_x(), 
    		cell_area.get_y(), 
    		ColorCellEditable::get_color_area_width(), 
    		cell_area.get_height());

    // Draw color text
    Glib::RefPtr< Gdk::Window > win = Glib::RefPtr<Gdk::Window>::cast_dynamic (window);    
    Glib::RefPtr<Pango::Layout> layout_ptr = widget.create_pango_layout( property_text_ );
    widget.get_style()->paint_layout (win,
          text_state,
          true,
          cell_area,
          widget,
          "cellrenderertext",
          cell_area.get_x() + ColorCellEditable::get_color_area_width() + x_offset + 2 * property_xpad(),
          cell_area.get_y() + y_offset + 2 * property_ypad(),
          layout_ptr);    
}

/* override */bool ColorCellRenderer::activate_vfunc (GdkEvent*, Gtk::Widget&, const Glib::ustring& path, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, Gtk::CellRendererState flags)
{
	return true;
}

/* override */Gtk::CellEditable* ColorCellRenderer::start_editing_vfunc(GdkEvent* event, Gtk::Widget& widget, const Glib::ustring& path, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, Gtk::CellRendererState flags)
{
	// If the cell isn't editable we return 0.
#ifdef GLIBMM_PROPERTIES_ENABLED
  if (!property_editable())
    return 0;
#else
  if (!(g_object_get_data(G_OBJECT(gobj()), "editable")))
    return 0;
#endif
		  
	std::auto_ptr< ColorCellEditable > color_cell_edit_ptr( new ColorCellEditable( path ) );
	
	Glib::ustring text;
		
#ifdef GLIBMM_PROPERTIES_ENABLED
	text = property_text();
#else
	get_property("text", text);
#endif

	color_cell_edit_ptr->set_text (text);
	color_cell_edit_ptr->signal_editing_done().connect(sigc::mem_fun(*this, &ColorCellRenderer::on_editing_done));
	color_cell_edit_ptr->show();
	
	color_cell_edit_ptr_ = Gtk::manage( color_cell_edit_ptr.release() );
	return color_cell_edit_ptr_;
}

void ColorCellRenderer::edited(const Glib::ustring& path, const Glib::ustring& new_text)
{
	signal_edited_.emit (path, new_text);
}

void ColorCellRenderer::on_editing_done()
{
	if (color_cell_edit_ptr_->get_editing_cancelled())
	{
		std::cout << "ColorCellRenderer Editing cancelled" << std::endl;
		stop_editing (true);
	}
	else
	{
		edited (color_cell_edit_ptr_->get_path(), color_cell_edit_ptr_->get_text());
	}
}
