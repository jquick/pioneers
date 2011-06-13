/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 1999 Dave Cole
 * Copyright (C) 2003 Bas Wijnen <shevek@fmf.nl>
 * Copyright (C) 2005 Roland Clobus <rclobus@bigfoot.com>
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
#include "frontend.h"
#include "theme.h"
#include "cost.h"

/* The order of the terrain_names is EXTREMELY important!  The order
 * must match the enum Terrain.
 */
static const gchar *terrain_names[] = {
	N_("Hill"),
	N_("Field"),
	N_("Mountain"),
	N_("Pasture"),
	N_("Forest"),
	N_("Desert"),
	N_("Sea"),
	N_("Gold")
};

static GtkWidget *legend_dlg = NULL;
static gboolean legend_did_connect = FALSE;

static void legend_theme_changed(void);
static void legend_rules_changed(void);

static void add_legend_terrain(GtkWidget * table, guint row, guint col,
			       Terrain terrain, Resource resource)
{
	GtkWidget *area;
	GtkWidget *label;
	GdkPixmap *p;
	GdkBitmap *b;

	area = gtk_drawing_area_new();
	gtk_widget_show(area);
	gtk_table_attach(GTK_TABLE(table), area,
			 col, col + 1, row, row + 1,
			 (GtkAttachOptions) GTK_FILL,
			 (GtkAttachOptions) GTK_FILL, 0, 0);
	gtk_widget_set_size_request(area, 30, 34);
	g_signal_connect(G_OBJECT(area), "expose_event",
			 G_CALLBACK(expose_terrain_cb),
			 GINT_TO_POINTER(terrain));

	label = gtk_label_new(_(terrain_names[terrain]));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label,
			 col + 1, col + 2, row, row + 1,
			 (GtkAttachOptions) GTK_FILL,
			 (GtkAttachOptions) GTK_FILL, 0, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	if (resource < NO_RESOURCE) {
		gui_get_resource_pixmap(resource, &p, &b);
		label = gtk_image_new_from_pixmap(p, b);
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label,
				 col + 2, col + 3, row, row + 1,
				 (GtkAttachOptions) GTK_FILL,
				 (GtkAttachOptions) GTK_FILL, 0, 0);
	}

	if (resource != NO_RESOURCE) {
		label = gtk_label_new(resource_name(resource, TRUE));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label,
				 col + 3, col + 4, row, row + 1,
				 (GtkAttachOptions) GTK_FILL,
				 (GtkAttachOptions) GTK_FILL, 0, 0);
		gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	}
}

