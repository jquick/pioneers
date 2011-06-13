/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 1999 Dave Cole
 * Copyright (C) 2003 Bas Wijnen <shevek@fmf.nl>
 * Copyright (C) 2004 Roland Clobus <rclobus@bigfoot.com>
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
#include "frontend.h"

static GtkWidget *identity_area;
static GuiMap bogus_map;
static GdkGC *identity_gc;

static int die_num[2];

static void calculate_width(GtkWidget * area, const Polygon * poly,
			    gint num, gint * fixedwidth,
			    gint * variablewidth)
{
	GdkRectangle rect;
	char buff[10];
	gint width, height;
	PangoLayout *layout;

	poly_bound_rect(poly, 0, &rect);

	sprintf(buff, "%d", num);
	layout = gtk_widget_create_pango_layout(area, buff);
	pango_layout_get_pixel_size(layout, &width, &height);
	g_object_unref(layout);

	*fixedwidth += width + 10;
	*variablewidth += rect.width;
}

static void calculate_optimum_size(GtkWidget * area, gint size)
{
	const GameParams *game_params = get_game_params();
	GdkPoint points[MAX_POINTS];
	Polygon poly;
	gint new_size;
	gint fixedwidth;	/* Size of fixed part (digits + spacing) */
	gint variablewidth;	/* Size of variable part (polygons) */

	if (game_params == NULL)
		return;

	guimap_scale_with_radius(&bogus_map, size);

	if (bogus_map.hex_radius <= MIN_HEX_RADIUS)
		return;

	fixedwidth = 0;
	variablewidth = 0;

	poly.points = points;
	if (game_params->num_build_type[BUILD_ROAD] > 0) {
		poly.num_points = MAX_POINTS;
		guimap_road_polygon(&bogus_map, NULL, &poly);
		calculate_width(area, &poly, stock_num_roads(),
				&fixedwidth, &variablewidth);
	}
	if (game_params->num_build_type[BUILD_SHIP] > 0) {
		poly.num_points = MAX_POINTS;
		guimap_ship_polygon(&bogus_map, NULL, &poly);
		calculate_width(area, &poly, stock_num_ships(),
				&fixedwidth, &variablewidth);
	}
	if (game_params->num_build_type[BUILD_BRIDGE] > 0) {
		poly.num_points = MAX_POINTS;
		guimap_bridge_polygon(&bogus_map, NULL, &poly);
		calculate_width(area, &poly, stock_num_bridges(),
				&fixedwidth, &variablewidth);
	}
	if (game_params->num_build_type[BUILD_SETTLEMENT] > 0) {
		poly.num_points = MAX_POINTS;
		guimap_settlement_polygon(&bogus_map, NULL, &poly);
		calculate_width(area, &poly, stock_num_settlements(),
				&fixedwidth, &variablewidth);
	}
	if (game_params->num_build_type[BUILD_CITY] > 0) {
		poly.num_points = MAX_POINTS;
		guimap_city_polygon(&bogus_map, NULL, &poly);
		calculate_width(area, &poly, stock_num_cities(),
				&fixedwidth, &variablewidth);
	}
	if (game_params->num_build_type[BUILD_CITY_WALL] > 0) {
		poly.num_points = MAX_POINTS;
		guimap_city_wall_polygon(&bogus_map, NULL, &poly);
		calculate_width(area, &poly, stock_num_city_walls(),
				&fixedwidth, &variablewidth);
	}

	new_size = bogus_map.hex_radius *
	    (area->allocation.width - 75 - fixedwidth) / variablewidth;
	if (new_size < bogus_map.hex_radius) {
		calculate_optimum_size(area, new_size);
	}
}

static int draw_building_and_count(GdkGC * gc, GtkWidget * area,
				   gint offset, Polygon * poly, gint num)
{
	GdkRectangle rect;
	char buff[10];
	gint width, height;
	PangoLayout *layout;

	poly_bound_rect(poly, 0, &rect);
	poly_offset(poly,
		    offset - rect.x,
		    area->allocation.height - 5 - rect.y - rect.height);
	poly_draw_old(area->window, gc, FALSE, poly);

	offset += 5 + rect.width;

	sprintf(buff, "%d", num);
	layout = gtk_widget_create_pango_layout(area, buff);
	pango_layout_get_pixel_size(layout, &width, &height);
	gdk_draw_layout(area->window, gc, offset,
			area->allocation.height - height - 5, layout);
	g_object_unref(layout);

	offset += 5 + width;

	return offset;
}

static void show_die(GdkGC * gc, GtkWidget * area, gint x_offset, gint num,
		     GdkColor * die_border_color, GdkColor * die_color,
		     GdkColor * die_dots_color)
{
	static GdkPoint die_points[4] = {
		{0, 0}, {30, 0}, {30, 30}, {0, 30}
	};
	static Polygon die_shape =
	    { die_points, G_N_ELEMENTS(die_points) };
	static GdkPoint dot_pos[7] = {
		{7, 7}, {22, 7},
		{7, 15}, {15, 15}, {22, 15},
		{7, 22}, {22, 22}
	};
	static gint draw_list[6][7] = {
		{0, 0, 0, 1, 0, 0, 0},
		{0, 1, 0, 0, 0, 1, 0},
		{1, 0, 0, 1, 0, 0, 1},
		{1, 1, 0, 0, 0, 1, 1},
		{1, 1, 0, 1, 0, 1, 1},
		{1, 1, 1, 0, 1, 1, 1}
	};
	gint y_offset = (area->allocation.height - 30) / 2;
	gint *list = draw_list[num - 1];
	gint idx;

	poly_offset(&die_shape, x_offset, y_offset);

	gdk_gc_set_foreground(gc, die_color);
	poly_draw_old(area->window, gc, TRUE, &die_shape);
	gdk_gc_set_foreground(gc, die_border_color);
	poly_draw_old(area->window, gc, FALSE, &die_shape);

	poly_offset(&die_shape, -x_offset, -y_offset);

	gdk_gc_set_foreground(gc, die_dots_color);
	for (idx = 0; idx < 7; idx++) {
		if (list[idx] == 0)
			continue;

		gdk_draw_arc(area->window, gc, TRUE,
			     x_offset + dot_pos[idx].x - 3,
			     y_offset + dot_pos[idx].y - 3,
			     7, 7, 0, 360 * 64);
	}
}

