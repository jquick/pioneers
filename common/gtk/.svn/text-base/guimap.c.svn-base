/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 1999 Dave Cole
 * Copyright (C) 2003 Bas Wijnen <shevek@fmf.nl>
 * Copyright (C) 2004-2011 Roland Clobus <rclobus@bigfoot.com>
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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "game.h"
#include "map.h"
#include "colors.h"
#include "guimap.h"
#include "log.h"
#include "theme.h"

#define ZOOM_AMOUNT 3

static gboolean single_click_build_active = FALSE;

typedef struct {
	GuiMap *gmap;
	gint old_highlight;
} HighlightInfo;

/* Local function prototypes */
static void calc_edge_poly(const GuiMap * gmap, const Edge * edge,
			   const Polygon * shape, Polygon * poly);
static void calc_node_poly(const GuiMap * gmap, const Node * node,
			   const Polygon * shape, Polygon * poly);
static void calc_hex_poly(const GuiMap * gmap, const Hex * hex,
			  const Polygon * shape, Polygon * poly,
			  double scale_factor, gint x_shift);
static void guimap_cursor_move(GuiMap * gmap, gint x, gint y,
			       MapElement * element);

/* Square */
static gint sqr(gint a)
{
	return a * a;
}

GuiMap *guimap_new(void)
{
	GuiMap *gmap;

	gmap = g_malloc0(sizeof(*gmap));
	gmap->highlight_chit = -1;
	gmap->initial_font_size = -1;
	gmap->show_nosetup_nodes = FALSE;
	return gmap;
}

void guimap_delete(GuiMap * gmap)
{
	if (gmap->area != NULL) {
		g_object_unref(gmap->area);
		gmap->area = NULL;
	}
	if (gmap->pixmap != NULL) {
		g_object_unref(gmap->pixmap);
		gmap->pixmap = NULL;
	}
	if (gmap->cr != NULL) {
		cairo_destroy(gmap->cr);
		gmap->cr = NULL;
	}
	if (gmap->layout) {
		/* Restore the font size */
		PangoContext *pc;
		PangoFontDescription *pfd;

		pc = pango_layout_get_context(gmap->layout);
		pfd = pango_context_get_font_description(pc);

		if (gmap->initial_font_size != -1) {
			pango_font_description_set_size(pfd,
							gmap->
							initial_font_size);
		}

		g_object_unref(gmap->layout);
		gmap->layout = NULL;
	}
	gmap->map = NULL;
	g_free(gmap);
}

void guimap_reset(GuiMap * gmap)
{
	gmap->highlight_chit = -1;
	gmap->player_num = -1;
}

static gint expose_map_cb(GtkWidget * area, GdkEventExpose * event,
			  gpointer user_data)
{
	GuiMap *gmap = user_data;
	cairo_t *cr;

	if (area->window == NULL || gmap->map == NULL)
		return FALSE;

	if (gmap->pixmap == NULL) {
		gmap->pixmap = gdk_pixmap_new(area->window,
					      area->allocation.width,
					      area->allocation.height, -1);
		guimap_display(gmap);
	}

	cr = gdk_cairo_create(area->window);
	gdk_cairo_set_source_pixmap(cr, gmap->pixmap, 0, 0);
	gdk_cairo_rectangle(cr, &event->area);
	cairo_fill(cr);
	cairo_destroy(cr);
	return FALSE;
}

static gint configure_map_cb(GtkWidget * area,
			     G_GNUC_UNUSED GdkEventConfigure * event,
			     gpointer user_data)
{
	GuiMap *gmap = user_data;

	if (area->window == NULL || gmap->map == NULL)
		return FALSE;

	if (gmap->pixmap) {
		g_object_unref(gmap->pixmap);
		gmap->pixmap = NULL;
	}
	guimap_scale_to_size(gmap,
			     area->allocation.width,
			     area->allocation.height);

	gtk_widget_queue_draw(area);
	return FALSE;
}

static gboolean motion_notify_map_cb(GtkWidget * area,
				     GdkEventMotion * event,
				     gpointer user_data)
{
	GuiMap *gmap = user_data;
	gint x;
	gint y;
	static gint last_x;
	static gint last_y;
	GdkModifierType state;
	MapElement dummyElement;
	g_assert(area != NULL);

	if (area->window == NULL || gmap->map == NULL)
		return FALSE;

	if (event->is_hint)
		gdk_window_get_pointer(event->window, &x, &y, &state);
	else {
		x = event->x;
		y = event->y;
		state = event->state;
	}

	if (state & GDK_BUTTON2_MASK) {
		gmap->is_custom_view = TRUE;
		gmap->x_margin += x - last_x;
		gmap->y_margin += y - last_y;

		guimap_display(gmap);
		gtk_widget_queue_draw(gmap->area);
	}
	last_x = x;
	last_y = y;

	dummyElement.pointer = NULL;
	guimap_cursor_move(gmap, x, y, &dummyElement);

	return FALSE;
}

static gboolean zoom_map_cb(GtkWidget * area, GdkEventScroll * event,
			    gpointer user_data)
{
	GuiMap *gmap = user_data;

	if (area->window == NULL || gmap->map == NULL)
		return FALSE;

	gint radius = gmap->hex_radius;
	gmap->is_custom_view = TRUE;

	if (event->direction == GDK_SCROLL_UP) {
		radius += ZOOM_AMOUNT;

		gmap->x_margin -= (event->x - gmap->x_margin) *
		    ZOOM_AMOUNT / radius;
		gmap->y_margin -= (event->y - gmap->y_margin) *
		    ZOOM_AMOUNT / radius;

	} else if (event->direction == GDK_SCROLL_DOWN) {
		gint old_radius = radius;
		radius -= ZOOM_AMOUNT;
		if (radius < MIN_HEX_RADIUS)
			radius = MIN_HEX_RADIUS;

		gmap->x_margin += (event->x - gmap->x_margin) *
		    (old_radius - radius) / radius;
		gmap->y_margin += (event->y - gmap->y_margin) *
		    (old_radius - radius) / radius;

	}
	gmap->hex_radius = radius;
	guimap_scale_to_size(gmap,
			     gmap->area->allocation.width,
			     gmap->area->allocation.height);

	guimap_display(gmap);
	gtk_widget_queue_draw(gmap->area);
	return FALSE;
}

GtkWidget *guimap_build_drawingarea(GuiMap * gmap, gint width, gint height)
{
	gmap->area = gtk_drawing_area_new();
	g_object_ref(gmap->area);

	gtk_widget_set_events(gmap->area, GDK_EXPOSURE_MASK
			      | GDK_POINTER_MOTION_MASK
			      | GDK_POINTER_MOTION_HINT_MASK);

	gtk_widget_set_size_request(gmap->area, width, height);
	g_signal_connect(G_OBJECT(gmap->area), "expose_event",
			 G_CALLBACK(expose_map_cb), gmap);
	g_signal_connect(G_OBJECT(gmap->area), "configure_event",
			 G_CALLBACK(configure_map_cb), gmap);

	g_signal_connect(G_OBJECT(gmap->area), "motion_notify_event",
			 G_CALLBACK(motion_notify_map_cb), gmap);
	g_signal_connect(G_OBJECT(gmap->area), "scroll_event",
			 G_CALLBACK(zoom_map_cb), gmap);

	gtk_widget_show(gmap->area);

	return gmap->area;
}

static GdkPoint settlement_points[] = {
	{20, 20}, {20, -8}, {0, -28}, {-20, -8},
	{-20, 20}, {20, 20}
};

static Polygon settlement_poly = {
	settlement_points,
	G_N_ELEMENTS(settlement_points)
};

static GdkPoint city_points[] = {
	{40, 20}, {40, -16}, {2, -16}, {2, -28},
	{-19, -48}, {-40, -28}, {-40, 20}, {40, 20}
};

static Polygon city_poly = {
	city_points,
	G_N_ELEMENTS(city_points)
};

static GdkPoint city_wall_points[] = {
	{50, 36}, {50, -64}, {-50, -64}, {-50, 36}
};

static Polygon city_wall_poly = {
	city_wall_points,
	G_N_ELEMENTS(city_wall_points)
};

static GdkPoint nosetup_points[] = {
	{0, 30}, {26, 15}, {26, -15}, {0, -30},
	{-26, -15}, {-26, 15}, {0, 30}
};

