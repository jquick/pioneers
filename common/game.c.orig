/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 1999 Dave Cole
 * Copyright (C) 2003 Bas Wijnen <shevek@fmf.nl>
 * Copyright (C) 2006 Roland Clobus <rclobus@bigfoot.com>
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
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <glib.h>
#include <stdio.h>

#include "game.h"
#include "cards.h"

const gchar *default_player_style = "square";

typedef enum {
	PARAM_STRING,
	PARAM_INT,
	PARAM_BOOL,
	PARAM_INTLIST
} ParamType;

typedef struct {
	const gchar *name;
	ParamType type;
	int offset;
} Param;

#define PARAM_V(name, type, var) #name, type, G_STRUCT_OFFSET(GameParams, var)
#define PARAM(var, type) PARAM_V(var, type, var)

/* *INDENT-OFF* */
static Param game_params[] = {
	{PARAM(title, PARAM_STRING)},
	{PARAM_V(random-terrain, PARAM_BOOL, random_terrain)},
	{PARAM_V(strict-trade, PARAM_BOOL, strict_trade)},
	{PARAM_V(domestic-trade, PARAM_BOOL, domestic_trade)},
	{PARAM_V(num-players, PARAM_INT, num_players)},
	{PARAM_V(sevens-rule, PARAM_INT, sevens_rule)},
	{PARAM_V(victory-points, PARAM_INT, victory_points)},
	{PARAM_V(check-victory-at-end-of-turn, PARAM_BOOL, check_victory_at_end_of_turn)},
	{PARAM_V(use-cities-and-knights-rules, PARAM_BOOL, use_cities_and_knights_rules)},
	{PARAM_V(num-roads, PARAM_INT, num_build_type[BUILD_ROAD])},
	{PARAM_V(num-bridges, PARAM_INT, num_build_type[BUILD_BRIDGE])},
	{PARAM_V(num-ships, PARAM_INT, num_build_type[BUILD_SHIP])},
	{PARAM_V(num-settlements, PARAM_INT, num_build_type[BUILD_SETTLEMENT])},
	{PARAM_V(num-cities, PARAM_INT, num_build_type[BUILD_CITY])},
	{PARAM_V(num-city-walls, PARAM_INT, num_build_type[BUILD_CITY_WALL])},
	{PARAM_V(resource-count, PARAM_INT, resource_count)},
	{PARAM_V(develop-road, PARAM_INT, num_develop_type[DEVEL_ROAD_BUILDING])},
	{PARAM_V(develop-monopoly, PARAM_INT, num_develop_type[DEVEL_MONOPOLY])},
	{PARAM_V(develop-plenty, PARAM_INT, num_develop_type[DEVEL_YEAR_OF_PLENTY])},
	{PARAM_V(develop-chapel, PARAM_INT, num_develop_type[DEVEL_CHAPEL])},
	{PARAM_V(develop-university, PARAM_INT, num_develop_type[DEVEL_UNIVERSITY])},
	{PARAM_V(develop-governor, PARAM_INT, num_develop_type[DEVEL_GOVERNORS_HOUSE])},
	{PARAM_V(develop-library, PARAM_INT, num_develop_type[DEVEL_LIBRARY])},
	{PARAM_V(develop-market, PARAM_INT, num_develop_type[DEVEL_MARKET])},
	{PARAM_V(develop-soldier, PARAM_INT, num_develop_type[DEVEL_SOLDIER])},
	{PARAM_V(use-pirate, PARAM_BOOL, use_pirate)},
	{PARAM_V(island-discovery-bonus, PARAM_INTLIST, island_discovery_bonus)}
};
/* *INDENT-ON* */

GameParams *params_new(void)
{
	GameParams *params;

	params = g_malloc0(sizeof(*params));
	return params;
}

void params_free(GameParams * params)
{
	if (params == NULL)
		return;

	if (params->title != NULL)
		g_free(params->title);
	if (params->map != NULL)
		map_free(params->map);
	g_free(params);
}

static gchar *skip_space(gchar * str)
{
	while (isspace(*str))
		str++;
	return str;
}

static gboolean match_word(gchar ** str, const gchar * word)
{
	gint word_len;

	word_len = strlen(word);
	if (strncmp(*str, word, word_len) == 0) {
		*str += word_len;
		*str = skip_space(*str);
		return TRUE;
	}
	return FALSE;
}

/* Create a new array, filled with the list in str.
 * If the array has length zero, NULL is returned.
 */
