/* tmaptext.c */

/* Texture-mapped text */

/* fsv - 3D File System Visualizer
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
#include "tmaptext.h"

#include <GL/gl.h>
#include <GL/glu.h> /* gluBuild2DMipmaps( ) */

#ifdef HAVE_GL_GLC_H
#include <GL/glc.h>
static GLint textFont=-1;
//static GLint ctx;
#endif
/* Bitmap font definition */
#define char_width 16
#define char_height 32
#include "xmaps/charset.xbm"


/* Text can be squeezed to at most half its normal width */
#define TEXT_MAX_SQUEEZE 2.0
/* Mipmaps make faraway text look nice */
#define TEXT_USE_MIPMAPS


/* Normal character aspect ratio */
static const double char_aspect_ratio = (double)char_width / (double)char_height;

/* Font texture object */
static GLuint text_tobj;


/* Simple XBM parser - bits to bytes. Caller assumes responsibility for
 * freeing the returned pixel buffer */
static byte *
xbm_pixels( const byte *xbm_bits, int pixel_count )
{
	int in_byte = 0;
	int bitmask = 1;
	int i;
	byte *pixels;

	pixels = NEW_ARRAY(byte, pixel_count);

	for (i = 0; i < pixel_count; i++) {
		/* Note: a 1 bit is black */
		if ((int)xbm_bits[in_byte] & bitmask)
			pixels[i] = 0;
		else
			pixels[i] = 255;

		if (bitmask & 128) {
			++in_byte;
			bitmask = 1;
		}
		else
			bitmask <<= 1;
	}

	return pixels;
}


/* Initializes texture-mapping state for drawing text */
void
text_init( void )
{
	float border_color[] = { 0.0, 0.0, 0.0, 1.0 };
	byte *charset_pixels;

	/* Set up text texture object */
	glGenTextures( 1, &text_tobj );
	glBindTexture( GL_TEXTURE_2D, text_tobj );

	/* Set up texture-mapping parameters */
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
#ifdef TEXT_USE_MIPMAPS
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR );
#else
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
#endif
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color );

	/* Load texture */
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	charset_pixels = xbm_pixels( charset_bits, charset_width * charset_height );
#ifdef TEXT_USE_MIPMAPS
	gluBuild2DMipmaps( GL_TEXTURE_2D, GL_INTENSITY4, charset_width, charset_height, GL_LUMINANCE, GL_UNSIGNED_BYTE, charset_pixels );
#else
	glTexImage2D( GL_TEXTURE_2D, 0, GL_INTENSITY4, charset_width, charset_height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, charset_pixels );*/
#endif
	xfree( charset_pixels );
#ifdef HAVE_GL_GLC_H
    GLint ctx = glcGenContext();
    glcContext(ctx);
    //glcStringType(GLC_UTF8_QSO);
//    glcAppendCatalog("/usr/lib/X11/fonts/Type1");

  /* Create a font "Sans Bold" */
  textFont = glcGenFontID();
  glcNewFontFromFamily(textFont, "Sans");
  glcFontFace(textFont, "Bold");
  glcFont(textFont);
#endif
}


/* Call before drawing text */
void
text_pre( void )
{
	glDisable( GL_LIGHTING );
	glDisable( GL_POLYGON_OFFSET_FILL );
	glEnable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, text_tobj );
}


/* Call after drawing text */
void
text_post( void )
{
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_POLYGON_OFFSET_FILL );
	glEnable( GL_LIGHTING );
}

#ifndef HAVE_GL_GLC_H
/* Given the length of a string, and the dimensions into which that string
 * has to be rendered, this returns the dimensions that should be used
 * for each character */
