#include "basicGUI.hpp"
#include <format>

void EditorLayer::editor()
{
    BGUI::MainWindow editorWindow;
    editorWindow.set_TargetFPS(60);
    editorWindow.set_WindowTitle("editor!");

    Image Icon = LoadImage("assets/tanom.png");

    SetWindowIcon(Icon);

    UnloadImage(Icon);

    enum class MousebuttonAction : uint8_t {
        NONE,LEFT_CLICK,RIGHT_CLICK, LEFT_DOWN, RIGHT_DOWN, LEFT_UP, RIGHT_UP
    };
    
    enum class EditMode : uint8_t {
        none,resize_p1,resize_p2,resize_p3,resize_p4,drag
    };

    enum class Direction : uint8_t {
        none, up,down,left,right
    };

    struct point_t {
        Vector2 *position;
        float radius;
        Color color;
    };
     
    // auto addVector2 = [](Vector2 vec1 , Vector2 vec2)
    // {
    //     return (Vector2){ vec1.x + vec2.x , vec1.y + vec2.y };
    // };

    // auto getEditMode = [](size_t index){
    //     switch(static_cast<int>(index)){
    //         case 0: return EditMode::none; break;
    //         case 1: return EditMode::resize_p1; break;
    //         case 2: return EditMode::resize_p2; break;
    //         case 3: return EditMode::resize_p3; break;
    //         default: return EditMode::none;
    //     }
    // };

    EditMode editmode = EditMode::none;
    //current mouse button pressed
    Rectangle rect = BGUI::zeroRectangle;
    Vector2 p1 = BGUI::zeroVector2;
    Vector2 p2 = BGUI::zeroVector2;
    Vector2 p3 = BGUI::zeroVector2;
    Vector2 p2_fixed = BGUI::zeroVector2;

    auto init_rect = [&]{
        rect = {100,100,200,200};
        p1 = {rect.x,rect.y};
        p2 = {p1.x+rect.width,p1.y+rect.height};
        p3 = {(p2.x + p1.x) / 2.0f,(p2.y + p1.y) / 2.0f};
        p2_fixed = p2;
    };

    auto save_rect = [&]{
        std::fstream file("save.bdl", std::ios::app);
        if (!file) std::cerr << " failed accessing file in append mode";
        else {
            const char* text = TextFormat("#rectangle {x %.0f, y %.0f, width %.0f, height %.0f}", rect.x,rect.y,rect.width,rect.height);
            file << text << "\n";
        }
    };

    auto get_saved_rect = [&]{
        std::fstream file("save.bdl", std::ios::in);
        if (!file) std::cerr << " failed accessing file in read mode";
        else {
            
        }
    };

    std::vector<point_t> points;
    
    points.push_back({&p1,10,BLUE});
    points.push_back({&p2,10,BLUE});
    points.push_back({&p3,10,BLUE});

    Vector2 mousePosition;
    Vector2 mouseDelta;

    bool isMode_reset = false;
    bool isMouseInWindow = true;

    float x = 700.0f;
    float yp = 5.0f;
    BGUI::container_t textContainer({x,yp,300.0f,500.0f});
    textContainer.properties.isOutlineVisible = true;
    textContainer.properties.font_size = 40;
    textContainer.add_text({
        .text = "rectangle",
        .font_size = 20,
        .color = RED,
    });

    auto logic = [&]
    {
        switch (GetKeyPressed()){
            case KEY_SPACE: init_rect(); break;
            case KEY_ENTER: save_rect(); break;
        }
        mouseDelta = GetMouseDelta();
        mousePosition = GetMousePosition();
        
        if(( mousePosition.x > editorWindow.Width || mousePosition.y > editorWindow.Height)
            || (mousePosition.x < 0 || mousePosition.y < 0))
        {
            isMouseInWindow = false;
        }
        else isMouseInWindow = true;
    
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            if (CheckCollisionPointCircle(mousePosition,*(points[0].position),points[0].radius)){
                editmode = EditMode::resize_p1;
            }
            else if (CheckCollisionPointCircle(mousePosition,*(points[1].position),points[1].radius)){
                editmode = EditMode::resize_p2;
            }
            else if (CheckCollisionPointCircle(mousePosition,*(points[2].position),points[2].radius)){
                editmode = EditMode::drag;
            } 

            isMode_reset = true;

            p3 = {(p2.x + p1.x) / 2.0f,(p2.y + p1.y) / 2.0f};
        }
        else if (IsMouseButtonUp(MOUSE_BUTTON_LEFT) && isMode_reset){
            //reset values
            editmode = EditMode::none;
            isMode_reset = false;
            p2_fixed = p2;
        }
    };

    auto render = [&]
    {
        if (isMouseInWindow){
            switch (editmode)
            {
                case EditMode::resize_p1:
                    DrawText(TextFormat("resize mode p1: %d",EditMode::resize_p1 ),10,10,30,YELLOW);
                   // if ( compareVector2(p1,Comparison::isEqual,p2))
                    p1 = mousePosition;
                    rect.x = p1.x;
                    rect.y = p1.y;
                    rect.width = p2_fixed.x - p1.x;
                    rect.height = p2_fixed.y - p1.y;
                break;

                case EditMode::resize_p2:
                    DrawText(TextFormat("resize mode p2: %d",EditMode::resize_p2),10,10,30,YELLOW);
                    p2 = mousePosition;
                    rect.width = p2.x-p1.x;
                    rect.height = p2.y-p1.y;
                break;

                case EditMode::drag:
                    DrawText(TextFormat("drag mode p3: %d",EditMode::drag),10,10,30,YELLOW);
                    p3 = mousePosition;
                    p1 = {p1.x+mouseDelta.x, p1.y+mouseDelta.y}; 
                    p2 = {p2.x+mouseDelta.x, p2.y+mouseDelta.y};
                    rect.x = p1.x;
                    rect.y = p1.y;
                break;
            }
        }
        
        for (const auto& i : points){
            DrawCircleV(*i.position, i.radius, i.color);
        }
          
        DrawRectangleLinesEx(rect,3,WHITE);

        if (!isMode_reset){
            DrawText("Mouse left button up",10,100,20,RED);
        }

        textContainer.live_text(TextFormat("x: %f", rect.x));
        textContainer.live_text(TextFormat("y: %f", rect.y));
        textContainer.live_text(TextFormat("width %f",rect.width));
        textContainer.live_text(TextFormat("heigth %f",rect.height));
        
        DrawText(TextFormat("mouse position %f,%f",mousePosition.x,mousePosition.y),10,30,30,GREEN);
        DrawFPS(10,50);

    };

    //editorWindow.running(logic,render);
}

void EditorLayer::push_vmbp_to_editorLayer(BGUI::vm_blueprint_t &vmbp)
{
    EditorLayer::vmbp = &vmbp;
}
