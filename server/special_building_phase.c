/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 1999 Dave Cole
 * Copyright (C) 2003 Bas Wijnen <shevek@fmf.nl>
 * Copyright (C) 2006 Roland Clobus <rclobus@bigfoot.com>
 * Copyright (C) 2011 JJ Foote <pioneers@thedopefish.com>
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

#include "special_building_phase.h"

// prototype from turn.c
void build_add(Player *player, BuildType type, gint x, gint y, gint pos);

void special_building_phase(Game *game)
{
	GList *list;
	for(list = player_first_real(game); list != NULL; list = player_next_real(list)) {
		Player *scan = list->data;

		scan->doing_special_building_phase = 1;

		sm_push(scan->sm, (StateFunc) mode_special_building_phase);

		player_send(scan, FIRST_VERSION, LATEST_VERSION,
			 "special building phase\n");
	}

	check_finished_special_building_phase(game);
}

void check_finished_special_building_phase(Game *game)
{
	GList *list;
	// is everyone finished yet?
	for(list = player_first_real(game); list != NULL; list = player_next_real(list)) {
		if(((Player *) list->data)->disconnected) {
			((Player *) list->data)->doing_special_building_phase = 0;
		}
		if(((Player *) list->data)->doing_special_building_phase) {
			break;
		}
	}
	if(list != NULL) {
		return;
	}

	for(list = player_first_real(game); list != NULL; list = player_next_real(list)) {
		Player *scan = list->data;
		player_send(scan, FIRST_VERSION, LATEST_VERSION, "OK\n");
		sm_pop(scan->sm);
	}

	// now that SBP is over, move on to the next turn
	turn_next_player(game);
}

gboolean mode_special_building_phase(Player *player, gint event)
{
	StateMachine *sm = player->sm;
	Game *game = player->game;
	sm_state_name(sm, "mode_special_building_phase");

	if(event != SM_RECV)
		return FALSE;

	// you're allowed to build, and that's it.  no trading, no D cards, no foolishness.
	BuildType build_type;
	gint x, y, pos;

	if (sm_recv(sm, "build %B %d %d %d", &build_type, &x, &y, &pos)) {
		build_add(player, build_type, x, y, pos);
		if (!game->params->check_victory_at_end_of_turn)
			check_victory(player);
		return TRUE;
	} else if(!sm_recv(sm, "done")) {
		return FALSE;
	}

	player->doing_special_building_phase = 0;
	sm_goto(sm, (StateFunc) mode_wait_for_other_special_building_phasing_players);
	check_finished_special_building_phase(game);
	return TRUE;
}

gboolean mode_wait_for_other_special_building_phasing_players(Player *player, gint event)
{
	StateMachine *sm = player->sm;
	sm_state_name(sm, "mode_wait_for_other_special_building_phasing_players");
	return FALSE;
}
