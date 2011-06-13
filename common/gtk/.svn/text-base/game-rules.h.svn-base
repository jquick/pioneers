#ifndef __GAMERULES_H__
#define __GAMERULES_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include "gtkbugs.h"

G_BEGIN_DECLS
#define GAMERULES_TYPE            (game_rules_get_type ())
#define GAMERULES(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GAMERULES_TYPE, GameRules))
#define GAMERULES_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GAMERULES_TYPE, GameRulesClass))
#define IS_GAMERULES(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GAMERULES_TYPE))
#define IS_GAMERULES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GAMERULES_TYPE))
typedef struct _GameRules GameRules;
typedef struct _GameRulesClass GameRulesClass;

struct _GameRules {
	GtkTable table;

	GtkCheckButton *random_terrain;
	GtkWidget *radio_sevens[3];	/* radio buttons for sevens rules */
	GtkCheckButton *use_pirate;
	GtkCheckButton *strict_trade;
	GtkCheckButton *domestic_trade;
	GtkCheckButton *check_victory_at_end_of_turn;
	GtkCheckButton *use_cities_and_knights_rules;
};

struct _GameRulesClass {
	GtkTableClass parent_class;
};

GType game_rules_get_type(void);
GtkWidget *game_rules_new(void);
GtkWidget *game_rules_new_metaserver(void);

void game_rules_set_random_terrain(GameRules * gr, gboolean val);
gboolean game_rules_get_random_terrain(GameRules * gr);
void game_rules_set_sevens_rule(GameRules * gr, guint sevens_rule);
guint game_rules_get_sevens_rule(GameRules * gr);
void game_rules_set_use_pirate(GameRules * gr, gboolean val,
			       gint num_ships);
gboolean game_rules_get_use_pirate(GameRules * gr);
void game_rules_set_strict_trade(GameRules * gr, gboolean val);
gboolean game_rules_get_strict_trade(GameRules * gr);
void game_rules_set_domestic_trade(GameRules * gr, gboolean val);
gboolean game_rules_get_domestic_trade(GameRules * gr);
void game_rules_set_victory_at_end_of_turn(GameRules * gr, gboolean val);
gboolean game_rules_get_victory_at_end_of_turn(GameRules * gr);
void game_rules_set_use_cities_and_knights_rules(GameRules * gr,
						 gboolean val);
gboolean game_rules_get_use_cities_and_knights_rules(GameRules * gr);

G_END_DECLS
#endif				/* __GAMERULES_H__ */
