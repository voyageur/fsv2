/* color.cxx 
 * Node coloration
 * fsv - 3D File System Visualizer
 * Copyright (C)1999 Daniel Richard G. <skunk@mit.edu>
 * Copyright (C)2009-2011 Yury P. Fedorchenko <yuryfdr@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "common.h"
#include "color.h"

#include <fnmatch.h>
#include <time.h>

#include "animation.h" /* redraw( ) */
#include "geometry.h"
#include "fsvwindow.h"
#include <iostream>
#include <glibmm.h>


/* Some fnmatch headers don't define FNM_FILE_NAME */
/* (*cough*Solaris*cough*) */
#ifndef FNM_FILE_NAME
	#define FNM_FILE_NAME FNM_PATHNAME
#endif

/* Number of shades in a spectrum */
#define SPECTRUM_NUM_SHADES 1024

/* Default configuration */
static const ColorMode default_color_mode = COLOR_BY_NODETYPE;
static const char *default_nodetype_colors[NUM_NODE_TYPES] = {
	NULL,		/* Metanode (not used) */
	"#A0A0A0",	/* Directory */
	"#FFFFA0",	/* Regular file */
	"#FFFFFF",	/* Symlink */
	"#00FF00",	/* FIFO */
	"#FF8000",	/* Socket */
	"#00FFFF",	/* Character device */
	"#4CA0FF",	/* Block device */
	"#FF0000"	/* unknown */
};
static const int default_timestamp_spectrum_type = SPECTRUM_RAINBOW;
static const int default_timestamp_timestamp_type = TIMESTAMP_MODIFY;
static const int default_timestamp_period = 7 * 24 * 60 * 60; /* 1 week */
static const char default_timestamp_old_color[] = "#0000FF";
static const char default_timestamp_new_color[] = "#FF0000";
static const char default_wpattern_default_color[] = "#FFFFA0";

static const char *keys_nodetype_node_type[NUM_NODE_TYPES] = {
	NULL,
	"directory",
	"regularfile",
	"symlink",
	"pipe",
	"socket",
	"chardevice",
	"blockdevice",
	"unknown"
};
/* Color configuration */
static struct ColorConfig color_config;

/* Color assignment mode */
static ColorMode color_mode;

/* Colors for spectrum */
static RGBcolor spectrum_underflow_color;
static RGBcolor spectrum_colors[SPECTRUM_NUM_SHADES];
static RGBcolor spectrum_overflow_color;

ColorMode color_get_mode()
{
	return color_mode;
}


/* Returns (a copy of) the current color configuration.*/
void color_get_config( struct ColorConfig *ccfg )
{
	*ccfg = color_config;
}


/* Returns the appropriate color for the given node, as per its type */
static const RGBcolor *
node_type_color( GNode *node )
{
	return &color_config.by_nodetype.colors[NODE_DESC(node)->type];
}


/* Returns the appropriate color for the given node, as per its timestamp */
static const RGBcolor *
time_color( GNode *node )
{
	double x;
        time_t node_time;
	int i;

	/* Directory override */
	if (NODE_IS_DIR(node))
		return node_type_color( node );

	/* Choose appropriate timestamp */
	switch (color_config.by_timestamp.timestamp_type) {
		case TIMESTAMP_ACCESS:
		node_time = NODE_DESC(node)->atime;
		break;

		case TIMESTAMP_MODIFY:
		node_time = NODE_DESC(node)->mtime;
		break;

		case TIMESTAMP_ATTRIB:
		node_time = NODE_DESC(node)->ctime;
		break;

		SWITCH_FAIL
	}

	/* Temporal position value (0 = old, 1 = new) */
	x = difftime( node_time, color_config.by_timestamp.old_time ) / difftime( color_config.by_timestamp.new_time, color_config.by_timestamp.old_time );

	if (x < 0.0) {
		/* Node is off the spectrum (too old) */
		return &spectrum_underflow_color;
	}

	if (x > 1.0) {
		/* Node is off the spectrum (too new) */
		return &spectrum_overflow_color;
	}

	/* Return a color somewhere in the spectrum */
	i = (int)floor( x * (double)(SPECTRUM_NUM_SHADES - 1) );
	return &spectrum_colors[i];
}


/* Returns the appropriate color for the given node, as matched (or not
 * matched) to the current set of wildcard patterns */
