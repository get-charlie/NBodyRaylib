#ifndef _SIMULATION_H
#define _SIMULATION_H

#define NAME_LEN 16

#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct{
    Vector2 position;
    Vector2 velocity;
    double mass;
    double radius;
    Color color;
    char name[NAME_LEN];
}SimulationBody;

#define MAX_BODIES  1000

typedef struct{
    SimulationBody bodies[MAX_BODIES];
    unsigned num_bodies;
}Simulation;

void add_simulation_body(Simulation* simulation, SimulationBody body);
void remove_simulation_body(Simulation* simulation, unsigned index);

SimulationBody random_body();

void debug_simulation(Simulation simulation);

#endif




