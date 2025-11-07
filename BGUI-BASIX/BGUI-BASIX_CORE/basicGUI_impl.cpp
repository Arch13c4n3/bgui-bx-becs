#include "basicGUI.hpp"
// this impl file contains IRS (INTERNAL RENDERING SYSTEM) , BluePrint_t and MainWindow implementations

BGUI::INTERNAL_RENDERING_SYSTEM::~INTERNAL_RENDERING_SYSTEM()
{
    UnloadRenderTexture(BATCHED_RENDERED_BLUEPRINTS);

    // for (auto& RBOS : RENDERED_BLUEPRINTS_OVERLAY_STACK){
    //     UnloadRenderTexture(RBOS);
    // }
    
    LOG.log_Text("program terminated");
}

BGUI::INTERNAL_RENDERING_SYSTEM::INTERNAL_RENDERING_SYSTEM(){
    LOG.log_Text("<<INTERNAL RENDERING SYSTEM INITIALIZED!>>");
}

void BGUI::INTERNAL_RENDERING_SYSTEM::PUSH_TO_BLUEPRINT_STACK(BluePrint_t& bluePrint)
{
    this->BLUEPRINT_STACK.push_back(&bluePrint);
    this->BLUEPRINT_STACK_SIZE = static_cast<int>(this->BLUEPRINT_STACK.size());

    LOG.log_Text(TextFormat(" BluePrint with id:%d Pushed to RENDER BODY STACK with Render Index:%d",bluePrint.id,Current_Render_Index));
    LOG.log_Text(TextFormat(" IRS BLUEPRINT_STACK_SIZE: %d", BLUEPRINT_STACK_SIZE));

    this->Current_Render_Index++;
}

void BGUI::INTERNAL_RENDERING_SYSTEM::BATCH_RENDER_BLUEPRINTS()
{
    {
        BGUI::ScopedTimer("INTERNAL_RENDERING_SYSTEM::BATCH_RENDER_BLUEPRINTS");
        //Pre renders blue prints stack
        this->BATCHED_RENDERED_BLUEPRINTS = LoadRenderTexture(SCREEN_WIDTH,SCREEN_HEIGHT);
        BeginTextureMode(BATCHED_RENDERED_BLUEPRINTS);
        ClearBackground(BLANK);

        for (auto& BS : this->BLUEPRINT_STACK)
        {
            BS->renderBody.Static();
            LOG.log_Text(TextFormat("[IRS: BATCH_RENDER_BLUEPRINT] BluePrint with id:%d is Rendered to BATCHED_RENDERED_BLUEPRINTS",BS->id));
        }

        EndTextureMode();
    }

}

void BGUI::INTERNAL_RENDERING_SYSTEM::push_vm_blueprint(vm_blueprint_t& vm)
{

    this->m_vm = &vm;
    LOG.log_Text("virtual model has been constructed");
    
}

void BGUI::INTERNAL_RENDERING_SYSTEM::Init_GUI_Grid_Space(int rows, int cols)
{
    this->Rows = rows;
    this->Cols = cols;
    this->cellWidth = SCREEN_WIDTH / rows;
    this->cellHeight = SCREEN_HEIGHT / cols;
    this->GUI_GRID_SIZE = rows*cols;
    for (int i = 0;i <cols;i++){
        for (int j = 0;j<rows;j++){
            this->cellRect_stack.push_back({
                static_cast<float>(j*cellWidth),
                static_cast<float>(i*cellHeight),
                cellWidth,cellHeight
            });
        }
    }
}

int BGUI::INTERNAL_RENDERING_SYSTEM::Get_Point_Grid_Position(Vector2 point)
{
    cached_Grid_Point = static_cast<int>(point.y / this->cellHeight) * this->Rows + static_cast<int>(point.x / this->cellWidth);
    if (cached_Grid_Point > this->GUI_GRID_SIZE) return (this->GUI_GRID_SIZE - 1);
    else if (cached_Grid_Point < 0) return 0;
    else return cached_Grid_Point;
}

