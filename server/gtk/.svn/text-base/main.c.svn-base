/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 1999 Dave Cole
 * Copyright (C) 2003 Bas Wijnen <shevek@fmf.nl>
 * Copyright (C) 2004-2007 Roland Clobus <rclobus@bigfoot.com>
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
#include "version.h"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <ctype.h>
#include <gtk/gtk.h>
#include <string.h>

#include "authors.h"
#include "aboutbox.h"
#include "game.h"
#include "common_gtk.h"

#include "config-gnome.h"
#include "server.h"

#include "select-game.h"	/* Custom widget */
#include "game-settings.h"	/* Custom widget */
#include "game-rules.h"
#include "theme.h"
#include "metaserver.h"		/* Custom widget */

#include "avahi.h"

#define MAINICON_FILE	"pioneers-server.png"

static GtkWidget *settings_notebook;	/* relevant settings */
static GtkWidget *game_frame;	/* the frame containing all settings regarding the game */
static GtkWidget *rules_frame;	/* the frame containing all rules regarding the game */
static GtkWidget *select_game;	/* select game type */
static GtkWidget *game_settings;	/* the settings of the game */
static GtkWidget *game_rules;	/* the rules of the game */
static GtkWidget *server_frame;	/* the frame containing all settings regarding the server */
static GtkWidget *ai_frame;	/* the frame containing all settings regarding the ai */
static GtkWidget *register_toggle;	/* register with meta server? */
static GtkWidget *chat_toggle;	/* disable AI chatting? */
static GtkWidget *meta_entry;	/* name of meta server */
static GtkWidget *overridden_hostname_entry;	/* name of server (allows masquerading) */
static GtkWidget *port_entry;	/* server port */
static GtkWidget *random_toggle;	/* randomize seating order? */
static GtkWidget *addcomputer_btn;	/* button to add computer players */
static GtkWidget *launchclient_btn;	/* button to launch client window */

static GtkWidget *start_btn;	/* start/stop the server */

static GtkListStore *store;	/* shows player connection status */
static Game *game = NULL;	/* the current game */

static gchar *overridden_hostname;	/* override reported hostname */
static gchar *server_port = NULL;	/* port of the game */
static gboolean register_server = TRUE;	/* Register at the meta server */
static gboolean want_ai_chat = TRUE;
static gboolean random_order = TRUE;	/* random seating order */
static gboolean enable_debug = FALSE;
static gboolean show_version = FALSE;

/* Local function prototypes */
static void add_game_to_list(gpointer name, gpointer user_data);
static void check_vp_cb(GObject * caller, gpointer main_window);
static void quit_cb(void);
static void help_about_cb(void);

enum {
	PLAYER_COLUMN_CONNECTED,
	PLAYER_COLUMN_NAME,
	PLAYER_COLUMN_LOCATION,
	PLAYER_COLUMN_NUMBER,
	PLAYER_COLUMN_ISVIEWER,
	PLAYER_COLUMN_LAST
};

/* Normal items */
static GtkActionEntry entries[] = {
	{"GameMenu", NULL,
	 /* Menu entry */
	 N_("_Game"), NULL, NULL, NULL},
	{"HelpMenu", NULL,
	 /* Menu entry */
	 N_("_Help"), NULL, NULL, NULL},
	{"GameCheckVP", GTK_STOCK_APPLY,
	 /* Menu entry */
	 N_("_Check Victory Point Target"),
	 NULL,
	 /* Tooltop for Check Victory Point Target menu entry */
	 N_("Check whether the game can be won"), G_CALLBACK(check_vp_cb)},
	{"GameQuit", GTK_STOCK_QUIT,
	 /* Menu entry */
	 N_("_Quit"), "<control>Q",
	 /* Tooltop for Quit menu entry */
	 N_("Quit the program"), quit_cb},
	{"HelpAbout", NULL,
	 /* Menu entry */
	 N_("_About Pioneers Server"), NULL,
	 /* Tooltop for About Pioneers Server menu entry */
	 N_("Information about Pioneers Server"), help_about_cb}
};

/* *INDENT-OFF* */
static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='GameMenu'>"
"      <menuitem action='GameCheckVP' />"
"      <separator/>"
"      <menuitem action='GameQuit' />"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='HelpAbout' />"
"    </menu>" 
"  </menubar>"
"</ui>";
/* *INDENT-ON* */

static void port_entry_changed_cb(GtkWidget * widget,
				  G_GNUC_UNUSED gpointer user_data)
{
	const gchar *text;

	text = gtk_entry_get_text(GTK_ENTRY(widget));
	if (server_port)
		g_free(server_port);
	server_port = g_strstrip(g_strdup(text));
}

