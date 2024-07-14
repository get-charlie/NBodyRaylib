#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#include "physics.h"
#include "simulation.h"
#include "gui.h"


void remove_infinity(Simulation* simulation){ // Provisional function
    for(unsigned i = 0; i < simulation->num_bodies; i++){
        SimulationBody body = simulation->bodies[i];
        if(isinf(body.position.x) || isinf(body.position.y) || isinf(body.velocity.x) || isinf(body.velocity.y)){
            printf("Infinity detected!\n");
            remove_simulation_body(simulation, i);
        }
    }
}
void init_random(Simulation* simulation, unsigned num_bodies){
    for(int i = 0; i < num_bodies; i++){
        add_simulation_body(simulation, random_body());
    }
}
void init_solar_system(Simulation* simulation){
    SimulationBody sun = {0}, earth = {0};
    sun.mass = SUN_MASS;
    sun.radius = 7000.0;
    sun.color = YELLOW;
    add_simulation_body(simulation, sun);
    earth.mass = EARTH_MASS;
    earth.radius = 1000.0;
    earth.position.y = -DIST_SCALE * 1;
    earth.color = BLUE;
    earth.velocity.x = 64.0e-3 * 9;
    add_simulation_body(simulation, earth);
}
int main (){
    const int screenWidth = 1200;
    const int screenHeight = 750;
    srand(time(NULL));
    InitWindow(screenWidth, screenHeight, "StarSim2D");


    Camera2D camera = {0};
    camera.zoom = 0.001f;


    SetTargetFPS(60);    
    Simulation simulation = {0};

    // init_random(&simulation, 300);
    init_solar_system(&simulation);
    double tSpeed = 1.0;
    while (!WindowShouldClose()){
        if(IsKeyPressed(KEY_F11)){
            ToggleFullscreen();
        }
        if(IsKeyPressed(KEY_RIGHT)){
            if(tSpeed > 0.0 && tSpeed < 10000000.0){
                tSpeed *= 10.0;
            }else if(tSpeed < 1){
                tSpeed = 1.0;
            }
        }
        if(IsKeyPressed(KEY_LEFT)){
            if(tSpeed > 1.0){
                tSpeed /= 10.0;
            }
            else{
                tSpeed *= 0.0;
            }
        }
        debug_simulation(simulation);
        update_camera_pos(&camera);        
        update_zoom(&camera);
        update_simulation(&simulation, tSpeed);
        remove_infinity(&simulation);
        draw(camera, simulation, tSpeed);
    }

    CloseWindow();
    return 0;
}





