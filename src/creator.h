#ifndef _CREATOR_H
#define _CREATOR_H

#include <stdbool.h>

#include "raylib.h"
#include "simulation.h"
#include "gui.h"

// Body creator: a small form to add bodies to a running simulation with
// their position, velocity, mass, radius and color.

#define CREATOR_TEXT_LEN    24
#define MESSAGE_LEN         64

typedef enum {
    FIELD_NAME = 0,
    FIELD_MASS,
    FIELD_RADIUS,
    FIELD_POS_X,
    FIELD_POS_Y,
    FIELD_POS_Z,
    FIELD_VEL_X,
    FIELD_VEL_Y,
    FIELD_VEL_Z,
    FIELD_COLOR_R,
    FIELD_COLOR_G,
    FIELD_COLOR_B,
    CREATOR_FIELDS
} CreatorField;

typedef struct {
    bool    active;
    int     field;                                      // field being edited
    char    values[CREATOR_FIELDS][CREATOR_TEXT_LEN];
    char    message[MESSAGE_LEN];                       // feedback line
    bool    message_error;
    double  message_time;                               // seconds left on screen
    double  repeat;                                     // backspace autorepeat
} Creator;

void creator_init(Creator* creator);

// Opens the form filled with the camera focus and, if a body is being
// followed, its velocity, which makes it easy to put a satellite in orbit.
void creator_open(Creator* creator, const Simulation* simulation, const OrbitCamera* camera);
void creator_close(Creator* creator);

// Handles the keyboard and mouse of the form. Returns true while the
// creator is capturing the input, so the simulation shortcuts stay off.
bool creator_update(Creator* creator, Simulation* simulation, DisplayFlags* flags);

// Screen area of the form, the camera ignores the mouse over it
Rectangle creator_area(void);

// Wireframe preview of the body being edited
GhostBody creator_ghost(const Creator* creator, const Simulation* simulation);
void creator_draw(const Creator* creator);

#endif
