#ifndef MATH_HPP
#define MATH_HPP

#include "utils/utils.hpp"

struct Math
{
    static float GetRandomValue(int min, int max);

    static vec2 StringToVec2(string s);
    static vec3 StringToVec3(string s);
    static vec4 StringToVec4(string s);

};


#endif