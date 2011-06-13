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

#ifndef __cards_h
#define __cards_h

#include "game.h"

/* It is important to know which turn a development card was bought
 * in.  You cannot play a card in the same turn that it was bought.
 */
typedef struct {
	DevelType type;
	gint turn_bought;
} DevelCard;

typedef struct {
	DevelCard *cards;
	gint num_cards;
	gint max_cards;
} DevelDeck;

gboolean is_victory_card(DevelType type);
const gchar *get_devel_name(DevelType type);
const gchar *get_devel_description(DevelType description);

DevelDeck *deck_new(GameParams * params);
void deck_free(DevelDeck * deck);
void deck_card_add(DevelDeck * deck, DevelType type, gint turn_bought);
gboolean deck_card_playable(const DevelDeck * deck,
			    gboolean played_develop, gint idx, gint turn);
gboolean deck_card_play(DevelDeck * deck,
			gboolean played_develop, gint idx, gint turn);
DevelType deck_card_type(const DevelDeck * deck, gint idx);

gint deck_card_amount(const DevelDeck * deck, DevelType type);
gint deck_card_oldest_card(const DevelDeck * deck, DevelType type);
#endif
