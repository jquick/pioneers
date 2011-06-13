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

/* Pioneers Console Server
 */
#include "config.h"
#include "version.h"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <glib.h>

#include "driver.h"
#include "game.h"
#include "cards.h"
#include "map.h"
#include "network.h"
#include "log.h"
#include "buildrec.h"
#include "server.h"

#include "glib-driver.h"

#include "admin.h"
#include "avahi.h"

static GMainLoop *event_loop;

static gint num_players = 0;
static gint num_points = 0;
static gint sevens_rule = -1;
static gint terrain = -1;
static gint timeout = 0;
static gint num_ai_players = 0;
static gchar *server_port = NULL;
static gchar *admin_port = NULL;
static gchar *game_title = NULL;
static gchar *game_file = NULL;
static gboolean disable_game_start = FALSE;
static gint tournament_time = -1;
static gboolean quit_when_done = FALSE;
static gchar *hostname = NULL;
static gboolean register_server = FALSE;
static gchar *meta_server_name = NULL;
static gboolean fixed_seating_order = FALSE;
static gboolean enable_debug = FALSE;
static gboolean show_version = FALSE;

static GOptionEntry commandline_game_entries[] = {
	{"game-title", 'g', 0, G_OPTION_ARG_STRING, &game_title,
	 /* Commandline server-console: game-title */
	 N_("Game title to use"), NULL},
	{"file", 0, 0, G_OPTION_ARG_STRING, &game_file,
	 /* Commandline server-console: file */
	 N_("Game file to use"), NULL},
	{"port", 'p', 0, G_OPTION_ARG_STRING, &server_port,
	 /* Commandline server-console: port */
	 N_("Port to listen on"), PIONEERS_DEFAULT_GAME_PORT},
	{"players", 'P', 0, G_OPTION_ARG_INT, &num_players,
	 /* Commandline server-console: players */
	 N_("Override number of players"), NULL},
	{"points", 'v', 0, G_OPTION_ARG_INT, &num_points,
	 /* Commandline server-console: points */
	 N_("Override number of points needed to win"), NULL},
	{"seven-rule", 'R', 0, G_OPTION_ARG_INT, &sevens_rule,
	 /* Commandline server-console: seven-rule */
	 N_("Override seven-rule handling"), "0|1|2"},
	{"terrain", 'T', 0, G_OPTION_ARG_INT, &terrain,
	 /* Commandline server-console: terrain */
	 N_("Override terrain type, 0=default 1=random"), "0|1"},
	{"computer-players", 'c', 0, G_OPTION_ARG_INT, &num_ai_players,
	 /* Commandline server-console: computer-players */
	 N_("Add N computer players"), "N"},
	{"version", '\0', 0, G_OPTION_ARG_NONE, &show_version,
	 /* Commandline option of server-console: version */
	 N_("Show version information"), NULL},
	{NULL, '\0', 0, 0, NULL, NULL, NULL}
};

static GOptionEntry commandline_meta_entries[] = {
	{"register", 'r', 0, G_OPTION_ARG_NONE, &register_server,
	 /* Commandline server-console: register */
	 N_("Register server with meta-server"), NULL},
	{"meta-server", 'm', 0, G_OPTION_ARG_STRING, &meta_server_name,
	 /* Commandline server-console: meta-server */
	 N_("Register at meta-server name (implies -r)"),
	 PIONEERS_DEFAULT_META_SERVER},
	{"hostname", 'n', 0, G_OPTION_ARG_STRING, &hostname,
	 /* Commandline server-console: hostname */
	 N_("Use this hostname when registering"), NULL},
	{NULL, '\0', 0, 0, NULL, NULL, NULL}
};

static GOptionEntry commandline_other_entries[] = {
	{"auto-quit", 'x', 0, G_OPTION_ARG_NONE, &quit_when_done,
	 /* Commandline server-console: auto-quit */
	 N_("Quit after a player has won"), NULL},
	{"empty-timeout", 'k', 0, G_OPTION_ARG_INT, &timeout,
	 /* Commandline server-console: empty-timeout */
	 N_("Quit after N seconds with no players"), "N"},
	{"tournament", 't', 0, G_OPTION_ARG_INT, &tournament_time,
	 /* Commandline server-console: tournament */
	 N_("Tournament mode, computer players added after N minutes"),
	 "N"},
	{"admin-port", 'a', 0, G_OPTION_ARG_STRING, &admin_port,
	 /* Commandline server-console: admin-port */
	 N_("Admin port to listen on"), PIONEERS_DEFAULT_ADMIN_PORT},
	{"admin-wait", 's', 0, G_OPTION_ARG_NONE, &disable_game_start,
	 /* Commandline server-console: admin-wait */
	 N_(""
	    "Don't start game immediately, wait for a command on admin port"),
	 NULL},
	{"fixed-seating-order", 0, 0, G_OPTION_ARG_NONE,
	 &fixed_seating_order,
	 /* Commandline server-console: fixed-seating-order */
	 N_(""
	    "Give players numbers according to the order they enter the game"),
	 NULL},
	{"debug", '\0', 0, G_OPTION_ARG_NONE, &enable_debug,
	 /* Commandline option of server: enable debug logging */
	 N_("Enable debug messages"), NULL},
	{NULL, '\0', 0, 0, NULL, NULL, NULL}
};

