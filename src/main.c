#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include <stdio.h>
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

static void handle_input(Simulation* simulation, DisplayFlags* flags)
{
    // Full screen
    if(IsKeyPressed(KEY_F11)){
        ToggleFullscreen();
    }
    // Time controls
    if(IsKeyPressed(KEY_RIGHT)){
        if(flags->tSpeed > 0.0 && flags->tSpeed < 100000000.0){
            flags->tSpeed *= 10.0;
        }
        else if(flags->tSpeed < 1){
            flags->tSpeed = 1.0;
        }
    }
    if(IsKeyPressed(KEY_LEFT)){
        if(flags->tSpeed > 1.0){
            flags->tSpeed /= 10.0;
        }
        else{
            flags->tSpeed *= 0.0;
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

int main (int argc, char** argv)
{
    Simulation* simulation = malloc(sizeof(Simulation));
    if(simulation == NULL){
        fprintf(stderr, "Could not allocate simulation.\n");
    }

    srand(time(NULL));

    help(argc, argv);

    if(load_simulation(simulation, argv[1])){
        fprintf(stderr, "Error loading the file.\n");
    }

    InitWindow(screenWidth, screenHeight, "StarSim2D");
    Camera2D camera = {0};
    camera.zoom = 0.1f;
    SetTargetFPS(60);    

    DisplayFlags flags = {0};
    flags.debug = true;
    flags.names = true;
    flags.displayTrayectory = true;
    flags.tSpeed = 1.0;
    
    double simtime = 0.0;

    while(!WindowShouldClose()){
        
        handle_input(simulation, &flags);

        if(flags.displayTrayectory){
            update_trayectories(simulation);
        }

        update_simulation(simulation, flags.tSpeed, simulation->scale);

        update_camera_pos(&camera);
        update_zoom(&camera);
        simtime += flags.tSpeed * GetFrameTime();
        draw(camera, *simulation, flags, simtime);
    }

    free(simulation);
    CloseWindow();
    return 0;
}