static Polygon nosetup_poly = {
	nosetup_points,
	G_N_ELEMENTS(nosetup_points)
};

/* Update this when a node polygon is changed */
#define NODE_MIN_X -50
#define NODE_MAX_X 50
#define NODE_MIN_Y -64
#define NODE_MAX_Y 36

static GdkPoint largest_node_points[] = {
	{NODE_MIN_X, NODE_MIN_Y},
	{NODE_MIN_X, NODE_MAX_Y},
	{NODE_MAX_X, NODE_MAX_Y},
	{NODE_MAX_X, NODE_MIN_Y}
};

static Polygon largest_node_poly = {
	largest_node_points,
	G_N_ELEMENTS(largest_node_points)
};

static GdkPoint road_points[] = {
	{10, 40}, {10, -40}, {-10, -40}, {-10, 40},
	{10, 40}
};

static Polygon road_poly = {
	road_points,
	G_N_ELEMENTS(road_points)
};

static GdkPoint ship_points[] = {
	{10, 32}, {10, 8}, {24, 18}, {42, 8},
	{48, 0}, {50, -12}, {10, -12}, {10, -32},
	{2, -32}, {-6, -26}, {-10, -16}, {-10, 16},
	{-6, 26}, {2, 32}, {10, 32}
};

static Polygon ship_poly = {
	ship_points,
	G_N_ELEMENTS(ship_points)
};

static GdkPoint bridge_points[] = {
	{13, 40}, {-14, 40}, {-14, 30}, {-1, 15},
	{-1, -15}, {-14, -30}, {-14, -40}, {13, -40},
	{13, 40}
};

static Polygon bridge_poly = {
	bridge_points,
	G_N_ELEMENTS(bridge_points)
};

/* Update this when an edge polygon is changed */
#define EDGE_MIN_X -14
#define EDGE_MAX_X 50
#define EDGE_MIN_Y -40
#define EDGE_MAX_Y 40

static GdkPoint largest_edge_points[] = {
	{EDGE_MIN_X, EDGE_MIN_Y},
	{EDGE_MIN_X, EDGE_MAX_Y},
	{EDGE_MAX_X, EDGE_MAX_Y},
	{EDGE_MAX_X, EDGE_MIN_Y}
};

static Polygon largest_edge_poly = {
	largest_edge_points,
	G_N_ELEMENTS(largest_edge_points)
};

static GdkPoint robber_points[] = {
	{30, 60}, {30, 4}, {28, -6}, {22, -15},
	{12, -20}, {22, -32}, {22, -48}, {10, -60},
	{-10, -60}, {-22, -48}, {-22, -32}, {-12, -20},
	{-22, -15}, {-28, -6}, {-30, 4}, {-30, 60},
	{30, 60}
};

static Polygon robber_poly = {
	robber_points,
	G_N_ELEMENTS(robber_points)
};

static GdkPoint pirate_points[] = {
	{42, 15}, {18, 15}, {28, 1}, {18, -17},
	{10, -23}, {-2, -25}, {-2, 15}, {-22, 15},
	{-22, 23}, {-16, 31}, {-6, 35}, {26, 35},
	{36, 31}, {42, 23}, {42, 15}
};

static Polygon pirate_poly = {
	pirate_points,
	G_N_ELEMENTS(pirate_points)
};

static gint chances[13] = {
	0, 0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1
};

static void reverse_calc_hex_pos(const GuiMap * gmap,
				 gint x_coor, gint y_coor, gint * hex_x,
				 gint * hex_y)
{
	y_coor -= gmap->y_margin + gmap->hex_radius;
	y_coor += gmap->hex_radius;
	*hex_y = y_coor / (gmap->hex_radius + gmap->y_point);

	x_coor -=
	    gmap->x_margin + gmap->x_point +
	    ((*hex_y % 2) ? gmap->x_point : 0);
	if (gmap->map->shrink_left)
		x_coor += gmap->x_point;
	x_coor += gmap->x_point;
	*hex_x = x_coor / (gmap->x_point * 2);

	/* The (0,0) will be on the upper left corner of the hex,
	 * outside the hex */
	gint relx;
	gint rely;
	relx = x_coor - *hex_x * (gmap->x_point * 2);
	rely = y_coor - *hex_y * (gmap->hex_radius + gmap->y_point);

	/* Choice between various hexes possible */
	if (rely < gmap->y_point) {
		if (relx < gmap->x_point) {
			/* At the left side of the hex */
			/* If point (relx, rely)) above the line from
			 * (0, y_point) to (x_point, 0) then it is the
			 * hex to NW, else it is the current hex
			 */
			if (-relx * (gdouble) gmap->y_point /
			    gmap->x_point + gmap->y_point > rely) {
				map_move_in_direction(HEX_DIR_NW, hex_x,
						      hex_y);
			}
		} else {
			/* At the right side of the hex */
			/* If point (relx, rely)) above the line from
			 * (x_point, 0) to (2*x_point, y_point) then
			 * it is the hex to NE, else it is the current hex
			 */
			if (relx * (gdouble) gmap->y_point /
			    gmap->x_point - gmap->y_point > rely) {
				map_move_in_direction(HEX_DIR_NE, hex_x,
						      hex_y);
			}
		}
	}
}

static void calc_hex_pos(const GuiMap * gmap,
			 gint x, gint y, gint * x_offset, gint * y_offset)
{
	*x_offset = gmap->x_margin
	    + gmap->x_point + ((y % 2) ? gmap->x_point : 0)
	    + x * gmap->x_point * 2;
	if (gmap->map->shrink_left)
		*x_offset -= gmap->x_point;
	*y_offset = gmap->y_margin
	    + gmap->hex_radius + y * (gmap->hex_radius + gmap->y_point);
}

static void get_hex_polygon(const GuiMap * gmap, Polygon * poly)
{
	GdkPoint *points;

	g_assert(poly->num_points >= 6);

	poly->num_points = 6;
	points = poly->points;
	points[0].x = gmap->x_point;
	points[0].y = -gmap->y_point;
	points[1].x = 0;
	points[1].y = -gmap->hex_radius;
	points[2].x = -gmap->x_point;
	points[2].y = -gmap->y_point;
	points[3].x = -gmap->x_point;
	points[3].y = gmap->y_point;
	points[4].x = 0;
	points[4].y = gmap->hex_radius;
	points[5].x = gmap->x_point;
	points[5].y = gmap->y_point;
}

static void calc_edge_poly(const GuiMap * gmap, const Edge * edge,
			   const Polygon * shape, Polygon * poly)
{
	gint idx;
	GdkPoint *poly_point, *shape_point;
	double theta, cos_theta, sin_theta, scale;

	g_assert(poly->num_points >= shape->num_points);
	poly->num_points = shape->num_points;

	/* Determine the angle for the polygon:
	 * Polygons on edges are rotated 0, 60 or 120 degrees CCW
	 * Polygons without edges are rotated 90 degrees CCW
	 */
	theta =
	    2 * M_PI * (edge != NULL ? 1 - (edge->pos % 3) / 6.0 : -0.25);
	cos_theta = cos(theta);
	sin_theta = sin(theta);
	scale = (2 * gmap->y_point) / 120.0;

	/* Rotate / scale all points
	 */
	poly_point = poly->points;
	shape_point = shape->points;
	for (idx = 0; idx < shape->num_points;
	     idx++, shape_point++, poly_point++) {
		poly_point->x = rint(scale * shape_point->x);
		poly_point->y = rint(scale * shape_point->y);
		if (edge == NULL || edge->pos % 3 > 0) {
			gint x = poly_point->x;
			gint y = poly_point->y;
			poly_point->x =
			    rint(x * cos_theta - y * sin_theta);
			poly_point->y =
			    rint(x * sin_theta + y * cos_theta);
		}
	}

	/* Offset shape to hex & edge
	 */
	if (edge != NULL) {
		gint x_offset, y_offset;

		/* Recalculate the angle for the position of the edge */
		theta = 2 * M_PI * (1 - edge->pos / 6.0);
		cos_theta = cos(theta);
		sin_theta = sin(theta);

		calc_hex_pos(gmap, edge->x, edge->y, &x_offset, &y_offset);
		x_offset += gmap->x_point * cos_theta;
		y_offset += gmap->x_point * sin_theta;
		poly_offset(poly, x_offset, y_offset);
	}
}

