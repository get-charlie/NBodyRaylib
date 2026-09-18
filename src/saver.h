#ifndef _SAVER_H
#define _SAVER_H

#include <stdbool.h>

#include "raylib.h"
#include "simulation.h"
#include "sim_io.h"
#include "gui.h"

// Dialog to write the running simulation to disk, either under a new name
// or replacing one of the files that are already there.

#define SAVE_MESSAGE_LEN    128

typedef struct {
    bool    active;
    char    name[PATH_LEN];
    char    files[MAX_FILES][PATH_LEN];
    int     file_count;
    int     scroll;
    char    message[SAVE_MESSAGE_LEN];
    bool    message_error;
    double  message_time;
    double  repeat;
} Saver;

void saver_init(Saver* saver);
void saver_open(Saver* saver, const char* suggested);
void saver_close(Saver* saver);

Rectangle saver_area(void);

// Handles the keyboard of the dialog, returns true while it is taking the
// input. The buttons are resolved by saver_draw(), like in the toolbar.
bool saver_update(Saver* saver, const Simulation* simulation);
void saver_draw(Saver* saver, const Simulation* simulation);

#endif
