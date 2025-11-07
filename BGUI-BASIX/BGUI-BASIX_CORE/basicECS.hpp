#pragma once

#include <iostream>
#include <raylib.h>
#include <vector>

//basic ECS 
namespace becs {
    struct entity {
        Vector2 position;
    };

    struct ball : entity {
        std::vector<float> radius;
        std::vector<Color> color;
    };
    
}