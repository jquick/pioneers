/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 1999 Dave Cole
 * Copyright (C) 2005 Brian Wellington <bwelling@xbill.org>
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

#include <gdk/gdk.h>

#include "colors.h"
#include "game.h"

GdkColor black = { 0, 0, 0, 0 };
GdkColor white = { 0, 0xff00, 0xff00, 0xff00 };
GdkColor red = { 0, 0xff00, 0, 0 };
GdkColor green = { 0, 0, 0xff00, 0 };
GdkColor blue = { 0, 0, 0, 0xff00 };
GdkColor lightblue = { 0, 0xbe00, 0xbe00, 0xff00 };

GdkColor ck_die_red = { 0, 0x8800, 0x0200, 0x0200 };
GdkColor ck_die_yellow = { 0, 0xab00, 0xbd00, 0x1300 };

static GdkColor token_colors[MAX_PLAYERS] = {
	{0, 0xCD00, 0x0000, 0x0000},	/* red */
	{0, 0x1E00, 0x9000, 0xFF00},	/* blue */
	{0, 0xE800, 0xE800, 0xE800},	/* white */
	{0, 0xFF00, 0x7F00, 0x0000},	/* orange */
	{0, 0xEE00, 0xEE00, 0x0000},	/* yellow */
	{0, 0x8E00, 0xE500, 0xEE00},	/* cyan */
	{0, 0xD100, 0x5F00, 0xEE00},	/* magenta */
	{0, 0x0000, 0xEE00, 0x7600}	/* green */
};

void colors_init(void)
{
	GdkColormap *cmap;
	gint idx;

	cmap = gdk_colormap_get_system();
	for (idx = 0; idx < G_N_ELEMENTS(token_colors); idx++) {
		/* allocate colours for the players */
		gdk_colormap_alloc_color(cmap, &token_colors[idx], FALSE,
					 TRUE);
	}

	gdk_colormap_alloc_color(cmap, &black, FALSE, TRUE);
	gdk_colormap_alloc_color(cmap, &white, FALSE, TRUE);
	gdk_colormap_alloc_color(cmap, &red, FALSE, TRUE);
	gdk_colormap_alloc_color(cmap, &green, FALSE, TRUE);
	gdk_colormap_alloc_color(cmap, &blue, FALSE, TRUE);
	gdk_colormap_alloc_color(cmap, &lightblue, FALSE, TRUE);

	gdk_colormap_alloc_color(cmap, &ck_die_red, FALSE, TRUE);
	gdk_colormap_alloc_color(cmap, &ck_die_yellow, FALSE, TRUE);
}

GdkColor *colors_get_player(gint player_num)
{
	g_assert(player_num >= 0);
	g_assert(player_num < MAX_PLAYERS);
	return &token_colors[player_num];
}
