<img width="1196" height="741" alt="BodySim" src="https://github.com/user-attachments/assets/b8002fa2-c4f6-4f46-a111-fd30b7c969cc" />  

# NBody Raylib
An n-body simulation made with Raylib and C.  
This projects takes a JSON file as an input and runs a real time n-body simulation with visual rendering.  
JSON files are loaded with cJSON.

## Building
1. Install Raylib on your system
2. Clone this repository
3. Compile with `make`

## Trying some examples
You can find some examples on the `examples/` directory.  
Run them with `./BodySim <example.json>`

## Creating Custom Simulations
Create a custom simulation by writing a JSON file.
| Parameter      |  Unit  | Description                           |
|----------------|--------|---------------------------------------|
| `scale`        | Un/AU  | Game units per Astronomical Unit (AU) |
| `name`         | -      | The bodies name                       |
| `color`        | RGB    | Visual color (0-255 values)           |
| `mass`         | kg     | Body mass                             |
| `radius`       | pixels | Displayed size                        |
| `position.x/y` | AU     | Initial position (fraction of AU)     |
| `velocity.x/y` | km/s   | Initial velocity                      |

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
      "position": { "x": -0.04, "y": 0.00 },
      "velocity": { "x": 0.00, "y": 0.25 }
    },
    {
      "name": "Beta",
      "color": { "r": 100, "g": 255, "b": 100 },
      "mass": 4e25,
      "radius": 100,
      "position": { "x": 0.04, "y": 0.00 },
      "velocity": { "x": 0.00, "y": -0.20 }
    }
  ]
}

```
Run your custom simulation with:
```bash
./BodySim path/to/your_sim.json
```

## Simulation Controls
The following keyboard controls are available during simulation:

| Key          | Action                     | 
|--------------|----------------------------|
| **F11**      | Toggle fullscreen mode     |
| **→**        | Increase simulation speed  | 
| **←**        | Decrease simulation speed  | 
| **T**        | Toggle trajectories        | 
| **D**        | Toggle debug information   | 
| **N**        | Toggle body names          |

