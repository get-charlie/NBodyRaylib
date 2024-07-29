#include "physics.h"

#include <stdio.h>
#include <string.h>

float get_distance(Vector2 p1, Vector2 p2){
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}
static Vector2 get_gforce(Body b1, Body b2){
    float d = get_distance(b1.position, b2.position) * SCALE_FACTOR;
    float F = (G_CONST * b1.mass * b2.mass) / pow(d, 2);
    Vector2 vF;
    vF.x = (F * (b2.position.x - b1.position.x)) / d;
    vF.y = (F * (b2.position.y - b1.position.y)) / d;
    return vF;
}
static void update_body_vel(Body* update, Body reference, float tSpeed){
    Vector2 vF = get_gforce(*update, reference);
    update->velocity.x += (vF.x / update->mass) * GetFrameTime() * tSpeed;
    update->velocity.y += (vF.y / update->mass) * GetFrameTime() * tSpeed;
}

static Color color_average(Color c1, Color c2){
    Color color;
    color.r = (c1.r + c2.r) / 2;
    color.g = (c1.g + c2.g) / 2;
    color.b = (c1.b + c2.b) / 2;
    color.a = (c1.a + c2.a) / 2;
    return color;
}
static Body get_more_massive(Body b1, Body b2){
    return b1.mass >= b2.mass ? b1 : b2;
}
static Body get_less_massive(Body b1, Body b2){
    return b1.mass < b2.mass ? b1 : b2;
}
static float get_new_radius(float r1, float r2){
    return cbrt(pow(r1, 3) + pow(r2, 3));
}

static Vector2 get_collision_velocity(Body b1, Body b2){
    Vector2 vf;
    float m1 = b1.mass, m2 = b2.mass;
    float x1 = b1.position.x, x2 = b2.position.x, y1 = b1.position.y, y2 = b2.position.y;
    float vx1 = b1.velocity.x, vx2 = b2.velocity.x, vy1 = b1.velocity.y, vy2 = b2.velocity.y;
    vf.x = vx1 - (2*m2/(m1+m2)) * (((vx1-vx2)*(x1-x2)+(vy1-vy2)*(y1-y2)) / ((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))) * (x1-x2);
    vf.y = vy1 - (2*m2/(m1+m2)) * (((vx1-vx2)*(x1-x2)+(vy1-vy2)*(y1-y2)) / ((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))) * (y1-y2);
    return vf;
}

static Body merge_bodies(Body b1, Body b2){
    Body new;
    new.mass = b1.mass + b2.mass;
    new.radius = get_new_radius(b1.radius, b2.radius);
    new.color = get_more_massive(b1, b2).color;
    new.position = get_more_massive(b1, b2).position;
    new.velocity = get_collision_velocity(get_more_massive(b1, b2), get_less_massive(b1, b2));
    new.trayectory = get_more_massive(b1, b2).trayectory;
    strncpy(new.name, get_more_massive(b1, b2).name, NAME_LEN);
    return new;
}
static bool body_collision(Body b1, Body b2){
    return get_distance(b1.position, b2.position) <= b1.radius + b2.radius;
}
void update_collisions(Simulation* simulation){
    for(unsigned i = 0; i < simulation->count; i++){
        for(unsigned j = i + 1; j < simulation->count; j++){
            if(i != j && body_collision(simulation->bodies[i], simulation->bodies[j])){
                simulation->bodies[i] = merge_bodies(simulation->bodies[i], simulation->bodies[j]);
                remove_simulation_body(simulation, j);
                break;
            }
        }
    }
}
void update_simulation(Simulation* simulation, float tSpeed){
    for(unsigned i = 0; i < simulation->count; i++){
        for(unsigned j = i + 1; j < simulation->count; j++){
            if(i != j){
                update_body_vel(&simulation->bodies[i], simulation->bodies[j], tSpeed);
                update_body_vel(&simulation->bodies[j], simulation->bodies[i], tSpeed);
            }
        }
        Body body = simulation->bodies[i];
        body.position.x += body.velocity.x * GetFrameTime() * tSpeed;
        body.position.y += body.velocity.y * GetFrameTime() * tSpeed;
        simulation->bodies[i] = body;
    }
}





