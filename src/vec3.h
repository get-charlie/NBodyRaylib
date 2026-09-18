#ifndef _VEC3_H
#define _VEC3_H

#include <math.h>

// Double precision 3D vector.
// This is the vector type used by the logical simulation, it is never
// handed to raylib: the graphics layer converts it to a (single precision)
// Vector3 only when something has to be drawn.
typedef struct {
    double x;
    double y;
    double z;
} Vec3;

static inline Vec3 vec3(double x, double y, double z)
{
    return (Vec3){ .x = x, .y = y, .z = z };
}

static inline Vec3 vec3_add(Vec3 a, Vec3 b)
{
    return (Vec3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

static inline Vec3 vec3_sub(Vec3 a, Vec3 b)
{
    return (Vec3){ a.x - b.x, a.y - b.y, a.z - b.z };
}

static inline Vec3 vec3_scale(Vec3 v, double s)
{
    return (Vec3){ v.x * s, v.y * s, v.z * s };
}

static inline double vec3_dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline double vec3_length(Vec3 v)
{
    return sqrt(vec3_dot(v, v));
}

static inline double vec3_distance(Vec3 a, Vec3 b)
{
    return vec3_length(vec3_sub(a, b));
}

static inline Vec3 vec3_normalize(Vec3 v)
{
    double len = vec3_length(v);
    if(len == 0.0){
        return vec3(0.0, 0.0, 0.0);
    }
    return vec3_scale(v, 1.0 / len);
}

static inline Vec3 vec3_cross(Vec3 a, Vec3 b)
{
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

#endif
