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

int main (int argc, char** argv){

    Simulation simulation;

    srand(time(NULL));

    if(argc != 2){
        printf("Use: starsim2d <simulation.json>\n");
        return 1;
    }
    if(strstr(argv[1], "random")){
        load_random(&simulation, argv[1]);
    }else{
        load_simulation(&simulation, argv[1]);
    }

    const int screenWidth = 1200;
    const int screenHeight = 750;
    InitWindow(screenWidth, screenHeight, "StarSim2D");


    Camera2D camera = {0};
    camera.zoom = 0.1f;
    SetTargetFPS(60);    

    float tSpeed = 1.0;
    DisplayFlags flags = {0};
    flags.debug = true;
    flags.names = true;
    flags.displayTrayectory = true;
    flags.tSpeed = tSpeed;

    while (!WindowShouldClose()){
        if(IsKeyPressed(KEY_F11)){
            ToggleFullscreen();
        }
        if(IsKeyPressed(KEY_RIGHT)){
            if(tSpeed > 0.0 && tSpeed < 100000000.0){
                tSpeed *= 10.0;
            }else if(tSpeed < 1){
                tSpeed = 1.0;
            }
            flags.tSpeed = tSpeed;
        }
        if(IsKeyPressed(KEY_LEFT)){
            if(tSpeed > 1.0){
                tSpeed /= 10.0;
            }
            else{
                tSpeed *= 0.0;
            }
            flags.tSpeed = tSpeed;
        }
        if(IsKeyPressed(KEY_C)){
            if(simulation.collision){
                simulation.collision = false;
            }else{
                simulation.collision = true;
            }
        }
        if(IsKeyPressed(KEY_T)){
            if(flags.displayTrayectory){
                flags.displayTrayectory = false;
                for(int i = 0; i < simulation.count; i++){
                    simulation.bodies[i].trayectory.count = 0;
                }
            }else{
                flags.displayTrayectory = true;
            }
        }
        if(IsKeyPressed(KEY_D)){
            if(flags.debug){
                flags.debug = false;
            }else{
                flags.debug = true;
            }
        }
        if(IsKeyPressed(KEY_N)){
            if(flags.names){
                flags.names = false;
            }else{
                flags.names = true;
            }
        }

        constrains(&simulation);

        if(simulation.collision == true){
            update_collisions(&simulation);
        }

        if(flags.displayTrayectory){
            update_trayectories(&simulation);
        }

        update_simulation(&simulation, tSpeed, simulation.scale);

        update_camera_pos(&camera);        
        update_zoom(&camera);
        draw(camera, simulation, flags);
    }

    CloseWindow();
    return 0;
}

