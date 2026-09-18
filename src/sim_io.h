#ifndef _SIM_IO_H
#define _SIM_IO_H

#include "simulation.h"

#define PATH_LEN    128
#define MAX_FILES   64

// Writes the simulation back to a json file with the same format the
// loader reads, so a saved simulation can be opened again. Returns 0 on
// success. An existing file is replaced.
int save_simulation(const Simulation* simulation, const char* path);

// Lists the json files of the working directory and of examples/, which is
// what the open and save dialogs offer. Returns how many were found.
int list_json_files(char list[][PATH_LEN], int max);

#endif