static void register_toggle_cb(GtkToggleButton * toggle,
			       G_GNUC_UNUSED gpointer user_data)
{
	register_server = gtk_toggle_button_get_active(toggle);
	gtk_widget_set_sensitive(meta_entry, register_server);
	gtk_widget_set_sensitive(overridden_hostname_entry,
				 register_server);
}

static void random_toggle_cb(GtkToggleButton * toggle,
			     G_GNUC_UNUSED gpointer user_data)
{
	random_order = gtk_toggle_button_get_active(toggle);
}

static void chat_toggle_cb(GtkToggleButton * toggle,
			   G_GNUC_UNUSED gpointer user_data)
{
	want_ai_chat = gtk_toggle_button_get_active(toggle);
}

/* The server does not need to respond to changed game settings directly
 * RC: Leaving this code here, for when the admin-interface will be built
static void game_settings_change_cb(GameSettings *gs, G_GNUC_UNUSED gpointer user_data) 
{
	printf("Settings: %d %d %d %d\n", 
			game_settings_get_terrain(gs),
			game_settings_get_players(gs),
			game_settings_get_victory_points(gs),
			game_settings_get_sevens_rule(gs)
	      );
}
*/

static void update_game_settings(const GameParams * params)
{
	g_return_if_fail(params != NULL);

	/* Update the UI */
	game_settings_set_players(GAMESETTINGS(game_settings),
				  params->num_players);
	game_settings_set_victory_points(GAMESETTINGS(game_settings),
					 params->victory_points);
	game_rules_set_victory_at_end_of_turn(GAMERULES(game_rules),
					      params->
					      check_victory_at_end_of_turn);
	game_rules_set_use_cities_and_knights_rules(GAMERULES(game_rules),
						    params->
						    use_cities_and_knights_rules);
	game_rules_set_random_terrain(GAMERULES(game_rules),
				      params->random_terrain);
	game_rules_set_sevens_rule(GAMERULES(game_rules),
				   params->sevens_rule);
	game_rules_set_domestic_trade(GAMERULES(game_rules),
				      params->domestic_trade);
	game_rules_set_strict_trade(GAMERULES(game_rules),
				    params->strict_trade);
	game_rules_set_use_pirate(GAMERULES(game_rules),
				  params->use_pirate,
				  params->num_build_type[BUILD_SHIP]);
}

static void game_activate(GtkWidget * widget,
			  G_GNUC_UNUSED gpointer user_data)
{
	const gchar *title;
	const GameParams *params;

	title = select_game_get_active(SELECTGAME(widget));
	params = game_list_find_item(title);
	update_game_settings(params);
}

static void gui_set_server_state(gboolean running)
{
	gboolean ai_settings_enabled = TRUE;
	gchar *fullname;

	gtk_widget_set_sensitive(gtk_notebook_get_nth_page
				 (GTK_NOTEBOOK(settings_notebook), 0),
				 !running);
	if (running)
		gtk_widget_show(gtk_notebook_get_nth_page
				(GTK_NOTEBOOK(settings_notebook), 1));
	else
		gtk_widget_hide(gtk_notebook_get_nth_page
				(GTK_NOTEBOOK(settings_notebook), 1));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(settings_notebook),
				      running ? 1 : 0);
	gtk_button_set_label(GTK_BUTTON(start_btn),
			     running ? _("Stop Server") :
			     _("Start Server"));
	gtk_widget_set_tooltip_text(start_btn,
				    running ? _("Stop the server") :
				    _("Start the server"));

	fullname = g_find_program_in_path(PIONEERS_AI_PATH);
	if (fullname) {
		g_free(fullname);
	} else {
		ai_settings_enabled = FALSE;
	}
	gtk_widget_set_sensitive(ai_frame, ai_settings_enabled);
}

