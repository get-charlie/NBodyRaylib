#ifndef _MENU_H
#define _MENU_H

#include <stdbool.h>

#include "raylib.h"
#include "simulation.h"
#include "sim_io.h"

// Main menu: the screen the program starts on. From here a simulation can
// be created empty, loaded from a json file or generated with random
// bodies, and the parameters of the random one are asked for here too.

#define RANDOM_TEXT_LEN 24

typedef enum {
    RANDOM_BODIES = 0,
    RANDOM_STAR_MASS,
    RANDOM_MASS_MIN,
    RANDOM_MASS_MAX,
    RANDOM_ORBIT_MIN,
    RANDOM_ORBIT_MAX,
    RANDOM_THICKNESS,
    RANDOM_RADIUS,
    RANDOM_SCALE,
    RANDOM_SEED,
    RANDOM_FIELDS
} RandomField;

typedef enum {
    PAGE_MAIN = 0,
    PAGE_OPEN,
    PAGE_RANDOM
} MenuPage;

typedef enum {
    MENU_NONE = 0,      // stay in the menu
    MENU_START,         // a new simulation is ready in the buffer
    MENU_RESUME,        // go back to the simulation that was already running
    MENU_QUIT
} MenuResult;

typedef struct {
    MenuPage    page;
    bool        can_resume;         // there is a simulation to go back to

    char        files[MAX_FILES][PATH_LEN];
    int         file_count;
    int         scroll;
    int         selected;

    char        values[RANDOM_FIELDS][RANDOM_TEXT_LEN];
    int         field;
    double      repeat;

    char        message[128];
    bool        message_error;
    char        source[PATH_LEN];   // file the current simulation came from
} Menu;

void menu_init(Menu* menu);
void menu_open(Menu* menu, bool can_resume);

// Draws the menu, handles its input and, when something is chosen, fills
// the simulation with it. Everything happens in one call because the menu
// owns the whole screen while it is up.
MenuResult menu_run(Menu* menu, Simulation* simulation);

#endif
