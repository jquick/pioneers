/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 1999 Dave Cole
 * Copyright (C) 2003,2006 Bas Wijnen <shevek@fmf.nl>
 * Copyright (C) 2010 Roland Clobus <rclobus@rclobus.nl>
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
#include <stdlib.h>
#include "frontend.h"
#include "player-icon.h"
#include "config-gnome.h"
#include "client.h"

typedef struct {
	GtkWidget *dlg;
	GtkWidget *name_entry;
	GtkWidget *check_btn;
	GtkWidget *image;
	GtkWidget *style_hbox;
	GtkWidget *color_btn1;
	GtkWidget *color_btn2;
	GtkWidget *variant_btn;
	gchar *original_name;
	gchar *original_style;
	gchar *current_style;
	gulong name_change_cb_id;
} DialogData;

static DialogData name_dialog;

static GdkPixbuf *create_icon(GtkWidget * widget, const gchar * style);

static void name_change_name_cb(NotifyingString * name)
{
	gchar *nm = notifying_string_get(name);
	gtk_entry_set_text(GTK_ENTRY(name_dialog.name_entry), nm);
	g_free(nm);
}

static void change_name_cb(G_GNUC_UNUSED GtkDialog * dlg, int response_id,
			   gpointer user_data)
{
	DialogData *dialog = user_data;
	g_signal_handler_disconnect(requested_name,
				    dialog->name_change_cb_id);
	if (response_id == GTK_RESPONSE_OK) {
		const gchar *new_name =
		    gtk_entry_get_text(GTK_ENTRY(dialog->name_entry));
		const gchar *new_style = dialog->current_style;
		if (0 != strcmp(new_name, dialog->original_name)) {
			notifying_string_set(requested_name, new_name);
		}
		if (0 != strcmp(new_style, dialog->original_style)) {
			notifying_string_set(requested_style, new_style);
		}
	}
	g_free(dialog->original_name);
	g_free(dialog->original_style);
	g_free(dialog->current_style);
	gtk_widget_destroy(GTK_WIDGET(dialog->dlg));
}

static void change_style_cb(G_GNUC_UNUSED GtkWidget * widget,
			    gpointer user_data)
{
	GdkPixbuf *icon;
	DialogData *dialog = user_data;

	g_free(dialog->current_style);
	if (gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON(dialog->check_btn))) {
		GdkColor c1;
		GdkColor c2;
		gint variant;
		gtk_color_button_get_color(GTK_COLOR_BUTTON
					   (dialog->color_btn1), &c1);
		gtk_color_button_get_color(GTK_COLOR_BUTTON
					   (dialog->color_btn2), &c2);
		variant = (int)
		    gtk_range_get_value(GTK_RANGE(dialog->variant_btn)) -
		    1;
		dialog->current_style =
		    playericon_create_human_style(&c1, variant, &c2);
	} else {
		dialog->current_style = g_strdup(default_player_style);
	}
	icon = create_icon(dialog->image, dialog->current_style);
	gtk_image_set_from_pixbuf(GTK_IMAGE(dialog->image), icon);
	g_object_unref(icon);
}

static void activate_style_cb(GtkWidget * widget, gpointer user_data)
{
	DialogData *dialog = user_data;
	gtk_widget_set_sensitive(dialog->style_hbox,
				 gtk_toggle_button_get_active
				 (GTK_TOGGLE_BUTTON(dialog->check_btn)));
	change_style_cb(widget, user_data);
}

