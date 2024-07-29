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

void init_sun_earth_moon(Simulation* simulation){
    Body sun = new_body("Sun", SUN_MASS, 6963.4, 0.0, 0.0, 0.0, 0.0, YELLOW);
    Body earth = new_body("Earth", EARTH_MASS, 63.71 * 2, 0.0, -1.0, 29.78, 0.0, BLUE);
    Body moon = new_body("Moon", 7.3e22, 17.37 * 2, 0.0, -1.00256955529, 1.022 + 29.78, 0.0, GRAY);
    add_simulation_body(simulation, earth);
    add_simulation_body(simulation, sun);
    add_simulation_body(simulation, moon);
}

int main (){
    const int screenWidth = 1200;
    const int screenHeight = 750;
    srand(time(NULL));
    InitWindow(screenWidth, screenHeight, "StarSim2D");


    Camera2D camera = {0};
    camera.zoom = 0.1f;


    SetTargetFPS(60);    
    Simulation simulation = {0};

    // init_random(&simulation, 250);
    // init_sun_earth_moon(&simulation);
    init_universe(&simulation, 10, 100, 300);
    float tSpeed = 1.0;
    DisplayFlags flags = {0};
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

        constrains(&simulation);

        if(simulation.collision == true){
            update_collisions(&simulation);
        }

        if(flags.displayTrayectory){
            update_trayectories(&simulation);
        }

        update_simulation(&simulation, tSpeed);

        update_camera_pos(&camera);        
        update_zoom(&camera);
        draw(camera, simulation, flags);
    }

    CloseWindow();
    return 0;
}





