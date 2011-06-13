#include "config.h"
#include "game.h"
#include <gtk/gtk.h>
#include <string.h>
#include <glib.h>

#include "game-resources.h"

static void game_resources_init(GameResources * gr);

/* Register the class */
GType game_resources_get_type(void)
{
	static GType gr_type = 0;

	if (!gr_type) {
		static const GTypeInfo gr_info = {
			sizeof(GameResourcesClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			NULL,	/* class init */
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof(GameResources),
			0,
			(GInstanceInitFunc) game_resources_init,
			NULL
		};
		gr_type =
		    g_type_register_static(GTK_TYPE_TABLE, "GameResources",
					   &gr_info, 0);
	}
	return gr_type;
}

/* Build the composite widget */
static void game_resources_init(GameResources * gr)
{
	GtkWidget *label;
	GtkWidget *spin;
	GtkObject *adjustment;

	gtk_table_resize(GTK_TABLE(gr), 1, 2);
	gtk_table_set_row_spacings(GTK_TABLE(gr), 3);
	gtk_table_set_col_spacings(GTK_TABLE(gr), 5);
	gtk_table_set_homogeneous(GTK_TABLE(gr), TRUE);

	label = gtk_label_new(_("Resource count"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(gr), label, 0, 1, 0, 1);

	adjustment = gtk_adjustment_new(0, 0, 100, 1, 5, 0);
	spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 1, 0);
	gtk_entry_set_alignment(GTK_ENTRY(spin), 1.0);
	gtk_table_attach_defaults(GTK_TABLE(gr), spin, 1, 2, 0, 1);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);
	gr->num_resources = GTK_SPIN_BUTTON(spin);
}

/* Create a new instance of the widget */
GtkWidget *game_resources_new(void)
{
	return GTK_WIDGET(g_object_new(game_resources_get_type(), NULL));
}

void game_resources_set_num_resources(GameResources * gr, gint num)
{
	gtk_spin_button_set_value(gr->num_resources, num);
}

gint game_resources_get_num_resources(GameResources * gr)
{
	return gtk_spin_button_get_value(gr->num_resources);
}