void BGUI::INTERNAL_RENDERING_SYSTEM::Overlay_Rendered_Texture()
{
    if (this->isBLUEPRINTS_HASHED == false) {
        this->HASH_BLUEPRINTS_TO_GRID();
        this->isBLUEPRINTS_HASHED = true;
    }

    this->mousePosition = GetMousePosition();
    this->mouse_Grid_Position_Index = Get_Point_Grid_Position(this->mousePosition);
    this->current_id_stack_size_Buffer = static_cast<int>(Grid_Id_Index_Stack[mouse_Grid_Position_Index].size());

    if ( prevMGPI != mouse_Grid_Position_Index)
    {
        this->prevMGPI = mouse_Grid_Position_Index;
        
        if (current_id_stack_size_Buffer > 0){
            LOG.log_Text(TextFormat("currently accessing mouse_Grid_Position_Index: %d, id_stack_size: %d",mouse_Grid_Position_Index, current_id_stack_size_Buffer)); 
        }
    }
    if (current_id_stack_size_Buffer > 0 && !isMouseHovering_Component){
        //hover checks only on rects present in the cell
        for (int i = 0 ; i < current_id_stack_size_Buffer ;i++)
        {
            // currently uses id as index for blueprint stack, might revise soon
            this->blueprintBuffer = INTERNAL_RENDERING_INSTANCE.BLUEPRINT_STACK.at(Grid_Id_Index_Stack[mouse_Grid_Position_Index][i]);
            if(CheckCollisionPointRec(this->mousePosition,blueprintBuffer->componentProp->rect))
            { 
                // temp solution.. inefficient
                this->blueprintBuffer->renderBody.Hovered();
                
                if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) blueprintBuffer->renderBody.Clicked();
                else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) blueprintBuffer->signal_event(Signal::CLICKED);

                // debug
                DrawText("mouse is hovering",1,500,20,YELLOW);
                DrawText(TextFormat("blueprintBuffer.id: %d",blueprintBuffer->id),1,290,20,YELLOW);
                DrawText(TextFormat("mgpi: %d",mouse_Grid_Position_Index),1,200,20,YELLOW);
                DrawText(TextFormat("Grid_Id_Index_Stack size: %d",Grid_Id_Index_Stack[mouse_Grid_Position_Index].size()),1,230,20,YELLOW);
                DrawText(TextFormat("current_id_stack_size_Buffer: %d",current_id_stack_size_Buffer),1,260,20,YELLOW);
            }
        }
    }

    // displays current mouse index rect, debug purposes
    DrawRectangleLinesEx(this->cellRect_stack[mouse_Grid_Position_Index],3,BLUE);
    DrawText(TextFormat("mouse: %f,%f", mouseDelta.x,mouseDelta.y),500,40,20,SKYBLUE);   
    DrawText(TextFormat("mouse delta: %.2f,%.2f", mousePosition.x,mousePosition.y),500,10,20,SKYBLUE);
}