static void start_clicked_cb(G_GNUC_UNUSED GtkButton * start_btn,
			     G_GNUC_UNUSED gpointer user_data)
{
	if (server_is_running(game)) {
		server_stop(game);
		gui_set_server_state(server_is_running(game));
		avahi_unregister_game();
	} else {		/* not running */
		const gchar *title;
		GameParams *params;
		gchar *meta_server_name;

		title = select_game_get_active(SELECTGAME(select_game));
		params = params_copy(game_list_find_item(title));
		cfg_set_num_players(params,
				    game_settings_get_players(GAMESETTINGS
							      (game_settings)));
		cfg_set_victory_points(params,
				       game_settings_get_victory_points
				       (GAMESETTINGS(game_settings)));
		params->check_victory_at_end_of_turn =
		    game_rules_get_victory_at_end_of_turn(GAMERULES
							  (game_rules));
		params->use_cities_and_knights_rules =
		    game_rules_get_use_cities_and_knights_rules(GAMERULES
								(game_rules));
		cfg_set_sevens_rule(params,
				    game_rules_get_sevens_rule(GAMERULES
							       (game_rules)));
		cfg_set_terrain_type(params,
				     game_rules_get_random_terrain
				     (GAMERULES(game_rules)));
		params->strict_trade =
		    game_rules_get_strict_trade(GAMERULES(game_rules));
		params->use_pirate =
		    game_rules_get_use_pirate(GAMERULES(game_rules));
		params->domestic_trade =
		    game_rules_get_domestic_trade(GAMERULES(game_rules));
		update_game_settings(params);

		meta_server_name = metaserver_get(METASERVER(meta_entry));

		g_assert(server_port != NULL);

		if (game != NULL)
			game_free(game);
		game =
		    server_start(params, overridden_hostname, server_port,
				 register_server, meta_server_name,
				 random_order);
		if (server_is_running(game)) {
			avahi_register_game(game);
			gui_set_server_state(TRUE);
			config_set_string("server/meta-server",
					  meta_server_name);
			config_set_string("server/port", server_port);
			config_set_int("server/register", register_server);
			config_set_string("server/overridden-hostname",
					  overridden_hostname);
			config_set_int("server/random-seating-order",
				       random_order);

			config_set_string("game/name", params->title);
			config_set_int("game/random-terrain",
				       params->random_terrain);
			config_set_int("game/num-players",
				       params->num_players);
			config_set_int("game/victory-points",
				       params->victory_points);
			config_set_int("game/check-victory-at-end-of-turn",
				       params->
				       check_victory_at_end_of_turn);
			config_set_int("game/use-cities-and-knights-rules",
				       params->
				       use_cities_and_knights_rules);
			config_set_int("game/sevens-rule",
				       params->sevens_rule);
			config_set_int("game/use-pirate",
				       params->use_pirate);
			config_set_int("game/strict-trade",
				       params->strict_trade);
			config_set_int("game/domestic-trade",
				       params->domestic_trade);
		}
		params_free(params);
		g_free(meta_server_name);
	}
}

static void launchclient_clicked_cb(G_GNUC_UNUSED GtkButton * start_btn,
				    G_GNUC_UNUSED gpointer user_data)
{
	gchar *child_argv[7];
	GSpawnFlags child_flags = G_SPAWN_STDOUT_TO_DEV_NULL |
	    G_SPAWN_STDERR_TO_DEV_NULL;
	GError *error = NULL;
	gint i;

	g_assert(server_port != NULL);

	/* Populate argv. */
	child_argv[0] = g_strdup(PIONEERS_CLIENT_GTK_PATH);
	child_argv[1] = g_strdup(PIONEERS_CLIENT_GTK_PATH);
	child_argv[2] = g_strdup("-s");
	child_argv[3] = g_strdup("localhost");
	child_argv[4] = g_strdup("-p");
	child_argv[5] = g_strdup(server_port);
	child_argv[6] = NULL;

	/* Spawn the child. */
	if (!g_spawn_async(NULL, child_argv, NULL, child_flags,
			   NULL, NULL, NULL, &error)) {
		/* Error message when program %1 is started, reason is %2 */
		log_message(MSG_ERROR,
			    _("Error starting %s: %s"),
			    PIONEERS_CLIENT_GTK_PATH, error->message);
		g_error_free(error);
	}

	/* Clean up after ourselves. */
	for (i = 0; child_argv[i] != NULL; i++) {
		g_free(child_argv[i]);
	}
}

static void addcomputer_clicked_cb(G_GNUC_UNUSED GtkButton * start_btn,
				   G_GNUC_UNUSED gpointer user_data)
{
	g_assert(server_port != NULL);
	add_computer_player(game, want_ai_chat);
	config_set_int("ai/enable-chat", want_ai_chat);
}


static void gui_player_add(void *data_in)
{
	Player *player = data_in;
	log_message(MSG_INFO, _("Player %s from %s entered\n"),
		    player->name, player->location);
}

static void gui_player_remove(void *data)
{
	Player *player = data;
	log_message(MSG_INFO, _("Player %s from %s left\n"), player->name,
		    player->location);
}

static void gui_player_rename(void *data)
{
	Player *player = data;
	log_message(MSG_INFO, _("Player %d is now %s\n"), player->num,
		    player->name);
}

static gboolean everybody_left(G_GNUC_UNUSED gpointer data)
{
	server_stop(game);
	gui_set_server_state(server_is_running(game));
	return FALSE;
}

