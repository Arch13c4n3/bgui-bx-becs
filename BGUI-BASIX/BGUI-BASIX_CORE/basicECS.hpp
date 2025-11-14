// basic ECS - minimal entity component system for entities

#pragma once

#include <iostream>
#include <raylib.h>
#include <vector>

//basic ECS 
namespace BECS 
{
    struct grid_prop_t 
    {
        int rows = 0, columns = 0;
        float layout_width = 0.0f, layout_height = 0.0f; // whole surface area of the grid
        //-- optional --
        float cell_size = 0.0f;
        int border_line_thickness = 1;
    };

    // grid space for entities
    class Grid
    { 
    public:
        Grid(){}
        Grid(grid_prop_t grid_properties); //define calculate grid & draw on texture 
        ~Grid(); // unload textures
        grid_prop_t properties;
        std::vector<Rectangle> cells_dimensions; // contains each cells rects dimensions
        inline void draw_texture(const Vector2& pos);
        void create_grid(grid_prop_t grid_properties);
        bool isCreated = false;
    private:
        RenderTexture2D gridRenderTexture;
    };

    // base ecs for structure of arrays
    // TODO:
    // hashing optimization idea:
    //  only get the velocity(speed,direction) 
    //  to predict the next occupied cells which avoids recalculating points
    //     
    struct entity_SOA_t 
    {
    public:
        inline void init_grid_space(grid_prop_t grid_properties)
        {
            this->grid_space.create_grid(grid_properties);
            this->grid_size = this->grid_space.properties.rows * this->grid_space.properties.columns;
            this->cell_entity_id_grid.resize(grid_size);
            this->m_bounding_box.resize(grid_size);
        }
        // used to assign a unused entity id for register_entity
        inline int get_entity_id()
        {
            int entity_id = s_entity_id_counter;
            s_entity_id_counter++;
            return entity_id;
        }
        // registers entity to the ECS and spatial hashing system
        // -> should be called on creation of the entity 
        // stores the bounding box for ecs handling
        inline void register_entity(int entity_id, Rectangle bounding_box)
        {
            if (grid_space.isCreated)
            {
                this->update_bounding_box(entity_id,bounding_box);
                this->update_entity_grid_position(entity_id);
            }
        }

        inline int get_cell_point(Vector2 position)
        {
            return std::clamp(
                static_cast<int>(position.y / 100) *
                 this->grid_space.properties.rows +
                 static_cast<int>(position.x / 100),
                0,
                this->grid_size - 1
            );
        }
        // for updating the entity's occupied cells / hashing
        void update_entity_grid_position(int entity_id)
        {
            Rectangle bounding_box = this->m_bounding_box[entity_id];
            int rows = this->grid_space.properties.rows;
            int p1 = get_cell_point({bounding_box.x,bounding_box.y});
            int p1_adj = get_cell_point({bounding_box.x + bounding_box.width, bounding_box.y});
            int p2_adj = get_cell_point({bounding_box.x, bounding_box.y + bounding_box.height});
            int p1_adj_d = p1_adj - p1;
            
            for (int i = 0; i <= p1_adj_d; i++){
                for (int i2 = p1 + i; i2 < p2_adj; i2 += rows)
                {
                    this->cell_entity_id_grid[i2].emplace_back(entity_id);
                }
            }
        }

        // for updating the specified bounding box in the bounding box vector
        inline void update_bounding_box(int entity_id, Rectangle new_bounding_box)
        {
            this->m_bounding_box[entity_id] = new_bounding_box;
        }

        inline void clear_cells()
        {
            for (int i =0;i<grid_size;i++)
                this->cell_entity_id_grid[i].clear();
        }

        std::vector<Rectangle> m_bounding_box;
        //std::vector<int> active_entity_id;
        std::vector<std::vector<int>> cell_entity_id_grid;
        Grid grid_space;
        int grid_size = 0;
        //int iter = 0;

    // --------------------------- debug --------------------------------
        inline void draw_grid()
        {
            for (int i = 0; i < grid_size;i++)
            {
                if (this->cell_entity_id_grid[i].empty())
                {
                    DrawRectangleLinesEx(grid_space.cells_dimensions[i],0.5f,WHITE);
                }
                else DrawRectangleLinesEx(grid_space.cells_dimensions[i],0.5f,BLUE);
            }

        }  

    private:
        static inline int s_entity_id_counter = 0;
    };
}