static void calc_node_poly(const GuiMap * gmap, const Node * node,
			   const Polygon * shape, Polygon * poly)
{
	gint idx;
	GdkPoint *poly_point, *shape_point;
	double scale;

	g_assert(poly->num_points >= shape->num_points);
	poly->num_points = shape->num_points;

	scale = (2 * gmap->y_point) / 120.0;

	/* Scale all points
	 */
	poly_point = poly->points;
	shape_point = shape->points;
	for (idx = 0; idx < shape->num_points;
	     idx++, shape_point++, poly_point++) {
		poly_point->x = rint(scale * shape_point->x);
		poly_point->y = rint(scale * shape_point->y);
	}

	/* Offset shape to hex & node
	 */
	if (node != NULL) {
		gint x_offset, y_offset;
		double theta;

		calc_hex_pos(gmap, node->x, node->y, &x_offset, &y_offset);
		theta = 2 * M_PI / 6.0 * node->pos + M_PI / 6.0;
		theta = 2 * M_PI - theta;
		x_offset += gmap->hex_radius * cos(theta);
		y_offset += gmap->hex_radius * sin(theta);
		poly_offset(poly, x_offset, y_offset);
	}
}

static void calc_hex_poly(const GuiMap * gmap, const Hex * hex,
			  const Polygon * shape, Polygon * poly,
			  double scale_factor, gint x_shift)
{
	GdkPoint *poly_point, *shape_point;
	double scale;
	gint x_offset, y_offset;
	gint idx;

	g_assert(poly->num_points >= shape->num_points);
	poly->num_points = shape->num_points;
	scale = (2 * gmap->y_point) / scale_factor;

	if (hex != NULL) {
		calc_hex_pos(gmap, hex->x, hex->y, &x_offset, &y_offset);
		x_offset += rint(scale * x_shift);
	} else
		x_offset = y_offset = 0;

	/* Scale all points, offset to right
	 */
	poly_point = poly->points;
	shape_point = shape->points;
	for (idx = 0; idx < shape->num_points;
	     idx++, shape_point++, poly_point++) {
		poly_point->x = x_offset + rint(scale * shape_point->x);
		poly_point->y = y_offset + rint(scale * shape_point->y);
	}
}

void guimap_road_polygon(const GuiMap * gmap, const Edge * edge,
			 Polygon * poly)
{
	calc_edge_poly(gmap, edge, &road_poly, poly);
}

void guimap_ship_polygon(const GuiMap * gmap, const Edge * edge,
			 Polygon * poly)
{
	calc_edge_poly(gmap, edge, &ship_poly, poly);
}

void guimap_bridge_polygon(const GuiMap * gmap, const Edge * edge,
			   Polygon * poly)
{
	calc_edge_poly(gmap, edge, &bridge_poly, poly);
}

void guimap_city_polygon(const GuiMap * gmap, const Node * node,
			 Polygon * poly)
{
	calc_node_poly(gmap, node, &city_poly, poly);
}

void guimap_settlement_polygon(const GuiMap * gmap, const Node * node,
			       Polygon * poly)
{
	calc_node_poly(gmap, node, &settlement_poly, poly);
}

void guimap_city_wall_polygon(const GuiMap * gmap, const Node * node,
			      Polygon * poly)
{
	calc_node_poly(gmap, node, &city_wall_poly, poly);
}

static void guimap_nosetup_polygon(const GuiMap * gmap, const Node * node,
				   Polygon * poly)
{
	calc_node_poly(gmap, node, &nosetup_poly, poly);
}

static void guimap_robber_polygon(const GuiMap * gmap, const Hex * hex,
				  Polygon * poly)
{
	calc_hex_poly(gmap, hex, &robber_poly, poly, 140.0, 50);
}

static void guimap_pirate_polygon(const GuiMap * gmap, const Hex * hex,
				  Polygon * poly)
{
	calc_hex_poly(gmap, hex, &pirate_poly, poly, 80.0, 0);
}

void draw_dice_roll(PangoLayout * layout, cairo_t * cr,
		    gdouble x_offset, gdouble y_offset, gdouble radius,
		    gint n, gint terrain, gboolean highlight)
{
	gchar num[10];
	gint height;
	gint width;
	gdouble x;
	gdouble y;
	gint idx;
	MapTheme *theme = theme_get_current();
	THEME_COLOR col;
	TColor *tcol;
	gint width_sqr;

#define col_or_ovr(ter,cno)												\
	((terrain < TC_MAX_OVRTILE && theme->ovr_colors[ter][cno].set) ?	\
	 &(theme->ovr_colors[ter][cno]) :									\
	 &(theme->colors[cno]))

	cairo_set_line_width(cr, 1.0);
	col = highlight ? TC_CHIP_H_BG : TC_CHIP_BG;
	tcol = col_or_ovr(terrain, col);
	if (!tcol->transparent) {
		gdk_cairo_set_source_color(cr, &(tcol->color));
		cairo_move_to(cr, x_offset + radius, y_offset);
		cairo_arc(cr, x_offset, y_offset, radius, 0.0, 2 * M_PI);
		cairo_fill(cr);
	}
	tcol = col_or_ovr(terrain, TC_CHIP_BD);
	if (!tcol->transparent) {
		gdk_cairo_set_source_color(cr, &(tcol->color));
		cairo_move_to(cr, x_offset + radius, y_offset);
		cairo_arc(cr, x_offset, y_offset, radius, 0.0, 2 * M_PI);
		cairo_stroke(cr);
	}
	col = (n == 6 || n == 8) ? TC_CHIP_H_FG : TC_CHIP_FG;
	tcol = col_or_ovr(terrain, col);
	if (!tcol->transparent) {
		sprintf(num, "<b>%d</b>", n);
		pango_layout_set_markup(layout, num, -1);
		pango_layout_get_pixel_size(layout, &width, &height);
		gdk_cairo_set_source_color(cr, &(tcol->color));
		cairo_move_to(cr, x_offset - width / 2,
			      y_offset - height / 2);
		pango_cairo_show_layout(cr, layout);

		width_sqr = sqr(radius) - sqr(height / 2);
		if (width_sqr >= sqr(6 * 2)) {
			/* Enough space available for the dots */
			x = x_offset - chances[n] * 4 / 2.0;
			y = y_offset - 1 + height / 2.0;
			for (idx = 0; idx < chances[n]; idx++) {
				cairo_arc(cr, x + 2, y + 2, 1.0, 0.0,
					  2 * M_PI);
				cairo_fill(cr);
				x += 4;
			}
		}
	}
}

