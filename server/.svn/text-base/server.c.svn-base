/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 1999 Dave Cole
 * Copyright (C) 2003 Bas Wijnen <shevek@fmf.nl>
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
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "server.h"

static GameParams *load_game_desc(const gchar * fname);

static GSList *_game_list = NULL;	/* The list of GameParams, ordered by title */

#define TERRAIN_DEFAULT	0
#define TERRAIN_RANDOM	1

static gboolean timed_out(gpointer data)
{
	Game *game = data;
	log_message(MSG_INFO,
		    _(""
		      "Was hanging around for too long without players... bye.\n"));
	request_server_stop(game);
	return FALSE;
}

void start_timeout(Game * game)
{
	if (!game->no_player_timeout)
		return;
	game->no_player_timer =
	    g_timeout_add(game->no_player_timeout * 1000, timed_out, game);
}

void stop_timeout(Game * game)
{
	if (game->no_player_timer != 0) {
		g_source_remove(game->no_player_timer);
		game->no_player_timer = 0;
	}
}

gint get_rand(gint range)
{
	return g_rand_int_range(g_rand_ctx, 0, range);
}

Game *game_new(const GameParams * params)
{
	Game *game;
	gint idx;

	game = g_malloc0(sizeof(*game));

	game->accept_tag = 0;
	game->accept_fd = -1;
	game->is_running = FALSE;
	game->is_game_over = FALSE;
	game->params = params_copy(params);
	game->curr_player = -1;

	for (idx = 0; idx < G_N_ELEMENTS(game->bank_deck); idx++)
		game->bank_deck[idx] = game->params->resource_count;
	develop_shuffle(game);
	if (params->random_terrain)
		map_shuffle_terrain(game->params->map);

	return game;
}

void game_free(Game * game)
{
	if (game == NULL)
		return;

	server_stop(game);

	g_assert(game->player_list_use_count == 0);
	if (game->server_port != NULL)
		g_free(game->server_port);
	params_free(game->params);
	g_free(game);
}

gint accept_connection(gint in_fd, gchar ** location)
{
	int fd;
	gchar *error_message;
	gchar *port;

	fd = net_accept(in_fd, &error_message);
	if (fd < 0) {
		log_message(MSG_ERROR, "%s\n", error_message);
		g_free(error_message);
		return -1;
	}

	g_assert(location != NULL);
	if (!net_get_peer_name(fd, location, &port, &error_message)) {
		log_message(MSG_ERROR, "%s\n", error_message);
		g_free(error_message);
	}
	g_free(port);
	return fd;
}

gint add_computer_player(Game * game, gboolean want_chat)
{
	gchar *child_argv[10];
	GError *error = NULL;
	gint ret = 0;
	gint n = 0;
	gint i;

	child_argv[n++] = g_strdup(PIONEERS_AI_PATH);
	child_argv[n++] = g_strdup(PIONEERS_AI_PATH);
	child_argv[n++] = g_strdup("-s");
	child_argv[n++] = g_strdup(PIONEERS_DEFAULT_GAME_HOST);
	child_argv[n++] = g_strdup("-p");
	child_argv[n++] = g_strdup(game->server_port);
	child_argv[n++] = g_strdup("-n");
	child_argv[n++] = player_new_computer_player(game);
	if (!want_chat)
		child_argv[n++] = g_strdup("-c");
	child_argv[n] = NULL;
	g_assert(n < 10);

	if (!g_spawn_async(NULL, child_argv, NULL, 0, NULL, NULL,
			   NULL, &error)) {
		log_message(MSG_ERROR,
			    _("Error starting %s: %s"),
			    PIONEERS_AI_PATH, error->message);
		g_error_free(error);
		ret = -1;
	}
	for (i = 0; child_argv[i] != NULL; i++)
		g_free(child_argv[i]);
	return ret;
}


static void player_connect(Game * game)
{
	gchar *location;
	gint fd = accept_connection(game->accept_fd, &location);

	if (fd > 0) {
		if (player_new_connection(game, fd, location) != NULL)
			stop_timeout(game);
	}
	g_free(location);
}

static gboolean game_server_start(Game * game, gboolean register_server,
				  const gchar * meta_server_name)
{
	gchar *error_message;

	game->accept_fd =
	    net_open_listening_socket(game->server_port, &error_message);
	if (game->accept_fd == -1) {
		log_message(MSG_ERROR, "%s\n", error_message);
		g_free(error_message);
		return FALSE;
	}
	game->is_running = TRUE;

	start_timeout(game);

	game->accept_tag = driver->input_add_read(game->accept_fd,
						  (InputFunc)
						  player_connect, game);

	if (register_server) {
		g_assert(meta_server_name != NULL);
		meta_register(meta_server_name, PIONEERS_DEFAULT_META_PORT,
			      game);
	}
	return TRUE;
}

