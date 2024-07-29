#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "raylib.h"
#include <math.h>

#define G_CONST         6.6743e-11

#define SUN_MASS        1.9891e30
#define JUPITER_MASS    1.899e27
#define EARTH_MASS      5.9722e24
#define MOON_MASS       7.3477e22
#define AU              1.496e11
#define SIM_AU          1e5

#define SCALE_FACTOR    (AU / SIM_AU)             // AU to game meters (px)
#define KM_S            (6.6846e-9 * SIM_AU)      

#include "simulation.h"

double get_distance(Vector2 p1, Vector2 p2);

void update_simulation(Simulation* simulation, double tSpeed);
void update_collisions(Simulation* simulation);

#endif
