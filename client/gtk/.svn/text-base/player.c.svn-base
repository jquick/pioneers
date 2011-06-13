/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 1999 Dave Cole
 * Copyright (C) 2003, 2006 Bas Wijnen <shevek@fmf.nl>
 * Copyright (C) 2004-2008 Roland Clobus <rclobus@bigfoot.com>
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
#include <stdlib.h>
#include "colors.h"
#include "frontend.h"
#include "log.h"
#include "gtkbugs.h"
#include "common_gtk.h"
#include "player-icon.h"
#include "audio.h"

static void player_show_connected_at_iter(gint player_num,
					  gboolean connected,
					  GtkTreeIter * iter);

static GdkColor ps_settlement = { 0, 0xbb00, 0x0000, 0x0000 };
static GdkColor ps_city = { 0, 0xff00, 0x0000, 0x0000 };
static GdkColor ps_city_wall = { 0, 0xff00, 0x0000, 0x0000 };
static GdkColor ps_largest = { 0, 0x1c00, 0xb500, 0xed00 };
static GdkColor ps_soldier = { 0, 0xe500, 0x8f00, 0x1600 };
static GdkColor ps_resource = { 0, 0x0000, 0x0000, 0xFF00 };
static GdkColor ps_development = { 0, 0xc600, 0xc600, 0x1300 };
static GdkColor ps_building = { 0, 0x0b00, 0xed00, 0x8900 };

typedef struct {
	const gchar *singular;
	const gchar *plural;
	GdkColor *textcolor;
} Statistic;

static Statistic statistics[] = {
	{N_("Settlement"), N_("Settlements"), &ps_settlement},
	{N_("City"), N_("Cities"), &ps_city},
	{N_("City wall"), N_("City walls"), &ps_city_wall},
	{N_("Largest army"), NULL, &ps_largest},
	{N_("Longest road"), NULL, &ps_largest},
	{N_("Chapel"), N_("Chapels"), &ps_building},
	{N_("Pioneer university"), N_("Pioneer universities"),
	 &ps_building},
	{N_("Governor's house"), N_("Governor's houses"), &ps_building},
	{N_("Library"), N_("Libraries"), &ps_building},
	{N_("Market"), N_("Markets"), &ps_building},
	{N_("Soldier"), N_("Soldiers"), &ps_soldier},
	{N_("Resource card"), N_("Resource cards"), &ps_resource},
	{N_("Development card"), N_("Development cards"), &ps_development}
};

enum {
	SUMMARY_COLUMN_PLAYER_ICON, /**< Player icon */
	SUMMARY_COLUMN_PLAYER_NUM, /**< Internal: player number */
	SUMMARY_COLUMN_TEXT, /**< Description of the items */
	SUMMARY_COLUMN_TEXT_COLOUR, /**< Colour of the description */
	SUMMARY_COLUMN_SCORE, /**< Score of the items (as string) */
	SUMMARY_COLUMN_STATISTIC, /**< enum Statistic value+1, or 0 if not in the enum */
	SUMMARY_COLUMN_POINTS_ID, /**< Id of points, or -1 */
	SUMMARY_COLUMN_LAST
};

static Player players[MAX_PLAYERS];
static GtkListStore *summary_store; /**< the player summary data */
static GtkWidget *summary_widget; /**< the player summary widget */
static gboolean summary_color_enabled = TRUE;

/** Structure to find combination of player and statistic */
struct Player_statistic {
	enum TFindResult result;
	GtkTreeIter iter;
	gint player_num;
	gint statistic;
};

/** Structure to find combination of player and points */
struct Player_point {
	enum TFindResult result;
	GtkTreeIter iter;
	gint player_num;
	gint point_id;
};

static GtkWidget *turn_area;	/** turn indicator in status bar */
static const gint turn_area_icon_width = 30;

void player_init(void)
{
	colors_init();
	playericon_init();
}

GdkColor *player_color(gint player_num)
{
	return colors_get_player(player_num);
}

GdkColor *player_or_viewer_color(gint player_num)
{
	if (player_is_viewer(player_num)) {
		/* viewer color is always black */
		return &black;
	}
	return colors_get_player(player_num);
}

GdkPixbuf *player_create_icon(GtkWidget * widget, gint player_num,
			      gboolean connected)
{
	return playericon_create_icon(widget, player_get_style(player_num),
				      player_or_viewer_color(player_num),
				      player_is_viewer(player_num),
				      connected, FALSE);
}