void BGUI::INTERNAL_RENDERING_SYSTEM::HASH_BLUEPRINTS_TO_GRID()
{
    this->Grid_Id_Index_Stack.resize(this->GUI_GRID_SIZE);
    LOG.log_Text(TextFormat("[HASH_BLUEPRINTS_TO_GRID] Grid_Id_Index resized, new size:%d",GUI_GRID_SIZE));
    int p1 = 0, p2 = 0 , p1_adj = 0, p2_adj = 0 ,  p1_adj_d = 0/*distance of p1 to its adj*/;
    Vector2 p_buffer = {0.0f,0.0f};
    
    {// sample all blue prints' rects
        BGUI::ScopedTimer timer("INTERNAL_RENDERING_SYSTEM::HASH_BLUEPRINTS_TO_GRID");
        for (int f = 0;f<INTERNAL_RENDERING_INSTANCE.BLUEPRINT_STACK_SIZE;f++)
        {
            int emptyChecks = 0;

            p1 = Get_Point_Grid_Position(INTERNAL_RENDERING_INSTANCE.BLUEPRINT_STACK[f]->componentProp->position);
            p_buffer = {
                INTERNAL_RENDERING_INSTANCE.BLUEPRINT_STACK[f]->componentProp->rect.x + INTERNAL_RENDERING_INSTANCE.BLUEPRINT_STACK[f]->componentProp->rect.width,
                INTERNAL_RENDERING_INSTANCE.BLUEPRINT_STACK[f]->componentProp->rect.y + INTERNAL_RENDERING_INSTANCE.BLUEPRINT_STACK[f]->componentProp->rect.height
            };
            p2 = Get_Point_Grid_Position(p_buffer);
            p1_adj = Get_Point_Grid_Position({p_buffer.x , INTERNAL_RENDERING_INSTANCE.BLUEPRINT_STACK[f]->componentProp->position.y});
            p2_adj = Get_Point_Grid_Position({INTERNAL_RENDERING_INSTANCE.BLUEPRINT_STACK[f]->componentProp->position.x , p_buffer.y}); 

            LOG.log_Text(TextFormat("Currently Hashing id: %d to Points p1: %d , p2: %d",INTERNAL_RENDERING_INSTANCE.BLUEPRINT_STACK[f]->id, p1,p2));

            // with empty checks sampling , will optimized further soon
            for (int i = p1; i <= p2; i++)
            {
                if (CheckCollisionRecs(INTERNAL_RENDERING_INSTANCE.BLUEPRINT_STACK[f]->componentProp->rect,cellRect_stack[i]))
                {
                    LOG.log_Text(TextFormat("Hashed. point %d",i));
                    this->Grid_Id_Index_Stack.at(i).push_back(INTERNAL_RENDERING_INSTANCE.BLUEPRINT_STACK[f]->id);
                }
                else emptyChecks++;
            } 
            LOG.log_Text(TextFormat("emptyChecks: %d",emptyChecks));
        }
    }

    LOG.log_Text("[HASH_BLUEPRINTS_TO_GRID] BLUEPRINTS_STACK has been hashed to Grid_Id_Index_Stack");
}

void BGUI::INTERNAL_RENDERING_SYSTEM::push_to_container_stack(container_prop_t *container)
{
    this->container_prop_stack.push_back(container);
}

void BGUI::INTERNAL_RENDERING_SYSTEM::push_to_task_stack(task_t& task)
{
    if (task.fn != nullptr){
        if (task.isActive){
            activeTask_stack.push_back(&task);
        }
        else inActiveTask_stack.push_back(&task);
    }
    else LOG.log_Text("[INTERNAL_RENDERING_SYSTEM::push_to_task_stack] add task failed");
}

void BGUI::MainWindow::DRAWGUI()
{
    // if (!this->isGUI_BATCHED) {
    //     INTERNAL_RENDERING_INSTANCE.BATCH_RENDER_BLUEPRINTS();
    //     this->isGUI_BATCHED = true;
    // }
    // else [[likely]] {
    //     DrawTextureRec(
    //         INTERNAL_RENDERING_INSTANCE.BATCHED_RENDERED_BLUEPRINTS.texture,
    //         this->GUI_TEXTURE_SURFACE_PROPERTIES.source,{0.0f,0.0f},WHITE
    //     );
    // }
    
    INTERNAL_RENDERING_INSTANCE.Overlay_Rendered_Texture();

    // for (const auto *i : INTERNAL_RENDERING_INSTANCE.activeTask_stack){
    //     i->fn();
    // }

    // for (auto& i : INTERNAL_RENDERING_INSTANCE.container_prop_stack){
    //     i->drawText();
    //     i->m_text_pos_live = i->m_text_pos_buffer;
    // }

    // auto isColorsEqual = [](Color c1,Color c2){
    //     return (c1.a == c2.a && c1.b == c2.b && c1.g == c2.g && c1.r == c2.r);
    // };
    // auto& vm = INTERNAL_RENDERING_INSTANCE.m_vm;

    // if (isColorsEqual(vm->backgroundColor, BLANK) == false) ClearBackground(vm->backgroundColor);
    // else ClearBackground(BLACK);
    
    ClearBackground(BLACK);
}