static GArray *build_int_list(const gchar * str)
{
	GArray *array = g_array_new(FALSE, FALSE, sizeof(gint));
	while (*str != '\0') {
		gint num;
		gint sign = +1;

		/* Skip leading space */
		while (isspace(*str))
			str++;
		if (*str == '\0')
			break;
		/* Get the next number and add it to the array */
		num = 0;
		if (*str == '-') {
			sign = -1;
			str++;
		}
		while (isdigit(*str))
			num = num * 10 + *str++ - '0';
		num *= sign;
		g_array_append_val(array, num);

		/* Skip the non-digits */
		while (!isdigit(*str) && *str != '-' && *str != '\0')
			str++;
	}
	if (array->len == 0) {
		g_array_free(array, FALSE);
		array = NULL;
	}
	return array;
}

/* This function allocates a buffer and returns it.  It must be freed by the
 * caller.  */
static gchar *format_int_list(const gchar * name, GArray * array)
{
	gchar *old, *str = NULL;
	int idx;

	if (array == NULL)
		return NULL;

	if (array->len == 0) {
		str = g_strdup(name);
		return str;
	}
	for (idx = 0; idx < array->len; idx++) {
		old = str;
		if (idx == 0)
			str = g_strdup_printf("%s %d", name,
					      g_array_index(array, gint,
							    idx));
		else
			str = g_strdup_printf("%s,%d", str,
					      g_array_index(array, gint,
							    idx));
		if (old)
			g_free(old);
	}
	return str;
}

struct nosetup_t {
	WriteLineFunc func;
	gpointer user_data;
};

static gboolean find_no_setup(const Hex * hex, gpointer closure)
{
	gint idx;
	struct nosetup_t *data = closure;
	for (idx = 0; idx < G_N_ELEMENTS(hex->nodes); ++idx) {
		const Node *node = hex->nodes[idx];
		if (node->no_setup) {
			gchar buff[50];
			if (node->x != hex->x || node->y != hex->y)
				continue;
			snprintf(buff, sizeof(buff), "nosetup %d %d %d",
				 node->x, node->y, node->pos);
			data->func(data->user_data, buff);
		}
	}
	return FALSE;
}

void params_write_lines(GameParams * params, gboolean write_secrets,
			WriteLineFunc func, gpointer user_data)
{
	gint idx;
	gint y;
	gchar *buff;
	gchar *str;

	switch (params->variant) {
	case VAR_DEFAULT:
		func(user_data, "variant default");
		break;
	case VAR_ISLANDS:
		func(user_data, "variant islands");
		break;
	case VAR_INTIMATE:
		func(user_data, "variant intimate");
		break;
	}
	for (idx = 0; idx < G_N_ELEMENTS(game_params); idx++) {
		Param *param = game_params + idx;

		switch (param->type) {
		case PARAM_STRING:
			str =
			    G_STRUCT_MEMBER(gchar *, params,
					    param->offset);
			if (!str)
				continue;
			buff = g_strdup_printf("%s %s", param->name, str);
			func(user_data, buff);
			g_free(buff);
			break;
		case PARAM_INT:
			buff = g_strdup_printf("%s %d", param->name,
					       G_STRUCT_MEMBER(gint,
							       params,
							       param->
							       offset));
			func(user_data, buff);
			g_free(buff);
			break;
		case PARAM_BOOL:
			if (G_STRUCT_MEMBER
			    (gboolean, params, param->offset)) {
				func(user_data, param->name);
			}
			break;
		case PARAM_INTLIST:
			buff =
			    format_int_list(param->name,
					    G_STRUCT_MEMBER(GArray *,
							    params,
							    param->
							    offset));
			/* Don't send empty intlists */
			if (buff != NULL) {
				func(user_data, buff);
				g_free(buff);
			}
			break;
		}
	}
	buff = format_int_list("chits", params->map->chits);
	func(user_data, buff);
	g_free(buff);
	func(user_data, "map");
	for (y = 0; y < params->map->y_size; y++) {
		buff = map_format_line(params->map, write_secrets, y);
		func(user_data, buff);
		g_free(buff);
	}
	func(user_data, ".");
	if (params->map) {
		struct nosetup_t tmp;
		tmp.user_data = user_data;
		tmp.func = func;
		map_traverse_const(params->map, find_no_setup, &tmp);
	}
}