/** Locate a line suitable for the statistic */
static gboolean summary_locate_statistic(GtkTreeModel * model,
					 G_GNUC_UNUSED GtkTreePath * path,
					 GtkTreeIter * iter,
					 gpointer user_data)
{
	struct Player_statistic *ps =
	    (struct Player_statistic *) user_data;
	gint current_player;
	gint current_statistic;
	gtk_tree_model_get(model, iter,
			   SUMMARY_COLUMN_PLAYER_NUM, &current_player,
			   SUMMARY_COLUMN_STATISTIC, &current_statistic,
			   -1);
	if (current_player > ps->player_num) {
		ps->result = FIND_MATCH_INSERT_BEFORE;
		ps->iter = *iter;
		return TRUE;
	} else if (current_player == ps->player_num) {
		if (current_statistic > ps->statistic) {
			ps->result = FIND_MATCH_INSERT_BEFORE;
			ps->iter = *iter;
			return TRUE;
		} else if (current_statistic == ps->statistic) {
			ps->result = FIND_MATCH_EXACT;
			ps->iter = *iter;
			return TRUE;
		}
	}
	return FALSE;
}

/** Locate a line suitable for the statistic */
static gboolean summary_locate_point(GtkTreeModel * model,
				     G_GNUC_UNUSED GtkTreePath * path,
				     GtkTreeIter * iter,
				     gpointer user_data)
{
	struct Player_point *pp = (struct Player_point *) user_data;
	gint current_player;
	gint current_point_id;
	gint current_statistic;

	gtk_tree_model_get(model, iter,
			   SUMMARY_COLUMN_PLAYER_NUM, &current_player,
			   SUMMARY_COLUMN_STATISTIC, &current_statistic,
			   SUMMARY_COLUMN_POINTS_ID, &current_point_id,
			   -1);
	if (current_player > pp->player_num) {
		pp->result = FIND_MATCH_INSERT_BEFORE;
		pp->iter = *iter;
		return TRUE;
	} else if (current_player == pp->player_num) {
		if (current_statistic >= STAT_SOLDIERS) {
			pp->result = FIND_MATCH_INSERT_BEFORE;
			pp->iter = *iter;
			return TRUE;
		}
		if (current_point_id > pp->point_id) {
			pp->result = FIND_MATCH_INSERT_BEFORE;
			pp->iter = *iter;
			return TRUE;
		} else if (current_point_id == pp->point_id) {
			pp->result = FIND_MATCH_EXACT;
			pp->iter = *iter;
			return TRUE;
		}
	}
	return FALSE;
}

/** Function to redisplay the running point total for the indicated player */
static void refresh_victory_point_total(int player_num)
{
	gchar points[16];
	GtkTreeIter iter;
	enum TFindResult found;

	if (player_num < 0 || player_num >= G_N_ELEMENTS(players))
		return;

	found =
	    find_integer_in_tree(GTK_TREE_MODEL(summary_store), &iter,
				 SUMMARY_COLUMN_PLAYER_NUM, player_num);

	if (found == FIND_MATCH_EXACT) {
		snprintf(points, sizeof(points), "%d",
			 player_get_score(player_num));
		gtk_list_store_set(summary_store, &iter,
				   SUMMARY_COLUMN_SCORE, points, -1);
	}
}

/** Apply colors to the summary */
static gboolean summary_apply_colors_cb(GtkTreeModel * model,
					G_GNUC_UNUSED GtkTreePath * path,
					GtkTreeIter * iter,
					G_GNUC_UNUSED gpointer user_data)
{
	gint current_statistic;
	gint point_id;

	gtk_tree_model_get(model, iter,
			   SUMMARY_COLUMN_STATISTIC, &current_statistic,
			   SUMMARY_COLUMN_POINTS_ID, &point_id, -1);
	if (current_statistic > 0)
		gtk_list_store_set(summary_store, iter,
				   SUMMARY_COLUMN_TEXT_COLOUR,
				   summary_color_enabled ?
				   statistics[current_statistic -
					      1].textcolor : &black, -1);
	else if (point_id >= 0)
		gtk_list_store_set(summary_store, iter,
				   SUMMARY_COLUMN_TEXT_COLOUR,
				   summary_color_enabled ?
				   &ps_largest : &black, -1);
	return FALSE;
}

void set_color_summary(gboolean flag)
{
	if (flag != summary_color_enabled) {
		summary_color_enabled = flag;
		if (summary_store)
			gtk_tree_model_foreach(GTK_TREE_MODEL
					       (summary_store),
					       summary_apply_colors_cb,
					       NULL);
	}
}

