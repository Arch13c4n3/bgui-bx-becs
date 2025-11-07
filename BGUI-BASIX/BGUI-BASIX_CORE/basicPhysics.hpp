// simple ECS-style physics engine 
// future features:
// Spatial Hashing/Partitioning AABB collision system
// Timed Collision Checking (applicable for linear time games like Flappy Bird, Geometry dash etc..)
// ^ checks collision at specified time/intervals using ticking system
// ^ can be paired with Spatial Hashing
#pragma once

#include <raylib.h>
#include "basicGUI.hpp"
#include <map>
#include <iostream>

namespace BASIX{

    inline void ball_invert_velocity(float &speed , bool &Inverted){
        if (!Inverted) Inverted = true;
        else Inverted = false;
        speed *= -1.0;
    }

    // randomizes Direction 
    inline void randVelocity(Vector2 &speed, bool &isInvertedX, bool &isInvertedY){   
        switch(GetRandomValue(1,3)){
            case 1: ball_invert_velocity(speed.x, isInvertedX); break;
            case 2: ball_invert_velocity(speed.y, isInvertedY);  break;
            case 3:
            ball_invert_velocity(speed.x,isInvertedX);
            ball_invert_velocity(speed.y,isInvertedY);
            break;
        }
    }

    //spatial hashing by tracking position indices
    // requires calling clearCells after each frame for a fast refresh
    struct GridMap {
        int rowsBuffer;
        int colsBuffer;
        std::vector<std::vector<int>> posIndex;
        size_t totalEntities;
        GridMap(int rows,int cols);
        void addPosIndexToCell(Vector2 pos,int currentIndex); // loop-based indexing of indices
        void clearCells();
        int findCell(Vector2 pos); // returns 1D from 2D cell index
        int cellWidth;
        int cellHeight;
        private:
        size_t gridSize;
        int cellX,cellY;
    };  

}