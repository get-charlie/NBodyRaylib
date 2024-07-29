#ifndef _SIMULATION_H
#define _SIMULATION_H

#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#define NAME_LEN        16
#define MAX_BODIES      1000
#define MAX_TPOINTS     100
#define TSIZE           1000

typedef struct{
    Vector2 points[MAX_TPOINTS];
    unsigned count;
}Trayectory;

typedef struct{
    Vector2 position;
    Vector2 velocity;
    float mass;
    float radius;
    Color color;
    char name[NAME_LEN];
    Trayectory trayectory;
}Body;


typedef struct{
    Body bodies[MAX_BODIES];
    unsigned count;
    double time;
}Simulation;

Body new_body(char * name, float mass, float radius, float posx, float posy, float velx, float vely, Color color);
void init_random(Simulation* simulation, unsigned count);
void init_universe(Simulation* simulation, int stars, int planets, int moons);

void add_simulation_body(Simulation* simulation, Body body);
void remove_simulation_body(Simulation* simulation, unsigned index);

void add_tpoint(Body* body, Vector2 point);
void update_trayectories(Simulation* simulation);

void constrains(Simulation* simulation);
void debug_simulation(Simulation simulation);

#endif




