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
#include <glib.h>
#include "game.h"
#include "driver.h"

typedef struct {
	void (*func) (gpointer);
	gpointer param;
	GIOChannel *channel;
} evl_io_func;


GHashTable *_evl_glib_hash = NULL;

/* Local function prototypes. */
guint evl_glib_input_add_read(gint fd, void (*func) (gpointer),
			      gpointer param);
guint evl_glib_input_add_write(gint fd, void (*func) (gpointer),
			       gpointer param);
void evl_glib_input_remove(guint tag);


/* event-loop related functions */
static gboolean evl_glib_call_func(G_GNUC_UNUSED GIOChannel * source,
				   G_GNUC_UNUSED GIOCondition condition,
				   gpointer data)
{
	evl_io_func *io_func = (evl_io_func *) data;
	io_func->func(io_func->param);
	return TRUE;
}

static void evl_glib_channel_destroyed(gpointer data)
{
	GIOChannel *io_channel = (GIOChannel *) data;
	/* free the srv_io_func structure associated with the channel */
	evl_io_func *io_func =
	    g_hash_table_lookup(_evl_glib_hash, io_channel);

	if (io_func)
		g_free(io_func);

	g_hash_table_remove(_evl_glib_hash, io_channel);
}


static guint evl_glib_input_add_watch(gint fd, GIOCondition condition,
				      void (*func) (gpointer),
				      gpointer param)
{
	GIOChannel *io_channel;
	evl_io_func *io_func = g_malloc0(sizeof(evl_io_func));
	guint tag;

	io_channel = g_io_channel_unix_new(fd);
	io_func->func = func;
	io_func->param = param;
	io_func->channel = io_channel;

	tag =
	    g_io_add_watch_full(io_channel, G_PRIORITY_DEFAULT, condition,
				evl_glib_call_func, io_func,
				evl_glib_channel_destroyed);

	/* allocate hash table if it hasn't yet been done */
	if (!_evl_glib_hash)
		_evl_glib_hash = g_hash_table_new(NULL, NULL);

	/* insert the channel and function into a hash table; key on channel */
	g_hash_table_insert(_evl_glib_hash, io_channel, io_func);

	return tag;
}


guint evl_glib_input_add_read(gint fd, InputFunc func, gpointer param)
{
	return evl_glib_input_add_watch(fd, G_IO_IN | G_IO_HUP, func,
					param);
}

guint evl_glib_input_add_write(gint fd, InputFunc func, gpointer param)
{
	return evl_glib_input_add_watch(fd, G_IO_OUT | G_IO_HUP, func,
					param);
}

void evl_glib_input_remove(guint tag)
{
	g_source_remove(tag);
}


UIDriver Glib_Driver = {
	NULL,			/* event_queue */

	log_message_string_console,	/* log_write */

	evl_glib_input_add_read,	/* add read input */
	evl_glib_input_add_write,	/* add write input */
	evl_glib_input_remove,	/* remove input */

	NULL,			/* player just added */
	NULL,			/* player just renamed */
	NULL,			/* player just removed */
	NULL			/* player just renamed */
};
