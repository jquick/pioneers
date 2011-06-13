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

#ifndef __playericon_h
#define __playericon_h

#include <gtk/gtk.h>

/** Initialize the player icons */
void playericon_init(void);

/** Create an icon for a player.
 *  The default size is suitable for a list view item.
 *  @param widget  The widget that will display the icon
 *  @param style   The style of the icon
 *  @param color   The base color of the player
 *  @param viewer  TRUE if a viewer icon is requested
 *  @param connected Is the player currently connected
 *  @param double_size Generate a double size pixmap
 *  @return THe icon for the player. Call g_object_unref() when not needed anymore.
 */
GdkPixbuf *playericon_create_icon(GtkWidget * widget, const gchar * style,
				  GdkColor * color, gboolean viewer,
				  gboolean connected,
				  gboolean double_size);

/** Create a style string for the player.
 *  @param face_color    The color of the face
 *  @param variant       The variant
 *  @param variant_color The color of the variant
 *  @return The style string. Call g_free() when not needed anymore.
 */
gchar *playericon_create_human_style(const GdkColor * face_color,
				     gint variant,
				     const GdkColor * variant_color);

/** Parse the style string in its components.
 *  @param  style         The style
 *  @retval face_color    The color of the face
 *  @retval variant       The variant
 *  @retval variant_color The color of the variant
 *  @return TRUE if the style could be parsed. When FALSE, the return values contain the default values
 */
gboolean playericon_parse_human_style(const gchar * style,
				      GdkColor * face_color,
				      gint * variant,
				      GdkColor * variant_color);

/** Convert a string to a color.
 *  The color is allocated in the system colormap.
 * @param spec   The name of the color
 * @retval color The color
 * @return TRUE if the conversion succeeded.
 */
gboolean string_to_color(const gchar * spec, GdkColor * color);

/** Convert a color to a string.
 *  After use, the string must be freed with g_free()
 *  @param color The color
 *  @return the string
 */
gchar *color_to_string(GdkColor color);

#endif
