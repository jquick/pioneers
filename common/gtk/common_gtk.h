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

#ifndef __common_gtk_h
#define __common_gtk_h

#include "driver.h"
#include "log.h"
#include <gtk/gtk.h>

/* Set the default logging function to write to the message window. */
void log_set_func_message_window(void);

/* Set if colors in message window are enabled */
void log_set_func_message_color_enable(gboolean enable);

/* set the text widget. */
void message_window_set_text(GtkWidget * textWidget);

enum TFindResult {
	FIND_MATCH_EXACT,
	FIND_MATCH_INSERT_BEFORE,
	FIND_NO_MATCH
};

enum TFindResult find_integer_in_tree(GtkTreeModel * model,
				      GtkTreeIter * iter, gint column,
				      gint number);

/** Check whether the game can be won, and display a messagebox
 *  about the distribution of the points.
 * @param param The game
 * @param main_window The main window for the dialog
 */
void check_victory_points(GameParams * param, GtkWindow * main_window);

extern UIDriver GTK_Driver;

/** Prepare Gtk for close buttons on tabs.
 *  Needs to be called once
*/
void prepare_gtk_for_close_button_on_tab(void);

/** Create a label with a close button.
 * @param label_text Text for the label
 * @param tooltip_text Tooltip for the close button
 * @retval button The close button
 * @return Composite widget with label and close button
 */
GtkWidget *create_label_with_close_button(const gchar * label_text,
					  const gchar * tooltip_text,
					  GtkWidget ** button);

#endif				/* __common_gtk_h */