static void identity_resize_cb(GtkWidget * area,
			       G_GNUC_UNUSED GtkAllocation * allocation,
			       G_GNUC_UNUSED gpointer user_data)
{
	calculate_optimum_size(area, 50);
}

static gint expose_identity_area_cb(GtkWidget * area,
				    G_GNUC_UNUSED GdkEventExpose * event,
				    G_GNUC_UNUSED gpointer user_data)
{
	GdkPoint points[MAX_POINTS];
	Polygon poly;
	gint offset;
	GdkColor *colour;
	const GameParams *game_params;
	gint i;

	if (area->window == NULL || my_player_num() < 0)
		return FALSE;

	if (identity_gc == NULL)
		identity_gc = gdk_gc_new(area->window);

	colour = player_or_viewer_color(my_player_num());
	gdk_gc_set_foreground(identity_gc, colour);
	gdk_draw_rectangle(area->window, identity_gc, TRUE, 0, 0,
			   area->allocation.width,
			   area->allocation.height);

	if (my_player_viewer())
		colour = &white;
	else
		colour = &black;
	gdk_gc_set_foreground(identity_gc, colour);

	game_params = get_game_params();
	if (game_params == NULL)
		return TRUE;
	offset = 5;

	poly.points = points;
	if (game_params->num_build_type[BUILD_ROAD] > 0) {
		poly.num_points = MAX_POINTS;
		guimap_road_polygon(&bogus_map, NULL, &poly);
		offset = draw_building_and_count(identity_gc, area, offset,
						 &poly, stock_num_roads());
	}
	if (game_params->num_build_type[BUILD_SHIP] > 0) {
		poly.num_points = MAX_POINTS;
		guimap_ship_polygon(&bogus_map, NULL, &poly);
		offset = draw_building_and_count(identity_gc, area, offset,
						 &poly, stock_num_ships());
	}
	if (game_params->num_build_type[BUILD_BRIDGE] > 0) {
		poly.num_points = MAX_POINTS;
		guimap_bridge_polygon(&bogus_map, NULL, &poly);
		offset = draw_building_and_count(identity_gc, area, offset,
						 &poly,
						 stock_num_bridges());
	}
	if (game_params->num_build_type[BUILD_SETTLEMENT] > 0) {
		poly.num_points = MAX_POINTS;
		guimap_settlement_polygon(&bogus_map, NULL, &poly);
		offset = draw_building_and_count(identity_gc, area, offset,
						 &poly,
						 stock_num_settlements());
	}
	if (game_params->num_build_type[BUILD_CITY] > 0) {
		poly.num_points = MAX_POINTS;
		guimap_city_polygon(&bogus_map, NULL, &poly);
		offset = draw_building_and_count(identity_gc, area, offset,
						 &poly,
						 stock_num_cities());
	}
	if (game_params->num_build_type[BUILD_CITY_WALL] > 0) {
		poly.num_points = MAX_POINTS;
		guimap_city_wall_polygon(&bogus_map, NULL, &poly);
		offset = draw_building_and_count(identity_gc, area, offset,
						 &poly,
						 stock_num_city_walls());
	}

	if (die_num[0] > 0 && die_num[1] > 0) {
		GdkColor *die_border_color[2];
		GdkColor *die_color[2];
		GdkColor *die_dots_color[2];
		if (game_params->use_cities_and_knights_rules) {
			die_border_color[0] = &ck_die_yellow;
			die_color[0] = &ck_die_red;
			die_dots_color[0] = &ck_die_yellow;
			die_border_color[1] = &ck_die_red;
			die_color[1] = &ck_die_yellow;
			die_dots_color[1] = &ck_die_red;
		} else {
			die_border_color[0] = &black;
			die_color[0] = &white;
			die_dots_color[0] = &black;
			die_border_color[1] = &black;
			die_color[1] = &white;
			die_dots_color[1] = &black;
		}
		for (i = 0; i < 2; i++) {
			show_die(identity_gc, area,
				 area->allocation.width - 70 + 35 * i,
				 die_num[i], die_border_color[i],
				 die_color[i], die_dots_color[i]);
		}
	}

	return TRUE;
}

void identity_draw(void)
{
	gtk_widget_queue_draw(identity_area);
}

void identity_set_dice(gint die1, gint die2)
{
	die_num[0] = die1;
	die_num[1] = die2;
	gtk_widget_queue_draw(identity_area);
}

GtkWidget *identity_build_panel(void)
{
	identity_area = gtk_drawing_area_new();
	g_signal_connect(G_OBJECT(identity_area), "expose_event",
			 G_CALLBACK(expose_identity_area_cb), NULL);
	g_signal_connect(G_OBJECT(identity_area), "size-allocate",
			 G_CALLBACK(identity_resize_cb), NULL);
	gtk_widget_set_size_request(identity_area, -1, 40);
	identity_reset();
	gtk_widget_show(identity_area);
	return identity_area;
}

void identity_reset(void)
{
	gint i;

	for (i = 0; i < 2; i++) {
		die_num[i] = 0;
	};
	/* 50 seems to give a good upper limit */
	if (identity_area != NULL)
		calculate_optimum_size(identity_area, 50);
}
