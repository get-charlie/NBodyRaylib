#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#include "physics.h"
#include "simulation.h"

#define UNIVERSE_SIZE 1e9

void update_camera_pos(Camera2D *camera){
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f/camera->zoom);
        camera->target = Vector2Add(camera->target, delta);
    }
}

#define MIN_ZOOM    0.0001f
#define MAX_ZOOM    100.0f
void update_zoom(Camera2D *camera){
    float wheel = GetMouseWheelMove();
    if (wheel != 0){
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), *camera);
        camera->offset = GetMousePosition();
        camera->target = mouseWorldPos;
        float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
        if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
        camera->zoom = Clamp(camera->zoom*scaleFactor, MIN_ZOOM, MAX_ZOOM);
    }
}
float roundNearest10(float num) {
    if (num <= 1) {
        return 1;
    }
    float exponent = floor(log10(num));
    return pow(10, exponent);
}
#define GRID_LINES 1000
#define GRID_STEP  1e3
void draw(Camera2D camera, Simulation simulation){
    BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);
            float step = 0.0;
            float mul = (1.0 / camera.zoom);
            mul = roundNearest10(mul);
            // Draw grid
            for(int i = 0; i < GRID_LINES; i++){
                DrawLine(step, -UNIVERSE_SIZE, step, UNIVERSE_SIZE, GRAY);
                step += GRID_STEP * mul;
            }
            step = 0.0;
            for(int i = 0; i < GRID_LINES; i++){
                DrawLine(step, -UNIVERSE_SIZE, step, UNIVERSE_SIZE, GRAY);
                step -= GRID_STEP * mul;
            }
            step = 0.0;
            for(int i = 0; i < GRID_LINES; i++){
                DrawLine(-UNIVERSE_SIZE, step, UNIVERSE_SIZE, step, GRAY);
                step += GRID_STEP * mul;
            }
            step = 0.0;
            for(int i = 0; i < GRID_LINES; i++){
                DrawLine(-UNIVERSE_SIZE, step, UNIVERSE_SIZE, step, GRAY);
                step -= GRID_STEP * mul;
            }
            // Draw asix lines
            DrawLine(0, -UNIVERSE_SIZE, 0, UNIVERSE_SIZE, WHITE);
            DrawLine(-UNIVERSE_SIZE, 0, UNIVERSE_SIZE, 0, WHITE);
            // Draw bodies
            for(unsigned i = 0; i < simulation.num_bodies; i++){
                SimulationBody body = simulation.bodies[i];
                DrawCircle(body.position.x, body.position.y, body.radius, body.color);
            }
            //DrawText("Sun", -50, -100, (1 / camera.zoom) * 40, LIGHTGRAY);
        EndMode2D();
    EndDrawing();
}
SimulationBody random_body(){
    SimulationBody new = {0};
    int mExp = rand() % 20;
    int mBase = rand() % 10 + 1;
    new.mass = (double) mBase * pow(10.0, mExp);
    new.radius = 100 * mExp;
    new.color.r = rand() % 256;
    new.color.g = rand() % 256;
    new.color.b = rand() % 256;
    new.color.a = 255;
    new.position.x = (float) (rand()%2000000 -1000000);
    new.position.y = (float) (rand()%2000000 -1000000);
    new.velocity.x = (float) (rand()%10 -5);
    new.velocity.y = (float) (rand()%10 -5);
    return new;
}
void add_simulation_body(Simulation* simulation, SimulationBody body){
    simulation->bodies[simulation->num_bodies] = body;
    simulation->num_bodies++;
}
void debug_simulation(Simulation simulation){
    for(unsigned i = 0; i < simulation.num_bodies; i++){
        SimulationBody body = simulation.bodies[i];
        printf("Body %d x=%f y=%f vx=%f vy=%f m=%f\n", i+1, body.position.x, body.position.y, body.velocity.x, body.velocity.y, body.mass);
    }
}

int main (){
    const int screenWidth = 2200;
    const int screenHeight = 1400;
    srand(time(NULL));
    InitWindow(screenWidth, screenHeight, "StarSim2D");


    Camera2D camera = {0};
    camera.zoom = 0.01f;


    SetTargetFPS(60);    
    Simulation simulation = {0};

    /* 
    SimulationBody b1, b2, b3, b4;
    b1.position.x = 0.0; b1.position.y = -1000; b1.velocity.x = -6.5; b1.color = BLUE;
    b1.mass = 2e11; b1.radius = 30; 
    b2.position.x = 0.0f; b2.position.y = 0.0; b2.color = YELLOW;
    b2.mass = 8e16; b2.radius = 100; 
    b3.position.x = -2000.0f; b3.velocity.y = 3.0; b3.color = RED;
    b3.mass = 10e8; b3.radius = 15;
    b4.position.y = 3000.0f; b4.velocity.x = 5.5; b4.color = BEIGE;
    b4.mass = 10e13; b4.radius = 50; 

    simulation.bodies[0] = b1;
    simulation.bodies[1] = b2;
    simulation.bodies[2] = b3;
    simulation.bodies[3] = b4;
    simulation.num_bodies = 4;
    */

    for(int i = 0; i < 400; i++){
        add_simulation_body(&simulation, random_body());
    }

    while (!WindowShouldClose()){
        update_camera_pos(&camera);        
        update_zoom(&camera);
        update_velocities(&simulation);
        update_positions(&simulation);
        draw(camera, simulation);
        //debug_simulation(simulation);
    }

    CloseWindow();
    return 0;
}





