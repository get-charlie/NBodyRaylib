#ifndef _SIMULATION_H
#define _SIMULATION_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "raylib.h"     // Only for Color, the state itself is graphics agnostic
#include "vec3.h"

#define NAME_LEN        32
#define MAX_BODIES      1024
#define MAX_TPOINTS     500

// A trayectory point is stored in simulation space (meters, double), it is
// projected to render space only when it is drawn.
typedef struct {
    Vec3        points[MAX_TPOINTS];
    unsigned    count;
} Trayectory;

// Logical state of a body. Everything here is SI and double precision:
// the old version kept positions as floats scaled to screen units, which
// lost precision as soon as the bodies moved away from the origin.
typedef struct {
    Vec3    position;       // m
    Vec3    velocity;       // m/s
    Vec3    accel;          // m/s^2, working storage of the integrator
    double  mass;           // kg
    double  radius;         // AU, visual radius only
    double  tstep;          // m, distance between trayectory samples
    Color   color;
    char    name[NAME_LEN];
} Body;

typedef struct {
    Body        bodies[MAX_BODIES];
    Trayectory  trayectories[MAX_BODIES];
    unsigned    count;
    double      scale;      // file units per AU, kept to read/write json radii
    double      time;       // simulated seconds
} Simulation;

// Positions are given in AU and velocities in km/s, like in the json files,
// and converted to SI here. radius is in file units (see Simulation.scale).
Body new_body(const char* name, Color color, double mass, double radius,
              Vec3 position_au, Vec3 velocity_kms, double scale);

bool add_simulation_body(Simulation* simulation, Body body);
void remove_simulation_body(Simulation* simulation, unsigned index);

void clear_trayectories(Simulation* simulation);
void update_trayectories(Simulation* simulation);

// Distance between trayectory samples, based on how far every body is from
// the closest heavier body, so both a planet and its moon leave a usable
// trail no matter the scale of the system.
void update_trayectory_steps(Simulation* simulation);

Vec3 simulation_center_of_mass(const Simulation* simulation);

#endif
