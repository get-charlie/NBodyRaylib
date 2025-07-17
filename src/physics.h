#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "raylib.h"
#include <math.h>

#define G_CONST         6.6743e-11
#define AU              1.496e11
#define KM_TO_AU        (1 / 1.496e8)

#include "simulation.h"

float get_distance(Vector2 p1, Vector2 p2);
void update_simulation(Simulation* simulation, float t_speed);

#endif