gboolean params_load_line(GameParams * params, gchar * line)
{
	gint idx;

	if (params->map == NULL)
		params->map = map_new();
	if (params->parsing_map) {
		if (strcmp(line, ".") == 0) {
			params->parsing_map = FALSE;
			if (!map_parse_finish(params->map)) {
				map_free(params->map);
				params->map = NULL;
				return FALSE;
			}
		} else
			return map_parse_line(params->map, line);
		return TRUE;
	}
	line = skip_space(line);
	if (*line == '#')
		return TRUE;
	if (*line == 0)
		return TRUE;
	if (match_word(&line, "variant")) {
		if (match_word(&line, "islands"))
			params->variant = VAR_ISLANDS;
		else if (match_word(&line, "intimate"))
			params->variant = VAR_INTIMATE;
		else
			params->variant = VAR_DEFAULT;
		return TRUE;
	}
	if (match_word(&line, "map")) {
		params->parsing_map = TRUE;
		return TRUE;
	}
	if (match_word(&line, "chits")) {
		if (params->map->chits != NULL)
			g_array_free(params->map->chits, TRUE);
		params->map->chits = build_int_list(line);
		if (params->map->chits == NULL) {
			g_warning("Zero length chits array");
			return FALSE;
		}
		return TRUE;
	}
	if (match_word(&line, "nosetup")) {
		gint x = 0, y = 0, pos = 0;
		Node *node;
		/* don't tolerate invalid game descriptions */
		g_assert(params->map != NULL);
		sscanf(line, "%d %d %d", &x, &y, &pos);
		node = map_node(params->map, x, y, pos);
		if (node) {
			node->no_setup = TRUE;
		} else {
			g_warning
			    ("Nosetup node %d %d %d is not in the map", x,
			     y, pos);
		}
		return TRUE;
	}

	for (idx = 0; idx < G_N_ELEMENTS(game_params); idx++) {
		Param *param = game_params + idx;
		gchar *str;
		GArray *array;

		if (!match_word(&line, param->name))
			continue;
		switch (param->type) {
		case PARAM_STRING:
			str =
			    G_STRUCT_MEMBER(gchar *, params,
					    param->offset);
			if (str)
				g_free(str);
			str = g_strchomp(g_strdup(line));
			G_STRUCT_MEMBER(gchar *, params, param->offset) =
			    str;
			return TRUE;
		case PARAM_INT:
			G_STRUCT_MEMBER(gint, params, param->offset) =
			    atoi(line);
			return TRUE;
		case PARAM_BOOL:
			G_STRUCT_MEMBER(gboolean, params, param->offset) =
			    TRUE;
			return TRUE;
		case PARAM_INTLIST:
			array =
			    G_STRUCT_MEMBER(GArray *, params,
					    param->offset);
			if (array != NULL)
				g_array_free(array, TRUE);
			array = build_int_list(line);
			if (array == NULL) {
				g_warning("Zero length array for %s",
					  param->name);
			}
			G_STRUCT_MEMBER(GArray *, params, param->offset) =
			    array;
			return array != NULL;
		}
	}
	g_warning("Unknown keyword: %s", line);
	return FALSE;
}

/* read a line from a file.  The memory needed is allocated.  The returned line
 * is unbounded.  Returns FALSE if no (partial) line could be read */
gboolean read_line_from_file(gchar ** line, FILE * f)
{
	gchar part[512];
	gint len;

	if (fgets(part, sizeof(part), f) == NULL)
		return FALSE;

	len = strlen(part);
	g_assert(len > 0);
	*line = g_strdup(part);
	while ((*line)[len - 1] != '\n') {
		gchar *oldline;
		if (fgets(part, sizeof(part), f) == NULL)
			break;
		oldline = *line;
		*line = g_strdup_printf("%s%s", *line, part);
		g_free(oldline);
		len = strlen(*line);
	}
	/* In case of error or EOF, just return the part we have.
	 * Otherwise, strip the newline.  */
	if ((*line)[len - 1] == '\n')
		(*line)[len - 1] = '\0';
	return TRUE;
}

GameParams *params_load_file(const gchar * fname)
{
	FILE *fp;
	gchar *line;
	GameParams *params;

	if ((fp = fopen(fname, "r")) == NULL) {
		g_warning("could not open '%s'", fname);
		return NULL;
	}

	params = params_new();
	while (read_line_from_file(&line, fp) && params) {
		if (!params_load_line(params, line)) {
			params_free(params);
			params = NULL;
		}
		g_free(line);
	}
	fclose(fp);
	if (params && !params_load_finish(params)) {
		params_free(params);
		return NULL;
	}
	return params;
}