/** Try to start a new server.
 * @param params The parameters of the game
 * @param hostname The hostname that will be visible in the meta server
 * @param port The port to listen to
 * @param register_server Register at the meta server
 * @param meta_server_name The hostname of the meta server
 * @param random_order Randomize the player number
 * @return A pointer to the new game, or NULL
*/
Game *server_start(const GameParams * params, const gchar * hostname,
		   const gchar * port, gboolean register_server,
		   const gchar * meta_server_name, gboolean random_order)
{
	Game *game;
	guint32 randomseed = time(NULL);

	g_return_val_if_fail(params != NULL, NULL);
	g_return_val_if_fail(port != NULL, NULL);

#ifdef PRINT_INFO
	g_print("game type: %s\n", params->title);
	g_print("num players: %d\n", params->num_players);
	g_print("victory points: %d\n", params->victory_points);
	g_print("terrain type: %s\n",
		(params->random_terrain) ? "random" : "default");
	g_print("Tournament time: %d\n", params->tournament_time);
	g_print("Quit when done: %d\n", params->quit_when_done);
#endif

	g_rand_ctx = g_rand_new_with_seed(randomseed);
	log_message(MSG_INFO, "%s #%" G_GUINT32_FORMAT ".%s.%03d\n",
		    /* Server: preparing game #..... */
		    _("Preparing game"), randomseed, "G", get_rand(1000));

	game = game_new(params);
	g_assert(game->server_port == NULL);
	game->server_port = g_strdup(port);
	g_assert(game->hostname == NULL);
	if (hostname && strlen(hostname) > 0) {
		game->hostname = g_strdup(hostname);
	}
	game->random_order = random_order;
	if (!game_server_start(game, register_server, meta_server_name)) {
		game_free(game);
		game = NULL;
	}
	return game;
}

/** Stop the server.
 * @param game A game
 * @return TRUE if the game changed from running to stopped
*/
gboolean server_stop(Game * game)
{
	GList *current;

	if (!server_is_running(game))
		return FALSE;

	meta_unregister();

	game->is_running = FALSE;
	if (game->accept_tag) {
		driver->input_remove(game->accept_tag);
		game->accept_tag = 0;
	}
	if (game->accept_fd >= 0) {
		close(game->accept_fd);
		game->accept_fd = -1;
	}

	playerlist_inc_use_count(game);
	current = game->player_list;
	while (current != NULL) {
		Player *player = current->data;
		player_remove(player);
		player_free(player);
		current = g_list_next(current);
	}
	playerlist_dec_use_count(game);

	return TRUE;
}

/** Return true if a game is running */
gboolean server_is_running(Game * game)
{
	if (game != NULL)
		return game->is_running;
	return FALSE;
}

static gint sort_function(gconstpointer a, gconstpointer b)
{
	return (strcmp(((const GameParams *) a)->title,
		       ((const GameParams *) b)->title));
}

static gboolean game_list_add_item(GameParams * item)
{
	/* check for name collisions */
	if (item->title && game_list_find_item(item->title)) {

		gchar *nt;
		gint i;

		/* append a number */
		for (i = 1; i <= INT_MAX; i++) {
			nt = g_strdup_printf("%s%d", item->title, i);
			if (!game_list_find_item(nt)) {
				g_free(item->title);
				item->title = nt;
				break;
			}
			g_free(nt);
		}
		/* give up and skip this game */
		if (item->title != nt) {
			g_free(nt);
			return FALSE;
		}
	}

	_game_list =
	    g_slist_insert_sorted(_game_list, item, sort_function);
	return TRUE;
}

/** Returns TRUE if the game list is empty */
static gboolean game_list_is_empty(void)
{
	return _game_list == NULL;
}

static gint game_list_locate(gconstpointer param, gconstpointer argument)
{
	const GameParams *data = param;
	const gchar *title = argument;
	return strcmp(data->title, title);
}

const GameParams *game_list_find_item(const gchar * title)
{
	GSList *result;
	if (!_game_list) {
		return NULL;
	}

	result = g_slist_find_custom(_game_list, title, game_list_locate);
	if (result)
		return result->data;
	else
		return NULL;
}

void game_list_foreach(GFunc func, gpointer user_data)
{
	if (_game_list) {
		g_slist_foreach(_game_list, func, user_data);
	}
}

GameParams *load_game_desc(const gchar * fname)
{
	GameParams *params;

	params = params_load_file(fname);
	if (params == NULL)
		g_warning("Skipping: %s", fname);
	return params;
}

static void game_list_prepare_directory(const gchar * directory)
{
	GDir *dir;
	const gchar *fname;
	gchar *fullname;

	log_message(MSG_INFO, _("Looking for games in '%s'\n"), directory);
	if ((dir = g_dir_open(directory, 0, NULL)) == NULL) {
		log_message(MSG_INFO, _("Game directory '%s' not found\n"),
			    directory);
		return;
	}

	while ((fname = g_dir_read_name(dir))) {
		GameParams *params;
		gint len = strlen(fname);

		if (len < 6 || strcmp(fname + len - 5, ".game") != 0)
			continue;
		fullname = g_build_filename(directory, fname, NULL);
		params = load_game_desc(fullname);
		g_free(fullname);
		if (params) {
			if (!game_list_add_item(params))
				params_free(params);
		}
	}
	g_dir_close(dir);
}

