/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 1999 Dave Cole
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

#ifndef __colors_h
#define __colors_h

#include <gdk/gdk.h>

extern GdkColor black;
extern GdkColor white;
extern GdkColor red;
extern GdkColor green;
extern GdkColor blue;
extern GdkColor lightblue;

extern GdkColor ck_die_red;
extern GdkColor ck_die_yellow;

void colors_init(void);

GdkColor *colors_get_player(gint player_num);

#endif