static gboolean display_hex(const Hex * hex, gpointer closure)
{
	gint x_offset, y_offset;
	GdkPoint points[MAX_POINTS];
	Polygon poly;
	int idx;
	const MapTheme *theme = theme_get_current();
	const GuiMap *gmap = closure;

	calc_hex_pos(gmap, hex->x, hex->y, &x_offset, &y_offset);
	cairo_set_line_width(gmap->cr, 1.0);

	/* Fill the hex with the nice pattern */
	poly.points = points;
	poly.num_points = G_N_ELEMENTS(points);
	get_hex_polygon(gmap, &poly);
	poly_offset(&poly, x_offset, y_offset);

	/* Draw the hex */
	gdk_cairo_set_source_pixmap(gmap->cr,
				    theme->terrain_tiles[hex->terrain],
				    x_offset - gmap->x_point,
				    y_offset - gmap->hex_radius);
	cairo_pattern_set_extend(cairo_get_source(gmap->cr),
				 CAIRO_EXTEND_REPEAT);
	poly_draw(gmap->cr, TRUE, &poly);

	/* Draw border around hex */
	if (!theme->colors[TC_HEX_BD].transparent) {
		gdk_cairo_set_source_color(gmap->cr,
					   &theme->
					   colors[TC_HEX_BD].color);
		poly_draw(gmap->cr, FALSE, &poly);
	}

	/* Draw the dice roll */
	if (hex->roll > 0 && gmap->chit_radius > 0) {
		g_assert(gmap->layout);
		draw_dice_roll(gmap->layout, gmap->cr,
			       x_offset, y_offset, gmap->chit_radius,
			       hex->roll, hex->terrain,
			       !hex->robber
			       && hex->roll == gmap->highlight_chit);
	}

	/* Draw ports */
	if (hex->resource != NO_RESOURCE && gmap->chit_radius > 0) {
		const gchar *str = "";
		gint width, height;
		int tileno = hex->resource == ANY_RESOURCE ?
		    ANY_PORT_TILE : hex->resource;
		gboolean drawit;
		gboolean typeind;

		/* Draw lines from port to shore */
		gdk_cairo_set_source_color(gmap->cr, &white);
		const double dashes[] = { 4.0 };
		cairo_set_dash(gmap->cr, dashes, G_N_ELEMENTS(dashes),
			       0.0);
		cairo_move_to(gmap->cr, x_offset, y_offset);
		cairo_line_to(gmap->cr, points[(hex->facing + 5) % 6].x,
			      points[(hex->facing + 5) % 6].y);
		cairo_move_to(gmap->cr, x_offset, y_offset);
		cairo_line_to(gmap->cr, points[hex->facing].x,
			      points[hex->facing].y);
		cairo_stroke(gmap->cr);
		cairo_set_dash(gmap->cr, NULL, 0, 0.0);

		/* Fill/tile port indicator */
		if (theme->port_tiles[tileno]) {
			gdk_cairo_set_source_pixmap(gmap->cr,
						    theme->port_tiles
						    [tileno],
						    x_offset -
						    theme->port_tiles_width
						    [tileno] / 2,
						    y_offset -
						    theme->port_tiles_height
						    [tileno] / 2);
			cairo_pattern_set_extend(cairo_get_source
						 (gmap->cr),
						 CAIRO_EXTEND_REPEAT);
			typeind = TRUE;
			drawit = TRUE;
		} else if (!theme->colors[TC_PORT_BG].transparent) {
			gdk_cairo_set_source_color(gmap->cr,
						   &theme->colors
						   [TC_PORT_BG].color);
			typeind = FALSE;
			drawit = TRUE;
		} else {
			typeind = FALSE;
			drawit = FALSE;
		}
		if (drawit) {
			cairo_arc(gmap->cr, x_offset, y_offset,
				  gmap->chit_radius, 0.0, 2 * M_PI);
			cairo_fill(gmap->cr);
		}

		/* Outline port indicator */
		if (!theme->colors[TC_PORT_BD].transparent) {
			gdk_cairo_set_source_color(gmap->cr,
						   &theme->colors
						   [TC_PORT_BD].color);
			cairo_arc(gmap->cr, x_offset, y_offset,
				  gmap->chit_radius, 0.0, 2 * M_PI);
			cairo_stroke(gmap->cr);
		}
		/* Print trading ratio */
		if (!theme->colors[TC_PORT_FG].transparent) {
			if (typeind) {
				if (hex->resource < NO_RESOURCE)
					/* Port indicator for a resource: trade 2 for 1 */
					str = _("2:1");
				else
					/* Port indicator: trade 3 for 1 */
					str = _("3:1");
			} else {
				switch (hex->resource) {
				case BRICK_RESOURCE:
					/* Port indicator for brick */
					str = Q_("Brick port|B");
					break;
				case GRAIN_RESOURCE:
					/* Port indicator for grain */
					str = Q_("Grain port|G");
					break;
				case ORE_RESOURCE:
					/* Port indicator for ore */
					str = Q_("Ore port|O");
					break;
				case WOOL_RESOURCE:
					/* Port indicator for wool */
					str = Q_("Wool port|W");
					break;
				case LUMBER_RESOURCE:
					/* Port indicator for lumber */
					str = Q_("Lumber port|L");
					break;
				default:
					/* General port indicator */
					str = _("3:1");
					break;
				}
			}
			pango_layout_set_markup(gmap->layout, str, -1);
			pango_layout_get_pixel_size(gmap->layout, &width,
						    &height);
			gdk_cairo_set_source_color(gmap->cr,
						   &theme->colors
						   [TC_PORT_FG].color);
			cairo_move_to(gmap->cr, x_offset - width / 2,
				      y_offset - height / 2);
			pango_cairo_show_layout(gmap->cr, gmap->layout);
		}
	}

	cairo_set_line_width(gmap->cr, 1.0);
	/* Draw all roads and ships */
	for (idx = 0; idx < G_N_ELEMENTS(hex->edges); idx++) {
		const Edge *edge = hex->edges[idx];
		if (edge->owner < 0)
			continue;

		poly.num_points = G_N_ELEMENTS(points);
		switch (edge->type) {
		case BUILD_ROAD:
			guimap_road_polygon(gmap, edge, &poly);
			break;
		case BUILD_SHIP:
			guimap_ship_polygon(gmap, edge, &poly);
			break;
		case BUILD_BRIDGE:
			guimap_bridge_polygon(gmap, edge, &poly);
			break;
		default:
			g_assert_not_reached();
			break;
		}
		gdk_cairo_set_source_color(gmap->cr,
					   colors_get_player(edge->owner));
		poly_draw_with_border(gmap->cr, &black, &poly);
	}

	/* Draw all buildings */
	for (idx = 0; idx < G_N_ELEMENTS(hex->nodes); idx++) {
		const Node *node = hex->nodes[idx];
		const GdkColor *color;

		if (node->owner < 0 && !gmap->show_nosetup_nodes)
			continue;

		if (node->owner < 0) {
			if (!node->no_setup)
				continue;
			color = &white;
			poly.num_points = G_N_ELEMENTS(points);
			guimap_nosetup_polygon(gmap, node, &poly);
		} else {
			color = colors_get_player(node->owner);

			/* If there are city walls, draw them. */
			if (node->city_wall) {
				poly.num_points = G_N_ELEMENTS(points);
				guimap_city_wall_polygon(gmap, node,
							 &poly);
				gdk_cairo_set_source_color(gmap->cr,
							   color);
				poly_draw_with_border(gmap->cr, &black,
						      &poly);
			}
			/* Draw the building */
			color = colors_get_player(node->owner);
			poly.num_points = G_N_ELEMENTS(points);
			if (node->type == BUILD_CITY)
				guimap_city_polygon(gmap, node, &poly);
			else
				guimap_settlement_polygon(gmap, node,
							  &poly);
		}
		gdk_cairo_set_source_color(gmap->cr, color);
		poly_draw_with_border(gmap->cr, &black, &poly);
	}

	/* Draw the robber */
	if (hex->robber) {
		poly.num_points = G_N_ELEMENTS(points);
		guimap_robber_polygon(gmap, hex, &poly);
		cairo_set_line_width(gmap->cr, 1.0);
		if (!theme->colors[TC_ROBBER_FG].transparent) {
			gdk_cairo_set_source_color(gmap->cr,
						   &theme->colors
						   [TC_ROBBER_FG].color);
			poly_draw(gmap->cr, TRUE, &poly);
		}
		if (!theme->colors[TC_ROBBER_BD].transparent) {
			gdk_cairo_set_source_color(gmap->cr,
						   &theme->colors
						   [TC_ROBBER_BD].color);
			poly_draw(gmap->cr, FALSE, &poly);
		}
	}

	/* Draw the pirate */
	if (hex == hex->map->pirate_hex) {
		poly.num_points = G_N_ELEMENTS(points);
		guimap_pirate_polygon(gmap, hex, &poly);
		cairo_set_line_width(gmap->cr, 1.0);
		if (!theme->colors[TC_ROBBER_FG].transparent) {
			gdk_cairo_set_source_color(gmap->cr,
						   &theme->colors
						   [TC_ROBBER_FG].color);
			poly_draw(gmap->cr, TRUE, &poly);
		}
		if (!theme->colors[TC_ROBBER_BD].transparent) {
			gdk_cairo_set_source_color(gmap->cr,
						   &theme->colors
						   [TC_ROBBER_BD].color);
			poly_draw(gmap->cr, FALSE, &poly);
		}
	}

	return FALSE;
}