static void gui_player_change(void *data)
{
	Game *game = data;
	GList *current;
	guint number_of_players = 0;

	gtk_list_store_clear(store);
	playerlist_inc_use_count(game);
	for (current = game->player_list; current != NULL;
	     current = g_list_next(current)) {
		GtkTreeIter iter;
		Player *p = current->data;
		gboolean isViewer;

		isViewer = player_is_viewer(p->game, p->num);
		if (!isViewer && !p->disconnected)
			number_of_players++;

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				   PLAYER_COLUMN_NAME, p->name,
				   PLAYER_COLUMN_LOCATION, p->location,
				   PLAYER_COLUMN_NUMBER, p->num,
				   PLAYER_COLUMN_CONNECTED,
				   !p->disconnected,
				   PLAYER_COLUMN_ISVIEWER, isViewer, -1);
	}
	playerlist_dec_use_count(game);
	if (number_of_players == 0 && game->is_game_over) {
		g_timeout_add(100, everybody_left, NULL);
	}
}

static void add_game_to_list(gpointer name,
			     G_GNUC_UNUSED gpointer user_data)
{
	GameParams *a = (GameParams *) name;
	select_game_add_with_map(SELECTGAME(select_game), a->title,
				 a->map);
}

static void overridden_hostname_changed_cb(GtkEntry * widget,
					   G_GNUC_UNUSED gpointer
					   user_data)
{
	const gchar *text;

	text = gtk_entry_get_text(widget);
	while (*text != '\0' && isspace(*text))
		text++;
	if (overridden_hostname)
		g_free(overridden_hostname);
	overridden_hostname = g_strdup(text);
}

static GtkWidget *build_game_settings(GtkWidget * parent,
				      GtkWindow * main_window)
{
	GtkWidget *frame;
	GtkWidget *vbox;

	frame = gtk_frame_new(_("Game parameters"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, TRUE, 0);

	vbox = gtk_vbox_new(FALSE, 3);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 3);

	select_game = select_game_new();
	gtk_widget_show(select_game);
	g_signal_connect(G_OBJECT(select_game), "activate",
			 G_CALLBACK(game_activate), NULL);
	gtk_box_pack_start(GTK_BOX(vbox), select_game, FALSE, FALSE, 0);

	game_settings = game_settings_new(TRUE);
	gtk_widget_show(game_settings);
	gtk_box_pack_start(GTK_BOX(vbox), game_settings, TRUE, TRUE, 0);

	g_signal_connect(G_OBJECT(game_settings), "check",
			 G_CALLBACK(check_vp_cb), main_window);

	return frame;
}

static GtkWidget *build_game_rules(GtkWidget * parent)
{
	GtkWidget *frame;
	GtkWidget *vbox;

	frame = gtk_frame_new(_("Rules"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, TRUE, 0);

	vbox = gtk_vbox_new(FALSE, 3);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 3);

	game_rules = game_rules_new();
	gtk_widget_show(game_rules);
	gtk_box_pack_start(GTK_BOX(vbox), game_rules, TRUE, TRUE, 0);

	return frame;
}

static void
my_cell_player_viewer_to_text(G_GNUC_UNUSED GtkTreeViewColumn *
			      tree_column, GtkCellRenderer * cell,
			      GtkTreeModel * tree_model,
			      GtkTreeIter * iter, gpointer data)
{
	gboolean b;

	/* Get the value from the model. */
	gtk_tree_model_get(tree_model, iter, GPOINTER_TO_INT(data), &b,
			   -1);
	g_object_set(cell, "text", b ?
		     /* Role of the player: viewer */
		     _("Viewer") :
		     /* Role of the player: player */
		     _("Player"), NULL);
}

