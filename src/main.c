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


void remove_infinity(Simulation* simulation){ // Provisional function
    for(unsigned i = 0; i < simulation->count; i++){
        SimulationBody body = simulation->bodies[i];
        if(isinf(body.position.x) || isinf(body.position.y) || isinf(body.velocity.x) || isinf(body.velocity.y)){
            printf("Infinity detected!\n");
            remove_simulation_body(simulation, i);
        }
    }
}
void init_random(Simulation* simulation, unsigned count){
    for(int i = 0; i < count; i++){
        add_simulation_body(simulation, random_body());
    }
}
SimulationBody new_body(double mass, double radius, double posx, double posy, double velx, double vely, char * name, Color color){
    SimulationBody new = {0};
    new.mass = mass;
    new.radius = radius;
    new.position.x = posx * SIM_AU;
    new.position.y = posy * SIM_AU;
    new.velocity.x = velx * KM_S;
    new.velocity.y = vely * KM_S;
    strncpy(new.name, name, NAME_LEN);
    new.color = color;
    return new;
}
void init_sun_earth_moon(Simulation* simulation){
    SimulationBody sun = new_body(SUN_MASS, 6963.4, 0.0, 0.0, 0.0, 0.0, "Sun", YELLOW);
    SimulationBody earth = new_body(EARTH_MASS, 63.71 * 2, 0.0, -1.0, 29.78, 0.0, "Earth", BLUE);
    SimulationBody moon = new_body(7.3e22, 17.37 * 2, 0.0, -1.00256955529, 1.022 + 29.78, 0.0, "Moon", GRAY);
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

    init_random(&simulation, 200);
    // init_sun_earth_moon(&simulation);
    double tSpeed = 1.0;
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
        // debug_simulation(simulation);
        update_camera_pos(&camera);        
        update_zoom(&camera);
        update_simulation(&simulation, tSpeed);
        update_collisions(&simulation);
        // remove_infinity(&simulation);
        update_trayectories(&simulation);
        draw(camera, simulation, flags);
    }

    CloseWindow();
    return 0;
}