void guimap_scale_with_radius(GuiMap * gmap, gint radius)
{
	if (radius < MIN_HEX_RADIUS)
		radius = MIN_HEX_RADIUS;

	gmap->hex_radius = radius;
	gmap->x_point = radius * cos(M_PI / 6.0);
	gmap->y_point = radius * sin(M_PI / 6.0);

	gmap->x_margin = gmap->y_margin = 0;

	if (gmap->map == NULL)
		return;

	gmap->width =
	    gmap->map->x_size * 2 * gmap->x_point + gmap->x_point;
	gmap->height =
	    (gmap->map->y_size - 1) * (gmap->hex_radius + gmap->y_point)
	    + 2 * gmap->hex_radius;

	if (gmap->map->shrink_left)
		gmap->width -= gmap->x_point;
	if (gmap->map->shrink_right)
		gmap->width -= gmap->x_point;

	if (gmap->cr != NULL) {
		cairo_destroy(gmap->cr);
		gmap->cr = NULL;
	}

	gmap->chit_radius = 15;

	theme_rescale(2 * gmap->x_point);
}

void guimap_scale_to_size(GuiMap * gmap, gint width, gint height)
{
	if (gmap->is_custom_view) {
		gint x_margin = gmap->x_margin;
		gint y_margin = gmap->y_margin;
		guimap_scale_with_radius(gmap, gmap->hex_radius);
		gmap->x_margin = x_margin;
		gmap->y_margin = y_margin;
		gmap->width = width;
		gmap->height = height;
		return;
	}
	const gint reserved_width = 0;
	const gint reserved_height = 0;
	gint width_radius;
	gint height_radius;

	width_radius = (width - reserved_width)
	    / ((gmap->map->x_size * 2 + 1
		- gmap->map->shrink_left
		- gmap->map->shrink_right) * cos(M_PI / 6.0));
	height_radius = (height - reserved_height)
	    / ((gmap->map->y_size - 1)
	       * (sin(M_PI / 6.0) + 1) + 2);

	if (width_radius < height_radius)
		guimap_scale_with_radius(gmap, width_radius);
	else
		guimap_scale_with_radius(gmap, height_radius);

	gmap->x_margin += (width - gmap->width) / 2;
	gmap->y_margin += (height - gmap->height) / 2;

	gmap->width = width;
	gmap->height = height;
}

/** @return The radius of the chit for the current font size */
gint guimap_get_chit_radius(PangoLayout * layout, gboolean show_dots)
{
	gint width, height;
	gint size_for_99_sqr;
	gint size_for_port_sqr;
	gint size_for_text_sqr;

	/* Calculate the maximum size of the text in the chits */
	pango_layout_set_markup(layout, "<b>99</b>", -1);
	pango_layout_get_pixel_size(layout, &width, &height);
	size_for_99_sqr = sqr(width) + sqr(height);

	pango_layout_set_markup(layout, "3:1", -1);
	pango_layout_get_pixel_size(layout, &width, &height);
	size_for_port_sqr = sqr(width) + sqr(height);

	size_for_text_sqr = MAX(size_for_99_sqr, size_for_port_sqr);
	if (show_dots) {
		gint size_with_dots = sqr(height / 2 + 2) + sqr(6 * 2);
		if (size_with_dots * 4 > size_for_text_sqr)
			return sqrt(size_with_dots);
	}
	/* Divide: calculations should have been sqr(width/2)+sqr(height/2) */
	return sqrt(size_for_text_sqr) / 2;
}

void guimap_display(GuiMap * gmap)
{
	gint maximum_size;
	gint size_for_text;
	PangoContext *pc;
	PangoFontDescription *pfd;
	gint font_size;

	if (gmap->pixmap == NULL)
		return;

	if (gmap->cr != NULL) {
		cairo_destroy(gmap->cr);
	}

	gmap->cr = gdk_cairo_create(gmap->pixmap);

	gdk_cairo_set_source_pixmap(gmap->cr,
				    theme_get_current()->terrain_tiles
				    [BOARD_TILE], 0, 0);
	cairo_pattern_set_extend(cairo_get_source(gmap->cr),
				 CAIRO_EXTEND_REPEAT);
	cairo_rectangle(gmap->cr, 0, 0, gmap->width, gmap->height);
	cairo_fill(gmap->cr);

	if (gmap->layout != NULL)
		g_object_unref(gmap->layout);
	gmap->layout = gtk_widget_create_pango_layout(gmap->area, "");

	/* Manipulate the font size */
	pc = pango_layout_get_context(gmap->layout);
	pfd = pango_context_get_font_description(pc);

	/* Store the initial font size, since it is remembered for the area */
	if (gmap->initial_font_size == -1) {
		font_size = pango_font_description_get_size(pfd);
		gmap->initial_font_size = font_size;
	} else {
		font_size = gmap->initial_font_size;
	}

	/* The radius of the chit is at most 67% of the tile,
	 * so the terrain can be seen.
	 */
	maximum_size = gmap->hex_radius * 2 / 3;

	/* First attempt to fit the text and the dots in the chit */
	pango_font_description_set_size(pfd, font_size);
	pango_layout_set_font_description(gmap->layout, pfd);

	size_for_text = guimap_get_chit_radius(gmap->layout, TRUE);

	/* Shrink the font size until the letters fit in the chit */
	while (maximum_size < size_for_text && font_size > 0) {
		pango_font_description_set_size(pfd, font_size);
		pango_layout_set_font_description(gmap->layout, pfd);
		font_size -= PANGO_SCALE;

		size_for_text =
		    guimap_get_chit_radius(gmap->layout, FALSE);
	};
	if (font_size <= 0) {
		gmap->chit_radius = 0;
	} else {
		gmap->chit_radius = size_for_text;
	}

	map_traverse_const(gmap->map, display_hex, gmap);
}

void guimap_zoom_normal(GuiMap * gmap)
{
	gmap->is_custom_view = FALSE;
	guimap_scale_to_size(gmap,
			     gmap->area->allocation.width,
			     gmap->area->allocation.height);
	guimap_display(gmap);
	gtk_widget_queue_draw(gmap->area);
}

void guimap_zoom_center_map(GuiMap * gmap)
{
	if (!gmap->is_custom_view)
		return;
	gint width = gmap->map->x_size * 2 * gmap->x_point + gmap->x_point;
	if (gmap->map->shrink_left)
		width -= gmap->x_point;
	if (gmap->map->shrink_right)
		width -= gmap->x_point;

	gmap->x_margin = gmap->area->allocation.width / 2 - width / 2;

	gint height = (gmap->map->y_size - 1) *
	    (gmap->hex_radius + gmap->y_point)
	    + 2 * gmap->hex_radius;

	gmap->y_margin = gmap->area->allocation.height / 2 - height / 2;

	guimap_display(gmap);
	gtk_widget_queue_draw(gmap->area);
}

static void find_edge(GuiMap * gmap, gint x, gint y, MapElement * element)
{
	Hex *hex = guimap_find_hex(gmap, x, y);
	if (hex) {
		gint center_x;
		gint center_y;
		calc_hex_pos(gmap, hex->x, hex->y, &center_x, &center_y);

		gdouble angle = atan2(y - center_y, x - center_x);
		gint idx =
		    (gint) (floor(-angle / 2.0 / M_PI * 6 + 0.5) + 6) % 6;
		element->edge = hex->edges[idx];
	} else {
		element->edge = NULL;
	}
}

Node *guimap_find_node(GuiMap * gmap, gint x, gint y)
{
	Hex *hex = guimap_find_hex(gmap, x, y);
	if (hex) {
		gint center_x;
		gint center_y;
		calc_hex_pos(gmap, hex->x, hex->y, &center_x, &center_y);

		gdouble angle = atan2(y - center_y, x - center_x);
		gint idx = (gint) (floor(-angle / 2.0 / M_PI * 6) + 6) % 6;
		return hex->nodes[idx];
	}
	return NULL;
}

static void find_node(GuiMap * gmap, gint x, gint y, MapElement * element)
{
	element->node = guimap_find_node(gmap, x, y);
}

Hex *guimap_find_hex(GuiMap * gmap, gint x, gint y)
{
	gint x_hex;
	gint y_hex;

	reverse_calc_hex_pos(gmap, x, y, &x_hex, &y_hex);
	return map_hex(gmap->map, x_hex, y_hex);
}

static void find_hex(GuiMap * gmap, gint x, gint y, MapElement * element)
{
	element->hex = guimap_find_hex(gmap, x, y);
}

