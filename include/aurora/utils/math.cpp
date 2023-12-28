#include "math.hpp"

float Math::GetRandomValue(int min, int max)
{
    if (min > max)
    {
        int tmp = max;
        max = min;
        min = tmp;
    }

    return (rand()%(abs(max - min) + 1) + min);
}

vec2 Math::StringToVec2(string s)
{
    // Pointer to point the word returned by the strtok() function.
    char* p;
    // Here, the delimiter is white space.
    p = strtok(s.data(), ",");
    int ctr = 0;
    vec2 v = {};
    while (p != NULL) {
        //cout << p << endl;
        v[ctr] = stof(string(p));
        p = strtok(NULL, ",");
        ctr++;
    }
    return v;
}

vec3 Math::StringToVec3(string s)
{
    // Pointer to point the word returned by the strtok() function.
    char* p;
    // Here, the delimiter is white space.
    p = strtok(s.data(), ",");
    int ctr = 0;
    vec3 v = {};
    while (p != NULL) {
        //cout << p << endl;
        v[ctr] = stof(string(p));
        p = strtok(NULL, ",");
        ctr++;
    }
    return v;
}

vec4 Math::StringToVec4(string s)
{
    // Pointer to point the word returned by the strtok() function.
    char* p;
    // Here, the delimiter is white space.
    p = strtok(s.data(), ",");
    int ctr = 0;
    vec4 v = {};
    while (p != NULL) {
        //cout << p << endl;
        v[ctr] = stof(string(p));
        p = strtok(NULL, ",");
        ctr++;
    }
    return v;
}

vec3 Math::FixEulers(vec3 e)
{
    return { e.x, e.y, e.z };
}
