#include "simulation.h"
#include "physics.h"

void add_simulation_body(Simulation* simulation, SimulationBody body){
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

void add_tpoint(SimulationBody* body, Vector2 point){
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
        SimulationBody* body = &simulation->bodies[i];
        if (body->trayectory.count > 0) {
            Vector2 lastp = body->trayectory.points[body->trayectory.count -1];
            if (sqrt(pow(body->position.x - lastp.x, 2) + pow(body->position.y - lastp.y, 2)) >= TSIZE) { // TODO FIX DISTANCE
                add_tpoint(body, body->position);
            }
        } else {
            add_tpoint(body, body->position);
        }
    }
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
    // int maxVel = 1000;
    // new.velocity.x = 1.0 / (float) (rand()%maxVel -maxVel/2);
    // new.velocity.y = 1.0 / (float) (rand()%maxVel -maxVel/2);
    sprintf(new.name, "R-%d", rand()%1000+1);
    return new;
}

void debug_simulation(Simulation simulation){
    for(unsigned i = 0; i < simulation.count; i++){
        SimulationBody body = simulation.bodies[i];
        printf("Body %3d: x=%.3lf\ty=%.3lf\tvx=%.3lf\t\tvy=%.3lf\n", 
        i+1, body.position.x, body.position.y, body.velocity.x, body.velocity.y);
        printf("Circular buffer: count %d\n", body.trayectory.count);
        for(unsigned j = 0; j < body.trayectory.count; j++){
            printf("Point %d x = %f, y = %f\n", j, body.trayectory.points[j].x, body.trayectory.points[j].y);
        }
        if(isnan(body.position.x) || isnan(body.position.y)){
            printf("Error: position is NAN!\n");
            exit(1);
        }
    }
}