void guimap_draw_edge(GuiMap * gmap, const Edge * edge)
{
	GdkRectangle rect;
	gint idx;
	Polygon poly;
	GdkPoint points[MAX_POINTS];

	g_return_if_fail(edge != NULL);
	g_return_if_fail(gmap->pixmap != NULL);

	poly.num_points = G_N_ELEMENTS(points);
	poly.points = points;
	calc_edge_poly(gmap, edge, &largest_edge_poly, &poly);
	poly_bound_rect(&poly, 1, &rect);

	gdk_cairo_set_source_pixmap(gmap->cr,
				    theme_get_current()->terrain_tiles
				    [BOARD_TILE], 0, 0);
	cairo_pattern_set_extend(cairo_get_source(gmap->cr),
				 CAIRO_EXTEND_REPEAT);
	cairo_rectangle(gmap->cr, rect.x, rect.y, rect.width, rect.height);
	cairo_fill(gmap->cr);

	for (idx = 0; idx < G_N_ELEMENTS(edge->hexes); idx++)
		if (edge->hexes[idx] != NULL)
			display_hex(edge->hexes[idx], gmap);

	gdk_window_invalidate_rect(gmap->area->window, &rect, FALSE);
}

static void draw_cursor(GuiMap * gmap, gint owner, const Polygon * poly)
{
	GdkRectangle rect;

	g_return_if_fail(gmap->cursor.pointer != NULL);

	cairo_set_line_width(gmap->cr, 3.0);
	gdk_cairo_set_source_color(gmap->cr, colors_get_player(owner));
	poly_draw_with_border(gmap->cr, &green, poly);

	poly_bound_rect(poly, 1, &rect);
	gdk_window_invalidate_rect(gmap->area->window, &rect, FALSE);
}

static void erase_edge_cursor(GuiMap * gmap)
{
	g_return_if_fail(gmap->cursor.pointer != NULL);
	guimap_draw_edge(gmap, gmap->cursor.edge);
}

static void draw_road_cursor(GuiMap * gmap)
{
	GdkPoint points[MAX_POINTS];
	Polygon poly;

	g_return_if_fail(gmap->cursor.pointer != NULL);

	poly.points = points;
	poly.num_points = G_N_ELEMENTS(points);
	guimap_road_polygon(gmap, gmap->cursor.edge, &poly);
	draw_cursor(gmap, gmap->cursor_owner, &poly);
}

static void draw_ship_cursor(GuiMap * gmap)
{
	GdkPoint points[MAX_POINTS];
	Polygon poly;

	g_return_if_fail(gmap->cursor.pointer != NULL);

	poly.points = points;
	poly.num_points = G_N_ELEMENTS(points);
	guimap_ship_polygon(gmap, gmap->cursor.edge, &poly);
	draw_cursor(gmap, gmap->cursor_owner, &poly);
}

static void draw_bridge_cursor(GuiMap * gmap)
{
	GdkPoint points[MAX_POINTS];
	Polygon poly;

	g_return_if_fail(gmap->cursor.pointer != NULL);

	poly.points = points;
	poly.num_points = G_N_ELEMENTS(points);
	guimap_bridge_polygon(gmap, gmap->cursor.edge, &poly);
	draw_cursor(gmap, gmap->cursor_owner, &poly);
}

void guimap_draw_node(GuiMap * gmap, const Node * node)
{
	GdkRectangle rect;
	int idx;
	Polygon poly;
	GdkPoint points[MAX_POINTS];

	g_return_if_fail(node != NULL);
	g_return_if_fail(gmap->pixmap != NULL);

	poly.num_points = G_N_ELEMENTS(points);
	poly.points = points;
	calc_node_poly(gmap, node, &largest_node_poly, &poly);
	poly_bound_rect(&poly, 1, &rect);

	gdk_cairo_set_source_pixmap(gmap->cr,
				    theme_get_current()->terrain_tiles
				    [BOARD_TILE], 0, 0);
	cairo_pattern_set_extend(cairo_get_source(gmap->cr),
				 CAIRO_EXTEND_REPEAT);
	cairo_rectangle(gmap->cr, rect.x, rect.y, rect.width, rect.height);
	cairo_fill(gmap->cr);

	for (idx = 0; idx < G_N_ELEMENTS(node->hexes); idx++)
		if (node->hexes[idx] != NULL)
			display_hex(node->hexes[idx], gmap);

	gdk_window_invalidate_rect(gmap->area->window, &rect, FALSE);
}

static void erase_node_cursor(GuiMap * gmap)
{
	g_return_if_fail(gmap->cursor.pointer != NULL);
	guimap_draw_node(gmap, gmap->cursor.node);
}

static void draw_settlement_cursor(GuiMap * gmap)
{
	GdkPoint points[MAX_POINTS];
	Polygon poly;

	g_return_if_fail(gmap->cursor.pointer != NULL);

	poly.points = points;
	poly.num_points = G_N_ELEMENTS(points);
	guimap_settlement_polygon(gmap, gmap->cursor.node, &poly);
	draw_cursor(gmap, gmap->cursor_owner, &poly);
}

static void draw_city_cursor(GuiMap * gmap)
{
	GdkPoint points[MAX_POINTS];
	Polygon poly;

	g_return_if_fail(gmap->cursor.pointer != NULL);

	poly.points = points;
	poly.num_points = G_N_ELEMENTS(points);
	guimap_city_polygon(gmap, gmap->cursor.node, &poly);
	draw_cursor(gmap, gmap->cursor_owner, &poly);
}

static void draw_city_wall_cursor(GuiMap * gmap)
{
	GdkPoint points[MAX_POINTS];
	Polygon poly;

	g_return_if_fail(gmap->cursor.pointer != NULL);

	poly.points = points;
	poly.num_points = G_N_ELEMENTS(points);
	guimap_city_wall_polygon(gmap, gmap->cursor.node, &poly);
	draw_cursor(gmap, gmap->cursor_owner, &poly);

	poly.points = points;
	poly.num_points = G_N_ELEMENTS(points);
	guimap_city_polygon(gmap, gmap->cursor.node, &poly);
	draw_cursor(gmap, gmap->cursor_owner, &poly);
}

static void draw_steal_building_cursor(GuiMap * gmap)
{
	GdkPoint points[MAX_POINTS];
	Polygon poly;
	const Node *node;

	g_return_if_fail(gmap->cursor.pointer != NULL);

	poly.points = points;
	poly.num_points = G_N_ELEMENTS(points);
	node = gmap->cursor.node;
	switch (node->type) {
	case BUILD_SETTLEMENT:
		guimap_settlement_polygon(gmap, node, &poly);
		draw_cursor(gmap, node->owner, &poly);
		break;
	case BUILD_CITY:
		guimap_city_polygon(gmap, node, &poly);
		draw_cursor(gmap, node->owner, &poly);
		break;
	default:
		g_assert_not_reached();
		break;
	}
}

static void draw_steal_ship_cursor(GuiMap * gmap)
{
	GdkPoint points[MAX_POINTS];
	Polygon poly;
	const Edge *edge;

	g_return_if_fail(gmap->cursor.pointer != NULL);

	poly.points = points;
	poly.num_points = G_N_ELEMENTS(points);
	edge = gmap->cursor.edge;
	switch (edge->type) {
	case BUILD_SHIP:
		guimap_ship_polygon(gmap, edge, &poly);
		draw_cursor(gmap, edge->owner, &poly);
		break;
	default:
		g_assert_not_reached();
		break;
	}
}

static void erase_robber_cursor(GuiMap * gmap)
{
	const Hex *hex = gmap->cursor.hex;
	GdkPoint points[MAX_POINTS];
	Polygon poly;
	GdkRectangle rect;

	if (hex == NULL)
		return;

	poly.points = points;
	poly.num_points = G_N_ELEMENTS(points);
	if (hex->terrain == SEA_TERRAIN)
		guimap_pirate_polygon(gmap, hex, &poly);
	else
		guimap_robber_polygon(gmap, hex, &poly);
	poly_bound_rect(&poly, 1, &rect);

	display_hex(hex, gmap);

	gdk_window_invalidate_rect(gmap->area->window, &rect, FALSE);
}

