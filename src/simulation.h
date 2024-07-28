#ifndef _SIMULATION_H
#define _SIMULATION_H

#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#define NAME_LEN        16
#define MAX_BODIES      1000
#define MAX_TPOINTS     300
#define TSIZE           3000

typedef struct{
    Vector2 points[MAX_TPOINTS];
    unsigned count;
}Trayectory;

typedef struct{
    Vector2 position;
    Vector2 velocity;
    double mass;
    double radius;
    Color color;
    char name[NAME_LEN];
    Trayectory trayectory;
}SimulationBody;


typedef struct{
    SimulationBody bodies[MAX_BODIES];
    unsigned count;
}Simulation;

void add_simulation_body(Simulation* simulation, SimulationBody body);
void remove_simulation_body(Simulation* simulation, unsigned index);


void add_tpoint(SimulationBody* body, Vector2 point);
void update_trayectories(Simulation* simulation);


SimulationBody random_body();

void debug_simulation(Simulation simulation);

#endif




