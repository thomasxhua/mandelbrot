#version 410 core

#include "shaders/utils.glsl"
#include "shaders/complex.glsl"

out vec4 frag_color;

const float THICC = 0.005;

float f(float x)
{
    return sin(x*x*x + x*x + x);
}

void main()
{
    vec2 uv = gl_FragCoord.xy;
    uv = uv / vec2(800.0, 600.0);
    uv = (uv * 2.0) - 1.0;
    if (is_within(uv.x, 0.0, THICC))
        frag_color = vec4(1,1,1,1);
    else if (is_within(uv.y, 0.0, THICC))
        frag_color = vec4(1,1,1,1);
    else if (is_within(uv.y, f(uv.x*2.0), THICC))
        frag_color = vec4(1,1,1,1);
    else
        frag_color = vec4(0,0,0,1);
}

