#include "config.h"
#include "game.h"
#include <gtk/gtk.h>
#include <string.h>
#include <glib.h>

#include "game-rules.h"

static void game_rules_init(GameRules * sg);

/* Register the class */
GType game_rules_get_type(void)
{
	static GType gp_type = 0;

	if (!gp_type) {
		static const GTypeInfo gp_info = {
			sizeof(GameRulesClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			NULL,	/* class init */
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof(GameRules),
			0,
			(GInstanceInitFunc) game_rules_init,
			NULL
		};
		gp_type =
		    g_type_register_static(GTK_TYPE_TABLE,
					   "GameRules", &gp_info, 0);
	}
	return gp_type;
}

static void add_row(GameRules * gr, const gchar * name,
		    const gchar * tooltip, gint row,
		    GtkCheckButton ** check)
{
	GtkWidget *check_btn;

	check_btn = gtk_check_button_new_with_label(name);
	gtk_widget_show(check_btn);
	gtk_table_attach_defaults(GTK_TABLE(gr), check_btn,
				  0, 2, row, row + 1);
	*check = GTK_CHECK_BUTTON(check_btn);

	gtk_widget_set_tooltip_text(check_btn, tooltip);
}

/* Build the composite widget */
static void game_rules_init(GameRules * gr)
{
	GtkWidget *label;
	GtkWidget *vbox_sevens;
	gint idx;
	gint row;

	gtk_table_resize(GTK_TABLE(gr), 5, 2);
	gtk_table_set_row_spacings(GTK_TABLE(gr), 3);
	gtk_table_set_col_spacings(GTK_TABLE(gr), 5);

	row = 0;

	/* Label */
	label = gtk_label_new(_("Sevens rule"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(gr), label, 0, 1, row, row + 1,
			 GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	gr->radio_sevens[0] = gtk_radio_button_new_with_label(NULL,
							      /* Sevens rule: normal */
							      _("Normal"));
	gr->radio_sevens[1] =
	    gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON
							(gr->radio_sevens
							 [0]),
							/* Sevens rule: reroll on 1st 2 turns */
							_(""
							  "Reroll on 1st 2 turns"));
	gr->radio_sevens[2] =
	    gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON
							(gr->radio_sevens
							 [0]),
							/* Sevens rule: reroll all 7s */
							_(""
							  "Reroll all 7s"));

	vbox_sevens = gtk_vbox_new(TRUE, 2);
	gtk_widget_show(vbox_sevens);
	gtk_widget_set_tooltip_text(gr->radio_sevens[0],
				    /* Tooltip for sevens rule normal */
				    _(""
				      "All sevens move the robber or pirate"));
	gtk_widget_set_tooltip_text(gr->radio_sevens[1],
				    /* Tooltip for sevens rule reroll on 1st 2 turns */
				    _(""
				      "In the first two turns all sevens are rerolled"));
	gtk_widget_set_tooltip_text(gr->radio_sevens[2],
				    /* Tooltip for sevens rule reroll all */
				    _("All sevens are rerolled"));

	for (idx = 0; idx < 3; ++idx) {
		gtk_widget_show(gr->radio_sevens[idx]);
		gtk_box_pack_start(GTK_BOX(vbox_sevens),
				   gr->radio_sevens[idx], TRUE, TRUE, 0);
	}
	gtk_table_attach(GTK_TABLE(gr), vbox_sevens, 1, 2, row, row + 1,
			 GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

	row++;
	add_row(gr, _("Randomize terrain"), _("Randomize the terrain"),
		row++, &gr->random_terrain);
	add_row(gr, _("Use pirate"), _("Use the pirate to block ships"),
		row++, &gr->use_pirate);
	add_row(gr, _("Strict trade"),
		_("Allow trade only before building or buying"), row++,
		&gr->strict_trade);
	add_row(gr, _("Domestic trade"), _("Allow trade between players"),
		row++, &gr->domestic_trade);
	add_row(gr, _("Victory at end of turn"),
		_("Check for victory only at end of turn"),
		row++, &gr->check_victory_at_end_of_turn);
	add_row(gr, _("Cities & Knights expansion rules **EXPERIMENTAL**"),
		_("Enable the Cities & Knights expansion rules"),
		row++, &gr->use_cities_and_knights_rules);
}

/* Create a new instance of the widget */
GtkWidget *game_rules_new(void)
{
	return GTK_WIDGET(g_object_new(game_rules_get_type(), NULL));
}

/* Create a new instance with only the changes that can be applied by the metaserver */
GtkWidget *game_rules_new_metaserver(void)
{
	GameRules *widget =
	    GAMERULES(g_object_new(game_rules_get_type(), NULL));
	gtk_widget_hide(GTK_WIDGET(widget->use_pirate));
	gtk_widget_hide(GTK_WIDGET(widget->strict_trade));
	gtk_widget_hide(GTK_WIDGET(widget->domestic_trade));
	gtk_widget_hide(GTK_WIDGET(widget->check_victory_at_end_of_turn));
	gtk_widget_hide(GTK_WIDGET(widget->use_cities_and_knights_rules));
	return GTK_WIDGET(widget);
}

void game_rules_set_random_terrain(GameRules * gr, gboolean val)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gr->random_terrain),
				     val);
}

