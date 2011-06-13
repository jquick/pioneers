/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 2007 Giancarlo Capella <giancarlo@comm.cc>
 * Copyright (C) 2007 Roland Clobus <rclobus@bigfoot.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "colors.h"
#include "player-icon.h"
#include "game.h"
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

static gboolean load_pixbuf(const gchar * name, GdkPixbuf ** pixbuf);
static void replace_colors(GdkPixbuf * pixbuf,
			   const GdkColor * replace_this,
			   const GdkColor * replace_with);

GdkColor default_face_color = { 0, 0xd500, 0x7f00, 0x2000 };
GdkColor default_variant_color = { 0, 0, 0, 0 };

typedef struct PlayerAvatar {
	GdkPixbuf *base;
	GSList *variation;
} PlayerAvatar;

PlayerAvatar player_avatar;
GdkPixbuf *ai_avatar;

static gboolean load_pixbuf(const gchar * name, GdkPixbuf ** pixbuf)
{
	gchar *filename;

	/* determine full path to pixmap file */
	filename =
	    g_build_filename(DATADIR, "pixmaps", "pioneers", name, NULL);
	if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
		GError *error = NULL;
		*pixbuf = gdk_pixbuf_new_from_file(filename, &error);
		if (error != NULL) {
			g_warning("Error loading pixmap %s\n", filename);
			g_error_free(error);
			return FALSE;
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

void playericon_init(void)
{
	GdkColormap *cmap;
	gint idx;
	gboolean good;

	cmap = gdk_colormap_get_system();
	gdk_colormap_alloc_color(cmap, &default_face_color, FALSE, TRUE);
	gdk_colormap_alloc_color(cmap, &default_variant_color, FALSE,
				 TRUE);

	load_pixbuf("style-human.png", &player_avatar.base);
	player_avatar.variation = NULL;
	idx = 1;
	do {
		gchar *name;
		GdkPixbuf *pixbuf;

		name = g_strdup_printf("style-human-%d.png", idx);
		good = load_pixbuf(name, &pixbuf);
		if (good) {
			player_avatar.variation =
			    g_slist_append(player_avatar.variation,
					   pixbuf);
		}
		++idx;
		g_free(name);
	} while (good);

	load_pixbuf("style-ai.png", &ai_avatar);
}

static void replace_colors(GdkPixbuf * pixbuf,
			   const GdkColor * replace_this,
			   const GdkColor * replace_with)
{
	gint i;
	guint new_color;
	guint old_color;
	guint *pixel;

	g_assert(gdk_pixbuf_get_colorspace(pixbuf) == GDK_COLORSPACE_RGB);
	g_assert(gdk_pixbuf_get_bits_per_sample(pixbuf) == 8);
	g_assert(gdk_pixbuf_get_has_alpha(pixbuf));
	g_assert(gdk_pixbuf_get_n_channels(pixbuf) == 4);

	pixel = (guint *) gdk_pixbuf_get_pixels(pixbuf);
	new_color = 0xff000000 |
	    ((replace_with->red >> 8) & 0xff) |
	    ((replace_with->green >> 8) & 0xff) << 8 |
	    ((replace_with->blue >> 8) & 0xff) << 16;
	old_color = 0xff000000 |
	    ((replace_this->red >> 8) & 0xff) |
	    ((replace_this->green >> 8) & 0xff) << 8 |
	    ((replace_this->blue >> 8) & 0xff) << 16;
	for (i = 0;
	     i <
	     (gdk_pixbuf_get_rowstride(pixbuf) / 4 *
	      gdk_pixbuf_get_height(pixbuf)); i++) {
		if (*pixel == old_color)
			*pixel = new_color;
		pixel++;
	}
}

GdkPixbuf *playericon_create_icon(GtkWidget * widget, const gchar * style,
				  GdkColor * color, gboolean viewer,
				  gboolean connected, gboolean double_size)
{
	gint width, height;
	GdkPixbuf *basic_image, *overlay_image, *scaled_image;
	gboolean basic_substitute;

	gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height);
	if (double_size) {
		width *= 2;
		height *= 2;
	}
	basic_image = NULL;
	overlay_image = NULL;
	switch (determine_player_type(style)) {
	case PLAYER_COMPUTER:
		/* This is an AI */
		basic_image = gdk_pixbuf_copy(ai_avatar);
		basic_substitute = TRUE;
		/** @todo RC 2007-07-17 For now, the AI does not have different appearances */
		break;
	case PLAYER_HUMAN:{
			/* Draw a bust */
			gint variant;
			GdkColor face_color, variant_color;

			playericon_parse_human_style(style, &face_color,
						     &variant,
						     &variant_color);
			basic_image = gdk_pixbuf_copy(player_avatar.base);
			basic_substitute = TRUE;

			/* Substitute the pure red face with the face color */
			replace_colors(basic_image, &red, &face_color);

			/* Substitute the pure blue base with the variant color */
			overlay_image =
			    gdk_pixbuf_copy(g_slist_nth
					    (player_avatar.variation,
					     variant)->data);
			replace_colors(overlay_image, &blue,
				       &variant_color);
			break;
		}
	default:{
			/* Unknown or square */
			GdkGC *gc;
			GdkPixmap *pixmap;

			pixmap =
			    gdk_pixmap_new(widget->window, width, height,
					   gdk_visual_get_system()->depth);
			gc = gdk_gc_new(pixmap);
			if (viewer) {
				GdkPixbuf *tmp;
				/* Viewers have a transparent icon */
				gdk_gc_set_foreground(gc, &black);
				gdk_draw_rectangle(pixmap, gc, TRUE, 0, 0,
						   width, height);

				tmp =
				    gdk_pixbuf_get_from_drawable(NULL,
								 pixmap,
								 NULL, 0,
								 0, 0, 0,
								 -1, -1);
				basic_image =
				    gdk_pixbuf_add_alpha(tmp, TRUE, 0, 0,
							 0);
			} else {
				gdk_gc_set_foreground(gc, &black);
				gdk_draw_rectangle(pixmap, gc, TRUE, 0, 0,
						   width, height);

				gdk_gc_set_foreground(gc, color);
				gdk_draw_rectangle(pixmap, gc, TRUE, 1, 1,
						   width - 2, height - 2);

				if (!connected) {
					gdk_gc_set_foreground(gc, &black);
					gdk_draw_rectangle(pixmap, gc,
							   FALSE, 3, 3,
							   width - 7,
							   height - 7);
					gdk_draw_rectangle(pixmap, gc,
							   FALSE, 6, 6,
							   width - 13,
							   height - 13);
					/* Don't draw the other emblem */
					connected = TRUE;
				}

				basic_image =
				    gdk_pixbuf_get_from_drawable(NULL,
								 pixmap,
								 NULL, 0,
								 0, 0, 0,
								 -1, -1);
			}
			basic_substitute = FALSE;
			g_object_unref(gc);
			g_object_unref(pixmap);
		}
	}

	if (basic_image && basic_substitute) {
		/* Substitute the pure blue base with the player color */
		replace_colors(basic_image, &blue, color);
	}

	if (overlay_image) {
		gdk_pixbuf_composite(overlay_image, basic_image, 0, 0,
				     gdk_pixbuf_get_width(basic_image),
				     gdk_pixbuf_get_height(basic_image),
				     0.0, 0.0, 1.0, 1.0,
				     GDK_INTERP_NEAREST, 255);
		g_object_unref(overlay_image);
	}

	scaled_image =
	    gdk_pixbuf_scale_simple(basic_image, width, height,
				    GDK_INTERP_BILINEAR);
	g_object_unref(basic_image);

	if (!connected) {
		GdkGC *gc;
		GdkPixmap *pixmap;
		GdkPixbuf *tmp1, *tmp2;

		pixmap =
		    gdk_pixmap_new(widget->window, width, height,
				   gdk_visual_get_system()->depth);
		gc = gdk_gc_new(pixmap);

		gdk_gc_set_foreground(gc, &black);	/* Black will become transparent */
		gdk_draw_rectangle(pixmap, gc, TRUE, 0, 0, width, height);

		gdk_gc_set_foreground(gc, &red);
		gdk_draw_arc(pixmap, gc, TRUE, width / 2, 0, width / 2,
			     height / 2, 0, 360 * 64);
		gdk_gc_set_foreground(gc, &white);
		gdk_draw_rectangle(pixmap, gc, TRUE,
				   width / 2 + 2, height / 4 - 1,
				   width / 2 - 4, height / 8);

		tmp1 =
		    gdk_pixbuf_get_from_drawable(NULL, pixmap, NULL, 0, 0,
						 0, 0, -1, -1);
		tmp2 = gdk_pixbuf_add_alpha(tmp1, TRUE, 0, 0, 0);
		gdk_pixbuf_composite(tmp2, scaled_image, 0, 0, width,
				     height, 0.0, 0.0, 1.0, 1.0,
				     GDK_INTERP_NEAREST, 255);
		g_object_unref(tmp2);
		g_object_unref(tmp1);
		g_object_unref(gc);
		g_object_unref(pixmap);
	}
	return scaled_image;
}

gchar *playericon_create_human_style(const GdkColor * face_color,
				     gint variant,
				     const GdkColor * variant_color)
{
	gchar *c1;
	gchar *c2;
	gchar *style;

	c1 = color_to_string(*face_color);
	c2 = color_to_string(*variant_color);

	style = g_strdup_printf("human %s %d %s", c1, variant, c2);

	g_free(c1);
	g_free(c2);
	return style;
}

gboolean playericon_parse_human_style(const gchar * style,
				      GdkColor * face_color,
				      gint * variant,
				      GdkColor * variant_color)
{
	gchar **style_parts;
	gboolean parse_ok;

	/* Determine the style for the player/viewer */
	style_parts = g_strsplit(style, " ", 0);
	parse_ok = FALSE;
	if (!strcmp(style_parts[0], "human")) {
		parse_ok = style_parts[1] != NULL
		    && string_to_color(style_parts[1], face_color);
		if (parse_ok) {
			parse_ok = style_parts[2] != NULL;
		}
		if (parse_ok) {
			*variant = atoi(style_parts[2]);
			parse_ok =
			    *variant <=
			    g_slist_length(player_avatar.variation);
		}
		if (parse_ok) {
			parse_ok = style_parts[3] != NULL
			    && string_to_color(style_parts[3],
					       variant_color);
		}
	}

	if (!parse_ok) {
		/* Something was wrong, revert to the default */
		*face_color = default_face_color;
		*variant = 0;
		*variant_color = default_variant_color;
	}
	g_strfreev(style_parts);
	return parse_ok;
}


gboolean string_to_color(const gchar * spec, GdkColor * color)
{
	PangoColor pango_color;
	GdkColormap *cmap;

	if (pango_color_parse(&pango_color, spec)) {
		color->red = pango_color.red;
		color->green = pango_color.green;
		color->blue = pango_color.blue;

		cmap = gdk_colormap_get_system();
		gdk_colormap_alloc_color(cmap, color, FALSE, TRUE);
		return TRUE;
	}
	return FALSE;
}

gchar *color_to_string(GdkColor color)
{
	return g_strdup_printf("#%04x%04x%04x", color.red, color.green,
			       color.blue);
/** @todo RC Enable this code when the minimum Gtk+ version is high enough
	PangoColor pango_color;

	pango_color.red = color.red;
	pango_color.green = color.green;
	pango_color.blue = color.blue;

	return pango_color_to_string(&pango_color);
*/
}