void game_list_prepare(void)
{
	gchar *directory;

	directory =
	    g_build_filename(g_get_user_data_dir(), "pioneers", NULL);
	game_list_prepare_directory(directory);
	g_free(directory);

	game_list_prepare_directory(get_pioneers_dir());

	if (game_list_is_empty())
		g_error("No games available");
}

void game_list_cleanup(void)
{
	GSList *games = _game_list;
	while (games) {
		params_free(games->data);
		games = g_slist_next(games);
	}
	g_slist_free(_game_list);
}

/* game configuration functions / callbacks */
void cfg_set_num_players(GameParams * params, gint num_players)
{
#ifdef PRINT_INFO
	g_print("cfg_set_num_players: %d\n", num_players);
#endif
	g_return_if_fail(params != NULL);
	params->num_players = CLAMP(num_players, 2, MAX_PLAYERS);
}

void cfg_set_sevens_rule(GameParams * params, gint sevens_rule)
{
#ifdef PRINT_INFO
	g_print("cfg_set_sevens_rule: %d\n", sevens_rule);
#endif
	g_return_if_fail(params != NULL);
	params->sevens_rule = CLAMP(sevens_rule, 0, 2);
}

void cfg_set_victory_points(GameParams * params, gint victory_points)
{
#ifdef PRINT_INFO
	g_print("cfg_set_victory_points: %d\n", victory_points);
#endif
	g_return_if_fail(params != NULL);
	params->victory_points = MAX(3, victory_points);
}

/** Attempt to find a game with @a title in @a directory.
 *  @param title The game must match this title
 *  @param directory Look in this directory for *.game files
 *  @return The GameParams, or NULL if the title was not found
 */
static GameParams *server_find_title_in_directory(const gchar * title,
						  const gchar * directory)
{
	GDir *dir;
	const gchar *fname;
	gchar *fullname;

	log_message(MSG_INFO, _("Looking for games in '%s'\n"), directory);
	if ((dir = g_dir_open(directory, 0, NULL)) == NULL) {
		log_message(MSG_INFO, _("Game directory '%s' not found\n"),
			    directory);
		return NULL;
	}

	while ((fname = g_dir_read_name(dir))) {
		GameParams *params;
		gint len = strlen(fname);

		if (len < 6 || strcmp(fname + len - 5, ".game") != 0)
			continue;
		fullname = g_build_filename(directory, fname, NULL);
		params = load_game_desc(fullname);
		g_free(fullname);
		if (params) {
			if (strcmp(params->title, title) == 0) {
				g_dir_close(dir);
				return params;
			}
			params_free(params);
		}
	}
	g_dir_close(dir);
	return NULL;
}

/** Find a game with @a title in the default locations.
 *  @param title The title of the game_new
 *  @return The GameParams or NULL if the title was not found
 */
static GameParams *server_find_title(const gchar * title)
{
	GameParams *result;
	gchar *directory;

	directory =
	    g_build_filename(g_get_user_data_dir(), "pioneers", NULL);
	result = server_find_title_in_directory(title, directory);
	g_free(directory);

	if (result == NULL) {
		result =
		    server_find_title_in_directory(title,
						   get_pioneers_dir());
	}
	return result;
}

GameParams *cfg_set_game(const gchar * game)
{
#ifdef PRINT_INFO
	g_print("cfg_set_game: %s\n", game);
#endif
	if (game_list_is_empty()) {
		return server_find_title(game);
	} else {
		return params_copy(game_list_find_item(game));
	}
}

GameParams *cfg_set_game_file(const gchar * game_filename)
{
#ifdef PRINT_INFO
	g_print("cfg_set_game_file: %s\n", game_filename);
#endif
	return params_load_file(game_filename);
}

void cfg_set_terrain_type(GameParams * params, gint terrain_type)
{
#ifdef PRINT_INFO
	g_print("cfg_set_terrain_type: %d\n", terrain_type);
#endif
	g_return_if_fail(params != NULL);
	params->random_terrain = (terrain_type == TERRAIN_RANDOM) ? 1 : 0;
}

void cfg_set_tournament_time(GameParams * params, gint tournament_time)
{
#ifdef PRINT_INFO
	g_print("cfg_set_tournament_time: %d\n", tournament_time);
#endif
	g_return_if_fail(params != NULL);
	params->tournament_time = tournament_time;
}

void cfg_set_quit(GameParams * params, gboolean quitdone)
{
#ifdef PRINT_INFO
	g_print("cfg_set_quit: %d\n", quitdone);
#endif
	g_return_if_fail(params != NULL);
	params->quit_when_done = quitdone;
}

void admin_broadcast(Game * game, const gchar * message)
{
	/* The message that is sent must not be translated */
	player_broadcast(player_none(game), PB_SILENT, FIRST_VERSION,
			 LATEST_VERSION, "NOTE1 %s|%s\n", message, "%s");
}

/* server initialization */
void server_init(void)
{
	/* Broken pipes can happen when multiple players disconnect
	 * simultaneously.  This mostly happens to AI's, which disconnect
	 * when the game is over. */
	/* SIGPIPE does not exist for G_OS_WIN32 */
#ifndef G_OS_WIN32
	struct sigaction sa;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);
#endif				/* G_OS_WIN32 */
}
