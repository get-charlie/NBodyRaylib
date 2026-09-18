#include "simulation.h"
#include "physics.h"

#define TSTEP_FACTOR    0.02        // fraction of the reference distance
#define TSTEP_MIN       1.0e6       // m, keeps the sampling sane for tight pairs

bool add_simulation_body(Simulation* simulation, Body body)
{
    if(simulation->count >= MAX_BODIES){
        return false;
    }
    simulation->bodies[simulation->count] = body;
    simulation->trayectories[simulation->count] = (Trayectory){0};
    simulation->count++;
    update_trayectory_steps(simulation);
    return true;
}

void remove_simulation_body(Simulation* simulation, unsigned index)
{
    if(index >= simulation->count){
        return;
    }
    for(unsigned i = index; i < simulation->count - 1; i++){
        simulation->bodies[i] = simulation->bodies[i + 1];
        simulation->trayectories[i] = simulation->trayectories[i + 1];
    }
    simulation->count--;
    update_trayectory_steps(simulation);
}

void clear_trayectories(Simulation* simulation)
{
    for(unsigned i = 0; i < simulation->count; i++){
        simulation->trayectories[i].count = 0;
    }
}

static void add_tpoint(Trayectory* trayectory, Vec3 point)
{
    if(trayectory->count < MAX_TPOINTS){
        trayectory->points[trayectory->count] = point;
        trayectory->count++;
    }
    else{
        for(unsigned i = 0; i < MAX_TPOINTS - 1; i++){
            trayectory->points[i] = trayectory->points[i + 1];
        }
        trayectory->points[MAX_TPOINTS - 1] = point;
    }
}

void update_trayectories(Simulation* simulation)
{
    for(unsigned i = 0; i < simulation->count; i++){
        Body* body = &simulation->bodies[i];
        Trayectory* trayectory = &simulation->trayectories[i];
        if(trayectory->count > 0){
            Vec3 last = trayectory->points[trayectory->count - 1];
            if(vec3_distance(body->position, last) >= body->tstep){
                add_tpoint(trayectory, body->position);
            }
        }
        else{
            add_tpoint(trayectory, body->position);
        }
    }
}

void update_trayectory_steps(Simulation* simulation)
{
    for(unsigned i = 0; i < simulation->count; i++){
        Body* body = &simulation->bodies[i];
        double heavier = 0.0;   // closest body with more mass
        double closest = 0.0;   // closest body of any mass
        for(unsigned j = 0; j < simulation->count; j++){
            if(j == i){
                continue;
            }
            const Body* other = &simulation->bodies[j];
            double distance = vec3_distance(body->position, other->position);
            if(closest == 0.0 || distance < closest){
                closest = distance;
            }
            if(other->mass > body->mass && (heavier == 0.0 || distance < heavier)){
                heavier = distance;
            }
        }
        // The heaviest body of the system has nothing to orbit, so it falls
        // back to the distance to its closest neighbour
        double reference = heavier > 0.0 ? heavier : closest;
        body->tstep = fmax(reference * TSTEP_FACTOR, TSTEP_MIN);
    }
}

Vec3 simulation_center_of_mass(const Simulation* simulation)
{
    Vec3 sum = vec3(0.0, 0.0, 0.0);
    double mass = 0.0;
    for(unsigned i = 0; i < simulation->count; i++){
        const Body* body = &simulation->bodies[i];
        sum = vec3_add(sum, vec3_scale(body->position, body->mass));
        mass += body->mass;
    }
    if(mass == 0.0){
        return vec3(0.0, 0.0, 0.0);
    }
    return vec3_scale(sum, 1.0 / mass);
}

Body new_body(const char* name, Color color, double mass, double radius,
              Vec3 position_au, Vec3 velocity_kms, double scale)
{
    Body new = {0};
    strncpy(new.name, name, NAME_LEN - 1);
    new.color    = color;
    new.mass     = mass;
    new.radius   = scale > 0.0 ? radius / scale : radius;   // file units -> AU
    new.position = vec3_scale(position_au, AU);
    new.velocity = vec3_scale(velocity_kms, KM);
    new.tstep    = TSTEP_MIN;
    return new;
}