static void add_legend_cost(GtkWidget * table, guint row,
			    const gchar * iconname, const gchar * item,
			    const gint * cost)
{
	GtkWidget *label;
	GtkWidget *icon;

	icon = gtk_image_new_from_stock(iconname, GTK_ICON_SIZE_MENU);
	gtk_widget_show(icon);
	gtk_table_attach(GTK_TABLE(table), icon, 0, 1, row, row + 1,
			 GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new(item);
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, row, row + 1,
			 GTK_FILL, GTK_FILL, 0, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	label = gtk_image_new();
	resource_format_type_image(GTK_IMAGE(label), cost, -1);
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, row, row + 1,
			 GTK_FILL, GTK_FILL, 0, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
}

GtkWidget *legend_create_content(void)
{
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *table;
	GtkWidget *vsep;
	GtkWidget *alignment;
	guint num_rows;

	hbox = gtk_hbox_new(FALSE, 6);

	vbox = gtk_vbox_new(FALSE, 6);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	label = gtk_label_new(NULL);
	/* Label */
	gtk_label_set_markup(GTK_LABEL(label), _("<b>Terrain yield</b>"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);

	alignment = gtk_alignment_new(0.0, 0.0, 0.0, 0.0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 12, 0);
	gtk_widget_show(alignment);
	gtk_box_pack_start(GTK_BOX(vbox), alignment, FALSE, FALSE, 0);

	table = gtk_table_new(4, 9, FALSE);
	gtk_widget_show(table);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	gtk_table_set_row_spacings(GTK_TABLE(table), 3);
	gtk_table_set_col_spacings(GTK_TABLE(table), 6);

	add_legend_terrain(table, 0, 0, HILL_TERRAIN, BRICK_RESOURCE);
	add_legend_terrain(table, 1, 0, FIELD_TERRAIN, GRAIN_RESOURCE);
	add_legend_terrain(table, 2, 0, MOUNTAIN_TERRAIN, ORE_RESOURCE);
	add_legend_terrain(table, 3, 0, PASTURE_TERRAIN, WOOL_RESOURCE);

	vsep = gtk_vseparator_new();
	gtk_widget_show(vsep);
	gtk_table_attach(GTK_TABLE(table), vsep, 4, 5, 0, 4,
			 GTK_FILL, GTK_FILL, 0, 0);

	add_legend_terrain(table, 0, 5, FOREST_TERRAIN, LUMBER_RESOURCE);
	add_legend_terrain(table, 1, 5, GOLD_TERRAIN, GOLD_RESOURCE);
	add_legend_terrain(table, 2, 5, DESERT_TERRAIN, NO_RESOURCE);
	add_legend_terrain(table, 3, 5, SEA_TERRAIN, NO_RESOURCE);

	label = gtk_label_new(NULL);
	/* Label */
	gtk_label_set_markup(GTK_LABEL(label), _("<b>Building costs</b>"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);

	alignment = gtk_alignment_new(0.0, 0.0, 0.0, 0.0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 12, 0);
	gtk_widget_show(alignment);
	gtk_box_pack_start(GTK_BOX(vbox), alignment, FALSE, FALSE, 0);

	num_rows = 4;
	if (have_ships())
		num_rows++;
	if (have_bridges())
		num_rows++;
	if (have_city_walls())
		num_rows++;
	table = gtk_table_new(num_rows, 3, FALSE);
	gtk_widget_show(table);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	gtk_table_set_row_spacings(GTK_TABLE(table), 3);
	gtk_table_set_col_spacings(GTK_TABLE(table), 5);

	num_rows = 0;
	add_legend_cost(table, num_rows++, PIONEERS_PIXMAP_ROAD, _("Road"),
			cost_road());
	if (have_ships())
		add_legend_cost(table, num_rows++, PIONEERS_PIXMAP_SHIP,
				_("Ship"), cost_ship());
	if (have_bridges())
		add_legend_cost(table, num_rows++, PIONEERS_PIXMAP_BRIDGE,
				_("Bridge"), cost_bridge());
	add_legend_cost(table, num_rows++, PIONEERS_PIXMAP_SETTLEMENT,
			_("Settlement"), cost_settlement());
	add_legend_cost(table, num_rows++, PIONEERS_PIXMAP_CITY, _("City"),
			cost_upgrade_settlement());
	if (have_city_walls())
		add_legend_cost(table, num_rows++,
				PIONEERS_PIXMAP_CITY_WALL, _("City wall"),
				cost_city_wall());
	add_legend_cost(table, num_rows++, PIONEERS_PIXMAP_DEVELOP,
			_("Development card"), cost_development());

	gtk_widget_show(vbox);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);
	return hbox;
}

GtkWidget *legend_create_dlg(void)
{
	GtkWidget *dlg_vbox;
	GtkWidget *vbox;

	if (legend_dlg != NULL) {
		gtk_window_present(GTK_WINDOW(legend_dlg));
		return legend_dlg;
	}

	/* Dialog caption */
	legend_dlg = gtk_dialog_new_with_buttons(_("Legend"),
						 GTK_WINDOW(app_window),
						 GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_STOCK_CLOSE,
						 GTK_RESPONSE_CLOSE, NULL);
	g_signal_connect(G_OBJECT(legend_dlg), "destroy",
			 G_CALLBACK(gtk_widget_destroyed), &legend_dlg);

	dlg_vbox = GTK_DIALOG(legend_dlg)->vbox;
	gtk_widget_show(dlg_vbox);

	vbox = legend_create_content();
	gtk_box_pack_start(GTK_BOX(dlg_vbox), vbox, TRUE, TRUE, 0);

	gtk_widget_show(legend_dlg);

	if (!legend_did_connect) {
		theme_register_callback(G_CALLBACK(legend_theme_changed));
		gui_rules_register_callback(G_CALLBACK
					    (legend_rules_changed));
		legend_did_connect = TRUE;
	}

	/* destroy dialog when OK is clicked */
	g_signal_connect(legend_dlg, "response",
			 G_CALLBACK(gtk_widget_destroy), NULL);

	return legend_dlg;
}

static void legend_theme_changed(void)
{
	if (legend_dlg)
		gtk_widget_queue_draw(legend_dlg);
}

static void legend_rules_changed(void)
{
	if (legend_dlg) {
		GtkWidget *dlg_vbox = GTK_DIALOG(legend_dlg)->vbox;
		GtkWidget *vbox;
		GList *list =
		    gtk_container_get_children(GTK_CONTAINER(dlg_vbox));

		if (g_list_length(list) > 0)
			gtk_widget_destroy(GTK_WIDGET(list->data));

		vbox = legend_create_content();
		gtk_box_pack_start(GTK_BOX(dlg_vbox), vbox, TRUE, TRUE, 0);

		g_list_free(list);
	}
}