GameParams *params_copy(const GameParams * params)
{
	/* Copy the const parameter to a non-const version, because
	 * G_STRUCT_MEMBER doesn't want const values.  Note that this
	 * variable does not own its pointers, that is, they don't have to
	 * be freed when it goes out of scope. */
	GameParams nonconst;
	GameParams *copy;
	gint idx;
	gchar *buff;

	if (params == NULL)
		return NULL;

	memcpy(&nonconst, params, sizeof(GameParams));
	copy = params_new();
	copy->map = map_copy(params->map);
	copy->variant = params->variant;

	for (idx = 0; idx < G_N_ELEMENTS(game_params); idx++) {
		Param *param = game_params + idx;

		switch (param->type) {
		case PARAM_STRING:
			G_STRUCT_MEMBER(gchar *, copy, param->offset)
			    =
			    g_strdup(G_STRUCT_MEMBER
				     (gchar *, &nonconst, param->offset));
			break;
		case PARAM_INT:
			G_STRUCT_MEMBER(gint, copy, param->offset)
			    = G_STRUCT_MEMBER(gint, &nonconst,
					      param->offset);
			break;
		case PARAM_BOOL:
			G_STRUCT_MEMBER(gboolean, copy, param->offset)
			    = G_STRUCT_MEMBER(gboolean, &nonconst,
					      param->offset);
			break;
		case PARAM_INTLIST:
			buff =
			    format_int_list("",
					    G_STRUCT_MEMBER(GArray *,
							    &nonconst,
							    param->
							    offset));
			if (buff != NULL) {
				G_STRUCT_MEMBER(GArray *, copy,
						param->offset) =
				    build_int_list(buff);
				g_free(buff);
			}
			break;
		}
	}

	copy->quit_when_done = params->quit_when_done;
	copy->tournament_time = params->tournament_time;
	return copy;
}

/** Returns TRUE if the params are valid */
gboolean params_load_finish(GameParams * params)
{
	if (!params->map) {
		g_warning("Missing map");
		return FALSE;
	}
	if (params->parsing_map) {
		g_warning("Map not complete. Missing . after the map?");
		return FALSE;
	}
	if (!params->map->chits) {
		g_warning("No chits defined");
		return FALSE;
	}
	if (params->map->chits->len < 1) {
		g_warning("At least one chit must be defined");
		return FALSE;
	}
	if (!params->title) {
		g_warning("Game has no title");
		return FALSE;
	}
	params->map->have_bridges =
	    params->num_build_type[BUILD_BRIDGE] > 0;
	params->map->has_pirate = params->use_pirate;
	return TRUE;
}

static void write_one_line(gpointer user_data, const gchar * line)
{
	FILE *fp = user_data;
	fprintf(fp, "%s\n", line);
}

gboolean params_write_file(GameParams * params, const gchar * fname)
{
	FILE *fp;

	if ((fp = fopen(fname, "w")) == NULL) {
		g_warning("could not open '%s'", fname);
		return FALSE;
	}
	params_write_lines(params, TRUE, write_one_line, fp);

	fclose(fp);

	return TRUE;
}

