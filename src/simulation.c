#include "simulation.h"
#include "physics.h"
#include <string.h>

void add_simulation_body(Simulation* simulation, Body body)
{
    simulation->bodies[simulation->count] = body;
    simulation->count++;
}

void remove_simulation_body(Simulation* simulation, unsigned index)
{
    if(index >= simulation->count){
        return;
    }
    for(unsigned i = index; i < simulation->count - 1; i++){
        simulation->bodies[i] = simulation->bodies[i + 1];
    }
    simulation->count--;
}

void add_tpoint(Body* body, Vector2 point)
{
    if(body->trayectory.count < MAX_TPOINTS){
        body->trayectory.points[body->trayectory.count] = point;
        body->trayectory.count++; 
    }
    else{
        for(unsigned i = 0; i < MAX_TPOINTS -1; i++){
            body->trayectory.points[i] = body->trayectory.points[i+1];
        }
        body->trayectory.points[MAX_TPOINTS -1] = point;
    }

}

void update_trayectories(Simulation* simulation)
{
    for(unsigned i = 0; i < simulation->count; i++){
        Body* body = &simulation->bodies[i];
        if(body->trayectory.count > 0){
            Vector2 lastp = body->trayectory.points[body->trayectory.count -1];
            if(get_distance(body->position, lastp) >= TSIZE){
                add_tpoint(body, body->position);
            }
        } 
        else{
            add_tpoint(body, body->position);
        }
    }
}

Body new_body(char * name, Color color, float mass, float radius, float posx, float posy, float velx, float vely, float scale)
{
    Body new = {0};
    strncpy(new.name, name, NAME_LEN);
    new.mass = mass;
    new.radius = radius;
    new.position.x = posx * scale;
    new.position.y = posy * scale * -1;
    new.velocity.x = velx * 6.6846e-9 * scale; // conversion to km/s
    new.velocity.y = vely * 6.6846e-9 * scale; // conversion to km/s
    new.color = color;
    return new;
}
// TODO remove
static Color random_color(){
   Color color;
   color.r = rand() % 256;
   color.g = rand() % 256;
   color.b = rand() % 256;
   color.a = 255;
   return color;
}
// TODO remove
static float random_mass(float massmul, int min, int max){
    int rm = rand()%(max-min+1) + min;
    return massmul * (float)rm;
}
// TODO remove
static float rad_from_mass(float mass, float radmul){
    return radmul * cbrt(mass/EARTH_MASS);
}
// TODO remove
static Vector2 random_pos(float maxdist, float scale){
    Vector2 pos;
    float rx = (float)rand() / RAND_MAX;
    float ry = (float)rand() / RAND_MAX;
    pos.x = (rx * 2 - 1) * scale * maxdist;
    pos.y = (ry * 2 - 1) * scale * maxdist;
    return pos;
}
// TODO remove
Body new_random_body(char * name, float mass, int minmassmul, int maxmassmul, float radmul, float maxdist, float scale){
    Body new = {0};
    strncpy(new.name, name, NAME_LEN);
    new.color = random_color();
    new.mass = random_mass(mass, minmassmul, maxmassmul);
    new.radius = rad_from_mass(new.mass, radmul);
    new.position = random_pos(maxdist, scale);
    return new;
}