BGUI::MainWindow::MainWindow(int width ,int height ,const char *title){
    this->Width = width;
    this->Height = height;
    InitWindow(width,height,title);

    set_gui_layout_defaults();

    set_TargetFPS(60);

    // pre rendered texture surface - for ui components
    this->GUI_TEXTURE_SURFACE_PROPERTIES.source = {
        0.0f,0.0f,static_cast<float>(this->Width),static_cast<float>(-this->Height)
    };
    this->GUI_TEXTURE_SURFACE_PROPERTIES.destination = {
        0.0f,0.0f,static_cast<float>(this->Width),static_cast<float>(this->Height)
    };

    init_grid_space();
}

void BGUI::MainWindow::set_WindowSize(int width, int height)
{
    this->Width = width;
    this->Height = height;
    SetWindowSize(Width,Height);
    set_gui_layout_defaults();
}

void BGUI::MainWindow::set_WindowTitle(const char *title)
{
    this->Title = title;
    SetWindowTitle(Title);
}

void BGUI::MainWindow::set_TargetFPS(int fps){
    SetTargetFPS(fps);
    this->Fps = fps;
}

void BGUI::MainWindow::set_gui_layout_defaults()
{
    BGUI::position_layout = {
        .top_left = 0.0f,
        .top_right = this->Width,
        .top_center = this->Width / 2.0f,
        .bottom_left = this->Height,
        .bottom_right = this->Height + this->Width,
        .bottom_center = position_layout.bottom_right / 2.0f,
        .center = (this->Height / 2.0f) + (this->Width / 2.0f),
        .center_left = position_layout.center - position_layout.top_center,
        .center_right = position_layout.center + position_layout.top_center
    };
}

BGUI::MainWindow::~MainWindow(){
    CloseWindow();
    onWindowClose();
}

BGUI::Sound_effect::Sound_effect(const char* sound_path)
{
    if (Sound_effect::isAudioDeviceReady == false) {
        InitAudioDevice();
        Sound_effect::isAudioDeviceReady = true;
    }
    this->sound = LoadSound(sound_path);
}

bool BGUI::Sound_effect::isAudioDeviceReady = false;

BGUI::Sound_effect::~Sound_effect()
{
    UnloadSound(this->sound);
    Sound_effect::isAudioDeviceReady = false;
}

void BGUI::Sound_effect::play()
{
    PlaySound(this->sound);
}

void BGUI::MainWindow::onWindowClose()
{
    LOG.log_Text("Window is Closed");
}

void BGUI::MainWindow::init_grid_space(int rows, int cols)
{
    INTERNAL_RENDERING_INSTANCE.Init_GUI_Grid_Space(rows,cols);
}

void BGUI::MainWindow::canvas(std::function<void()> drawingLogic)
{
    while(!WindowShouldClose()){
        if (isTickSet) update_tick_clock();
        if (this->logic != nullptr) this->logic();
        BeginDrawing();
        if (INTERNAL_RENDERING_INSTANCE.BLUEPRINT_STACK_SIZE > 0) DRAWGUI();
        else ClearBackground(BLACK);
        if (drawingLogic != nullptr) drawingLogic();
        EndDrawing();
    }
}

void BGUI::MainWindow::checkGUI_batched_status()
{
    this->isGUI_BATCHED_task = {
        .fn = [this]{
            if (!this->isGUI_BATCHED) {
                INTERNAL_RENDERING_INSTANCE.BATCH_RENDER_BLUEPRINTS();
                this->isGUI_BATCHED = true;
            }},
    };
    INTERNAL_RENDERING_INSTANCE.push_to_task_stack(this->isGUI_BATCHED_task);
}

void BGUI::MainWindow::set_tps(int tps)
{
    this->max_tick = tps;
    max_frame_time = 1.0f / max_tick;
    this->isTickSet = true;
}

void BGUI::MainWindow::update_tick_clock()
{
    this->c_frame_time += GetFrameTime();
    if (this->c_frame_time >= max_frame_time){
        if (this->activeTick++ == max_tick) this->activeTick = 0;
        this->accumulated_tick++;
        this->c_frame_time -= max_frame_time;
    }
}