WinnableState params_check_winnable_state(const GameParams * params,
					  gchar ** win_message,
					  gchar ** point_specification)
{
	gint target, building, development;
	gint road, army;
	gint idx;
	WinnableState return_value;
	gint total_island, max_island;
	guint number_of_islands;

	if (params == NULL) {
		*win_message = g_strdup("Error: no GameParams provided");
		*point_specification = g_strdup("");
		return PARAMS_NO_WIN;
	}

	/* Check whether the game is winnable at all */
	target = params->victory_points;
	building =
	    params->num_build_type[BUILD_SETTLEMENT] +
	    (params->num_build_type[BUILD_SETTLEMENT] > 0 ?
	     params->num_build_type[BUILD_CITY] * 2 : 0);
	road =
	    (params->num_build_type[BUILD_ROAD] +
	     params->num_build_type[BUILD_SHIP] +
	     params->num_build_type[BUILD_BRIDGE]) >= 5 ? 2 : 0;
	army = params->num_develop_type[DEVEL_SOLDIER] >= 3 ? 2 : 0;
	development = 0;
	for (idx = 0; idx < NUM_DEVEL_TYPES; idx++) {
		if (is_victory_card(idx))
			development += params->num_develop_type[idx];
	}

	number_of_islands = map_count_islands(params->map);
	if (number_of_islands == 0) {
		*win_message = g_strdup(_("This game cannot be won."));
		*point_specification = g_strdup(_("There is no land."));
		return PARAMS_NO_WIN;
	}

	/* It is not guaranteed that the islands can be reached */
	total_island = 0;
	max_island = 0;
	if (params->island_discovery_bonus != NULL
	    && params->island_discovery_bonus->len > 0
	    && (params->num_build_type[BUILD_SHIP] +
		params->num_build_type[BUILD_BRIDGE] > 0)) {
		gint i;
		for (i = 0; i < number_of_islands - 1; i++) {
			total_island +=
			    g_array_index(params->island_discovery_bonus,
					  gint,
					  MIN
					  (params->
					   island_discovery_bonus->len - 1,
					   i));
			/* The island score can be negative */
			if (max_island < total_island)
				max_island = total_island;
		}
	}

	if (target > building) {
		if (target >
		    building + development + road + army + max_island) {
			*win_message =
			    g_strdup(_("This game cannot be won."));
			return_value = PARAMS_NO_WIN;
		} else {
			*win_message =
			    g_strdup(_(""
				       "It is possible that this "
				       "game cannot be won."));
			return_value = PARAMS_WIN_PERHAPS;
		}
	} else {
		*win_message = g_strdup(_("This game can be won by only "
					  "building all settlements and "
					  "cities."));
		return_value = PARAMS_WIN_BUILD_ALL;
	}
	*point_specification =
	    g_strdup_printf(_(""
			      "Required victory points: %d\n"
			      "Points obtained by building all: %d\n"
			      "Points in development cards: %d\n"
			      "Longest road/largest army: %d+%d\n"
			      "Maximum island discovery bonus: %d\n"
			      "Total: %d"), target, building, development,
			    road, army, max_island,
			    building + development + road + army +
			    max_island);
	return return_value;
}

PlayerType determine_player_type(const gchar * style)
{
	gchar **style_parts;
	PlayerType type;

	if (style == NULL)
		return PLAYER_UNKNOWN;

	style_parts = g_strsplit(style, " ", 0);
	if (!strcmp(style_parts[0], "ai")) {
		type = PLAYER_COMPUTER;
	} else if (!strcmp(style_parts[0], "human")
		   || !strcmp(style, default_player_style)) {
		type = PLAYER_HUMAN;
	} else {
		type = PLAYER_UNKNOWN;
	}
	g_strfreev(style_parts);
	return type;
}

Points *points_new(gint id, const gchar * name, gint points)
{
	Points *p = g_malloc0(sizeof(Points));
	p->id = id;
	p->name = g_strdup(name);
	p->points = points;
	return p;
}

void points_free(Points * points)
{
	g_free(points->name);
}

/* Not translated, these strings are parts of the communication protocol */
static const gchar *resource_types[] = {
	"brick",
	"grain",
	"ore",
	"wool",
	"lumber"
};

static gint get_num(const gchar * str, gint * num)
{
	gint len = 0;
	gboolean is_negative = FALSE;

	if (*str == '-') {
		is_negative = TRUE;
		str++;
		len++;
	}
	*num = 0;
	while (isdigit(*str)) {
		*num = *num * 10 + *str++ - '0';
		len++;
	}
	if (is_negative)
		*num = -*num;
	return len;
}

gint game_scanf(const gchar * line, const gchar * fmt, ...)
{
	va_list ap;
	gint offset;

	va_start(ap, fmt);
	offset = game_vscanf(line, fmt, ap);
	va_end(ap);

	return offset;
}

