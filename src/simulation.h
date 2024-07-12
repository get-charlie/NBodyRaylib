#ifndef _SIMULATION_H
#define _SIMULATION_H

#define NAME_LEN 16

#include "raylib.h"

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

#endif