static void draw_robber_cursor(GuiMap * gmap)
{
	const Hex *hex = gmap->cursor.hex;
	GdkPoint points[MAX_POINTS];
	Polygon poly;
	GdkRectangle rect;

	if (hex == NULL)
		return;

	poly.points = points;
	poly.num_points = G_N_ELEMENTS(points);
	if (hex->terrain == SEA_TERRAIN)
		guimap_pirate_polygon(gmap, hex, &poly);
	else
		guimap_robber_polygon(gmap, hex, &poly);
	poly_bound_rect(&poly, 1, &rect);

	cairo_set_line_width(gmap->cr, 2.0);
	gdk_cairo_set_source_color(gmap->cr, &green);
	poly_draw(gmap->cr, FALSE, &poly);

	gdk_window_invalidate_rect(gmap->area->window, &rect, FALSE);
}

static gboolean highlight_chits(const Hex * hex, gpointer closure)
{
	HighlightInfo *highlight_info = closure;
	GuiMap *gmap = highlight_info->gmap;
	GdkPoint points[6];
	Polygon poly;
	gint x_offset, y_offset;
	GdkRectangle rect;

	if (hex->roll != highlight_info->old_highlight
	    && hex->roll != gmap->highlight_chit)
		return FALSE;

	display_hex(hex, gmap);

	poly.points = points;
	poly.num_points = G_N_ELEMENTS(points);
	get_hex_polygon(gmap, &poly);
	calc_hex_pos(gmap, hex->x, hex->y, &x_offset, &y_offset);
	poly_offset(&poly, x_offset, y_offset);
	poly_bound_rect(&poly, 1, &rect);

	gdk_window_invalidate_rect(gmap->area->window, &rect, FALSE);
	return FALSE;
}

void guimap_highlight_chits(GuiMap * gmap, gint roll)
{
	HighlightInfo closure;

	if (roll == gmap->highlight_chit)
		return;
	closure.gmap = gmap;
	closure.old_highlight = gmap->highlight_chit;
	gmap->highlight_chit = roll;
	if (gmap->pixmap != NULL)
		map_traverse_const(gmap->map, highlight_chits, &closure);
}

void guimap_draw_hex(GuiMap * gmap, const Hex * hex)
{
	GdkPoint points[MAX_POINTS];
	Polygon poly;
	GdkRectangle rect;
	gint x_offset, y_offset;

	if (hex == NULL)
		return;

	display_hex(hex, gmap);

	poly.points = points;
	poly.num_points = G_N_ELEMENTS(points);
	get_hex_polygon(gmap, &poly);
	calc_hex_pos(gmap, hex->x, hex->y, &x_offset, &y_offset);
	poly_offset(&poly, x_offset, y_offset);
	poly_bound_rect(&poly, 1, &rect);

	gdk_window_invalidate_rect(gmap->area->window, &rect, FALSE);
}

typedef struct {
	void (*find) (GuiMap * gmap, gint x, gint y, MapElement * element);
	void (*erase_cursor) (GuiMap * gmap);
	void (*draw_cursor) (GuiMap * gmap);
} ModeCursor;

/* This array must follow the enum CursorType */
static ModeCursor cursors[] = {
	{NULL, NULL, NULL},	/* NO_CURSOR */
	{find_edge, erase_edge_cursor, draw_road_cursor},	/* ROAD_CURSOR */
	{find_edge, erase_edge_cursor, draw_ship_cursor},	/* SHIP_CURSOR */
	{find_edge, erase_edge_cursor, draw_bridge_cursor},	/* BRIDGE_CURSOR */
	{find_node, erase_node_cursor, draw_settlement_cursor},	/* SETTLEMENT_CURSOR */
	{find_node, erase_node_cursor, draw_city_cursor},	/* CITY_CURSOR */
	{find_node, erase_node_cursor, draw_city_wall_cursor},	/* CITY_WALL_CURSOR */
	{find_node, erase_node_cursor, draw_steal_building_cursor},	/* STEAL_BUILDING_CURSOR */
	{find_edge, erase_edge_cursor, draw_steal_ship_cursor},	/* STEAL_SHIP_CURSOR */
	{find_hex, erase_robber_cursor, draw_robber_cursor}	/* ROBBER_CURSOR */
};

gboolean roadM, shipM, bridgeM, settlementM, cityM, cityWallM, shipMoveM;
CheckFunc roadF, shipF, bridgeF, settlementF, cityF, cityWallF, shipMoveF;
SelectFunc roadS, shipS, bridgeS, settlementS, cityS, cityWallS, shipMoveS;
CancelFunc shipMoveC;

/** Calculate the distance between the element and the cursor.
 *  @param gmap The GuiMap
 *  @param element An Edge or a Node
 *  @param isEdge TRUE if element is an Edge
 *  @param cursor_x X position of the cursor
 *  @param cursor_y Y position of the cursor
 *  @return The square of the distance
 */
gint guimap_distance_cursor(const GuiMap * gmap,
			    const MapElement * element,
			    MapElementType type, gint cursor_x,
			    gint cursor_y)
{
	static GdkPoint single_point = { 0, 0 };
	static const Polygon simple_poly = { &single_point, 1 };
	GdkPoint translated_point;
	Polygon poly;
	poly.num_points = 1;
	poly.points = &translated_point;
	switch (type) {
	case MAP_EDGE:
		calc_edge_poly(gmap, element->edge, &simple_poly, &poly);
		break;
	case MAP_NODE:
		calc_node_poly(gmap, element->node, &simple_poly, &poly);
		break;
	case MAP_HEX:
		calc_hex_poly(gmap, element->hex, &simple_poly, &poly,
			      120.0, 0);
		break;
	}
	return sqr(cursor_x - poly.points[0].x) + sqr(cursor_y -
						      poly.points[0].y);
}

