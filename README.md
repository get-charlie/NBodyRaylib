<img width="1196" height="741" alt="BodySim" src="https://github.com/user-attachments/assets/b8002fa2-c4f6-4f46-a111-fd30b7c969cc" />  

# NBody Raylib
A 3D n-body simulation made with Raylib and C.  
This projects takes a JSON file as an input and runs a real time n-body simulation with visual rendering.  
JSON files are loaded with cJSON.

The simulation runs in double precision SI units (meters, kilograms, seconds) on a pool of
POSIX threads, and the renderer draws it in 3D with an orbital camera that can follow any body.

## Building
1. Install Raylib on your system
2. Clone this repository
3. Compile with `make`

## Trying some examples
You can find some examples on the `examples/` directory.  
Run them with `./BodySim <example.json>`

```bash
./BodySim examples/solar_system.json         # 2D files still work, they run on the z = 0 plane
./BodySim examples/inclined_orbits.json      # orbits on different planes
./BodySim examples/solar_system.json -t 4    # limit the simulation to 4 threads
```

| Option       | Description                                            |
|--------------|--------------------------------------------------------|
| `-t threads` | Worker threads of the simulation (default: one per core) |

## Creating Custom Simulations
Create a custom simulation by writing a JSON file.
| Parameter        |  Unit  | Description                              |
|------------------|--------|------------------------------------------|
| `scale`          | Un/AU  | Game units per Astronomical Unit (AU)    |
| `name`           | -      | The bodies name                          |
| `color`          | RGB    | Visual color (0-255 values)              |
| `mass`           | kg     | Body mass                                |
| `radius`         | Un     | Displayed size, in units of `scale`      |
| `position.x/y/z` | AU     | Initial position (fraction of AU)        |
| `velocity.x/y/z` | km/s   | Initial velocity                         |

`z` is optional on both vectors, files written for the 2D version load unchanged and run on
the `z = 0` plane. Positions and velocities are physical values: `scale` only decides how big
the bodies look, it no longer affects the simulation itself.

### Example: 
```json
{
  "scale": 250000,
  "bodies": [
    {
      "name": "Alpha",
      "color": { "r": 255, "g": 100, "b": 100 },
      "mass": 4e25,
      "radius": 100,
      "position": { "x": -0.04, "y": 0.00, "z": 0.00 },
      "velocity": { "x": 0.00, "y": 0.25, "z": 0.05 }
    },
    {
      "name": "Beta",
      "color": { "r": 100, "g": 255, "b": 100 },
      "mass": 4e25,
      "radius": 100,
      "position": { "x": 0.04, "y": 0.00, "z": 0.00 },
      "velocity": { "x": 0.00, "y": -0.20, "z": 0.00 }
    }
  ]
}

```
Run your custom simulation with:
```bash
./BodySim path/to/your_sim.json
```

## Simulation Controls
The following controls are available during simulation:

| Key / Mouse         | Action                                      |
|---------------------|---------------------------------------------|
| **Left drag**       | Orbit the camera around its focus           |
| **Right drag**      | Pan the camera                              |
| **Wheel**           | Zoom in and out                             |
| **Left click**      | Select a body                               |
| **W A S D / Q E**   | Orbit and zoom with the keyboard            |
| **F**               | Follow the selected body                    |
| **TAB / SHIFT+TAB** | Select and follow the next / previous body  |
| **R**               | Frame the whole system                      |
| **SPACE**           | Pause                                       |
| **→**               | Increase simulation speed                   |
| **←**               | Decrease simulation speed                   |
| **C**               | Open the body creator                       |
| **T**               | Toggle trayectories                         |
| **N**               | Toggle body names                           |
| **G**               | Toggle grid                                 |
| **I**               | Toggle debug information                    |
| **B / V**           | Bigger / smaller bodies                     |
| **H**               | Toggle the controls panel                   |
| **F11**             | Toggle fullscreen mode                      |
| **ESC**             | Quit                                        |

### Camera
The camera always orbits around a focus point. `F` locks that focus on the selected body, so
the camera travels with it and its neighbours can be watched from a moving frame, and `R`
pulls back until the whole system fits on screen. The focus is kept in double precision and
everything is drawn relative to it, which keeps the picture stable at any zoom level.

### Body creator
`C` opens a form to add a body to the running simulation, with its name, mass, radius,
position, velocity and color. Fields are edited by typing, `TAB` and the arrow keys move
between them, `ENTER` creates the body and `ESC` closes the form. A wireframe preview shows
where the body is going to appear.

The form opens filled with the position of the camera focus, and with the velocity of the
body being followed when there is one, so creating a satellite is a matter of adding a few
km/s to the velocity it inherits.

## How it works
- **Logical state and rendering are separate.** The simulation keeps positions, velocities
  and masses in SI units and double precision (`Vec3`, meters, m/s, kg), and it never uses
  the frame time or any raylib type to advance. The renderer converts to single precision
  only when drawing, rebasing every position on the camera focus first, so the floats it
  produces are always small numbers no matter how far the bodies are from the origin.
- **Leapfrog integration with substeps.** Each frame of simulated time is integrated in
  kick-drift-kick substeps of at most one hour, which keeps the energy of the system stable
  even at the highest simulation speeds. The debug line shows the energy drift.
- **Parallel forces.** The O(n²) force computation is split between POSIX threads by body
  index: every worker writes only to its own range of bodies, so no locks are needed. The
  threads are created once and parked on a condition variable, and small systems fall back
  to a single thread where synchronizing would cost more than it saves.
