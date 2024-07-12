#ifndef _GUI_H
#define _GUI_H

#define GRID_LEN    1e9
#define MIN_ZOOM    0.0001f
#define MAX_ZOOM    100.0f
#define GRID_LINES  1000
#define GRID_STEP   1e3

#include "simulation.h"

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <math.h>

void update_camera_pos(Camera2D *camera);
void update_zoom(Camera2D *camera);
void draw(Camera2D camera, Simulation simulation);

#endif