static int
get_char_dims(const char *text, const XYvec *max_dims, XYvec *cdims , double *scale,int rl)
{
        int len;
	double min_width, max_width;

	len = strlen( text );
	/* Maximum and minimum widths of the string if it were to occupy
	 * the full depth (y-dimension) available to it */
	max_width = (double)len * max_dims->y * char_aspect_ratio;
	min_width = max_width / TEXT_MAX_SQUEEZE;

	if (max_width > max_dims->x) {
		if (min_width > max_dims->x) {
			/* Text will span full avaiable width, squeezed
			 * horizontally as much as it can take */
			cdims->x = max_dims->x / (double)len;
			cdims->y = TEXT_MAX_SQUEEZE * cdims->x / char_aspect_ratio;
		}
		else {
			/* Text will occupy full available width and
			 * height, squeezed horizontally a bit */
			cdims->x = max_dims->x / (double)len;
			cdims->y = max_dims->y;
		}
	}
	else {
		/* Text will use full available height (characters
		 * will have their natural aspect ratio) */
		cdims->y = max_dims->y;
		cdims->x = cdims->y * char_aspect_ratio;
	}
	return len;
}


/* Returns the texture-space coordinates of the bottom-left and upper-right
 * corners of the specified character (glyph) */
static void
get_char_tex_coords( int c, XYvec *t_c0, XYvec *t_c1 )
{
	static const XYvec t_char_dims = {
		(double)char_width / (double)charset_width,
		(double)char_height / (double)charset_height
	};
	XYvec gpos;
	int g;

	/* Get position of lower-left corner of glyph
	 * (in bitmap coordinates, w/origin at top-left)
	 * Note: The following code is character-set-specific */
	g = c;
	if ((g < 32) || (g > 127))
		g = 63; /* question mark */
	gpos.x = (double)(((g - 32) & 31) * char_width);
	gpos.y = (double)(((g - 32) >> 5) * char_height);

	/* Texture coordinates */
	t_c0->x = gpos.x / (double)charset_width;
	t_c1->y = gpos.y / (double)charset_height;
	t_c1->x = t_c0->x + t_char_dims.x;
	t_c0->y = t_c1->y + t_char_dims.y;
}
#else
static int
get_char_dims(const char *text, const XYvec *max_dims, XYvec *cdims ,double *scale, int reallen){
    int len;
    GLint ctx = glcGenContext();
    glcContext(ctx);
    glcFont(textFont);
    glcStringType(GLC_UTF8_QSO);
    glcPushMatrixQSO();
    glcScale(max_dims->y,max_dims->y);
    if(g_utf8_validate(text,-1,NULL)){
	len = g_utf8_strlen(text,-1);
	GLfloat outvec[8];
	glcMeasureString(GL_FALSE,text);
	glcGetStringMetric(GLC_BOUNDS,outvec);
	cdims->x=outvec[2]-outvec[0];
	cdims->y=outvec[5]-outvec[1];
/*	if(strncmp(text,"dialog",6)==0){
	g_print("meas %d:%e %e %e %e \t%e %e %e %e \n%e %e - %e %e\n%s\n",len,outvec[0],outvec[1],outvec[2],outvec[3]
							  ,outvec[4],outvec[5],outvec[6],outvec[7],
							  max_dims->x,max_dims->y,
							  cdims->x,cdims->y,
							  text);
							  }*/
	if(cdims->x>max_dims->x){
	    *scale=max_dims->y/(cdims->x/max_dims->x);
	    cdims->x=max_dims->x;
	}else
    	    *scale=max_dims->y;
/*	if(strncmp(text,"dialog",6)==0){
	    g_printf("res: %e %e scl %e\n",cdims->x,cdims->y,*scale);
	}*/
    }else{
	g_print("vot :%s invalid\n",text);
    }
    glcPopMatrixQSO();
  return (reallen==0)?1:len;
}

#endif

/* Draws a straight line of text centered at the given position,
 * fitting within the dimensions specified */