void name_create_dlg(void)
{
	GtkWidget *dlg_vbox;
	GtkWidget *hbox;
	GtkWidget *lbl;
	GdkPixbuf *icon;
	GdkColor face_color, variant_color;
	gint variant;
	gboolean parse_ok;

	if (name_dialog.dlg != NULL) {
		gtk_window_present(GTK_WINDOW(name_dialog.dlg));
		return;
	};

	name_dialog.dlg = gtk_dialog_new_with_buttons(
							     /* Dialog caption */
							     _(""
							       "Change Player Name"),
							     GTK_WINDOW
							     (app_window),
							     GTK_DIALOG_DESTROY_WITH_PARENT,
							     GTK_STOCK_CANCEL,
							     GTK_RESPONSE_CANCEL,
							     GTK_STOCK_OK,
							     GTK_RESPONSE_OK,
							     NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(name_dialog.dlg),
					GTK_RESPONSE_OK);
	g_signal_connect(G_OBJECT(name_dialog.dlg), "destroy",
			 G_CALLBACK(gtk_widget_destroyed),
			 &name_dialog.dlg);
	gtk_widget_realize(name_dialog.dlg);
	gdk_window_set_functions(name_dialog.dlg->window,
				 GDK_FUNC_MOVE | GDK_FUNC_CLOSE);

	dlg_vbox = GTK_DIALOG(name_dialog.dlg)->vbox;
	gtk_widget_show(dlg_vbox);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(dlg_vbox), hbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);

	/* Label */
	lbl = gtk_label_new(_("Player name:"));
	gtk_widget_show(lbl);
	gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, TRUE, 0);
	gtk_misc_set_alignment(GTK_MISC(lbl), 1, 0.5);

	name_dialog.name_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(name_dialog.name_entry),
				 MAX_NAME_LENGTH);
	gtk_widget_show(name_dialog.name_entry);
	gtk_box_pack_start(GTK_BOX(hbox), name_dialog.name_entry, TRUE,
			   TRUE, 0);
	gtk_entry_set_width_chars(GTK_ENTRY(name_dialog.name_entry),
				  MAX_NAME_LENGTH);
	name_dialog.original_name = notifying_string_get(requested_name);
	gtk_entry_set_text(GTK_ENTRY(name_dialog.name_entry),
			   name_dialog.original_name);

	gtk_entry_set_activates_default(GTK_ENTRY(name_dialog.name_entry),
					TRUE);
	name_dialog.name_change_cb_id =
	    g_signal_connect(requested_name, "changed",
			     G_CALLBACK(name_change_name_cb), NULL);

	name_dialog.original_style = notifying_string_get(requested_style);
	name_dialog.current_style = notifying_string_get(requested_style);
	parse_ok =
	    playericon_parse_human_style(name_dialog.current_style,
					 &face_color, &variant,
					 &variant_color);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(dlg_vbox), hbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);

	name_dialog.style_hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_set_sensitive(name_dialog.style_hbox, FALSE);

	name_dialog.check_btn = gtk_check_button_new();
	gtk_widget_show(name_dialog.check_btn);
	gtk_box_pack_start(GTK_BOX(hbox), name_dialog.check_btn, FALSE,
			   TRUE, 0);
	g_signal_connect(name_dialog.check_btn, "toggled",
			 G_CALLBACK(activate_style_cb), &name_dialog);

	icon = create_icon(hbox, name_dialog.original_style);
	name_dialog.image = gtk_image_new_from_pixbuf(icon);
	g_object_unref(icon);
	gtk_widget_show(name_dialog.image);
	gtk_box_pack_start(GTK_BOX(hbox), name_dialog.image, FALSE, TRUE,
			   0);

	gtk_widget_show(name_dialog.style_hbox);
	gtk_box_pack_start(GTK_BOX(hbox), name_dialog.style_hbox, TRUE,
			   TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER
				       (name_dialog.style_hbox), 5);

	/* Label: set player icon preferences */
	lbl = gtk_label_new(_("Face:"));
	gtk_widget_show(lbl);
	gtk_box_pack_start(GTK_BOX(name_dialog.style_hbox), lbl, FALSE,
			   TRUE, 0);
	gtk_misc_set_alignment(GTK_MISC(lbl), 1, 0.5);

	name_dialog.color_btn1 = gtk_color_button_new();
	gtk_color_button_set_color(GTK_COLOR_BUTTON
				   (name_dialog.color_btn1), &face_color);
	gtk_widget_show(name_dialog.color_btn1);
	gtk_box_pack_start(GTK_BOX(name_dialog.style_hbox),
			   name_dialog.color_btn1, FALSE, TRUE, 0);
	g_signal_connect(name_dialog.color_btn1, "color-set",
			 G_CALLBACK(change_style_cb), &name_dialog);

	/* Label: set player icon preferences */
	lbl = gtk_label_new(_("Variant:"));
	gtk_widget_show(lbl);
	gtk_box_pack_start(GTK_BOX(name_dialog.style_hbox), lbl, FALSE,
			   TRUE, 0);
	gtk_misc_set_alignment(GTK_MISC(lbl), 1, 0.5);

	name_dialog.color_btn2 = gtk_color_button_new();
	gtk_color_button_set_color(GTK_COLOR_BUTTON
				   (name_dialog.color_btn2),
				   &variant_color);
	gtk_widget_show(name_dialog.color_btn2);
	gtk_box_pack_start(GTK_BOX(name_dialog.style_hbox),
			   name_dialog.color_btn2, FALSE, TRUE, 0);
	g_signal_connect(name_dialog.color_btn2, "color-set",
			 G_CALLBACK(change_style_cb), &name_dialog);

	name_dialog.variant_btn = gtk_hscale_new_with_range(1.0, 7.0, 1.0);
	gtk_scale_set_digits(GTK_SCALE(name_dialog.variant_btn), 0);
	gtk_scale_set_value_pos(GTK_SCALE(name_dialog.variant_btn),
				GTK_POS_LEFT);
	gtk_range_set_value(GTK_RANGE(name_dialog.variant_btn),
			    variant + 1);
	gtk_widget_show(name_dialog.variant_btn);
	gtk_box_pack_start(GTK_BOX(name_dialog.style_hbox),
			   name_dialog.variant_btn, TRUE, TRUE, 0);
	g_signal_connect(name_dialog.variant_btn, "value-changed",
			 G_CALLBACK(change_style_cb), &name_dialog);
	/* destroy dialog when OK or Cancel button gets clicked */
	g_signal_connect(name_dialog.dlg, "response",
			 G_CALLBACK(change_name_cb), &name_dialog);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				     (name_dialog.check_btn), parse_ok);

	gtk_widget_show(name_dialog.dlg);
	gtk_widget_grab_focus(name_dialog.name_entry);
}

GdkPixbuf *create_icon(GtkWidget * widget, const gchar * style)
{
	return playericon_create_icon(widget, style,
				      player_or_viewer_color(my_player_num
							     ()),
				      my_player_viewer(), TRUE, TRUE);
}
