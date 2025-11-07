#include "basicPhysics.hpp"

BASIX::GridMap::GridMap(int rows,int cols)
{
    this->rowsBuffer = rows;
    this->colsBuffer = cols;
    this->cellWidth = GetScreenWidth() / rows;
    this->cellHeight = GetScreenHeight() / cols;
    this->gridSize = (size_t)(this->rowsBuffer * this->colsBuffer);
    posIndex.resize(gridSize);
    for (int i = 0; i<(int)gridSize;i++){
        posIndex[i].reserve(totalEntities);
    }
}

void BASIX::GridMap::addPosIndexToCell(Vector2 pos, int currentIndex)
{
    this->posIndex[findCell(pos)].push_back(currentIndex);
}

void BASIX::GridMap::clearCells()
{
    for (auto &cell : this->posIndex) cell.clear();
}

int BASIX::GridMap::findCell(Vector2 pos)
{
    return static_cast<int>(pos.x / this->cellWidth) * this->rowsBuffer + static_cast<int>(pos.y / this->cellHeight);
}


