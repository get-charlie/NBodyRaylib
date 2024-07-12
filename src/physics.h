#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "simulation.h"
#include "raylib.h"
#include <math.h>

#define G_CONST         6.6743e-11

#define SUN_MASS        1.9891e30
#define JUPITER_MASS    1.899e27
#define EARTH_MASS      5.9722e24

void update_positions(Simulation* simulation);
void update_velocities(Simulation* simulation);

#endif