void
text_draw_straight( const char *text, const XYZvec *text_pos, const XYvec *text_max_dims )
{
	XYvec cdims;
	XYvec t_c0, t_c1, c0, c1;
	int len, i;
	double scale=1.;
	len = get_char_dims( text, text_max_dims, &cdims ,&scale,0);
	/* Corners of first character */
	c0.x = text_pos->x - 0.5 * (double)len * cdims.x;
	c0.y = text_pos->y - 0.5 * cdims.y;
	c1.x = c0.x + cdims.x;
	c1.y = c0.y + cdims.y;

//#if 1
#ifndef HAVE_GL_GLC_H
	glBegin( GL_QUADS );
	for (i = 0; i < len; i++) {
		get_char_tex_coords( text[i], &t_c0, &t_c1 );

		/* Lower left */
		glTexCoord2d( t_c0.x, t_c0.y );
		glVertex3d( c0.x, c0.y, text_pos->z );
		/* Lower right */
		glTexCoord2d( t_c1.x, t_c0.y );
		glVertex3d( c1.x, c0.y, text_pos->z );
		/* Upper right */
		glTexCoord2d( t_c1.x, t_c1.y );
		glVertex3d( c1.x, c1.y, text_pos->z );
		/* Upper left */
		glTexCoord2d( t_c0.x, t_c1.y );
		glVertex3d( c0.x, c1.y, text_pos->z );

		c0.x = c1.x;
		c1.x += cdims.x;
	}
	glEnd( );
#else
    //glcScale(text_max_dims->y,text_max_dims->y);
    //glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glcPushMatrixQSO();
    //glcScale(text_max_dims->y,text_max_dims->y);
    glTranslated(c0.x,c0.y,text_pos->z);
    glScalef(scale,text_max_dims->y,text_max_dims->y);
    //glcRenderStyle(GLC_TRIANGLE);
    glcRenderStyle(GLC_TEXTURE);
    glcRenderString(text);
    glcPopMatrixQSO();
    glPopMatrix();
    //glEnable(GL_TEXTURE_2D);
#endif
}


/* Draws a straight line of text centered at the given position, rotated
 * to be tangent to a circle around the origin, and fitting within the
 * dimensions specified (which are also rotated) */
void
text_draw_straight_rotated( const char *text, const RTZvec *text_pos, const XYvec *text_max_dims )
{
	XYvec cdims;
	XYvec t_c0, t_c1, c0, c1;
	XYvec hdelta, vdelta;
	double sin_theta, cos_theta,scale;
	int len, i;

	len = get_char_dims( text, text_max_dims, &cdims ,&scale,0);

	sin_theta = sin( RAD(text_pos->theta) );
	cos_theta = cos( RAD(text_pos->theta) );

	/* Vector to move from one character to the next */
	hdelta.x = sin_theta * cdims.x;
	hdelta.y = - cos_theta * cdims.x;
	/* Vector to move from bottom of character to top */
	vdelta.x = cos_theta * cdims.y;
	vdelta.y = sin_theta * cdims.y;

	/* Corners of first character */
	c0.x = cos_theta * text_pos->r - 0.5 * ((double)len * hdelta.x + vdelta.x);
	c0.y = sin_theta * text_pos->r - 0.5 * ((double)len * hdelta.y + vdelta.y);
	c1.x = c0.x + hdelta.x + vdelta.x;
	c1.y = c0.y + hdelta.y + vdelta.y;

#ifndef HAVE_GL_GLC_H
	glBegin( GL_QUADS );
	for (i = 0; i < len; i++) {
		get_char_tex_coords( text[i], &t_c0, &t_c1 );

		/* Lower left */
		glTexCoord2d( t_c0.x, t_c0.y );
		glVertex3d( c0.x, c0.y, text_pos->z );
		/* Lower right */
		glTexCoord2d( t_c1.x, t_c0.y );
		glVertex3d( c0.x + hdelta.x, c0.y + hdelta.y, text_pos->z );
		/* Upper right */
		glTexCoord2d( t_c1.x, t_c1.y );
		glVertex3d( c1.x, c1.y, text_pos->z );
		/* Upper left */
		glTexCoord2d( t_c0.x, t_c1.y );
		glVertex3d( c1.x - hdelta.x, c1.y - hdelta.y, text_pos->z );

		c0.x += hdelta.x;
		c0.y += hdelta.y;
		c1.x += hdelta.x;
		c1.y += hdelta.y;
	}
	glEnd( );
#else
    //glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glcPushMatrixQSO();
    //glcScale(text_max_dims->y,text_max_dims->y);
    glTranslated(c0.x,c0.y,text_pos->z);
    glRotated(-90.+text_pos->theta,0.,0.,1.);
    glScalef(scale,text_max_dims->y,text_max_dims->y);
    //glcRenderStyle(GLC_TRIANGLE);
    glcRenderStyle(GLC_TEXTURE);
    glcRenderString(text);
    glcPopMatrixQSO();
    glPopMatrix();
    //glEnable(GL_TEXTURE_2D);
#endif
}