void frontend_new_statistics(gint player_num, StatisticType type,
			     G_GNUC_UNUSED gint num)
{
	Player *player = player_get(player_num);
	gint value;
	gchar points[16];
	GtkTreeIter iter;
	struct Player_statistic ps;

	value = player->statistics[type];
	if (stat_get_vp_value(type) > 0)
		refresh_victory_point_total(player_num);

	ps.result = FIND_NO_MATCH;
	ps.player_num = player_num;
	ps.statistic = type + 1;
	gtk_tree_model_foreach(GTK_TREE_MODEL(summary_store),
			       summary_locate_statistic, &ps);

	if (value == 0) {
		if (ps.result == FIND_MATCH_EXACT)
			gtk_list_store_remove(summary_store, &ps.iter);
	} else {
		gchar *desc;
		if (value == 1) {
			if (statistics[type].plural != NULL)
				desc = g_strdup_printf("%d %s", value,
						       gettext(statistics
							       [type].
							       singular));
			else
				desc = g_strdup(gettext
						(statistics
						 [type].singular));
		} else
			desc = g_strdup_printf("%d %s", value,
					       gettext(statistics
						       [type].plural));
		if (stat_get_vp_value(type) > 0)
			sprintf(points, "%d",
				value * stat_get_vp_value(type));
		else
			strcpy(points, "");

		switch (ps.result) {
		case FIND_NO_MATCH:
			gtk_list_store_append(summary_store, &iter);
			break;
		case FIND_MATCH_INSERT_BEFORE:
			gtk_list_store_insert_before(summary_store, &iter,
						     &ps.iter);
			break;
		case FIND_MATCH_EXACT:
			iter = ps.iter;
			break;
		default:
			g_error("unknown case in frontend_new_statistics");
		};
		gtk_list_store_set(summary_store, &iter,
				   SUMMARY_COLUMN_PLAYER_NUM, player_num,
				   SUMMARY_COLUMN_TEXT, desc,
				   SUMMARY_COLUMN_TEXT_COLOUR,
				   summary_color_enabled ?
				   statistics[type].textcolor : &black,
				   SUMMARY_COLUMN_STATISTIC, type + 1,
				   SUMMARY_COLUMN_POINTS_ID, -1,
				   SUMMARY_COLUMN_SCORE, points, -1);
		g_free(desc);
	}
	frontend_gui_update();
}

void frontend_new_points(gint player_num, Points * points, gboolean added)
{
	GtkTreeIter iter;
	struct Player_point pp;
	gchar score[16];

	refresh_victory_point_total(player_num);

	pp.result = FIND_NO_MATCH;
	pp.player_num = player_num;
	pp.point_id = points->id;
	gtk_tree_model_foreach(GTK_TREE_MODEL(summary_store),
			       summary_locate_point, &pp);

	if (!added) {
		if (pp.result != FIND_MATCH_EXACT)
			g_error("cannot remove point");
		gtk_list_store_remove(summary_store, &pp.iter);
		frontend_gui_update();
		return;
	}

	switch (pp.result) {
	case FIND_NO_MATCH:
		gtk_list_store_append(summary_store, &iter);
		break;
	case FIND_MATCH_INSERT_BEFORE:
		gtk_list_store_insert_before(summary_store, &iter,
					     &pp.iter);
		break;
	case FIND_MATCH_EXACT:
		iter = pp.iter;
		break;
	default:
		g_error("unknown case in frontend_new_points");
	};
	snprintf(score, sizeof(score), "%d", points->points);
	gtk_list_store_set(summary_store, &iter,
			   SUMMARY_COLUMN_PLAYER_NUM, player_num,
			   SUMMARY_COLUMN_TEXT, _(points->name),
			   SUMMARY_COLUMN_TEXT_COLOUR,
			   summary_color_enabled ? &ps_largest : &black,
			   SUMMARY_COLUMN_STATISTIC, 0,
			   SUMMARY_COLUMN_POINTS_ID, points->id,
			   SUMMARY_COLUMN_SCORE, score, -1);
	frontend_gui_update();
}

static void player_create_find_player(gint player_num, GtkTreeIter * iter)
{
	GtkTreeIter found_iter;
	enum TFindResult result;

	/* Search for a place to add information about the player/viewer */
	result =
	    find_integer_in_tree(GTK_TREE_MODEL(summary_store),
				 &found_iter, SUMMARY_COLUMN_PLAYER_NUM,
				 player_num);

	switch (result) {
	case FIND_NO_MATCH:
		gtk_list_store_append(summary_store, iter);
		gtk_list_store_set(summary_store, iter,
				   SUMMARY_COLUMN_PLAYER_NUM, player_num,
				   SUMMARY_COLUMN_POINTS_ID, -1, -1);
		break;
	case FIND_MATCH_INSERT_BEFORE:
		gtk_list_store_insert_before(summary_store, iter,
					     &found_iter);
		gtk_list_store_set(summary_store, iter,
				   SUMMARY_COLUMN_PLAYER_NUM, player_num,
				   SUMMARY_COLUMN_POINTS_ID, -1, -1);
		break;
	case FIND_MATCH_EXACT:
		*iter = found_iter;
		break;
	default:
		g_error("unknown case in player_create_find_player");
	};
}

