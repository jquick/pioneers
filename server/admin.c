/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 1999 Dave Cole
 * Copyright (C) 2003, 2006 Bas Wijnen <shevek@fmf.nl>
 * Copyright (C) 2007 Roland Clobus <rclobus@bigfoot.com>
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

/* Pioneers Console Server Adminstrator interface
 *
 * The strings in the admin interface are intentionally not translated.
 * They would otherwise reflect the language of the server that is
 * running the server, instead of the language of the connecting user.
 */
#include "config.h"
#include <stdlib.h>
#include <string.h>

#include "admin.h"
#include "game.h"
#include "server.h"

/* network administration functions */
comm_info *_accept_info = NULL;

gint admin_dice_roll = 0;

typedef enum {
	BADCOMMAND,
	SETPORT,
	STARTSERVER,
	STOPSERVER,
	REGISTERSERVER,
	NUMPLAYERS,
	SEVENSRULE,
	VICTORYPOINTS,
	RANDOMTERRAIN,
	SETGAME,
	QUIT,
	MESSAGE,
	HELP,
	INFO,
	FIXDICE
} AdminCommandType;

typedef struct {
	AdminCommandType type;
	const gchar *command;
	gboolean need_argument;
	gboolean stop_server;
	gboolean need_gameparam;
} AdminCommand;

/* *INDENT-OFF* */
static AdminCommand admin_commands[] = {
	{ BADCOMMAND,     "",                    FALSE, FALSE, FALSE },
	{ SETPORT,        "set-port",            TRUE,  TRUE,  TRUE  },
	{ STARTSERVER,    "start-server",        FALSE, TRUE,  TRUE  },
	{ STOPSERVER,     "stop-server",         FALSE, TRUE,  FALSE },
	{ REGISTERSERVER, "set-register-server", TRUE,  TRUE,  FALSE },
	{ NUMPLAYERS,     "set-num-players",     TRUE,  TRUE,  TRUE  },
	{ SEVENSRULE,     "set-sevens-rule",     TRUE,  TRUE,  TRUE  },
	{ VICTORYPOINTS,  "set-victory-points",  TRUE,  TRUE,  TRUE  },
	{ RANDOMTERRAIN,  "set-random-terrain",  TRUE,  TRUE,  TRUE  },
	{ SETGAME,        "set-game",            TRUE,  TRUE,  FALSE },
	{ QUIT,           "quit",                FALSE, FALSE, FALSE },
	{ MESSAGE,        "send-message",        TRUE,  FALSE, TRUE  },
	{ HELP,           "help",                FALSE, FALSE, FALSE },
	{ INFO,           "info",                FALSE, FALSE, FALSE },
	{ FIXDICE,        "fix-dice",            TRUE,  FALSE, FALSE },
};
/* *INDENT-ON* */

/* parse 'line' and run the command requested */
void admin_run_command(Session * admin_session, const gchar * line)
{
	const gchar *command_start;
	gchar *command;
	gchar *argument;
	gint command_number;

	static gchar *server_port = NULL;
	static gboolean register_server = TRUE;
	static GameParams *params = NULL;
	static Game *game = NULL;

	if (!g_str_has_prefix(line, "admin")) {
		net_printf(admin_session,
			   "no admin prefix in command: '%s'\n", line);
		return;
	}

	line += 5;		/* length of "admin" */
	while (*line && g_ascii_isspace(*line))
		++line;
	if (!*line) {
		net_printf(admin_session, "no command found: '%s'\n",
			   line);
		return;
	}

	/* parse the line down into command and argument */
	command_start = line;
	while (*line && !g_ascii_isspace(*line))
		++line;
	command = g_strndup(command_start, line - command_start);

	if (*line) {
		while (*line && g_ascii_isspace(*line))
			++line;
		argument = g_strdup(line);
	} else {
		argument = NULL;
	}

	/* command[0] is the fall-back */
	for (command_number = 1;
	     command_number < G_N_ELEMENTS(admin_commands);
	     ++command_number) {
		if (!strcmp
		    (command, admin_commands[command_number].command)) {
			break;
		}
	}
	if (command_number == G_N_ELEMENTS(admin_commands)) {
		command_number = 0;
	}
	if (admin_commands[command_number].need_argument
	    && NULL == argument) {
		net_printf(admin_session,
			   "ERROR command '%s' needs an argument\n",
			   command);
	} else if (admin_commands[command_number].need_gameparam
		   && NULL == params) {
		net_printf(admin_session,
			   "ERROR command '%s' needs a valid game\n",
			   command);
	} else {
		if (admin_commands[command_number].stop_server
		    && server_is_running(game)) {
			server_stop(game);
			game_free(game);
			game = NULL;
			net_write(admin_session, "INFO server stopped\n");
		}
		switch (admin_commands[command_number].type) {
		case BADCOMMAND:
			net_printf(admin_session,
				   "ERROR unrecognized command: '%s'\n",
				   command);
			break;
		case SETPORT:
			if (server_port)
				g_free(server_port);
			server_port = g_strdup(argument);
			break;
		case STARTSERVER:
			{
				gchar *meta_server_name =
				    get_meta_server_name(TRUE);
				if (!server_port)
					server_port =
					    g_strdup
					    (PIONEERS_DEFAULT_GAME_PORT);
				if (game != NULL)
					game_free(game);
				game =
				    server_start(params, get_server_name(),
						 server_port,
						 register_server,
						 meta_server_name, TRUE);
				g_free(meta_server_name);
			}
			break;
		case STOPSERVER:
			server_stop(game);
			break;
		case REGISTERSERVER:
			register_server = atoi(argument);
			break;
		case NUMPLAYERS:
			cfg_set_num_players(params, atoi(argument));
			break;
		case SEVENSRULE:
			cfg_set_sevens_rule(params, atoi(argument));
			break;
		case VICTORYPOINTS:
			cfg_set_victory_points(params, atoi(argument));
			break;
		case RANDOMTERRAIN:
			cfg_set_terrain_type(params, atoi(argument));
			break;
		case SETGAME:
			if (params)
				params_free(params);
			params = cfg_set_game(argument);
			if (!params) {
				net_printf(admin_session,
					   "ERROR game '%s' not set\n",
					   argument);
			}
			break;
		case QUIT:
			net_close(admin_session);
			/* Quit the server if the admin leaves */
			if (!server_is_running(game))
				exit(0);
			break;
		case MESSAGE:
			g_strdelimit(argument, "|", '_');
			if (server_is_running(game))
				admin_broadcast(game, argument);
			break;
		case HELP:
			for (command_number = 1;
			     command_number < G_N_ELEMENTS(admin_commands);
			     ++command_number) {
				if (admin_commands
				    [command_number].need_argument) {
					net_printf(admin_session,
						   "INFO %s argument\n",
						   admin_commands
						   [command_number].
						   command);
				} else {
					net_printf(admin_session,
						   "INFO %s\n",
						   admin_commands
						   [command_number].
						   command);
				}
			}
			break;
		case INFO:
			net_printf(admin_session, "INFO server-port %s\n",
				   server_port ? server_port :
				   PIONEERS_DEFAULT_GAME_PORT);
			net_printf(admin_session,
				   "INFO register-server %d\n",
				   register_server);
			net_printf(admin_session,
				   "INFO server running %d\n",
				   server_is_running(game));
			if (params) {
				net_printf(admin_session, "INFO game %s\n",
					   params->title);
				net_printf(admin_session,
					   "INFO players %d\n",
					   params->num_players);
				net_printf(admin_session,
					   "INFO victory-points %d\n",
					   params->victory_points);
				net_printf(admin_session,
					   "INFO random-terrain %d\n",
					   params->random_terrain);
				net_printf(admin_session,
					   "INFO sevens-rule %d\n",
					   params->sevens_rule);
			} else {
				net_printf(admin_session,
					   "INFO no game set\n");
			}
			if (admin_dice_roll != 0)
				net_printf(admin_session,
					   "INFO dice fixed to %d\n",
					   admin_dice_roll);
			break;
		case FIXDICE:
			admin_dice_roll = CLAMP(atoi(argument), 0, 12);
			if (admin_dice_roll == 1)
				admin_dice_roll = 0;
			if (admin_dice_roll != 0)
				net_printf(admin_session,
					   "INFO dice fixed to %d\n",
					   admin_dice_roll);
			else
				net_printf(admin_session,
					   "INFO dice rolled normally\n");
		}
	}
	g_free(command);
	if (argument)
		g_free(argument);
}