static const RGBcolor *
wpattern_color( GNode *node )
{
	/* Directory override */
	if (NODE_IS_DIR(node))
		return node_type_color( node );

	const char* name = NODE_DESC(node)->name;

	/* Search for a match in the wildcard pattern groups */
  for(std::vector<WPatternGroup>::iterator wpgit = 
      color_config.by_wpattern.wpgroup_list.begin();
      wpgit < color_config.by_wpattern.wpgroup_list.end(); ++wpgit){
    for(std::vector<std::string>::iterator it = wpgit->wp_list.begin();
        it < wpgit->wp_list.end();++it){
			if (!fnmatch( (*it).c_str(), name, FNM_FILE_NAME | FNM_PERIOD ))
				return &wpgit->color; /* A match! */
		}
  }
	/* No match */
	return &color_config.by_wpattern.default_color;
}


/* (Re)assigns colors to all nodes rooted at the given node */
void
color_assign_recursive( GNode *dnode )
{
  if(NULL == dnode)return;
	GNode *node;
	const RGBcolor *color;

	g_assert( NODE_IS_DIR(dnode) || NODE_IS_METANODE(dnode) );

	geometry_queue_rebuild( dnode );

	node = dnode->children;
	while (node != NULL) {
		switch (color_mode) {
			case COLOR_BY_NODETYPE:
			color = node_type_color( node );
			break;

			case COLOR_BY_TIMESTAMP:
			color = time_color( node );
			break;

			case COLOR_BY_WPATTERN:
			color = wpattern_color( node );
			break;

			SWITCH_FAIL
		}
                NODE_DESC(node)->color = color;

		if (NODE_IS_DIR(node))
			color_assign_recursive( node );

		node = node->next;
	}
}


/* Changes the current color mode */
void
color_set_mode( ColorMode mode )
{
	color_mode = mode;
	color_assign_recursive( globals.fstree );
  redraw( );
}


/* Returns a color in the given type of spectrum, at the given position
 * x = [0, 1]. If the spectrum is of a type which requires parameters,
 * those are passed in via the data argument */
RGBcolor
color_spectrum_color( SpectrumType type, double x, void *data )
{
	RGBcolor color;
	RGBcolor *zero_color, *one_color;

	g_assert( (x >= 0.0) && (x <= 1.0) );

	switch (type) {
		case SPECTRUM_RAINBOW:
		return rainbow_color( 1.0 - x );

		case SPECTRUM_HEAT:
		return heat_color( x );

		case SPECTRUM_GRADIENT:
		zero_color = ((RGBcolor **)data)[0];
		one_color = ((RGBcolor **)data)[1];
		color.r = zero_color->r + x * (one_color->r - zero_color->r);
		color.g = zero_color->g + x * (one_color->g - zero_color->g);
		color.b = zero_color->b + x * (one_color->b - zero_color->b);
		return color;

		SWITCH_FAIL
	}

	/* cc: duh... shouldn't there be a return value here? */
	color.r = -1.0; color.g = -1.0; color.b = -1.0; return color;
}


/* This sets up the spectrum color array */
static void
generate_spectrum_colors( void )
{
	RGBcolor *boundary_colors[2];
        double x;
	int i;
	void *data = NULL;

	if (color_config.by_timestamp.spectrum_type == SPECTRUM_GRADIENT) {
		boundary_colors[0] = &color_config.by_timestamp.old_color;
		boundary_colors[1] = &color_config.by_timestamp.new_color;
		data = boundary_colors;
	}

	for (i = 0; i < SPECTRUM_NUM_SHADES; i++) {
		x = (double)i / (double)(SPECTRUM_NUM_SHADES - 1);
		spectrum_colors[i] = color_spectrum_color( color_config.by_timestamp.spectrum_type, x, data ); /* struct assign */
	}

        /* Off-spectrum colors - make them dark */

	spectrum_underflow_color = spectrum_colors[0]; /* struct assign */
	spectrum_underflow_color.r *= 0.5;
	spectrum_underflow_color.g *= 0.5;
	spectrum_underflow_color.b *= 0.5;

	spectrum_overflow_color = spectrum_colors[(SPECTRUM_NUM_SHADES - 1)]; /* struct assign */
	spectrum_overflow_color.r *= 0.5;
	spectrum_overflow_color.g *= 0.5;
	spectrum_overflow_color.b *= 0.5;
}


/* Changes the current color configuration, and if mode is not COLOR_NONE,
 * sets the color mode as well */
