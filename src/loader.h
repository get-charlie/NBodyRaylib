#ifndef _LOADER_H
#define _LOADER_H

#include <stdio.h>
#include <stdlib.h>

#include "cjson/cJSON.h"
#include "raylib.h"
#include "simulation.h"

int load_simulation(Simulation * simulation, const char * file);


#endif