/* Draws a curved arc of text, occupying no more than the depth and arc
 * width specified. text_pos indicates outer edge (not center) of text */
void
text_draw_curved( const char *text, const RTZvec *text_pos, const RTvec *text_max_dims )
{
	XYvec straight_dims, cdims;
	XYvec char_pos, fwsl, bwsl;
	XYvec t_c0, t_c1;
	double char_arc_width, theta;
	double sin_theta, cos_theta;
	double text_r,scale;
	int len, i;

	/* Convert curved dimensions to straight equivalent */
	straight_dims.x = (PI / 180.0) * text_pos->r * text_max_dims->theta;
	straight_dims.y = text_max_dims->r;

	len = get_char_dims( text, &straight_dims, &cdims ,&scale,1);

	/* Radius of center of text line */
	text_r = text_pos->r - 0.5 * cdims.y;

	/* Arc width occupied by each character */
	char_arc_width = (180.0 / PI) * cdims.x / text_r;

#ifndef HAVE_GL_GLC_H
	theta = text_pos->theta + 0.5 * (double)(len - 1) * char_arc_width;
	glBegin( GL_QUADS );
	for (i = 0; i < len; i++) {
		sin_theta = sin( RAD(theta) );
		cos_theta = cos( RAD(theta) );

		/* Center of character and deltas from center to corners */
		char_pos.x = cos_theta * text_r;
		char_pos.y = sin_theta * text_r;
		/* "forward slash / backward slash" */
		fwsl.x = 0.5 * (cdims.y * cos_theta + cdims.x * sin_theta);
		fwsl.y = 0.5 * (cdims.y * sin_theta - cdims.x * cos_theta);
		bwsl.x = 0.5 * (- cdims.y * cos_theta + cdims.x * sin_theta);
		bwsl.y = 0.5 * (- cdims.y * sin_theta - cdims.x * cos_theta);

		get_char_tex_coords( text[i], &t_c0, &t_c1 );

		/* Lower left */
		glTexCoord2d( t_c0.x, t_c0.y );
		glVertex3d( char_pos.x - fwsl.x, char_pos.y - fwsl.y, text_pos->z );
		/* Lower right */
		glTexCoord2d( t_c1.x, t_c0.y );
		glVertex3d( char_pos.x + bwsl.x, char_pos.y + bwsl.y, text_pos->z );
		/* Upper right */
		glTexCoord2d( t_c1.x, t_c1.y );
		glVertex3d( char_pos.x + fwsl.x, char_pos.y + fwsl.y, text_pos->z );
		/* Upper left */
		glTexCoord2d( t_c0.x, t_c1.y );
		glVertex3d( char_pos.x - bwsl.x, char_pos.y - bwsl.y, text_pos->z );

		theta -= char_arc_width;
	}
	glEnd( );
#else
    theta = text_pos->theta + 0.5 * char_arc_width;
    sin_theta = sin( RAD(theta) );
    cos_theta = cos( RAD(theta) );

    glcPushMatrixQSO();
    //glcScale(straight_dims.y,straight_dims.y);
    char_pos.x = cos_theta * text_r;
    char_pos.y = sin_theta * text_r;
    glPushMatrix();
    glTranslated(char_pos.x,char_pos.y,text_pos->z);
    glRotated(-90.,0,0,1);
    glTranslated(0,-straight_dims.y/3.,0);
    glScalef(scale,straight_dims.y,straight_dims.y);
    glcRenderStyle(GLC_TEXTURE);
    glcRenderString(text);
    glcPopMatrixQSO();
    glPopMatrix();
#endif	
}


/* end tmaptext.c */