void BGUI::MainWindow::update_perTick(const int& tickSpeed, std::function<void()> logic)
{
    if (this->accumulated_tick % tickSpeed == 0) logic();
}


//----------UI-----------//
// will be moved to different impl file

BGUI::Button::Button(Component_prop_t Properties)
{
    this->properties = Properties;
    this->properties.assemble_Rect();
    this->default_BluePrint.componentProp = &this->properties;
    this->default_BluePrint.renderBody.set_component_prop(this->properties);
    this->default_BluePrint.eventBody = &this->eventBody;
    INTERNAL_RENDERING_INSTANCE.PUSH_TO_BLUEPRINT_STACK(default_BluePrint); 
}

void BGUI::BluePrint_t::signal_event(BGUI::Signal signal){
    if (eventBody != nullptr) {
        switch(signal){
            case Signal::HOVERED: 
                LOG.log_Text("Hovered!!"); 
                this->eventBody->isOnHover_Triggered = true;
            break;//this->event.onHover(); break;
            case Signal::CLICKED: 
                this->eventBody->onClick();
                this->eventBody->isOnClick_Triggered = true;
            break;//this->event.onClick(); break;
            case Signal::UN_HOVERED:
                LOG.log_Text("un_Hovered!! "); 
                this->eventBody->isOnHover_Triggered = false;
            break;
            case Signal::UN_CLICKED:
                LOG.log_Text("un_Clicked!!"); 
                this->eventBody->isOnClick_Triggered = false;
            break;
        }
    }
}

bool BGUI::BluePrint_t::is_event_triggered(Signal signal)
{
    if (eventBody == nullptr) return false;
    switch (signal){
    case Signal::HOVERED:
        return this->eventBody->isOnHover_Triggered;
    case Signal::CLICKED:
        return this->eventBody->isOnClick_Triggered;
    default: LOG.log_Text("INVALID SIGNAL STATE TO TRIGGER"); return false;
    }
}

BGUI::Grid::Grid(grid_prop_t grid_properties)
{
    float cellWidth = grid_properties.layout_width / grid_properties.rows;
    float cellHeight = grid_properties.layout_height / grid_properties.columns;

    //calculate cells
    for (int i = 0; i < grid_properties.columns;i++){
        for (int j = 0;j< grid_properties.rows;j++){
            this->gridBuffer.push_back({j * cellWidth,i * cellHeight, cellWidth , cellHeight});  
        }
    }

    //draw on texture
    gridRenderTexture = LoadRenderTexture(grid_properties.layout_width,grid_properties.layout_height);
    BeginTextureMode(gridRenderTexture);
    ClearBackground(BLANK);
    for (int i = 0;i <grid_properties.rows*grid_properties.columns;i++){
        DrawRectangleLinesEx(gridBuffer[i],grid_properties.border_line_thickness,WHITE);
    }
    EndTextureMode();
}

void BGUI::Grid::drawGridTexture(const Vector2& pos){
    DrawTextureV(gridRenderTexture.texture,pos,WHITE);
}

BGUI::Grid::~Grid(){  
    UnloadRenderTexture(gridRenderTexture);
}

bool BGUI::compare_Colors(Color a ,Comparison comp, Color b){
    switch (comp){
        case Comparison::isEqual:
            return (a.a == b.a && a.b == b.b && a.g == b.g && a.r == b.r);
        break;
        default: return false; break;
    };
}

bool BGUI::compareVector2(Vector2 vec1, Comparison comp, Vector2 vec2)
{
    switch(comp){
        case Comparison::isEqual:
            return (vec1.x == vec2.x && vec2.y == vec1.y);
        break;
        case Comparison::isLesser:
            return (vec1.x < vec2.x && vec1.y < vec2.y);
        break;
        case Comparison::isGreater:
            return (vec1.x > vec2.x && vec1.y > vec2.y);
        break;
        default: return false;
    }
}
