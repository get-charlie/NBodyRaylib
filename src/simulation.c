#include "simulation.h"
#include "physics.h"
#include <string.h>

void add_simulation_body(Simulation* simulation, Body body){
    simulation->bodies[simulation->count] = body;
    simulation->count++;
}

void remove_simulation_body(Simulation* simulation, unsigned index){
    if(index >= simulation->count){
        return;
    }
    for(unsigned i = index; i < simulation->count - 1; i++){
        simulation->bodies[i] = simulation->bodies[i + 1];
    }
    simulation->count--;
}

void add_tpoint(Body* body, Vector2 point){
    if(body->trayectory.count < MAX_TPOINTS){
        body->trayectory.points[body->trayectory.count] = point;
        body->trayectory.count++; 
    }else{
        for(unsigned i = 0; i < MAX_TPOINTS -1; i++){
            body->trayectory.points[i] = body->trayectory.points[i+1];
        }
        body->trayectory.points[MAX_TPOINTS -1] = point;
    }

}
void update_trayectories(Simulation* simulation){
    for(unsigned i = 0; i < simulation->count; i++){
        Body* body = &simulation->bodies[i];
        if (body->trayectory.count > 0) {
            Vector2 lastp = body->trayectory.points[body->trayectory.count -1];
            if(get_distance(body->position, lastp) >= TSIZE){
                add_tpoint(body, body->position);
            }
        } else {
            add_tpoint(body, body->position);
        }
    }
}

Body new_body(char * name, Color color, float mass, float radius, float posx, float posy, float velx, float vely){
    Body new = {0};
    strncpy(new.name, name, NAME_LEN);
    new.mass = mass;
    new.radius = radius;
    new.position.x = posx * SIM_AU;
    new.position.y = posy * SIM_AU;
    new.velocity.x = velx * KM_S;
    new.velocity.y = vely * KM_S;
    new.color = color;
    return new;
}

static Body random_body(char * name, float massMul, int maxMul, int radMul, int maxDist){
    Body new = {0};
    strncpy(new.name, name, NAME_LEN);
    
    int mBase = (rand()%maxMul)+1;
    new.mass = mBase * massMul;

    new.radius = radMul * (log2(new.mass) + 1);
    new.color.r = rand() % 256;
    new.color.g = rand() % 256;
    new.color.b = rand() % 256;
    new.color.a = 255;
    new.position.x = (float) (rand()%maxDist - maxDist/2);
    new.position.y = (float) (rand()%maxDist - maxDist/2);
    return new;
}

void init_universe(Simulation* simulation, int stars, int planets, int moons){
    char name[NAME_LEN];
    for(int i = 0; i < stars; i++){
        sprintf(name, "S-%d", i+1);
        add_simulation_body(simulation, random_body(name, SUN_MASS, 10, 10, 1e7));
    }
    for(int i = 0; i < planets; i++){
        sprintf(name, "P-%d", i+1);
        add_simulation_body(simulation, random_body(name, EARTH_MASS, 100, 10, 1e7));
    }
    for(int i = 0; i < moons; i++){
        sprintf(name, "M-%d", i+1);
        add_simulation_body(simulation, random_body(name, MOON_MASS, 30, 10, 1e7));
    }
}

void init_random(Simulation* simulation, unsigned count){
    for(int i = 0; i < count; i++){
        char name[NAME_LEN];
        sprintf(name, "R-%d", i+1);
        add_simulation_body(simulation, random_body(name, EARTH_MASS, 1000, 100, 1e6));
    }
}

void constrains(Simulation* simulation){
    for(unsigned i = 0; i < simulation->count; i++){
        Body body = simulation->bodies[i];
        if(isinf(body.position.x) || isinf(body.position.y) || isinf(body.velocity.x) || isinf(body.velocity.y)){
            printf("Warning: SimulationBody %s had a infinity value and was removed\n", simulation->bodies[i].name);
            printf("t=%lf, m=%lf EM x=%.3lf y=%.3lf vx=%.3lf vy=%.3lf\n", 
            simulation->time, body.mass / EARTH_MASS, body.position.x, body.position.y, body.velocity.x, body.velocity.y);
            remove_simulation_body(simulation, i);
            continue;
        }
        if(isnan(body.position.x) || isnan(body.position.y) || isnan(body.velocity.x) || isnan(body.velocity.y)){
            printf("Warning: SimulationBody %s had a NAN value and was removed\n", simulation->bodies[i].name);
            printf("t=%lf m=%.3lf EM x=%.3lf y=%.3lf vx=%.3lf vy=%.3lf\n", 
            simulation->time, body.mass / EARTH_MASS, body.position.x, body.position.y, body.velocity.x, body.velocity.y);
            remove_simulation_body(simulation, i);
        }
    }
}
void debug_simulation(Simulation simulation){
    for(unsigned i = 0; i < simulation.count; i++){
        Body body = simulation.bodies[i];
        printf("Body %3d: x=%.3lf\ty=%.3lf\tvx=%.3lf\t\tvy=%.3lf\n", 
        i+1, body.position.x, body.position.y, body.velocity.x, body.velocity.y);
        for(unsigned j = 0; j < body.trayectory.count; j++){
            printf("Point %d x = %f, y = %f\n", j, body.trayectory.points[j].x, body.trayectory.points[j].y);
        }
    }
}










