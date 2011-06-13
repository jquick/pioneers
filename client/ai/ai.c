/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 1999 Dave Cole
 * Copyright (C) 2003,2006 Bas Wijnen <shevek@fmf.nl>
 * Copyright (C) 2011 Roland Clobus <rclobus@rclobus.nl>
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
#include "game.h"
#include "ai.h"
#include "client.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

static char *server = NULL;
static char *port = NULL;
static char *name = NULL;
static char *ai;
static int waittime = 1000;
static gboolean silent = FALSE;
static gboolean enable_debug = FALSE;
static gboolean show_version = FALSE;
static Map *map = NULL;

static void logbot_init(void);

static const struct algorithm_info {
	/** Name of the algorithm (for commandline) */
	const gchar *name;
	/** Init function */
	void (*init_func) (void);
	/** Request to be a player? */
	gboolean request_player;
	/** Quit if request not honoured */
	gboolean quit_if_not_request;
} algorithms[] = {
/* *INDENT-OFF* */
	{ "greedy", &greedy_init, TRUE, TRUE},
	{ "lobbybot", &lobbybot_init, FALSE, TRUE},
	{ "logbot", &logbot_init, FALSE, TRUE},
/* *INDENT-ON* */
};

static int active_algorithm = 0;

UIDriver Glib_Driver;

static GOptionEntry commandline_entries[] = {
	{"server", 's', 0, G_OPTION_ARG_STRING, &server,
	 /* Commandline pioneersai: server */
	 N_("Server Host"), PIONEERS_DEFAULT_GAME_HOST},
	{"port", 'p', 0, G_OPTION_ARG_STRING, &port,
	 /* Commandline pioneersai: port */
	 N_("Server Port"), PIONEERS_DEFAULT_GAME_PORT},
	{"name", 'n', 0, G_OPTION_ARG_STRING, &name,
	 /* Commandline pioneersai: name */
	 N_("Computer name (mandatory)"), NULL},
	{"time", 't', 0, G_OPTION_ARG_INT, &waittime,
	 /* Commandline pioneersai: time */
	 N_("Time to wait between turns (in milliseconds)"), "1000"},
	{"chat-free", 'c', 0, G_OPTION_ARG_NONE, &silent,
	 /* Commandline pioneersai: chat-free */
	 N_("Stop computer player from talking"), NULL},
	{"algorithm", 'a', 0, G_OPTION_ARG_STRING, &ai,
	 /* Commandline pioneersai: algorithm */
	 N_("Type of computer player"), "greedy"},
	{"debug", '\0', 0, G_OPTION_ARG_NONE, &enable_debug,
	 /* Commandline option of ai: enable debug logging */
	 N_("Enable debug messages"), NULL},
	{"version", '\0', 0, G_OPTION_ARG_NONE, &show_version,
	 /* Commandline option of ai: version */
	 N_("Show version information"), NULL},
	{NULL, '\0', 0, 0, NULL, NULL, NULL}
};

static void ai_init_glib_et_al(int argc, char **argv)
{
	GOptionContext *context;
	GError *error = NULL;

	context =
	    /* Long description in the commandline for pioneersai: help */
	    g_option_context_new(_("- Computer player for Pioneers"));
	g_option_context_add_main_entries(context, commandline_entries,
					  PACKAGE);
	g_option_context_parse(context, &argc, &argv, &error);
	g_option_context_free(context);

	if (error != NULL) {
		g_print("%s\n", error->message);
		g_error_free(error);
		exit(1);
	}
	if (show_version) {
		g_print(_("Pioneers version:"));
		g_print(" ");
		g_print(FULL_VERSION);
		g_print("\n");
		exit(0);
	}

	g_type_init();
	set_ui_driver(&Glib_Driver);
	log_set_func_default();
}

static void ai_init(void)
{
	set_enable_debug(enable_debug);

	if (server == NULL)
		server = g_strdup(PIONEERS_DEFAULT_GAME_HOST);
	if (port == NULL)
		port = g_strdup(PIONEERS_DEFAULT_GAME_PORT);

	printf("ai port is %s\n", port);

	g_random_set_seed(time(NULL) + getpid());

	if (!name) {
		/* ai commandline error */
		g_print(_("A name must be provided.\n"));
		exit(0);
	}

	if (ai != NULL) {
		gint i;
		for (i = 0; i < G_N_ELEMENTS(algorithms); i++) {
			if (!strcmp(algorithms[i].name, ai))
				active_algorithm = i;
		}
	}
	log_message(MSG_INFO, _("Type of computer player: %s\n"),
		    algorithms[active_algorithm].name);
	algorithms[active_algorithm].init_func();
}

void ai_panic(const char *message)
{
	cb_chat(message);
	callbacks.quit();
}

static void ai_offline(void)
{
	gchar *style;

	callbacks.offline = callbacks.quit;
	notifying_string_set(requested_name, name);
	style =
	    g_strdup_printf("ai %s", algorithms[active_algorithm].name);
	notifying_string_set(requested_style, style);
	cb_connect(server, port,
		   !algorithms[active_algorithm].request_player);
	g_free(style);
	g_free(name);
}

static void ai_start_game(void)
{
	if (algorithms[active_algorithm].request_player ==
	    my_player_viewer()
	    && algorithms[active_algorithm].quit_if_not_request) {
		ai_panic(N_("The game is already full. I'm leaving."));
	}
}

void ai_wait(void)
{
	g_usleep(waittime * 1000);
}

void ai_chat(const char *message)
{
	if (!silent)
		cb_chat(message);
}

static Map *ai_get_map(void)
{
	return map;
}

static void ai_set_map(Map * new_map)
{
	map = new_map;
}

void frontend_set_callbacks(void)
{
	callbacks.init_glib_et_al = &ai_init_glib_et_al;
	callbacks.init = &ai_init;
	callbacks.offline = &ai_offline;
	callbacks.start_game = &ai_start_game;
	callbacks.get_map = &ai_get_map;
	callbacks.set_map = &ai_set_map;
}

/* The logbot is intended to be used as a viewer in a game, and to collect
 * a transcript of the game in human readable form, which can be analysed
 * using external tools. */
void logbot_init(void)
{
}
