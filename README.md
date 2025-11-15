this repo contains the headers and implementations for:

🖼️BGUI - basic gui - super fast minimal gui library for multi state components - still working on backend and some front-end layers

👌bx - basic physics - light weight physics solver - not implemented , might use jolt 

💅becs - basic ecs - implemented entity_soa_t with built in spatial hashing and handles

these headers are designed to be self contained with some seemeless interoptability 

currently uses raylib -> plans to move to webgpu(dawn) / vulkan / sokol

using becs' soa ecs example: (simplified, incomplete)
```c++
your_entity : becs::entity_soa_t
{
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
// your entites' attributes in soa format
  std::vector<Vector2> position;
  //... other std::vector<> elements

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

