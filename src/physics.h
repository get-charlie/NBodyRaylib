#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "raylib.h"
#include <math.h>

#define G_CONST         6.6743e-11

#define SUN_MASS        1.9891e30
#define JUPITER_MASS    1.899e27
#define EARTH_MASS      5.9722e24
#define AU              1.496e11

#define DIST_SCALE      AU / 1e6

#include "simulation.h"

void update_simulation(Simulation* simulation, double tSpeed);


#endif
