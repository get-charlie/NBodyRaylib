#ifndef _loader_h
#define _loader_h

#include <stdio.h>
#include <stdlib.h>

#include "cjson/cJSON.h"
#include "raylib.h"
#include "simulation.h"

int load_simulation(Simulation * simulation, const char * file);


#endif
