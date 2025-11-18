this repo contains the headers and implementations for:

🖼️BGUI - basic gui - super fast minimal gui library for multi state components - still working on backend and some front-end layers

👌bx - basic physics - light weight physics solver - not implemented , might use jolt 

💅becs - basic ecs - implemented entity_soa_t with built in spatial hashing and handles

these headers are designed to be self contained with some seemeless interoptability 

currently uses raylib -> plans to move to webgpu(dawn) / vulkan / sokol

using becs' soa example: (simplified, incomplete)
```c++
struct your_entity : becs::entity_soa_t
{
// your entites' attributes in soa format
  std::vector<Vector2> position;
  //... other std::vector<> elements

  your_entity(){
// initializing the grid for spatial partitioning
    becs::entity_soa_t::init_grid_space({
      .rows = 10,
      .columns = 10,
      .layout_width = 1000.0f,
      .layout_height = 1000.0f,
      // .... other optional properties
    });
  }

  void spawn(Vector2 position) // for adding entities
  {
    this->position.emplace_back(position);
//... other entity spawning logic
    Rectangle bounding_box = {position.x,position.y,100.0f,100.0f};

// register an entity with an id and bounding box
// which will both be managed by the ecs' spatial hashing system
    becs::entity_soa_t::register_entity( 
      becs::entity_soa_t::get_entity_id(),
      bounding_box
    );
  }

  void move(int entity_id, Vector2 new_position) // movement logic
  {
    this->position[entity_id] = new_position;
    Rectangle new_bounding_box = {position.x,position.y,100.0f,100.0f};

// update the bounding box in the ecs to ensure changes reflect
    becs::entity_soa_t::update_entity_bounding_box(entity_id,new_bounding_box);
  }

  //...
}
```

⌀⌀ practical sample, particle simulator. large entities. fps fluctations around ~3000 
```c++
    struct circle_t : becs::entity_soa_t
    {
        std::vector<Vector2> pos;
        std::vector<float> radius;
        std::vector<Vector2> speed;
        std::vector<Color> color;
        std::vector<State> state;
        bool isPaused = false;

        circle_t()
        {
            const int max_entities = 10000;
            becs::entity_soa_t::set_max_entites(max_entities);
            becs::entity_soa_t::init_grid_space({10,10,1000.0f,1000.0f});
            
            pos.reserve(max_entities);
            radius.reserve(max_entities);
            speed.reserve(max_entities);
            color.reserve(max_entities);
            state.reserve(max_entities);
        }

        void spawn(Vector2 p,float r,Vector2 s, Color c)
        {
            this->pos.emplace_back(p);
            this->radius.emplace_back(r);
            this->speed.emplace_back(s);
            this->color.emplace_back(c);
            this->state.emplace_back(State::active);

            Rectangle bounding_box = {p.x - r, p.y - r, r*2, r*2};

            becs::entity_soa_t::register_entity(becs::entity_soa_t::assign_entity_id(),bounding_box);
        }

        void move(int entity_id)  
        {
            Vector2& pos = this->pos[entity_id];         
            Vector2 speed = this->speed[entity_id];
            float radius = this->radius[entity_id];
            float r2 = radius*2;
             
            if (!isPaused){
                pos.x += speed.x;
                pos.y += speed.y;
            }

            Rectangle new_bounding_box = {pos.x - radius,pos.y - radius,r2,r2};

            becs::entity_soa_t::update_entity_bounding_box(entity_id,new_bounding_box);
        }         
    };
```