static GtkWidget *build_interface(GtkWindow * main_window)
{
	GtkWidget *vbox;
	GtkWidget *vbox_settings;
	GtkWidget *vbox_connected;
	GtkWidget *vbox_ai;
	GtkWidget *frame;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *scroll_win;
	GtkWidget *message_text;
	gint novar;
	gboolean default_returned;
	gchar *meta_server_name;
	gchar *gamename;
	gint temp;
	GameParams *params;
	GtkWidget *stop_game_button_on_tab;
	GtkWidget *label_with_close_button;

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_widget_show(vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

	settings_notebook = gtk_notebook_new();
	gtk_widget_show(settings_notebook);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(settings_notebook),
				     FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), settings_notebook, FALSE, TRUE,
			   0);

	vbox_settings = gtk_vbox_new(FALSE, 5);
	gtk_widget_show(vbox_settings);
	gtk_notebook_append_page(GTK_NOTEBOOK(settings_notebook),
				 vbox_settings,
				 gtk_label_new(_("Game settings")));

	/* Game settings frame */
	game_frame = build_game_settings(vbox_settings, main_window);

	/* Rules frame */
	rules_frame = build_game_rules(vbox_settings);

	/* Fill the GUI with the saved settings */
	gamename =
	    config_get_string("game/name=Default", &default_returned);
	params = cfg_set_game(gamename);
	if (params == NULL)
		params = cfg_set_game("Default");
	select_game_set_default(SELECTGAME(select_game), gamename);
	game_list_foreach(add_game_to_list, NULL);
	g_free(gamename);

	/* If a setting is not found, don't override the settings that came
	 * with the game */
	g_assert(params != NULL);
	temp = config_get_int("game/random-terrain", &default_returned);
	if (!default_returned)
		cfg_set_terrain_type(params, temp);
	temp = config_get_int("game/num-players", &default_returned);
	if (!default_returned)
		cfg_set_num_players(params, temp);
	temp = config_get_int("game/victory-points", &default_returned);
	if (!default_returned)
		cfg_set_victory_points(params, temp);
	temp =
	    config_get_int("game/victory-at-end-of-turn",
			   &default_returned);
	if (!default_returned)
		params->check_victory_at_end_of_turn = temp;
	temp =
	    config_get_int("game/use-cities-and-knights-rules",
			   &default_returned);
	if (!default_returned)
		params->use_cities_and_knights_rules = temp;
	temp = config_get_int("game/sevens-rule", &default_returned);
	if (!default_returned)
		cfg_set_sevens_rule(params, temp);
	temp = config_get_int("game/use-pirate", &default_returned);
	if (!default_returned)
		params->use_pirate = temp;
	temp = config_get_int("game/strict-trade", &default_returned);
	if (!default_returned)
		params->strict_trade = temp;
	temp = config_get_int("game/domestic-trade", &default_returned);
	if (!default_returned)
		params->domestic_trade = temp;
	update_game_settings(params);
	params_free(params);

	server_frame = gtk_frame_new(_("Server parameters"));
	gtk_widget_show(server_frame);
	gtk_box_pack_start(GTK_BOX(vbox_settings), server_frame, FALSE,
			   TRUE, 0);

	table = gtk_table_new(6, 2, FALSE);
	gtk_widget_show(table);
	gtk_container_add(GTK_CONTAINER(server_frame), table);
	gtk_container_set_border_width(GTK_CONTAINER(table), 3);
	gtk_table_set_row_spacings(GTK_TABLE(table), 3);
	gtk_table_set_col_spacings(GTK_TABLE(table), 5);

	label = gtk_label_new(_("Server port"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
			 GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	port_entry = gtk_entry_new();
	g_signal_connect(G_OBJECT(port_entry), "changed",
			 G_CALLBACK(port_entry_changed_cb), NULL);
	gtk_widget_show(port_entry);
	gtk_table_attach(GTK_TABLE(table), port_entry, 1, 2, 0, 1,
			 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0,
			 0);
	gtk_widget_set_tooltip_text(port_entry,
				    _("The port for the game server"));

	register_toggle =
	    gtk_check_button_new_with_label(_("Register server"));
	gtk_widget_show(register_toggle);
	gtk_table_attach(GTK_TABLE(table), register_toggle, 0, 2, 1, 2,
			 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0,
			 0);
	g_signal_connect(G_OBJECT(register_toggle), "toggled",
			 G_CALLBACK(register_toggle_cb), NULL);
	gtk_widget_set_tooltip_text(register_toggle,
				    _(""
				      "Register this game at the meta server"));

	label = gtk_label_new(_("Meta server"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
			 GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	meta_entry = metaserver_new();
	gtk_widget_show(meta_entry);
	gtk_table_attach(GTK_TABLE(table), meta_entry, 1, 2, 2, 3,
			 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0,
			 0);

	label = gtk_label_new(_("Reported hostname"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4,
			 GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	overridden_hostname_entry = gtk_entry_new();
	g_signal_connect(G_OBJECT(overridden_hostname_entry), "changed",
			 G_CALLBACK(overridden_hostname_changed_cb), NULL);
	gtk_widget_show(overridden_hostname_entry);
	gtk_table_attach(GTK_TABLE(table), overridden_hostname_entry, 1, 2,
			 3, 4, GTK_EXPAND | GTK_FILL,
			 GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_widget_set_tooltip_text(overridden_hostname_entry,
				    _(""
				      "The public name of this computer "
				      "(needed when playing behind a firewall)"));

	random_toggle =
	    gtk_check_button_new_with_label(_("Random turn order"));
	gtk_widget_show(random_toggle);
	gtk_table_attach(GTK_TABLE(table), random_toggle, 0, 2, 4, 5,
			 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0,
			 0);
	g_signal_connect(G_OBJECT(random_toggle), "toggled",
			 G_CALLBACK(random_toggle_cb), NULL);
	gtk_widget_set_tooltip_text(random_toggle,
				    _("Randomize turn order"));

	/* Initialize server-settings */
	server_port = config_get_string("server/port="
					PIONEERS_DEFAULT_GAME_PORT,
					&novar);
	gtk_entry_set_text(GTK_ENTRY(port_entry), server_port);

	novar = 0;
	meta_server_name = config_get_string("server/meta-server", &novar);
	if (novar || !strlen(meta_server_name)
	    || !strncmp(meta_server_name, "gnocatan.debian.net",
			strlen(meta_server_name) + 1))
		meta_server_name = get_meta_server_name(TRUE);
	metaserver_add(METASERVER(meta_entry), meta_server_name);
	g_free(meta_server_name);

	novar = 0;
	overridden_hostname =
	    config_get_string("server/overridden-hostname", &novar);
	if (novar)
		overridden_hostname = g_strdup("");
	gtk_entry_set_text(GTK_ENTRY(overridden_hostname_entry),
			   overridden_hostname);

	register_server =
	    config_get_int_with_default("server/register", TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(register_toggle),
				     register_server);
	register_toggle_cb(GTK_TOGGLE_BUTTON(register_toggle), NULL);

	random_order =
	    config_get_int_with_default("server/random-seating-order",
					TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(random_toggle),
				     random_order);

	label_with_close_button = create_label_with_close_button(
									/* Tab name */
									_
									(""
									 "Running game"),
									/* Tab tooltip */
									_
									(""
									 "Stop the server"),
									&stop_game_button_on_tab);
	g_signal_connect(G_OBJECT(stop_game_button_on_tab), "clicked",
			 G_CALLBACK(start_clicked_cb), NULL);

	vbox_settings = gtk_vbox_new(FALSE, 5);
	gtk_notebook_append_page(GTK_NOTEBOOK(settings_notebook),
				 vbox_settings, label_with_close_button);

	/* Caption */
	frame = gtk_frame_new(_("Players connected"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(vbox_settings), frame, TRUE, TRUE, 0);

	vbox_connected = gtk_vbox_new(FALSE, 5);
	gtk_widget_show(vbox_connected);
	gtk_container_set_border_width(GTK_CONTAINER(vbox_connected), 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox_connected);

	scroll_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scroll_win);
	gtk_box_pack_start(GTK_BOX(vbox_connected), scroll_win, TRUE, TRUE,
			   0);

	gtk_container_set_border_width(GTK_CONTAINER(scroll_win), 3);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);

	/* Create model */
	store = gtk_list_store_new(PLAYER_COLUMN_LAST,
				   G_TYPE_BOOLEAN,
				   G_TYPE_STRING,
				   G_TYPE_STRING,
				   G_TYPE_INT, G_TYPE_BOOLEAN);

	/* Create graphical representation of the model */
	label = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	/* The theme should decide if hints are used */
	/* gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(label), TRUE); */
	gtk_container_add(GTK_CONTAINER(scroll_win), label);
	gtk_widget_set_tooltip_text(label,
				    /* Tooltip for server connection overview */
				    _(""
				      "Shows all players and viewers connected to the server"));

	/* Now create columns */
	column =
	    gtk_tree_view_column_new_with_attributes(_("Connected"),
						     gtk_cell_renderer_toggle_new
						     (), "active",
						     PLAYER_COLUMN_CONNECTED,
						     NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(label), column);
	gtk_widget_set_tooltip_text(column->button,
				    /* Tooltip for column Connected */
				    _(""
				      "Is the player currently connected?"));
	column =
	    gtk_tree_view_column_new_with_attributes(_("Name"),
						     gtk_cell_renderer_text_new
						     (), "text",
						     PLAYER_COLUMN_NAME,
						     NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(label), column);
	gtk_widget_set_tooltip_text(column->button,
				    /* Tooltip for column Name */
				    _("Name of the player"));
	column =
	    gtk_tree_view_column_new_with_attributes(_("Location"),
						     gtk_cell_renderer_text_new
						     (), "text",
						     PLAYER_COLUMN_LOCATION,
						     NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(label), column);
	gtk_widget_set_tooltip_text(column->button,
				    /* Tooltip for column Location */
				    _("Host name of the player"));

	renderer = gtk_cell_renderer_text_new();
	column =
	    gtk_tree_view_column_new_with_attributes(_("Number"), renderer,
						     "text",
						     PLAYER_COLUMN_NUMBER,
						     NULL);
	g_object_set(renderer, "xalign", 1.0f, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(label), column);
	/* Tooltip for colum Number */
	gtk_widget_set_tooltip_text(column->button, _("Player number"));

	renderer = gtk_cell_renderer_text_new();
	column =
	    gtk_tree_view_column_new_with_attributes(_("Role"), renderer,
						     NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(label), column);
	/* Tooltip for column Role */
	gtk_widget_set_tooltip_text(column->button, _("Player or viewer"));

	gtk_tree_view_column_set_cell_data_func(column, renderer,
						my_cell_player_viewer_to_text,
						GINT_TO_POINTER
						(PLAYER_COLUMN_ISVIEWER),
						NULL);

	gtk_widget_show(label);

	launchclient_btn = gtk_button_new_with_label(
							    /* Button text */
							    _(""
							      "Launch Pioneers Client"));
	gtk_widget_show(launchclient_btn);
	gtk_box_pack_start(GTK_BOX(vbox_connected), launchclient_btn,
			   FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(launchclient_btn), "clicked",
			 G_CALLBACK(launchclient_clicked_cb), NULL);
	gtk_widget_set_tooltip_text(launchclient_btn,
				    /* Tooltip */
				    _("Launch the Pioneers client"));

	/* Caption */
	ai_frame = gtk_frame_new(_("Computer players"));
	gtk_widget_show(ai_frame);
	gtk_box_pack_start(GTK_BOX(vbox_settings), ai_frame, FALSE, FALSE,
			   0);
	vbox_ai = gtk_vbox_new(FALSE, 5);
	gtk_widget_show(vbox_ai);
	gtk_container_set_border_width(GTK_CONTAINER(vbox_ai), 5);
	gtk_container_add(GTK_CONTAINER(ai_frame), vbox_ai);

	/* Label */
	chat_toggle = gtk_check_button_new_with_label(_("Enable chat"));
	gtk_widget_show(chat_toggle);
	gtk_box_pack_start(GTK_BOX(vbox_ai), chat_toggle, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT(chat_toggle), "toggled",
			 G_CALLBACK(chat_toggle_cb), NULL);
	gtk_widget_set_tooltip_text(chat_toggle,
				    /* Tooltip */
				    _("Enable chat messages"));

	want_ai_chat = config_get_int_with_default("ai/enable-chat", TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chat_toggle),
				     want_ai_chat);

	addcomputer_btn = gtk_button_new_with_label(
							   /* Button text */
							   _(""
							     "Add Computer Player"));
	gtk_widget_show(addcomputer_btn);
	gtk_box_pack_start(GTK_BOX(vbox_ai), addcomputer_btn, FALSE, FALSE,
			   0);
	g_signal_connect(G_OBJECT(addcomputer_btn), "clicked",
			 G_CALLBACK(addcomputer_clicked_cb), NULL);
	gtk_widget_set_tooltip_text(addcomputer_btn,
				    /* Tooltip */
				    _(""
				      "Add a computer player to the game"));

	start_btn = gtk_button_new();
	gtk_widget_show(start_btn);

	gtk_box_pack_start(GTK_BOX(vbox), start_btn, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(start_btn), "clicked",
			 G_CALLBACK(start_clicked_cb), NULL);

	/* Caption */
	frame = gtk_frame_new(_("Messages"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

	scroll_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scroll_win);
	gtk_container_add(GTK_CONTAINER(frame), scroll_win);
	gtk_container_set_border_width(GTK_CONTAINER(scroll_win), 3);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);

	message_text = gtk_text_view_new();
	gtk_widget_show(message_text);
	gtk_container_add(GTK_CONTAINER(scroll_win), message_text);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(message_text),
				    GTK_WRAP_WORD);
	gtk_widget_set_tooltip_text(message_text,
				    /* Tooltip */
				    _("Messages from the server"));
	message_window_set_text(message_text);

	gui_set_server_state(FALSE);
	return vbox;
}

static void check_vp_cb(G_GNUC_UNUSED GObject * caller,
			gpointer main_window)
{
	const gchar *title;
	GameParams *params;

	title = select_game_get_active(SELECTGAME(select_game));
	params = params_copy(game_list_find_item(title));
	cfg_set_num_players(params,
			    game_settings_get_players(GAMESETTINGS
						      (game_settings)));
	cfg_set_victory_points(params,
			       game_settings_get_victory_points
			       (GAMESETTINGS(game_settings)));
	params->check_victory_at_end_of_turn =
	    game_rules_get_victory_at_end_of_turn(GAMERULES(game_rules));
	params->use_cities_and_knights_rules =
	    game_rules_get_use_cities_and_knights_rules(GAMERULES
							(game_rules));
	cfg_set_sevens_rule(params,
			    game_rules_get_sevens_rule(GAMERULES
						       (game_rules)));
	cfg_set_terrain_type(params,
			     game_rules_get_random_terrain(GAMERULES
							   (game_rules)));
	params->strict_trade =
	    game_rules_get_strict_trade(GAMERULES(game_rules));
	params->use_pirate =
	    game_rules_get_use_pirate(GAMERULES(game_rules));
	params->domestic_trade =
	    game_rules_get_domestic_trade(GAMERULES(game_rules));
	update_game_settings(params);

	check_victory_points(params, main_window);
}

static void quit_cb(void)
{
	gtk_main_quit();
	avahi_unregister_game();
}

static void help_about_cb(void)
{
	const gchar *authors[] = {
		AUTHORLIST
	};
	/* Dialog caption of about box */
	aboutbox_display(_("The Pioneers Game Server"), authors);
}

void game_is_over(G_GNUC_UNUSED Game * game)
{
	/* Wait for all players to disconnect,
	 * then enable the UI
	 */
	log_message(MSG_INFO, _("The game is over.\n"));
}

void request_server_stop(Game * game)
{
	server_stop(game);
	gui_set_server_state(server_is_running(game));
}

static GOptionEntry commandline_entries[] = {
	{"debug", '\0', 0, G_OPTION_ARG_NONE, &enable_debug,
	 /* Commandline option of server-gtk: enable debug logging */
	 N_("Enable debug messages"), NULL},
	{"version", '\0', 0, G_OPTION_ARG_NONE, &show_version,
	 /* Commandline option of server-gtk: version */
	 N_("Show version information"), NULL},
	{NULL, '\0', 0, 0, NULL, NULL, NULL}
};

int main(int argc, char *argv[])
{
	gchar *icon_file;
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *menubar;
	GtkActionGroup *action_group;
	GtkUIManager *ui_manager;
	GtkAccelGroup *accel_group;
	GError *error = NULL;
	GOptionContext *context;

	net_init();

	/* set the UI driver to GTK_Driver, since we're using gtk */
	set_ui_driver(&GTK_Driver);

	/* flush out the rest of the driver with the server callbacks */
	driver->player_added = gui_player_add;
	driver->player_renamed = gui_player_rename;
	driver->player_removed = gui_player_remove;
	driver->player_change = gui_player_change;

	/* Initialize frontend inspecific things */
	server_init();

#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	/* Gtk+ handles the locale, we must bind the translations */
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
#endif

	/* Long description in the commandline for server-gtk: help */
	context = g_option_context_new(_("- Host a game of Pioneers"));
	g_option_context_add_main_entries(context, commandline_entries,
					  PACKAGE);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	g_option_context_parse(context, &argc, &argv, &error);
	if (error != NULL) {
		g_print("%s\n", error->message);
		g_error_free(error);
		return 1;
	}
	if (show_version) {
		g_print(_("Pioneers version:"));
		g_print(" ");
		g_print(FULL_VERSION);
		g_print("\n");
		return 0;
	}

	set_enable_debug(enable_debug);

	prepare_gtk_for_close_button_on_tab();

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	/* Name in the titlebar of the server */
	gtk_window_set_title(GTK_WINDOW(window), _("Pioneers Server"));

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	action_group = gtk_action_group_new("MenuActions");
	gtk_action_group_set_translation_domain(action_group, PACKAGE);
	gtk_action_group_add_actions(action_group, entries,
				     G_N_ELEMENTS(entries), window);

	ui_manager = gtk_ui_manager_new();
	gtk_ui_manager_insert_action_group(ui_manager, action_group, 0);

	accel_group = gtk_ui_manager_get_accel_group(ui_manager);
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

	error = NULL;
	if (!gtk_ui_manager_add_ui_from_string
	    (ui_manager, ui_description, -1, &error)) {
		/* Error message */
		g_message(_("Building menus failed: %s"), error->message);
		g_error_free(error);
		return 1;
	}

	config_init("pioneers-server");

	themes_init();
	game_list_prepare();

	icon_file =
	    g_build_filename(DATADIR, "pixmaps", MAINICON_FILE, NULL);
	if (g_file_test(icon_file, G_FILE_TEST_EXISTS)) {
		gtk_window_set_default_icon_from_file(icon_file, NULL);
	} else {
		g_warning("Pixmap not found: %s", icon_file);
	}
	g_free(icon_file);

	menubar = gtk_ui_manager_get_widget(ui_manager, "/MainMenu");
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox),
			   build_interface(GTK_WINDOW(window)), TRUE, TRUE,
			   0);

	gtk_widget_show_all(window);
	gui_set_server_state(FALSE);
	g_signal_connect(G_OBJECT(window), "delete_event",
			 G_CALLBACK(quit_cb), NULL);

	/* in theory, all windows are created now...
	 *   set logging to message window */
	log_set_func_message_window();

	gtk_main();

	config_finish();
	net_finish();
	g_option_context_free(context);
	game_list_cleanup();
	return 0;
}
