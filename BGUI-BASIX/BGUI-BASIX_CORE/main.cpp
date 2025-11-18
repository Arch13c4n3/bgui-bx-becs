// tests

#define BGUI_DEBUG_MODE
#include "basicGUI.hpp"
#include "basicECS.hpp"

using namespace BGUI;

int main(){
    MainWindow window;
    window.set_WindowTitle("entity particle simulator frfr istg");

    enum class State : uint8_t {
        resting = 0, active, inActive, moving
    };

    auto randf = [](int min, int max){
        return 1.0f * GetRandomValue(min,max);
    };

    // Button button({{100.0f,100.f},100.0f,100.0f});
    // Button button1({{180.0f,100.f},200.0f,500.0f});
    // Button button2({{260.0f,100.f},700.0f,500.0f});

    // large entities sample
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

    circle_t circle;

    auto flip_velocity = [](float& speed){
        speed *= -1;
    };

    auto randColor = []{
        Color color = BLANK;
        switch (GetRandomValue(1,5))
        {
            case 1: color = WHITE; break;
            case 2: color = RED; break;
            case 3: color = BLUE; break;
            case 4: color = YELLOW; break;
            case 5: color = PINK; break;
        }
        return color;
    };

    // auto kill = [&](const int& iterator){
    //     circle.state[iterator] = State::inActive;
    // };

    auto randVel = [&]{
        Vector2 vec = {randf(-10,10),randf(-10,10)};
        return vec;
    };

    Sound_effect growSoundEffect("sound_effects/cartoon-jump.mp3");

    int iter = 0;
    //int iter2 = 0;
    Vector2 vec[200];
    int vec_idx = 0;

    window.logic = [&]
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_SPACE)) {
            circle.spawn(GetMousePosition(),5.0f,randVel(),randColor());
        }

        switch(GetKeyPressed())
        {
            case KEY_P: circle.isPaused = !circle.isPaused; break;
            case KEY_B: circle.do_show_bounding_box = !circle.do_show_bounding_box; break;
            case KEY_G: circle.do_show_grid = !circle.do_show_grid; break;
        }

        for (auto&& [pos,radius,speed,color,state] : std::views::zip(circle.pos,circle.radius,circle.speed,circle.color,circle.state))
        {
            // circumferences
            float c_rightX = pos.x + radius;
            float c_leftX = pos.x - radius;
            float c_leftY = pos.y - radius;
            float c_rightY = pos.y + radius;

            //window collision
            if (c_rightX >= window.Width){
                flip_velocity(speed.x);
                pos.x -= std::abs(c_rightX - window.Width);
                // growSoundEffect.play();
            } 
            else if (c_leftX <= 0.0f){
                flip_velocity(speed.x);
                pos.x += std::abs(0.0f - c_leftX);
                // growSoundEffect.play();
            }

            if (c_leftY <= 0.0f){
                flip_velocity(speed.y);
                pos.y += std::abs(0.0f - c_leftY);
                // growSoundEffect.play();
            } 
            else if (c_rightY >= window.Height){
                flip_velocity(speed.y);
                pos.y -= std::abs(c_rightY - window.Height);
                //growSoundEffect.play();
            } 

            circle.move(iter);

                // //velocity / energy decay
                // if (speed.x > 0.0f) speed.x -= 0.1f;
                // else if (speed.x < 0.0f) speed.x += 0.1f;
                // else speed.x = 0.0f;

                // if (speed.y > 0.0f) speed.y -= 0.0f;
                // else if (speed.y < 0.0f) speed.y += 0.1f;
                // else speed.y = 0.0f;                  

                //entity collision
                // for (auto&& [pos2,radius2,speed2, color2,state2] : std::views::zip(circle.pos,circle.radius,circle.speed,circle.color,circle.state))
                // {
                //     if (iter != iter2 && (state2 == State::active || state2 == State::resting))
                //     {
                //         if (CheckCollisionCircles(pos,radius,pos2,radius2))
                //         {
        
                //             if (compare_Colors(color,Comparison::isEqual,color2)){
                //                 radius += radius2;
                //                 kill(iter2);
                //                 PlaySound(growSoundEffect);
                //                 if (radius > 90) kill(iter);
                //             } 
                //         }
                //     } 
                //     iter2++;
                // }
            

            //iter2 = 0;
            iter++;
        }

        iter = 0;
    };

    window.canvas([&]{
        for (auto&& [pos,radius,color,state] : std::views::zip(circle.pos,circle.radius,circle.color,circle.state)){
            if (state != State::inActive) DrawCircleV(pos,radius,color);
        }

        circle.draw_debug();

        DrawText(TextFormat("entity count: %d", circle.get_entity_count()),100,100,20,YELLOW);

        DrawFPS(100,150);

        circle.clear_cells();

        int fps = GetFPS();

        // debug
        if (fps < 60 && vec_idx < 200) {
            float vecX = 100.0f + vec_idx*2;
            float vecY = 300.0f - fps;
            vec[vec_idx] = {vecX,vecY};

            vec_idx++;
        }

        DrawLineStrip(vec,200,YELLOW);
    });

    return 0;
}
