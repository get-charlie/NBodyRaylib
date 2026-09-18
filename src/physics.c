#include "physics.h"

// Bodies are split between threads by index: every worker only writes to the
// bodies of its own range while reading all of them, so the tasks need no
// synchronization beyond the barrier threadpool_run() already provides.
//
// Computing the forces is O(n^2) and is what the threads are for, while
// moving the bodies is O(n) and only pays for the synchronization on big
// systems, hence the two thresholds.
#define MIN_PARALLEL_FORCES 64
#define MIN_PARALLEL_MOVES  512

typedef struct {
    Simulation* simulation;
    double      dt;
} StepJob;

static void accel_task(void* arg, unsigned start, unsigned end, unsigned worker)
{
    (void)worker;
    Simulation* simulation = ((StepJob*)arg)->simulation;
    Body* bodies = simulation->bodies;
    unsigned count = simulation->count;
    const double softening2 = SOFTENING * SOFTENING;

    for(unsigned i = start; i < end; i++){
        Vec3 accel = vec3(0.0, 0.0, 0.0);
        for(unsigned j = 0; j < count; j++){
            if(j == i){
                continue;
            }
            Vec3 delta = vec3_sub(bodies[j].position, bodies[i].position);
            double dist2 = vec3_dot(delta, delta) + softening2;
            // a = G * m_j * d / |d|^3
            double factor = (G_CONST * bodies[j].mass) / (dist2 * sqrt(dist2));
            accel = vec3_add(accel, vec3_scale(delta, factor));
        }
        bodies[i].accel = accel;
    }
}

static void kick_task(void* arg, unsigned start, unsigned end, unsigned worker)
{
    (void)worker;
    StepJob* job = arg;
    Body* bodies = job->simulation->bodies;
    for(unsigned i = start; i < end; i++){
        bodies[i].velocity = vec3_add(bodies[i].velocity, vec3_scale(bodies[i].accel, job->dt));
    }
}

static void drift_task(void* arg, unsigned start, unsigned end, unsigned worker)
{
    (void)worker;
    StepJob* job = arg;
    Body* bodies = job->simulation->bodies;
    for(unsigned i = start; i < end; i++){
        bodies[i].position = vec3_add(bodies[i].position, vec3_scale(bodies[i].velocity, job->dt));
    }
}

static void forces(ThreadPool* pool, StepJob* job)
{
    threadpool_run(pool, accel_task, job, job->simulation->count, MIN_PARALLEL_FORCES);
}

static void move(ThreadPool* pool, ThreadTask task, StepJob* job)
{
    threadpool_run(pool, task, job, job->simulation->count, MIN_PARALLEL_MOVES);
}

void update_simulation(Simulation* simulation, ThreadPool* pool, double elapsed)
{
    if(simulation->count == 0 || elapsed == 0.0){
        return;
    }

    // Long frames are integrated in several substeps: the error of the
    // integrator grows with dt^2, and a single huge step makes close orbits
    // blow up at high simulation speeds.
    double steps = ceil(fabs(elapsed) / MAX_SUBSTEP);
    if(steps < 1.0){
        steps = 1.0;
    }
    if(steps > MAX_SUBSTEPS){
        steps = MAX_SUBSTEPS;
    }

    StepJob job = { .simulation = simulation, .dt = elapsed / steps };
    double half = job.dt * 0.5;

    forces(pool, &job);
    for(unsigned step = 0; step < (unsigned)steps; step++){
        job.dt = half;
        move(pool, kick_task, &job);        // half kick
        job.dt = elapsed / steps;
        move(pool, drift_task, &job);       // drift
        forces(pool, &job);
        job.dt = half;
        move(pool, kick_task, &job);        // half kick
    }
    simulation->time += elapsed;
}

double simulation_energy(const Simulation* simulation)
{
    double kinetic = 0.0;
    double potential = 0.0;
    for(unsigned i = 0; i < simulation->count; i++){
        const Body* body = &simulation->bodies[i];
        kinetic += 0.5 * body->mass * vec3_dot(body->velocity, body->velocity);
        for(unsigned j = i + 1; j < simulation->count; j++){
            double distance = vec3_distance(body->position, simulation->bodies[j].position);
            if(distance > 0.0){
                potential -= (G_CONST * body->mass * simulation->bodies[j].mass) / distance;
            }
        }
    }
    return kinetic + potential;
}
