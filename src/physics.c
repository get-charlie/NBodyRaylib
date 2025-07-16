#include "physics.h"

#include <string.h>

float get_distance(Vector2 p1, Vector2 p2)
{
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

static Vector2 get_gforce(Body b1, Body b2, double scale)
{
    float d = get_distance(b1.position, b2.position) * (AU / scale);
    float F = (G_CONST * b1.mass * b2.mass) / pow(d, 2);
    Vector2 vF;
    vF.x = (F * (b2.position.x - b1.position.x)) / d;
    vF.y = (F * (b2.position.y - b1.position.y)) / d;
    return vF;
}

static void update_body_vel(Body* update, Body reference, float tSpeed, float scale)
{
    Vector2 vF = get_gforce(*update, reference, scale);
    update->velocity.x += (vF.x / update->mass) * GetFrameTime() * tSpeed;
    update->velocity.y += (vF.y / update->mass) * GetFrameTime() * tSpeed;
}

void update_simulation(Simulation* simulation, float tSpeed, float scale){
    for(unsigned i = 0; i < simulation->count; i++){
        for(unsigned j = i + 1; j < simulation->count; j++){
            update_body_vel(&simulation->bodies[i], simulation->bodies[j], tSpeed, scale);
            update_body_vel(&simulation->bodies[j], simulation->bodies[i], tSpeed, scale);
        }
        Body body = simulation->bodies[i];
        float dt = GetFrameTime();
        body.position.x += body.velocity.x * dt * tSpeed;
        body.position.y += body.velocity.y * dt * tSpeed;
        simulation->bodies[i] = body;
    }
}





