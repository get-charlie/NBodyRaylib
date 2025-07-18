#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "physics.h"
#include "simulation.h"
#include "gui.h"
#include "loader.h"

const int screenWidth = 1200;
const int screenHeight = 750;

static void help(int argc, char** argv)
{
    if(argc != 2){
        printf("Use: %s <simulation.json>\n", argv[0]);
        exit(0);
    }
    if(!strstr(argv[1], ".json")){
        printf("Simulation must be a .json\n");
        exit(0);
    }
}

#define MAX_SIM_SPEED   10000000.0
static void handle_input(Simulation* simulation, DisplayFlags* flags)
{
    // Full screen
    if(IsKeyPressed(KEY_F11)){
        ToggleFullscreen();
    }
    // Time controls
    if(IsKeyPressed(KEY_RIGHT)){
        if(flags->t_speed > 0.0 && flags->t_speed < MAX_SIM_SPEED){
            flags->t_speed *= 10.0;
        }
        else if(flags->t_speed < 1){
            flags->t_speed = 1.0;
        }
    }
    if(IsKeyPressed(KEY_LEFT)){
        if(flags->t_speed > 1.0){
            flags->t_speed /= 10.0;
        }
        else{
            flags->t_speed *= 0.0;
        }
    }
    // Trayectories
    if(IsKeyPressed(KEY_T)){
        flags->displayTrayectory = !flags->displayTrayectory;
        if(flags->displayTrayectory){
            for(int i = 0; i < (int)simulation->count; i++){
                simulation->bodies[i].trayectory.count = 0;
            }
        }
    }
    // Toggle debug display
    if(IsKeyPressed(KEY_D)){
        flags->debug = !flags->debug;
    }
    // Toggle body names
    if(IsKeyPressed(KEY_N)){
        flags->names = !flags->names;
    }
}

int main(int argc, char** argv)
{
    help(argc, argv);

    Simulation* simulation = malloc(sizeof(Simulation));
    if(simulation == NULL){
        fprintf(stderr, "Could not allocate simulation.\n");
        exit(1);
    }

    if(load_simulation(simulation, argv[1])){
        fprintf(stderr, "Error loading the file.\n");
        free(simulation);
        exit(1);
    }

    InitWindow(screenWidth, screenHeight, "StarSim2D");
    Camera2D camera = {0};
    camera.zoom = 0.1f;
    SetTargetFPS(60);

    DisplayFlags flags = {0};
    flags.debug = true;
    flags.names = true;
    flags.displayTrayectory = true;
    flags.t_speed = 1.0;
    
    double simtime = 0.0;

    while(!WindowShouldClose()){
        
        handle_input(simulation, &flags);

        if(flags.displayTrayectory){
            update_trayectories(simulation);
        }

        update_simulation(simulation, flags.t_speed);
        update_camera(&camera);
        simtime += flags.t_speed * GetFrameTime();
        draw(camera, *simulation, flags, simtime);
    }

    free(simulation);
    CloseWindow();
    return 0;
}