void guimap_cursor_move(GuiMap * gmap, gint x, gint y,
			MapElement * element)
{
	ModeCursor *mode;

	if (single_click_build_active) {
		MapElement dummyElement;
		gboolean can_build_road = FALSE;
		gboolean can_build_ship = FALSE;
		gboolean can_build_bridge = FALSE;
		gboolean can_build_settlement = FALSE;
		gboolean can_build_city = FALSE;
		gboolean can_build_city_wall = FALSE;
		gboolean can_move_ship = FALSE;
		gboolean can_build_edge = FALSE;
		gboolean can_build_node = FALSE;
		gint distance_edge = 0;
		gint distance_node = 0;

		dummyElement.pointer = NULL;
		find_edge(gmap, x, y, element);
		if (element->pointer) {
			can_build_road = (roadM
					  && roadF(*element,
						   gmap->player_num,
						   dummyElement));
			can_build_ship = (shipM
					  && shipF(*element,
						   gmap->player_num,
						   dummyElement));
			can_build_bridge = (bridgeM
					    && bridgeF(*element,
						       gmap->player_num,
						       dummyElement));
			can_move_ship = (shipMoveM &&
					 shipMoveF(*element,
						   gmap->player_num,
						   dummyElement));

			/* When both a road and a ship can be built,
			 * build a road when the cursor is over land,
			 * build a ship when the cursor is over sea
			 */
			if (can_build_road && can_build_ship) {
				MapElement hex1, hex2;
				gint distance1, distance2;
				hex1.hex = element->edge->hexes[0];
				hex2.hex = element->edge->hexes[1];
				distance1 =
				    guimap_distance_cursor(gmap, &hex1,
							   MAP_HEX, x, y);
				distance2 =
				    guimap_distance_cursor(gmap, &hex2,
							   MAP_HEX, x, y);
				if (distance1 == distance2)
					can_build_ship = FALSE;
				else {
					if (distance2 < distance1)
						hex1.hex = hex2.hex;
					if (hex1.hex->terrain ==
					    SEA_TERRAIN)
						can_build_road = FALSE;
					else
						can_build_ship = FALSE;
				}
			}

			/* When both a ship and a bridge can be built,
			 * divide the edge in four segments.
			 * The two segments near the nodes are for the 
			 * bridge (the pillars).
			 * The two segments in the middle are for the
			 * ship (open sea).
			 */
			if (can_build_ship && can_build_bridge) {
				MapElement node1, node2;
				gint distanceNode, distanceEdge;
				node1.node = element->edge->nodes[0];
				node2.node = element->edge->nodes[1];
				distanceNode =
				    MIN(guimap_distance_cursor
					(gmap, &node1, MAP_NODE, x, y),
					guimap_distance_cursor(gmap,
							       &node2,
							       MAP_NODE, x,
							       y));
				distanceEdge =
				    guimap_distance_cursor(gmap, element,
							   MAP_EDGE, x, y);
				if (distanceNode < distanceEdge)
					can_build_ship = FALSE;
				else
					can_build_bridge = FALSE;
			}

			can_build_edge = can_build_road || can_build_ship
			    || can_build_bridge || can_move_ship;
			if (can_build_edge)
				distance_edge =
				    guimap_distance_cursor(gmap, element,
							   MAP_EDGE, x, y);
		}

		find_node(gmap, x, y, element);
		if (element->pointer) {
			can_build_settlement = (settlementM
						&& settlementF(*element,
							       gmap->
							       player_num,
							       dummyElement));
			can_build_city = (cityM
					  && cityF(*element,
						   gmap->player_num,
						   dummyElement));
			can_build_city_wall = (cityWallM
					       && cityWallF(*element,
							    gmap->
							    player_num,
							    dummyElement));
			can_build_node = can_build_settlement
			    || can_build_city || can_build_city_wall;
			if (can_build_node)
				distance_node =
				    guimap_distance_cursor(gmap, element,
							   MAP_NODE, x, y);
		}

		/* When both edge and node can be built,
		 * build closest to the cursor.
		 * When equidistant, prefer the node.
		 */
		if (can_build_edge && can_build_node) {
			if (can_build_bridge) {
				/* Prefer bridge over node */
				can_build_node = FALSE;
			} else if (distance_node <= distance_edge) {
				can_build_edge = FALSE;
			} else {
				can_build_node = FALSE;
			}
		}

		/* Prefer the most special road segment, if possible */
		if (can_build_bridge && can_build_edge)
			guimap_cursor_set(gmap, BRIDGE_CURSOR,
					  gmap->player_num, bridgeF,
					  bridgeS, NULL, NULL, TRUE);
		else if (can_build_ship && can_build_edge)
			guimap_cursor_set(gmap, SHIP_CURSOR,
					  gmap->player_num, shipF, shipS,
					  NULL, NULL, TRUE);
		else if (can_build_road && can_build_edge)
			guimap_cursor_set(gmap, ROAD_CURSOR,
					  gmap->player_num, roadF, roadS,
					  NULL, NULL, TRUE);
		else if (can_build_settlement && can_build_node)
			guimap_cursor_set(gmap, SETTLEMENT_CURSOR,
					  gmap->player_num, settlementF,
					  settlementS, NULL, NULL, TRUE);
		else if (can_build_city && can_build_node)
			guimap_cursor_set(gmap, CITY_CURSOR,
					  gmap->player_num, cityF, cityS,
					  NULL, NULL, TRUE);
		else if (can_build_city_wall && can_build_node)
			guimap_cursor_set(gmap, CITY_WALL_CURSOR,
					  gmap->player_num, cityWallF,
					  cityWallS, NULL, NULL, TRUE);
		else if (can_move_ship && can_build_edge)
			guimap_cursor_set(gmap, SHIP_CURSOR,
					  gmap->player_num, shipMoveF,
					  shipMoveS, NULL, NULL, TRUE);
		else
			guimap_cursor_set(gmap, NO_CURSOR,
					  gmap->player_num, NULL, NULL,
					  NULL, NULL, TRUE);
	}

	if (gmap->cursor_type == NO_CURSOR) {
		element->pointer = NULL;
		return;
	}

	mode = cursors + gmap->cursor_type;
	mode->find(gmap, x, y, element);
	if (element->pointer != gmap->cursor.pointer) {
		if (gmap->cursor.pointer != NULL)
			mode->erase_cursor(gmap);
		if (gmap->check_func == NULL
		    || (element->pointer != NULL
			&& gmap->check_func(*element, gmap->cursor_owner,
					    gmap->user_data))) {
			gmap->cursor = *element;
			mode->draw_cursor(gmap);
		} else {
			gmap->cursor.pointer = NULL;
		}
	}
}

void guimap_cursor_select(GuiMap * gmap, gint x, gint y)
{
	MapElement cursor;
	SelectFunc select;
	MapElement user_data;
	guimap_cursor_move(gmap, x, y, &cursor);

	if (cursor.pointer == NULL)
		return;

	if (gmap->check_func != NULL
	    && !gmap->check_func(cursor, gmap->cursor_owner,
				 gmap->user_data)) {
		if (gmap->check_cancel != NULL)
			gmap->check_cancel();
		return;
	}

	if (gmap->check_select != NULL) {
		/* Before processing the select, clear the cursor */
		select = gmap->check_select;
		user_data.pointer = gmap->user_data.pointer;

		if (gmap->cursor.pointer != NULL)
			cursors[gmap->cursor_type].erase_cursor(gmap);
		gmap->cursor_owner = -1;
		gmap->check_func = NULL;
		gmap->cursor_type = NO_CURSOR;
		gmap->cursor.pointer = NULL;
		gmap->user_data.pointer = NULL;
		gmap->check_select = NULL;
		select(cursor, user_data);
	}
}

void guimap_cursor_set(GuiMap * gmap, CursorType cursor_type, gint owner,
		       CheckFunc check_func, SelectFunc check_select,
		       CancelFunc cancel_func,
		       const MapElement * user_data,
		       gboolean set_by_single_click)
{
	single_click_build_active = set_by_single_click;
	if (cursor_type != NO_CURSOR)
		g_assert(owner >= 0);
	if (gmap->check_cancel != NULL)
		gmap->check_cancel();
	gmap->cursor_owner = owner;
	gmap->check_func = check_func;
	gmap->check_select = check_select;
	gmap->check_cancel = cancel_func;
	if (user_data != NULL) {
		gmap->user_data.pointer = user_data->pointer;
	} else {
		gmap->user_data.pointer = NULL;
	}
	if (cursor_type == gmap->cursor_type)
		return;

	if (gmap->cursor.pointer != NULL) {
		g_assert(gmap->cursor_type != NO_CURSOR);
		cursors[gmap->cursor_type].erase_cursor(gmap);
	}
	gmap->cursor_type = cursor_type;
	gmap->cursor.pointer = NULL;
}

void guimap_single_click_set_functions(CheckFunc road_check_func,
				       SelectFunc road_select_func,
				       CheckFunc ship_check_func,
				       SelectFunc ship_select_func,
				       CheckFunc bridge_check_func,
				       SelectFunc bridge_select_func,
				       CheckFunc
				       settlement_check_func,
				       SelectFunc
				       settlement_select_func,
				       CheckFunc city_check_func,
				       SelectFunc city_select_func,
				       CheckFunc city_wall_check_func,
				       SelectFunc city_wall_select_func,
				       CheckFunc
				       ship_move_check_func,
				       SelectFunc
				       ship_move_select_func,
				       CancelFunc ship_move_cancel_func)
{
	roadF = road_check_func;
	roadS = road_select_func;
	shipF = ship_check_func;
	shipS = ship_select_func;
	bridgeF = bridge_check_func;
	bridgeS = bridge_select_func;
	settlementF = settlement_check_func;
	settlementS = settlement_select_func;
	cityF = city_check_func;
	cityS = city_select_func;
	cityWallF = city_wall_check_func;
	cityWallS = city_wall_select_func;
	shipMoveF = ship_move_check_func;
	shipMoveS = ship_move_select_func;
	shipMoveC = ship_move_cancel_func;
	single_click_build_active = TRUE;
}

void guimap_single_click_set_road_mask(gboolean mask)
{
	roadM = mask;
}

void guimap_single_click_set_ship_mask(gboolean mask)
{
	shipM = mask;
}

void guimap_single_click_set_bridge_mask(gboolean mask)
{
	bridgeM = mask;
}

void guimap_single_click_set_settlement_mask(gboolean mask)
{
	settlementM = mask;
}

void guimap_single_click_set_city_mask(gboolean mask)
{
	cityM = mask;
}

void guimap_single_click_set_city_wall_mask(gboolean mask)
{
	cityWallM = mask;
}

void guimap_single_click_set_ship_move_mask(gboolean mask)
{
	shipMoveM = mask;
}

void guimap_set_show_no_setup_nodes(GuiMap * gmap, gboolean show)
{
	gboolean old_show = gmap->show_nosetup_nodes;
	gmap->show_nosetup_nodes = show;
	if (old_show != show) {
		/* Repaint and redraw the map */
		guimap_display(gmap);
		if (gmap->area)
			gtk_widget_queue_draw(gmap->area);
	}
}
