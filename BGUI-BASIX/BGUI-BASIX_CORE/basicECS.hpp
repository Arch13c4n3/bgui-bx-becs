// basic ECS - minimal entity component system for entities
#pragma once

#include <iostream>
#include <raylib.h>
#include <vector>
#include <functional>

//basic ECS 
namespace becs
{
    struct grid_prop_t 
    {
        int rows = 0, columns = 0;
        float layout_width = 0.0f, layout_height = 0.0f; // whole surface area of the grid
        //-- optional --
        float cell_height = 0.0f;
        float cell_width = 0.0f;
        float border_line_thickness = 0.5f;
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
        void draw_texture(const Vector2& pos) const;
        void create_grid(grid_prop_t grid_properties);
        bool isCreated = false;
    private:
        RenderTexture2D gridRenderTexture;
    };

    // rendertexture2d handling 
    class pre_render_texture_2d
    {
    public:
        pre_render_texture_2d(){}

        pre_render_texture_2d(int texture_width,int texture_height,std::function<void()> draw_call)
        {   
            this->render(texture_width,texture_height,draw_call);
        }

        ~pre_render_texture_2d()
        {
            UnloadRenderTexture(rendered_texture);
        }
        
        void render(int texture_width,int texture_height,std::function<void()> draw_call)
        {
            rendered_texture = LoadRenderTexture(texture_width,texture_height);
            BeginTextureMode(rendered_texture);
            ClearBackground(BLANK);
            draw_call();
            EndTextureMode();

            this->isPreRendered = true;
        }

        bool isPreRendered = false;
        RenderTexture2D rendered_texture;
    };

    // base ecs for structure of arrays    
    // NOTE: only works for a 10x10 grid for 1000x1000 layout (will be flexible soon)
    struct entity_soa_t 
    {
    public:
        inline void init_grid_space(grid_prop_t grid_properties)
        {
            m_grid_space.create_grid(grid_properties);
            m_bounding_box_stack.resize(m_max_entities);

            m_grid_size = m_grid_space.properties.rows * m_grid_space.properties.columns;
           // m_cell_entity_id_grid.resize(m_grid_size);
        }
        // used to assign a unused entity id for register_entity
        inline int assign_entity_id(){
            int entity_id = s_entity_id_counter;
            s_entity_id_counter++;
            return entity_id;
        }
        // registers entity to the ECS and spatial hashing system
        // -> should be called on creation of the entity 
        // stores the bounding box for ecs handling
        inline void register_entity(int entity_id, Rectangle bounding_box){
            if (m_grid_space.isCreated){
                this->update_entity_bounding_box(entity_id,bounding_box);
            }
        }

        inline int get_cell_point(Vector2 position) const
        {
            return std::clamp(
                static_cast<int>(position.y / m_grid_space.properties.cell_height) *
                 m_grid_space.properties.rows +
                 static_cast<int>(position.x /  m_grid_space.properties.cell_width),
                0,
                m_grid_size - 1
            );
        }
        // for updating the entity's occupied cells / hashing
        void update_entity_grid_position(int entity_id) const
        {
            const Rectangle bounding_box = m_bounding_box_stack[entity_id];
            const int rows = m_grid_space.properties.rows;
            const int p1 = get_cell_point({bounding_box.x,bounding_box.y});
            const int p1_adj = get_cell_point({bounding_box.x + bounding_box.width, bounding_box.y});
            const int p2_adj = get_cell_point({bounding_box.x, bounding_box.y + bounding_box.height});
            const int p1_adj_d = p1_adj - p1;
            const int cell_limit = m_grid_size;
            
            if (p1_adj_d == 0 && p1_adj == p2_adj && p1 < cell_limit) {
                m_cell_entity_id_grid[p1].emplace_back(entity_id);
            }
            else {
                for (int i = 0; i <= p1_adj_d; i++){
                    for (int i2 = p1 + i;i2 <= p2_adj + 1; i2 += rows){
                        if (i2 < cell_limit) m_cell_entity_id_grid[i2].emplace_back(entity_id);
                    }
                }
            }
        }

        // for updating the specified bounding box in the bounding box storage
        // also updates entity grid position
        inline void update_entity_bounding_box(int entity_id, Rectangle new_bounding_box){
            m_bounding_box_stack[entity_id] = new_bounding_box;
            this->update_entity_grid_position(entity_id);
        }

        inline void set_max_entities(int max){
            m_max_entities = max;
        }

        inline int get_entity_count() const {
            return s_entity_id_counter;
        }
        // clears internal ecs storage buffers but remain reusable
        inline void clear_buffers(){
            m_bounding_box_stack.clear();
            this->clear_cells();
        }

        inline void clear_cells(){
            for (int i = 0;i<m_grid_size;i++)
                m_cell_entity_id_grid[i].clear();
        }

    public: // --------------------------- debug --------------------------------
        bool do_show_bounding_box = false;
        bool do_show_grid = false;

        inline void draw_debug()
        {
            if (this->do_show_grid)
            {
                for (int i = 0;i < m_grid_size ;i++)
                {
                    if (m_cell_entity_id_grid[i].empty()){
                        DrawRectangleLinesEx(m_grid_space.cells_dimensions[i],0.5f,WHITE);
                    }
                    else {
                        DrawText(
                            TextFormat("%d", m_cell_entity_id_grid[i].size()),
                            static_cast<int>(m_grid_space.cells_dimensions[i].x),
                            static_cast<int>(m_grid_space.cells_dimensions[i].y),30,PINK
                        );
                    } 
                }

                m_grid_space.draw_texture({0.0f,0.0f});
            }

            if (this->do_show_bounding_box){
                for (const auto i : m_bounding_box_stack)
                    DrawRectangleLinesEx(i,0.8f,RED);
            }

            DrawText(TextFormat("entity grid space rows: %d", m_grid_space.properties.rows), 50,50,20,YELLOW);
        } 

    private:
        static inline int s_entity_id_counter = 0;
        int m_max_entities = 100;
        mutable std::vector<Rectangle> m_bounding_box_stack;
        //std::vector<int> active_entity_id;
        mutable std::vector<int> m_cell_entity_id_grid[100];
        Grid m_grid_space;
        int m_grid_size = 0;
    };
}
