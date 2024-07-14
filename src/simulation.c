#include "simulation.h"
#include "physics.h"
void add_simulation_body(Simulation* simulation, SimulationBody body){
    simulation->bodies[simulation->num_bodies] = body;
    simulation->num_bodies++;
}

void remove_simulation_body(Simulation* simulation, unsigned index){
    if(index >= simulation->num_bodies){
        return;
    }
    for(unsigned i = index; i < simulation->num_bodies - 1; i++){
        simulation->bodies[i] = simulation->bodies[i + 1];
    }
    simulation->num_bodies--;
}

SimulationBody random_body(){
    SimulationBody new = {0};
    // int mExp = rand() % 20;
    int mBase = rand() % 1000000 + 1;
    new.mass = (double)mBase * EARTH_MASS;
    new.radius = 100 * (log2(mBase) + 1);
    new.color.r = rand() % 256;
    new.color.g = rand() % 256;
    new.color.b = rand() % 256;
    new.color.a = 255;
    long maxDist = AU / 50000;
    new.position.x = (float) (rand()%maxDist - maxDist/2);
    new.position.y = (float) (rand()%maxDist - maxDist/2);
    int maxVel = 1000;
    // new.velocity.x = 1.0 / (float) (rand()%maxVel -maxVel/2);
    // new.velocity.y = 1.0 / (float) (rand()%maxVel -maxVel/2);
    return new;
}

void debug_simulation(Simulation simulation){
    for(unsigned i = 0; i < simulation.num_bodies; i++){
        SimulationBody body = simulation.bodies[i];
        printf("Body %3d: x=%.3lf\ty=%.3lf\tvx=%.3lf\t\tvy=%.3lf\n", 
        i+1, body.position.x, body.position.y, body.velocity.x, body.velocity.y);
        if(isnan(body.position.x) || isnan(body.position.y)){
            printf("Error: position is NAN!\n");
            exit(1);
        }
    }
}










