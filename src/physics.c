#include "physics.h"

float get_distance(Vector2 p1, Vector2 p2)
{
    return hypotf(p1.x - p2.x, p1.y - p2.y);
}

static Vector2 get_gforce(Body b1, Body b2, double scale)
{
    float d = get_distance(b1.position, b2.position) * (AU / scale);
    float F = (G_CONST * b1.mass * b2.mass) / pow(d, 2);
    return (Vector2){
        .x = (F * (b2.position.x - b1.position.x)) / d,
        .y = (F * (b2.position.y - b1.position.y)) / d
    };
}

static void update_body_vel(Body* update, Body reference, float t_speed, float scale)
{
    Vector2 vF = get_gforce(*update, reference, scale);
    update->velocity.x += (vF.x / update->mass) * GetFrameTime() * t_speed;
    update->velocity.y += (vF.y / update->mass) * GetFrameTime() * t_speed;
}

void update_simulation(Simulation* simulation, float t_speed)
{
    for(unsigned i = 0; i < simulation->count; i++){
        Body* b1 = &simulation->bodies[i];
        for(unsigned j = i + 1; j < simulation->count; j++){
            Body* b2 = &simulation->bodies[j];
            update_body_vel(b1, *b2, t_speed, simulation->scale);
            update_body_vel(b2, *b1, t_speed, simulation->scale);
        }
        float dt = GetFrameTime();
        b1->position.x += b1->velocity.x * dt * t_speed;
        b1->position.y += b1->velocity.y * dt * t_speed;
    }
}





