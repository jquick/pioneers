#include "config.h"
#include "game.h"
#include <gtk/gtk.h>
#include <string.h>
#include <glib.h>

#include "game-buildings.h"

static const gchar *building_names[NUM_BUILD_TYPES] = {
	NULL, N_("Road"), N_("Bridge"), N_("Ship"), N_("Settlement"),
	N_("City"), N_("City wall")
};

static void game_buildings_init(GameBuildings * gb);

/* Register the class */
GType game_buildings_get_type(void)
{
	static GType gb_type = 0;

	if (!gb_type) {
		static const GTypeInfo gb_info = {
			sizeof(GameBuildingsClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			NULL,	/* class init */
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof(GameBuildings),
			0,
			(GInstanceInitFunc) game_buildings_init,
			NULL
		};
		gb_type =
		    g_type_register_static(GTK_TYPE_TABLE, "GameBuildings",
					   &gb_info, 0);
	}
	return gb_type;
}

/* Build the composite widget */
static void game_buildings_init(GameBuildings * gb)
{
	GtkWidget *label;
	GtkWidget *spin;
	GtkObject *adjustment;
	gint row;

	gtk_table_resize(GTK_TABLE(gb), NUM_BUILD_TYPES - 1, 2);
	gtk_table_set_row_spacings(GTK_TABLE(gb), 3);
	gtk_table_set_col_spacings(GTK_TABLE(gb), 5);
	gtk_table_set_homogeneous(GTK_TABLE(gb), TRUE);

	for (row = 1; row < NUM_BUILD_TYPES; row++) {
		label = gtk_label_new(gettext(building_names[row]));
		gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
		gtk_table_attach_defaults(GTK_TABLE(gb), label,
					  0, 1, row - 1, row);

		adjustment = gtk_adjustment_new(0, 0, 100, 1, 5, 0);
		spin =
		    gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 1, 0);
		gtk_entry_set_alignment(GTK_ENTRY(spin), 1.0);
		gtk_table_attach_defaults(GTK_TABLE(gb), spin,
					  1, 2, row - 1, row);
		gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);
		gb->num_buildings[row] = GTK_SPIN_BUTTON(spin);
	}
}

/* Create a new instance of the widget */
GtkWidget *game_buildings_new(void)
{
	return GTK_WIDGET(g_object_new(game_buildings_get_type(), NULL));
}

void game_buildings_set_num_buildings(GameBuildings * gb, gint type,
				      gint num)
{
	gtk_spin_button_set_value(gb->num_buildings[type], num);
}

gint game_buildings_get_num_buildings(GameBuildings * gb, gint type)
{
	return gtk_spin_button_get_value(gb->num_buildings[type]);
}