void
color_set_config( struct ColorConfig *new_ccfg, ColorMode mode )
{
	color_config = *new_ccfg;

	generate_spectrum_colors( );

	if (globalsc.fsv_mode == FSV_SPLASH) {
		g_assert( mode != COLOR_NONE );
		color_mode = mode;
	}
	else if (mode != COLOR_NONE)
		color_set_mode( mode );
	else
		color_set_mode( color_mode );
  window_set_color_mode (color_mode);
  color_write_config();
}

void wpattern_reset(){
	// Wildcard pattern groups 
	std::vector<WPatternGroup>().swap(color_config.by_wpattern.wpgroup_list);
  WPatternGroup grp;
  grp.color = hex2rgb("#FF3333");
  grp.wp_list.push_back("*.arj");grp.wp_list.push_back("*.gz");grp.wp_list.push_back("*.lzh");grp.wp_list.push_back("*.bz2");
  color_config.by_wpattern.wpgroup_list.push_back(grp);
  grp.color = hex2rgb("#FF33FF");
  grp.wp_list.clear();
  grp.wp_list.push_back("*.gif");grp.wp_list.push_back("*.jpg");grp.wp_list.push_back("*.png");
  color_config.by_wpattern.wpgroup_list.push_back(grp);
  grp.color = hex2rgb("#FFFFFF");
  grp.wp_list.clear();
  grp.wp_list.push_back("*.mov");grp.wp_list.push_back("*.mpeg");grp.wp_list.push_back("*.avi");grp.wp_list.push_back("*.mkv");
  color_config.by_wpattern.wpgroup_list.push_back(grp);
  grp.color = hex2rgb("#FFFF00");
  grp.wp_list.clear();
  grp.wp_list.push_back("*.c");grp.wp_list.push_back("*.cxx");
  grp.wp_list.push_back("*.cpp");grp.wp_list.push_back("*.h");
  color_config.by_wpattern.wpgroup_list.push_back(grp);
	// Default color 
	color_config.by_wpattern.default_color = hex2rgb( default_wpattern_default_color ); // struct assign 
}
void color_reset()
{
	color_mode = (ColorMode)default_color_mode;

	// ColorByNodeType configuration 
	for (int i = 1; i < NUM_NODE_TYPES; i++) {
		color_config.by_nodetype.colors[i] = hex2rgb(default_nodetype_colors[i]); 
	}
	// ColorByTime configuration 
	color_config.by_timestamp.spectrum_type = (SpectrumType)default_timestamp_spectrum_type;
	color_config.by_timestamp.timestamp_type = (TimeStampType)default_timestamp_timestamp_type;
	color_config.by_timestamp.new_time = time( NULL );
	color_config.by_timestamp.old_time = color_config.by_timestamp.new_time - (time_t)default_timestamp_period;
	color_config.by_timestamp.old_color = hex2rgb( default_timestamp_old_color ); // struct assign 
	color_config.by_timestamp.new_color = hex2rgb( default_timestamp_new_color ); // struct assign 
  wpattern_reset();
}

#include <fstream>