void frontend_player_name(gint player_num, const gchar * name)
{
	GtkTreeIter iter;

	player_create_find_player(player_num, &iter);
	gtk_list_store_set(summary_store, &iter,
			   SUMMARY_COLUMN_TEXT, name, -1);

	player_show_connected_at_iter(player_num, TRUE, &iter);
	if (callback_mode != MODE_INIT)
		play_sound(SOUND_ANNOUNCE);

	chat_player_name(player_num, name);
}

void frontend_player_style(gint player_num,
			   G_GNUC_UNUSED const gchar * style)
{
	GtkTreeIter iter;

	player_create_find_player(player_num, &iter);
	player_show_connected_at_iter(player_num, TRUE, &iter);
	chat_player_style(player_num);
}

void frontend_viewer_name(gint viewer_num, const gchar * name)
{
	GtkTreeIter iter;

	player_create_find_player(viewer_num, &iter);
	gtk_list_store_set(summary_store, &iter,
			   SUMMARY_COLUMN_TEXT, name, -1);
	if (callback_mode != MODE_INIT)
		play_sound(SOUND_ANNOUNCE);

	chat_player_name(viewer_num, name);
}

void frontend_player_quit(gint player_num)
{
	GtkTreeIter iter;

	player_create_find_player(player_num, &iter);
	player_show_connected_at_iter(player_num, FALSE, &iter);

	chat_player_quit(player_num);
}

void frontend_viewer_quit(gint viewer_num)
{
	GtkTreeIter iter;

	player_create_find_player(viewer_num, &iter);
	gtk_list_store_remove(summary_store, &iter);

	chat_viewer_quit(viewer_num);
}

static void player_show_connected_at_iter(gint player_num,
					  gboolean connected,
					  GtkTreeIter * iter)
{
	GdkPixbuf *pixbuf =
	    player_create_icon(summary_widget, player_num, connected);

	gtk_list_store_set(summary_store, iter,
			   SUMMARY_COLUMN_PLAYER_ICON, pixbuf, -1);
	g_object_unref(pixbuf);
}

/* Get the top and bottom row for player summary and make sure player
 * is visible
 */
static void player_show_summary(gint player_num)
{
	GtkTreeIter found_iter;
	enum TFindResult result;
	gboolean scroll_to_end = FALSE;

	result =
	    find_integer_in_tree(GTK_TREE_MODEL(summary_store),
				 &found_iter, SUMMARY_COLUMN_PLAYER_NUM,
				 player_num + 1);

	if (result == FIND_NO_MATCH) {
		scroll_to_end = TRUE;
	} else {
		GtkTreePath *path =
		    gtk_tree_model_get_path(GTK_TREE_MODEL(summary_store),
					    &found_iter);
		if (gtk_tree_path_prev(path))
			gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW
						     (summary_widget),
						     path, NULL, FALSE,
						     0.0, 0.0);
		gtk_tree_path_free(path);
	}

	result =
	    find_integer_in_tree(GTK_TREE_MODEL(summary_store),
				 &found_iter, SUMMARY_COLUMN_PLAYER_NUM,
				 player_num);
	if (result != FIND_NO_MATCH) {
		GtkTreePath *path =
		    gtk_tree_model_get_path(GTK_TREE_MODEL(summary_store),
					    &found_iter);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(summary_widget),
					     path, NULL, scroll_to_end,
					     0.0, 0.0);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(summary_widget),
					 path, NULL, FALSE);
		gtk_tree_path_free(path);
	}
}

