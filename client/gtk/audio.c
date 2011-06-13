/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 2008 Roland Clobus <rclobus@bigfoot.com>
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
#include <gdk/gdk.h>

#include "audio.h"

static gboolean silent_mode = FALSE;
static gboolean announce_player = TRUE;

gboolean get_announce_player(void)
{
	return announce_player;
}

void set_announce_player(gboolean announce)
{
	announce_player = announce;
}

gboolean get_silent_mode(void)
{
	return silent_mode;
}

void set_silent_mode(gboolean silent)
{
	silent_mode = silent;
}

void play_sound(SoundType sound)
{
	if (get_silent_mode()) {
		return;
	}
	switch (sound) {
	case SOUND_BEEP:
		gdk_beep();
		break;
	case SOUND_TURN:
		gdk_beep();
		break;
	case SOUND_ANNOUNCE:
		if (get_announce_player())
			gdk_beep();
		break;
	}
}
