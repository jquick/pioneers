#ifndef __SPECIAL_BUILDING_PHASE_H__
#define __SPECIAL_BUILDING_PHASE_H__

#include "config.h"
#include "cost.h"
#include "server.h"

void special_building_phase(Game *game);
void check_finished_special_building_phase(Game *game);
gboolean mode_special_building_phase(Player *player, gint event);
gboolean mode_wait_for_other_special_building_phasing_players(Player *player, gint event);

#endif