GtkWidget *player_build_summary(void)
{
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *scroll_win;
	GtkWidget *alignment;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_widget_show(vbox);

	alignment = gtk_alignment_new(0.0, 0.0, 1.0, 1.0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 3, 3);
	gtk_widget_show(alignment);
	gtk_box_pack_start(GTK_BOX(vbox), alignment, FALSE, FALSE, 0);

	label = gtk_label_new(NULL);
	/* Caption for the overview of the points and card of other players */
	gtk_label_set_markup(GTK_LABEL(label), _("<b>Player summary</b>"));
	gtk_widget_show(label);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_container_add(GTK_CONTAINER(alignment), label);

	summary_store = gtk_list_store_new(SUMMARY_COLUMN_LAST, GDK_TYPE_PIXBUF,	/* player icon */
					   G_TYPE_INT,	/* player number */
					   G_TYPE_STRING,	/* text */
					   GDK_TYPE_COLOR,	/* text colour */
					   G_TYPE_STRING,	/* score */
					   G_TYPE_INT,	/* statistic */
					   G_TYPE_INT);	/* points */
	summary_widget =
	    gtk_tree_view_new_with_model(GTK_TREE_MODEL(summary_store));

	column = gtk_tree_view_column_new_with_attributes("",
							  gtk_cell_renderer_pixbuf_new
							  (), "pixbuf",
							  SUMMARY_COLUMN_PLAYER_ICON,
							  NULL);
	gtk_tree_view_column_set_sizing(column,
					GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_append_column(GTK_TREE_VIEW(summary_widget), column);

	column = gtk_tree_view_column_new_with_attributes("",
							  gtk_cell_renderer_text_new
							  (), "text",
							  SUMMARY_COLUMN_TEXT,
							  "foreground-gdk",
							  SUMMARY_COLUMN_TEXT_COLOUR,
							  NULL);
	gtk_tree_view_column_set_sizing(column,
					GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(summary_widget), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("",
							  renderer,
							  "text",
							  SUMMARY_COLUMN_SCORE,
							  "foreground-gdk",
							  SUMMARY_COLUMN_TEXT_COLOUR,
							  NULL);
	g_object_set(renderer, "xalign", 1.0f, NULL);
	gtk_tree_view_column_set_sizing(column,
					GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_append_column(GTK_TREE_VIEW(summary_widget), column);

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(summary_widget),
					  FALSE);
	gtk_widget_show(summary_widget);

	scroll_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW
					    (scroll_win), GTK_SHADOW_IN);
	gtk_widget_show(scroll_win);
	gtk_box_pack_start(GTK_BOX(vbox), scroll_win, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);

	gtk_container_add(GTK_CONTAINER(scroll_win), summary_widget);

	return vbox;
}

static gint expose_turn_area_cb(GtkWidget * area,
				G_GNUC_UNUSED GdkEventExpose * event,
				G_GNUC_UNUSED gpointer user_data)
{
	static GdkGC *turn_gc;
	gint offset;
	gint idx;

	if (area->window == NULL)
		return FALSE;

	if (turn_gc == NULL)
		turn_gc = gdk_gc_new(area->window);

	offset = 0;
	for (idx = 0; idx < num_players(); idx++) {
		gdk_gc_set_foreground(turn_gc, player_color(idx));
		gdk_draw_rectangle(area->window, turn_gc, TRUE,
				   offset + 2, 2,
				   turn_area_icon_width - 4,
				   area->allocation.height - 4);
		gdk_gc_set_foreground(turn_gc, &black);
		if (idx == current_player()) {
			gdk_gc_set_line_attributes(turn_gc, 3,
						   GDK_LINE_SOLID,
						   GDK_CAP_BUTT,
						   GDK_JOIN_MITER);
			gdk_draw_rectangle(area->window, turn_gc, FALSE,
					   offset + 3, 3,
					   turn_area_icon_width - 6,
					   area->allocation.height - 6);
		} else {
			gdk_gc_set_line_attributes(turn_gc, 1,
						   GDK_LINE_SOLID,
						   GDK_CAP_BUTT,
						   GDK_JOIN_MITER);
			gdk_draw_rectangle(area->window, turn_gc, FALSE,
					   offset + 2, 2,
					   turn_area_icon_width - 4,
					   area->allocation.height - 4);
		}

		offset += turn_area_icon_width;
	}

	return FALSE;
}

GtkWidget *player_build_turn_area(void)
{
	turn_area = gtk_drawing_area_new();
	g_signal_connect(G_OBJECT(turn_area), "expose_event",
			 G_CALLBACK(expose_turn_area_cb), NULL);
	gtk_widget_set_size_request(turn_area,
				    turn_area_icon_width * num_players(),
				    -1);
	gtk_widget_show(turn_area);

	return turn_area;
}

void set_num_players(gint num)
{
	gtk_widget_set_size_request(turn_area, turn_area_icon_width * num,
				    -1);
}

void player_show_current(gint player_num)
{
	gtk_widget_queue_draw(turn_area);
	player_show_summary(player_num);
}

void player_clear_summary(void)
{
	gtk_list_store_clear(GTK_LIST_STORE(summary_store));
}
