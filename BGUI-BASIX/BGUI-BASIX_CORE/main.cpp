#define BGUI_DEBUG_MODE
#include "basicGUI.hpp"
#include "basicECS.hpp"

using namespace BGUI;

int main(){
    MainWindow window;

    enum class State : uint8_t {
        resting = 0, active, inActive, moving
    };

    // using namespace BECS;

    auto randf = [](int min, int max){
        return 1.000f * GetRandomValue(min,max);
    };

    // Button button({{100.0f,100.f},100.0f,100.0f});
    // Button button1({{180.0f,100.f},200.0f,500.0f});
    // Button button2({{260.0f,100.f},700.0f,500.0f});

    struct circle_t : BECS::entity_SOA_t
    {
        circle_t()
        {
            this->init_grid_space({10,10,1000.0f,1000.0f});
        }

        std::vector<Vector2> pos;
        std::vector<float> radius;
        std::vector<Vector2> speed;
        std::vector<Color> color;
        std::vector<State> state;

        void add(Vector2 p,float r,Vector2 s, Color c)
        {
            this->pos.push_back(p);
            this->radius.push_back(r);
            this->speed.push_back(s);
            this->color.push_back(c);
            this->state.push_back(State::active);

            Rectangle bounding_box = {p.x - r, p.y - r, r*2, r*2};
            this->register_entity(this->get_entity_id(),bounding_box);
        }

        void move(int entity_id)
        {
            Vector2 pos = this->pos[entity_id];
            Vector2 speed = this->speed[entity_id];
            float radius = this->radius[entity_id];

            pos.x += speed.x;
            pos.y += speed.y;

            Rectangle bounding_box = {pos.x - radius,pos.y - radius,radius * 2,radius * 2};
            this->update_bounding_box(entity_id,bounding_box);
            this->update_entity_grid_position(entity_id);
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

    auto kill = [&](const int& iterator){
        circle.state[iterator] = State::inActive;
    };

    auto randVel = [&]{
        Vector2 vec = {randf(-10,10),randf(-10,10)};
        return vec;
    };

    Sound_effect growSoundEffect("sound_effects/cartoon-jump.mp3");

    int iter = 0;
    int iter2 = 0;

    auto check_iter_state = [&](const int& iterator){
        return circle.state[iterator];
    };

    window.logic = [&]
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_SPACE)) circle.add(GetMousePosition(),30.0f,randVel(),randColor());

        for (auto&& [pos,radius,speed,color,state] : std::views::zip(circle.pos,circle.radius,circle.speed,circle.color,circle.state))
        {
            if (state == State::active)
            {
                // window collision
                if (pos.x + radius >= window.Width){
                    flip_velocity(speed.x);
                    pos.x -= 1.0f;
                    growSoundEffect.play();
                } 
                else if (pos.x - radius <= 0.0f){
                    flip_velocity(speed.x);
                    pos.x += 1.0f;
                    growSoundEffect.play();
                }

                if (pos.y - radius <= 0.0f){
                    flip_velocity(speed.y);
                    pos.y += 1.0f;
                    growSoundEffect.play();
                } 
                else if (pos.y + radius >= window.Height){
                    flip_velocity(speed.y);
                    pos.y -= 1.0f;
                    growSoundEffect.play();
                } 
                else if (pos.y + radius <= window.Height - 9.8f ){
                    pos.y += 9.8f; // gravity
                } 
                
                circle.move(iter);

                //velocity / energy decay
                if (speed.x > 0.0f) speed.x -= 0.1f;
                else if (speed.x < 0.0f) speed.x += 0.1f;
                else speed.x = 0.0f;

                if (speed.y > 0.0f) speed.y -= 0.0f;
                else if (speed.y < 0.0f) speed.y += 0.1f;
                else speed.y = 0.0f;                  

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
            }

            //iter2 = 0;
            iter++;
        }

        iter = 0;
        
    };

    window.canvas([&]{
        for (auto&& [pos,radius,color,state] : std::views::zip(circle.pos,circle.radius,circle.color,circle.state)){
            if (state == State::active || state == State::resting) DrawCircleV(pos,radius,color);
        }

        circle.draw_grid();

        DrawFPS(500,500);

        circle.clear_cells();
    });
     
    return 0;
}
