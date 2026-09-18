#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "simulation.h"
#include "threadpool.h"

#define G_CONST         6.67430e-11     // m^3 kg^-1 s^-2
#define AU              1.495978707e11  // m
#define KM              1.0e3           // m

// Close encounters would produce an infinite force with point masses, the
// softening length keeps the integrator stable without changing the orbits
// at the distances these simulations work with.
#define SOFTENING       1.0e7           // m

#define MAX_SUBSTEP     3600.0          // s, longest integration step
#define MAX_SUBSTEPS    64              // per frame, bounds the frame cost

// Thread policy: the program picks the thread count from the size of the
// system, the user does not choose it. Small systems are cheaper to solve on
// a single thread than to hand out to the pool.
#define MIN_THREADED_BODIES 64
#define BODIES_PER_THREAD   32

// All of these work in double precision and do not touch raylib, the
// simulation advances the same way no matter what the renderer is doing.

// Advances the simulation elapsed simulated seconds using a leapfrog
// (kick-drift-kick) integrator, split in fixed substeps.
void update_simulation(Simulation* simulation, ThreadPool* pool, double elapsed);

// Threads the simulation uses for a given body count, bounded by the
// processors of the machine.
unsigned threads_for_bodies(unsigned count);

// Total mechanical energy of the system, its drift is a good measure of how
// much precision the integrator is losing.
double simulation_energy(const Simulation* simulation);

#endif
