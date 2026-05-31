vec2 complex_add(vec2 a, vec2 b)
{
    return vec2(a.x+b.x, a.y+b.y);
}

vec2 complex_mul(vec2 a, vec2 b)
{
    return vec2(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}

float complex_abs(vec2 c)
{
    return sqrt(c.x*c.x + c.y*c.y);
}

vec2 complex_exp(vec2 c)
{
    return exp(c.x)*vec2(cos(c.y), sin(c.y));
}
