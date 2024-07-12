#include <math.h>
#include "physics.h"
#include "simulation.h"
#include "raylib.h"

static float get_distance(SimulationBody b1, SimulationBody b2){
   return sqrt(pow(b2.position.x - b1.position.x, 2) + pow(b2.position.y - b1.position.y, 2));
}
static Vector2 get_gforce(SimulationBody b1, SimulationBody b2){
    float F = (G_CONST * b1.mass * b2.mass) / pow(get_distance(b1, b2), 2);
    float d = get_distance(b1, b2); 
    Vector2 vF;
    vF.x = (F * (b2.position.x - b1.position.x)) / d;
    vF.y = (F * (b2.position.y - b1.position.y)) / d;
    return vF;
}
static void update_body_vel(SimulationBody* update, SimulationBody reference){
    Vector2 vF = get_gforce(*update, reference);
    update->velocity.x += (vF.x * GetFrameTime()) / update->mass;
    update->velocity.y += (vF.y * GetFrameTime()) / update->mass;
}
void update_velocities(Simulation* simulation){
    for(unsigned i = 0; i < simulation->num_bodies; i++){
        for(unsigned j = i + 1; j < simulation->num_bodies; j++){
            if(i != j){
                update_body_vel(&simulation->bodies[i], simulation->bodies[j]);
                update_body_vel(&simulation->bodies[j], simulation->bodies[i]);
            }
        }
    }
}
void update_positions(Simulation* simulation){
    for(unsigned i = 0; i < simulation->num_bodies; i++){
        SimulationBody body = simulation->bodies[i];
        body.position.x += body.velocity.x;
        body.position.y += body.velocity.y;
        simulation->bodies[i] = body;
    }
}