gboolean game_rules_get_random_terrain(GameRules * gr)
{
	return
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
					 (gr->random_terrain));
}

/* Set the sevens rule 
 * 0 = Normal
 * 1 = Reroll first two turns
 * 2 = Reroll all
 */
void game_rules_set_sevens_rule(GameRules * gr, guint sevens_rule)
{
	g_return_if_fail(sevens_rule < G_N_ELEMENTS(gr->radio_sevens));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				     (gr->radio_sevens[sevens_rule]),
				     TRUE);

}

/* Get the sevens rule */
guint game_rules_get_sevens_rule(GameRules * gr)
{
	gint idx;

	for (idx = 0; idx < G_N_ELEMENTS(gr->radio_sevens); idx++)
		if (gtk_toggle_button_get_active
		    (GTK_TOGGLE_BUTTON(gr->radio_sevens[idx])))
			return idx;
	return 0;
}


void game_rules_set_use_pirate(GameRules * gr, gboolean val,
			       gint num_ships)
{
	if (num_ships == 0) {
		gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON
						   (gr->use_pirate), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(gr->use_pirate),
					 FALSE);
	} else {
		gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON
						   (gr->use_pirate),
						   FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					     (gr->use_pirate), val);
		gtk_widget_set_sensitive(GTK_WIDGET(gr->use_pirate), TRUE);
	}
}

gboolean game_rules_get_use_pirate(GameRules * gr)
{
	return
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
					 (gr->use_pirate));
}

void game_rules_set_strict_trade(GameRules * gr, gboolean val)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gr->strict_trade),
				     val);
}

gboolean game_rules_get_strict_trade(GameRules * gr)
{
	return
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
					 (gr->strict_trade));
}

void game_rules_set_domestic_trade(GameRules * gr, gboolean val)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gr->domestic_trade),
				     val);
}

gboolean game_rules_get_domestic_trade(GameRules * gr)
{
	return
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
					 (gr->domestic_trade));
}

void game_rules_set_victory_at_end_of_turn(GameRules * gr, gboolean val)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				     (gr->check_victory_at_end_of_turn),
				     val);
}

gboolean game_rules_get_victory_at_end_of_turn(GameRules * gr)
{
	return
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
					 (gr->
					  check_victory_at_end_of_turn));
}

void game_rules_set_use_cities_and_knights_rules(GameRules * gr,
						 gboolean val)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				     (gr->use_cities_and_knights_rules),
				     val);
}

gboolean game_rules_get_use_cities_and_knights_rules(GameRules * gr)
{
	return
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
					 (gr->
					  use_cities_and_knights_rules));
}