/* network event handler, just like the one in meta.c, state.c, etc. */
void admin_event(NetEvent event, Session * admin_session,
		 const gchar * line)
{
#ifdef PRINT_INFO
	g_print
	    ("admin_event: event = %#x, admin_session = %p, line = %s\n",
	     event, admin_session, line);
#endif

	switch (event) {
	case NET_READ:
		/* there is data to be read */

#ifdef PRINT_INFO
		g_print("admin_event: NET_READ: line = '%s'\n", line);
#endif
		admin_run_command(admin_session, line);
		break;
	case NET_CLOSE:
		/* connection has been closed */

#ifdef PRINT_INFO
		g_print("admin_event: NET_CLOSE\n");
#endif
		net_free(&admin_session);
		break;
	case NET_CONNECT:
		/* connect() succeeded -- shouldn't get here */

#ifdef PRINT_INFO
		g_print("admin_event: NET_CONNECT\n");
#endif
		break;
	case NET_CONNECT_FAIL:
		/* connect() failed -- shouldn't get here */

#ifdef PRINT_INFO
		g_print("admin_event: NET_CONNECT_FAIL\n");
#endif
		break;
	default:
		/* To kill a warning... */
		break;
	}
}

/* accept a connection made to the admin port */
void admin_connect(comm_info * admin_info)
{
	Session *admin_session;
	gint new_fd;
	gchar *location;

	/* somebody connected to the administration port, so we... */

	/* (1) create a new network session */
	admin_session = net_new((NetNotifyFunc) admin_event, NULL);

	/* (2) set the session as the session's user data, so we can free it 
	 * later (this way we don't have to keep any globals holding all the 
	 * sessions) */
	admin_session->user_data = admin_session;

	/* (3) accept the connection into a new file descriptor */
	new_fd = accept_connection(admin_info->fd, &location);

	/* (4) tie the new file descriptor to the session we created earlier.
	 * Don't use keepalive pings on this connection.  */
	net_use_fd(admin_session, new_fd, FALSE);
}

/* set up the administration port */
gboolean admin_listen(const gchar * port)
{
	gchar *error_message;

	if (!_accept_info) {
		_accept_info = g_malloc0(sizeof(comm_info));
	}

	/* open up a socket on which to listen for connections */
	_accept_info->fd = net_open_listening_socket(port, &error_message);
	if (_accept_info->fd == -1) {
		log_message(MSG_ERROR, "%s\n", error_message);
		g_free(error_message);
		return FALSE;
	}
#ifdef PRINT_INFO
	g_print("admin_listen: fd = %d\n", _accept_info->fd);
#endif

	/* set up the callback to handle connections */
	_accept_info->read_tag =
	    driver->input_add_read(_accept_info->fd,
				   (InputFunc) admin_connect,
				   _accept_info);
	return TRUE;
}
