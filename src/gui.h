#ifndef _GUI_H
#define _GUI_H

#include <math.h>
#include <stdbool.h>

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include "simulation.h"
#include "physics.h"

// Render space
// ------------
// The simulation works in meters and doubles, the renderer works in render
// units relative to the camera focus. Rebasing every position on the focus
// before casting it to float is what keeps the picture stable: an absolute
// position in meters does not even fit in a float, while the offset from the
// point the camera is looking at always does.
#define UNITS_PER_AU    1000.0
#define RENDER_SCALE    (UNITS_PER_AU / AU)      // render units per meter

#define FOVY            45.0f
#define MIN_DISTANCE    1.0e-2
#define MAX_DISTANCE    1.0e9
#define MAX_PITCH       1.55f                    // rad, just under 90 degrees
#define ORBIT_SPEED     0.005f
#define ZOOM_STEP       1.2
#define KEY_ORBIT_SPEED 1.5f                     // rad/s
#define PICK_RADIUS     14.0f                    // px
#define MIN_BODY_PIXELS 2.5f                     // bodies never shrink below this
#define MAX_LABELS      256                      // names drawn on a frame
#define LABEL_SPACING   10.0f                    // px between two names
#define CLICK_SLOP      6.0f                     // px, drag under this is a click

#define GRID_LINES      20
#define TRAIL_FADE      0.25f

#define TEXT_X          20
#define TEXT_Y          20
#define TEXT_SIZE       20
#define TEXT_OFFSET     (TEXT_SIZE + 6)
#define PANEL_ALPHA     0.75f

// Orbital camera: it always looks at focus, which is a point of the
// simulation and therefore lives in double precision.
typedef struct {
    Camera3D    camera;         // rebuilt from the fields below every frame
    Vec3        focus;          // looked at point, simulation space (m)
    double      distance;       // render units between the camera and focus
    float       yaw;            // rad
    float       pitch;          // rad
    int         follow;         // followed body, -1 for a free camera
    float       drag;           // px dragged with the left button
} OrbitCamera;

typedef struct {
    double  t_speed;
    bool    paused;
    bool    debug;
    bool    names;
    bool    display_trayectory;
    bool    grid;
    bool    help;
    float   body_scale;         // visual radius multiplier
    int     selected;           // selected body, -1 for none
} DisplayFlags;

// Preview of the body being edited in the creator, drawn as a wireframe
// so the position and radius can be checked before it exists
typedef struct {
    bool    visible;
    Vec3    position;   // m
    double  radius;     // AU
    Color   color;
} GhostBody;

typedef struct {
    unsigned    threads;
    double      energy;         // current mechanical energy
    double      energy0;        // energy when the simulation started
} SimStats;

void init_camera(OrbitCamera* camera, const Simulation* simulation);
void update_camera(OrbitCamera* camera, const Simulation* simulation, bool mouse, bool keyboard);
void frame_simulation(OrbitCamera* camera, const Simulation* simulation);
void follow_body(OrbitCamera* camera, const Simulation* simulation, int index);
void focus_point(OrbitCamera* camera, Vec3 point);

// Simulation space (m, double) -> render space (units, float)
Vector3 to_render(const OrbitCamera* camera, Vec3 position);
// Body under a screen position, -1 if there is none
int pick_body(const OrbitCamera* camera, const Simulation* simulation, Vector2 point);

void begin_frame(void);
void draw_scene(const OrbitCamera* camera, const Simulation* simulation, const DisplayFlags* flags, const GhostBody* ghost);
void draw_hud(const OrbitCamera* camera, const Simulation* simulation, const DisplayFlags* flags, const SimStats* stats);
void end_frame(void);

// Shared panel look, used by the body creator too
void draw_panel(Rectangle area, const char* title);

#endif