int main(int argc, char *argv[])
{
	int i;
	GOptionContext *context;
	GOptionGroup *context_group;
	GError *error = NULL;
	GameParams *params;
	Game *game = NULL;

	/* set the UI driver to Glib_Driver, since we're using glib */
	set_ui_driver(&Glib_Driver);
	driver->player_added = srv_glib_player_added;
	driver->player_renamed = srv_glib_player_renamed;
	driver->player_removed = srv_player_removed;

	driver->player_change = srv_player_change;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	/* have gettext return strings in UTF-8 */
	bind_textdomain_codeset(PACKAGE, "UTF-8");

	server_init();

	/* Long description in the commandline for server-console: help */
	context = g_option_context_new(_("- Host a game of Pioneers"));
	g_option_context_add_main_entries(context,
					  commandline_game_entries,
					  PACKAGE);
	context_group = g_option_group_new("meta",
					   /* Commandline server-console: Short description of meta group */
					   _("Meta-server Options"),
					   /* Commandline server-console: Long description of meta group */
					   _(""
					     "Options for the meta-server"),
					   NULL, NULL);
	g_option_group_set_translation_domain(context_group, PACKAGE);
	g_option_group_add_entries(context_group,
				   commandline_meta_entries);
	g_option_context_add_group(context, context_group);
	context_group = g_option_group_new("misc",
					   /* Commandline server-console: Short description of misc group */
					   _("Miscellaneous Options"),
					   /* Commandline server-console: Long description of misc group */
					   _("Miscellaneous options"),
					   NULL, NULL);
	g_option_group_set_translation_domain(context_group, PACKAGE);
	g_option_group_add_entries(context_group,
				   commandline_other_entries);
	g_option_context_add_group(context, context_group);
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

	if (server_port == NULL)
		server_port = g_strdup(PIONEERS_DEFAULT_GAME_PORT);
	if (disable_game_start)
		if (admin_port == NULL)
			admin_port = g_strdup(PIONEERS_DEFAULT_ADMIN_PORT);

	if (game_title && game_file) {
		/* server-console commandline error */
		g_print(_(""
			  "Cannot set game title and filename at the same time\n"));
		return 2;
	}
	if (game_file == NULL) {
		if (game_title == NULL) {
			if (num_players > 4)
				params = cfg_set_game("5/6-player");
			else
				params = cfg_set_game("Default");
		} else
			params = cfg_set_game(game_title);
	} else {
		params = cfg_set_game_file(game_file);
	}
	if (params == NULL) {
		/* server-console commandline error */
		g_print(_("Cannot load the parameters for the game\n"));
		return 3;
	}

	if (meta_server_name != NULL)
		register_server = TRUE;
	if (register_server && meta_server_name == NULL)
		meta_server_name = get_meta_server_name(TRUE);

	g_assert(server_port != NULL);

	if (num_players)
		cfg_set_num_players(params, num_players);

	if (sevens_rule != -1)
		cfg_set_sevens_rule(params, sevens_rule);

	if (num_points)
		cfg_set_victory_points(params, num_points);

	if (tournament_time != -1)
		cfg_set_tournament_time(params, tournament_time);

	cfg_set_quit(params, quit_when_done);

	if (terrain != -1)
		cfg_set_terrain_type(params, terrain ? 1 : 0);

	net_init();

	if (admin_port != NULL) {
		if (!admin_listen(admin_port)) {
			/* Error message */
			g_print(_("Admin port not available.\n"));
			return 5;
		}
	}

	if (disable_game_start && admin_port == NULL) {
		/* server-console commandline error */
		g_print(_(""
			  "Admin port is not set, "
			  "cannot disable game start too\n"));
		return 4;

	}

	if (!disable_game_start) {
		game =
		    server_start(params, hostname, server_port,
				 register_server, meta_server_name,
				 !fixed_seating_order);
		if (game != NULL) {
			game->no_player_timeout = timeout;
			num_ai_players =
			    CLAMP(num_ai_players, 0,
				  game->params->num_players);
			for (i = 0; i < num_ai_players; ++i)
				add_computer_player(game, TRUE);
			avahi_register_game(game);
		}
	}
	if (disable_game_start || game != NULL) {
		event_loop = g_main_loop_new(NULL, FALSE);
		g_main_loop_run(event_loop);
		g_main_loop_unref(event_loop);
		game_free(game);
		game = NULL;

		avahi_unregister_game();
	}

	net_finish();

	g_free(hostname);
	g_free(server_port);
	g_free(admin_port);
	g_option_context_free(context);
	params_free(params);
	return 0;
}

static gboolean exit_func(G_GNUC_UNUSED gpointer data)
{
	g_main_loop_quit(event_loop);
	return TRUE;
}

void game_is_over(Game * game)
{
	/* quit in ten seconds if configured */
	if (game->params->quit_when_done) {
		g_timeout_add(10 * 1000, &exit_func, NULL);
	}
}

void request_server_stop(Game * game)
{
	if (server_stop(game)) {
		g_main_loop_quit(event_loop);
	}
}
