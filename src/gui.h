#ifndef _GUI_H
#define _GUI_H

#include <math.h>
#include <limits.h>

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define GRID_LEN    INT_MAX
#define MIN_ZOOM    0.00001f
#define MAX_ZOOM    100.0f
#define GRID_LINES  10000
#define GRID_STEP   1e3

#include "simulation.h"

typedef struct{
    float   tSpeed;
    bool    debug;
    bool    names;
    bool    displayTrayectory;
}DisplayFlags;

void update_camera_pos(Camera2D *camera);
void update_zoom(Camera2D *camera);
void draw(Camera2D camera, Simulation simulation, DisplayFlags flags);

#endif