static std::string get_value(const Glib::KeyFile& file,const Glib::ustring& grp,
                                                const Glib::ustring& key,
                                                const Glib::ustring& default_val)
{
  try{
    return file.get_value(grp,key);
  }catch(Glib::KeyFileError& err){
    return default_val;
  }                        
}
int get_integer(const Glib::KeyFile& file,const Glib::ustring& grp,
                                                const Glib::ustring& key,
                                                const int default_val)
{
  try{
    return file.get_integer(grp,key);
  }catch(Glib::KeyFileError& err){
    return default_val;
  }                        
}
/* Reads color configuration from file */
static void color_read_config( void )
{
  Glib::KeyFile colconf;
  Glib::RefPtr<Gio::File> file = Gio::File::create_for_parse_name(CONFIG_FILE);
  try{
    colconf.load_from_file(file->get_path());
  }catch(Glib::KeyFileError& err){
      std::cerr<<"kferr fail"<<err.what()<<std::endl;
    color_reset();
    return;
  }catch(Glib::FileError& err){
      std::cerr<<"ferr fail"<<err.what()<<std::endl;
    color_reset();
    return;
  }
  try{
    color_mode = (ColorMode)colconf.get_integer("General","color_mode");
  }catch(Glib::KeyFileError& err){
    color_mode = default_color_mode;
  }    
  for (int i = 1; i < NUM_NODE_TYPES; i++) {
      std::string str = get_value(colconf,"NodeType",keys_nodetype_node_type[i],default_nodetype_colors[i]);
      color_config.by_nodetype.colors[i] = hex2rgb( str.c_str() );  
  }

	color_config.by_timestamp.spectrum_type = 
    (SpectrumType)get_integer(colconf,"TimeStamp","type",default_timestamp_timestamp_type);
	int period = get_integer(colconf,"TimeStamp","period",default_timestamp_period);
                                                                      
	color_config.by_timestamp.new_time = time( NULL );
	color_config.by_timestamp.old_time = color_config.by_timestamp.new_time - (time_t)period;

  color_config.by_timestamp.old_color = 
    hex2rgb( get_value(colconf,"TimeStamp","old_color", default_timestamp_old_color).c_str() );
  color_config.by_timestamp.new_color = 
    hex2rgb( get_value(colconf,"TimeStamp","new_color", default_timestamp_new_color).c_str() );
  if(colconf.has_group("WildPattern")){
    try{
      Glib::ustring str = colconf.get_value("WildPattern","default_color");
      color_config.by_wpattern.default_color = hex2rgb(str.c_str());
    }catch(Glib::KeyFileError& err){
      wpattern_reset();
      std::cerr<<"wpattern fail"<<std::endl;
      return;
    }
	  std::vector<WPatternGroup>().swap(color_config.by_wpattern.wpgroup_list);
    int nwp(0);
    Glib::ustring str = Glib::ustring::compose("wpcolor%1",nwp);
    while(colconf.has_key("WildPattern",str)){
      WPatternGroup grp;
      str = colconf.get_value("WildPattern",str);
      grp.color = hex2rgb(str.c_str());
      str = Glib::ustring::compose("wp%1",nwp);
      std::stringstream wstr(get_value(colconf,"WildPattern",str,""));
      while(!wstr.eof() && ! wstr.fail()){
        std::string tstr;
        std::getline(wstr,tstr,';');
        std::stringstream ts(tstr);ts>>tstr;
        if(!tstr.empty())grp.wp_list.push_back(tstr);
      }
      color_config.by_wpattern.wpgroup_list.push_back(grp);
      str = Glib::ustring::compose("wpcolor%1",++nwp);
    }
  }else wpattern_reset();
}


/* Writes color configuration to file */
void color_write_config()
{
  Glib::KeyFile colconf;
  std::string full_path;
  colconf.set_integer("General","color_mode",color_mode);
  for (int i = 1; i < NUM_NODE_TYPES; i++) {
    colconf.set_value("NodeType",keys_nodetype_node_type[i],
                      rgb2hex(&color_config.by_nodetype.colors[i]));
  }
  colconf.set_integer("TimeStamp","type",color_config.by_timestamp.spectrum_type);
  colconf.set_integer("TimeStamp","period",color_config.by_timestamp.new_time-
                      color_config.by_timestamp.old_time);
                      
  colconf.set_value("TimeStamp","old_color",
                    rgb2hex(&color_config.by_timestamp.old_color));
  colconf.set_value("TimeStamp","new_color",
                    rgb2hex(&color_config.by_timestamp.new_color));

  colconf.set_value("WildPattern","default_color",
                    rgb2hex(&color_config.by_wpattern.default_color));

  int nwp(0);
  for(std::vector<WPatternGroup>::iterator 
      wgit = color_config.by_wpattern.wpgroup_list.begin();
      wgit < color_config.by_wpattern.wpgroup_list.end();++wgit,++nwp)
  {
    Glib::ustring str = Glib::ustring::compose("wpcolor%1",nwp);
    colconf.set_value("WildPattern",str,rgb2hex(&wgit->color));
    str = Glib::ustring::compose("wp%1",nwp);
    std::stringstream pstr;
    for(std::vector<std::string>::const_iterator it = wgit->wp_list.begin();
        it < wgit->wp_list.end();++it){
        if(it!=wgit->wp_list.begin())pstr<<";";
        pstr<<*it;
		}
    colconf.set_value("WildPattern",str,pstr.str());
  }
  Glib::RefPtr<Gio::File> of=Gio::File::create_for_parse_name(CONFIG_FILE);
  std::ofstream ofs(of->get_path().c_str());
  ofs<<colconf.to_data();
}


/* First-time initialization */
void
color_init( void )
{
	/* Read configuration file */
	color_read_config( );

	/* Update radio menu in window with configured color mode */
	window_set_color_mode( color_mode );

	/* Generate spectrum color table */
	generate_spectrum_colors( );
}
/* end color.c */
