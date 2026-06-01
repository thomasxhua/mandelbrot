#version 410 core

#include "shaders/utils.glsl"
#include "shaders/complex.glsl"

out vec4 frag_color;

uniform float time;
uniform float width;
uniform float height;

const float THICC = 0.002;

vec2 f(vec2 z)
{
    return z;
}

vec2 mandelbrot_once(vec2 z, vec2 c)
{
    return complex_add(complex_mul(z, z), c);
}

const uint OUTSIDE_MANDELBROT = -1;

uint mandelbrot(vec2 c, uint iterations, float escape_gate)
{
    vec2 z = vec2(0.0,0.0);
    for (uint i=0; i<iterations; ++i)
    {
        vec2 z_ = mandelbrot_once(z, c);
        if (complex_abs(z) >= escape_gate)
            return i;
        z = z_;
    }
    return OUTSIDE_MANDELBROT;
}

void main()
{
    // prepare coords
    vec2 uv = gl_FragCoord.xy;
    uv = uv / 1000.0;
    //uv = uv / vec2(width, height);
    uv = (uv * 2.0) - 1.0;
    uv = uv - vec2(0.5, 0.0);
    uv = 1.5*uv;
    // https://www.fractalset.com/learn/mandelbrot-zoom-locations
    vec2 zoom_point = vec2(-0.743643887037, 0.131825904205);
    //vec2 zoom_point = vec2(-0.7453, 0.113);
    uv = uv/exp(time/8) + zoom_point;
    // draw
    uint iterations = mandelbrot(uv, 1000, INFINITY);
    if (iterations == OUTSIDE_MANDELBROT)
        frag_color = vec4(0,0,0,1);
    else if (iterations % 2 == 0)
        frag_color = vec4(
            float(iterations)/100.0,
            float(iterations)/200.0,
            float(iterations)/50.0,
            1);
    else if (iterations % 3 == 0)
        frag_color = vec4(
            float(iterations)/400.0,
            float(iterations)/400.0,
            float(iterations)/50.0,
            1);
    else if (iterations % 5 == 0)
        frag_color = vec4(
            float(iterations)/50.0,
            float(iterations)/600.0,
            float(iterations)/200.0,
            1);
    else if (iterations % 7 == 0)
        frag_color = vec4(
            float(iterations)/50.0,
            float(iterations)/800.0,
            float(iterations)/100.0,
            1);
    else 
        frag_color = vec4(
            float(iterations)/100.0,
            float(iterations)/1000.0,
            float(iterations)/100.0,
            1);
    // draw coords
    if (false)
    {
        if (is_within(uv.x, 0.0, THICC))
            frag_color = vec4(1,1,1,0.1);
        else if (is_within(uv.y, 0.0, THICC))
            frag_color = vec4(1,1,1,0.1);
    }
}