gint game_vscanf(const gchar * line, const gchar * fmt, va_list ap)
{
	gint offset = 0;

	while (*fmt != '\0' && line[offset] != '\0') {
		gchar **str;
		gint *num;
		gint idx;
		gint len;
		BuildType *build_type;
		Resource *resource;

		if (*fmt != '%') {
			if (line[offset] != *fmt)
				return -1;
			fmt++;
			offset++;
			continue;
		}
		fmt++;

		switch (*fmt++) {
		case 'S':	/* string from current position to end of line */
			str = va_arg(ap, gchar **);
			*str = g_strdup(line + offset);
			offset += strlen(*str);
			break;
		case 'd':	/* integer */
			num = va_arg(ap, gint *);
			len = get_num(line + offset, num);
			if (len == 0)
				return -1;
			offset += len;
			break;
		case 'B':	/* build type */
			build_type = va_arg(ap, BuildType *);
			if (strncmp(line + offset, "road", 4) == 0) {
				*build_type = BUILD_ROAD;
				offset += 4;
			} else if (strncmp(line + offset, "bridge", 6) ==
				   0) {
				*build_type = BUILD_BRIDGE;
				offset += 6;
			} else if (strncmp(line + offset, "ship", 4) == 0) {
				*build_type = BUILD_SHIP;
				offset += 4;
			} else if (strncmp(line + offset, "settlement", 10)
				   == 0) {
				*build_type = BUILD_SETTLEMENT;
				offset += 10;
			} else if (strncmp(line + offset, "city_wall", 9)
				   == 0) {
				*build_type = BUILD_CITY_WALL;
				offset += 9;
			} else if (strncmp(line + offset, "city", 4) == 0) {
				*build_type = BUILD_CITY;
				offset += 4;
			} else
				return -1;
			break;
		case 'R':	/* list of 5 integer resource counts */
			num = va_arg(ap, gint *);
			for (idx = 0; idx < NO_RESOURCE; idx++) {
				while (line[offset] == ' ')
					offset++;
				len = get_num(line + offset, num);
				if (len == 0)
					return -1;
				offset += len;
				num++;
			}
			break;
		case 'D':	/* development card type */
			num = va_arg(ap, gint *);
			len = get_num(line + offset, num);
			if (len == 0)
				return -1;
			offset += len;
			break;
		case 'r':	/* resource type */
			resource = va_arg(ap, Resource *);
			for (idx = 0; idx < NO_RESOURCE; idx++) {
				const gchar *type = resource_types[idx];
				len = strlen(type);
				if (strncmp(line + offset, type, len) == 0) {
					offset += len;
					*resource = idx;
					break;
				}
			}
			if (idx == NO_RESOURCE)
				return -1;
			break;
		}
	}
	if (*fmt != '\0')
		return -1;
	return offset;
}

#define buff_append(result, format, value) \
	do { \
		gchar *old = result; \
		result = g_strdup_printf("%s" format, result, value); \
		g_free(old); \
	} while (0)

gchar *game_printf(const gchar * fmt, ...)
{
	va_list ap;
	gchar *result;

	va_start(ap, fmt);
	result = game_vprintf(fmt, ap);
	va_end(ap);

	return result;
}

gchar *game_vprintf(const gchar * fmt, va_list ap)
{
	/* initialize result to an allocated empty string */
	gchar *result = g_strdup("");

	while (*fmt != '\0') {
		gchar *pos = strchr(fmt, '%');
		if (pos == NULL) {
			buff_append(result, "%s", fmt);
			break;
		}
		/* add format until next % to result */
		result = g_realloc(result, strlen(result) + pos - fmt + 1);
		result[strlen(result) + pos - fmt] = '\0';
		memcpy(&result[strlen(result)], fmt, pos - fmt);
		fmt = pos + 1;

		switch (*fmt++) {
			BuildType build_type;
			const gint *num;
			gint idx;
		case 's':	/* string */
			buff_append(result, "%s", va_arg(ap, gchar *));
			break;
		case 'd':	/* integer */
		case 'D':	/* development card type */
			buff_append(result, "%d", va_arg(ap, gint));
			break;
		case 'B':	/* build type */
			build_type = va_arg(ap, BuildType);
			switch (build_type) {
			case BUILD_ROAD:
				buff_append(result, "%s", "road");
				break;
			case BUILD_BRIDGE:
				buff_append(result, "%s", "bridge");
				break;
			case BUILD_SHIP:
				buff_append(result, "%s", "ship");
				break;
			case BUILD_SETTLEMENT:
				buff_append(result, "%s", "settlement");
				break;
			case BUILD_CITY:
				buff_append(result, "%s", "city");
				break;
			case BUILD_CITY_WALL:
				buff_append(result, "%s", "city_wall");
				break;
			case BUILD_NONE:
				g_error
				    ("BUILD_NONE passed to game_printf");
				break;
			case BUILD_MOVE_SHIP:
				g_error
				    ("BUILD_MOVE_SHIP passed to game_printf");
				break;
			}
			break;
		case 'R':	/* list of 5 integer resource counts */
			num = va_arg(ap, gint *);
			for (idx = 0; idx < NO_RESOURCE; idx++) {
				if (idx > 0)
					buff_append(result, " %d",
						    num[idx]);
				else
					buff_append(result, "%d",
						    num[idx]);
			}
			break;
		case 'r':	/* resource type */
			buff_append(result, "%s",
				    resource_types[va_arg(ap, Resource)]);
			break;
		}
	}
	return result;
}
