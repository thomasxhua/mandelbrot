const float INFINITY = 1.0/0.0;

bool is_within(float ist, float soll, float max_dist)
{
    return abs(ist - soll) <= max_dist;
}

bool is_within_vec2(vec2 ist, vec2 soll, float max_dist)
{
    float d_x = ist.x-soll.x;
    float d_y = ist.y-soll.y;
    return sqrt(d_x*d_x + d_y*d_y) <= max_dist;
}

