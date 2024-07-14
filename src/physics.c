#include "physics.h"

#include <stdio.h>
#include <string.h>



static double get_distance(SimulationBody b1, SimulationBody b2){
    return  sqrt(pow(b2.position.x - b1.position.x, 2) + pow(b2.position.y - b1.position.y, 2));
}
static Vector2 get_gforce(SimulationBody b1, SimulationBody b2){
    double d = get_distance(b1, b2) * DIST_SCALE;
    double F = (G_CONST * b1.mass * b2.mass) / pow(d, 2);
    Vector2 vF;
    vF.x = (F * (b2.position.x - b1.position.x)) / d;
    vF.y = (F * (b2.position.y - b1.position.y)) / d;
    return vF;
}
static void update_body_vel(SimulationBody* update, SimulationBody reference, double tSpeed){
    Vector2 vF = get_gforce(*update, reference);
    update->velocity.x += (vF.x / update->mass) * GetFrameTime() * tSpeed;
    update->velocity.y += (vF.y / update->mass) * GetFrameTime() * tSpeed;
}
void update_velocities(Simulation* simulation, double tSpeed){
    for(unsigned i = 0; i < simulation->num_bodies; i++){
        for(unsigned j = i + 1; j < simulation->num_bodies; j++){
            if(i != j){
                update_body_vel(&simulation->bodies[i], simulation->bodies[j], tSpeed);
                update_body_vel(&simulation->bodies[j], simulation->bodies[i], tSpeed);
            }
        }
    }
}
void update_positions(Simulation* simulation, double tSpeed){
    for(unsigned i = 0; i < simulation->num_bodies; i++){
        SimulationBody body = simulation->bodies[i];
        body.position.x += body.velocity.x;
        body.position.y += body.velocity.y;
        simulation->bodies[i] = body;
    }
}

static Color color_average(Color c1, Color c2){
    Color color;
    color.r = (c1.r + c2.r) / 2;
    color.g = (c1.g + c2.g) / 2;
    color.b = (c1.b + c2.b) / 2;
    color.a = (c1.a + c2.a) / 2;
    return color;
}
static SimulationBody get_more_massive(SimulationBody b1, SimulationBody b2){
    if(b1.mass > b2.mass){
        return b1;
    }else{
        return b2;
    }
}
static double get_new_radius(double r1, double r2){
    return cbrt(pow(r1, 3) + pow(r2, 3));
}
static SimulationBody merge_bodies(SimulationBody b1, SimulationBody b2){
    SimulationBody new;
    new.mass = b1.mass + b2.mass;
    new.radius = get_new_radius(b1.radius, b2.radius);
    new.color = get_more_massive(b1, b2).color;
    new.position = get_more_massive(b1, b2).position;
    new.velocity = get_more_massive(b1, b2).velocity; // TODO add velocities according to physics
    return new;
}
static bool body_collision(SimulationBody b1, SimulationBody b2){
    return get_distance(b1, b2) <= b1.radius + b2.radius;
}
void update_collisions(Simulation* simulation){
    for(unsigned i = 0; i < simulation->num_bodies; i++){
        for(unsigned j = i + 1; j < simulation->num_bodies; j++){
            if(i != j && body_collision(simulation->bodies[i], simulation->bodies[j])){
                simulation->bodies[i] = merge_bodies(simulation->bodies[i], simulation->bodies[j]);
                remove_simulation_body(simulation, j);
                break;
            }
        }
    }
}
void update_simulation(Simulation* simulation, double tSpeed){
    for(unsigned i = 0; i < simulation->num_bodies; i++){
        for(unsigned j = i + 1; j < simulation->num_bodies; j++){
            if(i != j){
                update_body_vel(&simulation->bodies[i], simulation->bodies[j], tSpeed);
                update_body_vel(&simulation->bodies[j], simulation->bodies[i], tSpeed);
            }
        }
        SimulationBody body = simulation->bodies[i];
        body.position.x += body.velocity.x * GetFrameTime() * tSpeed;
        body.position.y += body.velocity.y * GetFrameTime() * tSpeed;
        simulation->bodies[i] = body;
    }
    update_collisions(simulation);
}







