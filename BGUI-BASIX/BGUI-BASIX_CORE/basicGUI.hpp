///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                   minimal UI library on top of Raylib 5.5
// version: 0!
// designed to be extreme light weight and blazing fast 
// ** uses Tree Architecture for Internal Tracking of components
// ** mimics retained mode systems
// by archie <3
// 
// Contains WRAPPERS for handling auto cleanup/unloading of textures and etc.. used in the libraries
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                          HOW TO USE & HOW IT WORKS:
//  > Initilize / Describe GUI ahead of time using constructors
//      ^ NEVER INITIALIZE A COMPONENT AT GAME LOOP TO AVOID PERFORMANCE ISSUES
//      ^ everything will be rendered / managed by DRAWGUI 
//      ^ which will be managed internally inside .running member of the window instance
//  > use State logic (Trigger,Toggle etc..) for dynamic Components at Runtime
//      ^ Triggers -> Re-Rendering of the specific Components
//      ^ Toggles -> Toggle Specified Logic State of component
//  > ' <your window instance>.DRAWGUI() ' only 1 Draw Call inside internal game loop for EVERYTHING! 
//      ^ DRAWGUI() is a member function of your window instance that contains a pre-rendered batched texture
//
//  Draw Logic:
//  > All Components (PRE-DEFINED & USER-DEFINED) are Pre-Rendered in GPU memory (CACHED),
//      - Managed Using Internal 'GUI TREE',
//      - automatically reduce Draw calls whenever possible using flags and state machines
//  > components not in a scene will always draw unless remove() is triggered
//      - components in a scene will be managed by that scene
//
//                          (!) LIMITATIONS (!):
// > Cannot Handle resizable windows yet
// > (!) ON GOING ISSUES with BATCHING PRE RENDERS, Currently using a work around by manual rendering on IRS 'Push_to_RENDER_STACK' method
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef BASICGUI_HPP
#define BASICGUI_HPP

#include <raylib.h>
#include <cstdint> 
#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <fstream>
#include <chrono>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <map>
#include <ranges>

//not implemented yet
// #ifdef DEBUG
//     #define log_text(text) BGUI::LOG.log_text(text)
// #else 
//     #define log_text(text) ((void)0)
// #endif

namespace BGUI {
    class Button;
    class Component_prop_t;
    class container_prop_t;
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Architecture Design:  
    // All elements are tracked in an internally tree structure
    // Prevent bugs at compile time and handle debugging
    // Clean, Intuitive and Extensive style
    // All Components and their different states will be pre-rendered with Internal Drawing Instance
    //
    // TODO:
    // (1) Organize implementation files > move UI components to different file
    // (2) [-OPTIMIZE RENDERING-] Blueprints with similar properties should only be rendered one time 
    //  ^ could be on a rendered atlas and map rectangle coords to draw it to BATCHED_RENDERED_BLUEPRINTS (promising)
    //  ^ could be implemented thru using the id to check for similar blueprints (UNSURE)
    // (3) Overlay State Logic: Currently Overlays state textures (RENDERED_BLUEPRINTS_OVERLAY_STACK) on top of IRS' BATCHED RENDERED BLUEPRINTS
    //  ^ for simplicity
    //  ^ WILL BE CHANGED/UPDATED by batching the RENDERED_BLUEPRINTS_OVERLAY_STACK to one big rendertexture atlas 
    //      and map rectangle coords accordingly (Minecraft texture atlas style)
    // (4) Fix Components not rendering their hover bodies in overlay 
    // (5) physics Collision detection
    // (6) pre renderer
    // (7) finish task manager in irs and drawgui
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Hours:minutes:seconds in local time zone
    std::string getCurrentTime();
    //returns rect dimensions in single string, uses const char* for portability
    const char* formatText_RectDimensions(Rectangle rect);

    enum class Signal {
        CLICKED,HOVERED, UN_CLICKED, UN_HOVERED
    };

    // basic ui layout positions - refers to the corners, edges and centers
    struct Position_layout_t {
        float top_left, 
            top_right, 
            top_center,
            bottom_left,
            bottom_right,
            bottom_center,
            center,
            center_left,
            center_right;
    };

    // comparison operators
    enum class Comparison : uint8_t {
        isGreater,isLesser,isEqual, isNotEqual,
        // for Vector2 comparison - determining only one point
        isGreater_OR, isLesser_OR, isEqual_OR
    };

    bool compare_Colors(Color a ,Comparison comp, Color b);
    bool compareVector2(Vector2 vec1, Comparison comp, Vector2 vec2);
    //bool isShapeInShape()
    
    // Rectangle with pointer members
    struct Rectangle_ptr_members_t {
        Vector2* position;
        float* width;
        float* height;
    };

    //for logging debug texts to a text file
    class Text_File_Logger {
        public:
        Text_File_Logger();
        const char* fileName = "BGUI_LOGS.txt";
        void log_Text(const char *text);
        void log_Text(std::string str_text);
        void clearFile();
        bool alwaysClearFile = false;
        void set_AlwaysClearFile(bool t);
        void set_doTimeKeeping(bool t);
        bool doLogging = true;
        bool doTimeKeeping = true;
        
    };

    inline Text_File_Logger LOG;

    // buggy
    class Timer {
        public:
            Timer() { Reset(); }
            void Reset() { m_Start = std::chrono::high_resolution_clock::now(); }
            float Elapsed() const { return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f; }
            float ElapsedMillis() const { return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f; }
        private:
            std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
        };
    
    class ScopedTimer {
        public:
            ScopedTimer(const char* name) : m_Name(name) {}
            ~ScopedTimer()
            {
                LOG.log_Text(TextFormat("%s - %f ms", m_Name, m_Timer.ElapsedMillis()));
            }
        private:
            Timer m_Timer;
            const char* m_Name;
    };
    
    // screen size defaults
    constexpr int SCREEN_WIDTH = 1000, SCREEN_HEIGHT = 1000;
    constexpr Vector2 zeroVector2 = {0.0f,0.0f}; 
    constexpr Rectangle zeroRectangle = {0.0f,0.0f,0.0f,0.0f};
    inline Position_layout_t position_layout;

    // Surface Properties Container for pre-rendered textures
    // NOTE: height should be negative to make sure its flippped correctly
    struct TextureSurface_properties{
        int width = 0;
        int height = 0;
        Rectangle source;
        // drawing position , can scale the texture's size
        Rectangle destination;
        float rotation = 0.0f;
        Vector2 origin = zeroVector2;
    };

    //component properties
    struct Component_prop_t {
        Vector2 position;
        float width;
        float height;
        // where the assembled rect is stored
        Rectangle rect;
        // assembles the rect member using other members
        void assemble_Rect(){
            this->rect = {this->position.x,this->position.y,this->width,this->height};
        }
    }; 
    // contains draw functions of different states to be pre-rendered by IRS
    struct RenderBody_t {
        Rectangle m_rect;
        Component_prop_t *componentProp;

        int signature = 0;

        void set_component_prop(Component_prop_t &comp){
            this->componentProp = &comp;
            this->componentProp->assemble_Rect();
        }

        Rectangle getCurrentRect() const {
            if (componentProp != nullptr){
                this->componentProp->assemble_Rect();
                return this->componentProp->rect;
            }
            return {0,0,0,0};
        }

        std::function<void()> Static = [this]{
            this->m_rect = this->getCurrentRect();
            DrawRectangleRec(this->m_rect,WHITE);
            DrawRectangleLinesEx(this->m_rect,5,GRAY);
        };
        std::function<void()> Hovered = [this]{
            this->m_rect = this->getCurrentRect();
            DrawRectangleRec(this->m_rect,GRAY);
            DrawRectangleLinesEx(this->m_rect,5,GREEN);
        };
        std::function<void()> Clicked = [this]{
            this->m_rect = this->getCurrentRect();
            DrawRectangleRec(this->m_rect,BLUE);
        };
    };

    //custom event handling
    struct eventBody_t {
        int signature = 0;

        std::function<void()> onClick = []{ LOG.log_Text("buttonClicked! from class"); };

        std::function<void()> onHover = []{ LOG.log_Text("Hovered! from class"); };

        bool isOnClick_Triggered = false;
        bool isOnHover_Triggered = false;
    };

    // properties of a interactable render body , essentials for a component to be passed to IRS
    // universal blue print for components to interact with the IRS and MainWindow
    struct BluePrint_t {
        RenderBody_t renderBody;
        eventBody_t* eventBody;
        Component_prop_t* componentProp;
        int id;

        BluePrint_t(){      
            this->id = this->BluePrint_Index;
            //this->assembleRect();
            BluePrint_Index++;
        }

        void signal_event(Signal signal);

        // checks if the specified signal event has been sent once 
        bool is_event_triggered(Signal signal);

        private:
        inline static int BluePrint_Index = 0;
    };

    // handles automatic cleanup/unloading and pre-rendering of textures.
    //rendertexture2d wrapper with ease of use modifications
    //Designed to be loop-safe , Should only load texture once before game loop, Unless Explicitly reloaded
    class PreRender2d {

    };

    // virtual model blueprint
    // handled by parser - sent to IRS to be constructed
    struct vm_blueprint_t {
        std::vector<Color> color;
        std::vector<Vector2> pos;

        std::vector<std::function<void()>> fn_stack;
        Color backgroundColor = BLANK;
    };

    // for task management 
    struct task_t {
        std::function<void()> fn = nullptr;
        bool isActive = false;
        bool do_once = false;
    };

    // Manages Rendering Architecture
    // Works but Needs to be Improved/updated Since the current design implementation uses Work-Arounds
    class INTERNAL_RENDERING_SYSTEM {
        public:
        INTERNAL_RENDERING_SYSTEM();
        // responsible for unloading render textures 
        ~INTERNAL_RENDERING_SYSTEM();
        // stores blue prints to be rendered
        std::vector<BluePrint_t*> BLUEPRINT_STACK;
        int BLUEPRINT_STACK_SIZE;
        int Current_Render_Index = 0;
        void PUSH_TO_BLUEPRINT_STACK(BluePrint_t& bluePrint);
        // the final texture where all pre-renders are batched to 1 texture
        RenderTexture2D BATCHED_RENDERED_BLUEPRINTS;
        // stores the batched overlays for blueprints states
        // Currently Only has Hovered State
        std::vector<RenderTexture2D> RENDERED_BLUEPRINTS_OVERLAY_STACK;
        size_t RENDERED_BLUEPRINTS_OVERLAY_STACK_SIZE;
        // draw pre rendered
        void BATCH_RENDER_BLUEPRINTS();
        Vector2 mousePosition;

        //---- parser interfacing ------

        vm_blueprint_t* m_vm;
        //constructs the virtual model representation
        void push_vm_blueprint(vm_blueprint_t& vm);

        //----Grid Space Hashing-----

        float cellWidth,cellHeight;
        int Rows,Cols;
        int GUI_GRID_SIZE;
        BluePrint_t* blueprintBuffer;
        // for spatial Hashing of GUI interactive components relative to mouse's position index
        // will be indexed by mouse grid position
        // inner vector will contain id's of blue prints 
        std::vector<std::vector<int>> Grid_Id_Index_Stack;
        // stores the current id size of the Grid_Id_Index_Stack of the cell hovered
        int current_id_stack_size_Buffer;
        //stores cell Dimensions from Init_GUI_Grid_space()
        std::vector<Rectangle> cellRect_stack;
        // Creates the Grid Space for the GUI
        // used for Spatial Hashing of the mouse's position
        void Init_GUI_Grid_Space(int rows, int cols);
        // stores the grid index of current mouse position
        int mouse_Grid_Position_Index;
        // stores previous mouse_Grid_Position_Index
        int prevMGPI;
        // determines the position of a point in the GUI grid space
        // returns the index position
        int Get_Point_Grid_Position( Vector2 point);
        int cached_Grid_Point = 0;
        // draws the overlays for displaying different states
        // responsible for determining which component to draw in the stack 
        // heavily optimized by only updating checks for cell access changes and mouse movement
        // event triggers only happen once per component access
        void Overlay_Rendered_Texture();
        // Determins and Hashes the rect to GUI Grid cells Occupied
        void HASH_BLUEPRINTS_TO_GRID();
        bool isBLUEPRINTS_HASHED = false;
        // for reducing checks at runtime
        Vector2 mouseDelta;
        bool isMouseHovering_Component = false;

        // ---- container -----

        std::vector<container_prop_t*> container_prop_stack;
        void push_to_container_stack(container_prop_t *container);

        // -----task manager-----
        // for efficiently managing task load in DRAWGUI
        std::vector<task_t*> activeTask_stack;
        std::vector<task_t*> inActiveTask_stack;
        void push_to_task_stack(task_t& stack);
    };

    // a global IRS instance for the BGUI to interface with
    inline INTERNAL_RENDERING_SYSTEM INTERNAL_RENDERING_INSTANCE;
    
    // Thin Wrapper for window initializing & handling 
    class MainWindow {
        public:
        // sets window properties, has defaults
        // initializes window
        MainWindow(int width = BGUI::SCREEN_WIDTH,int height = BGUI::SCREEN_HEIGHT,const char *title = "UNTITLED WINDOW");
        ~MainWindow();
        
        // responsible for drawing the gui
        void DRAWGUI();
        bool isGUI_BATCHED = false;
        
        TextureSurface_properties GUI_TEXTURE_SURFACE_PROPERTIES;

        void set_WindowSize(int width,int height);
        float Width = 0 ,Height = 0;

        void set_WindowTitle(const char *title);
        const char* Title;

        void set_TargetFPS(int fps = 60);
        int Fps = 60;

        // sets the values for basic ui layout
        void set_gui_layout_defaults();

        // triggers when window is closed
        void onWindowClose();   

        // creates the grid space, necessary for spatial hashing of objects
        void init_grid_space(int rows = 10, int cols = 10);

        // manages game loop logic and drawing
        std::function<void()> logic = nullptr;
        void canvas(std::function<void()> drawingLogic);

        task_t isGUI_BATCHED_task;
        void checkGUI_batched_status();

        // --- tick system ---
        // sets the ticks per second and starts a tick clock 
        // ticks are tracked using 'activeTick' 
        // uses accumulated_tick for update checks
        void set_tps(int tps);
        // stores current tick 
        int activeTick = 0;
        int max_tick;
        int accumulated_tick = 0;
        // stores current frame time
        float c_frame_time = 0.0f;
        float max_frame_time;
        bool isTickSet = false;
        void update_tick_clock();
        // logic that updates per specified tick
        void update_perTick(const int& tickSpeed, std::function<void()> logic);
        //void interpolate_position(Vector2& start, Vector2 end, int tick);     
    };

    class Sound_effect {
        public:
        Sound_effect(const char *sound_path);
        ~Sound_effect();
        void play();
        private:
        Sound sound;
        static bool isAudioDeviceReady;
    };

    // pressable interactive component
    class Button {
        public:
        Button(Component_prop_t properties); // Creates blueprint using properties and sends to IRS
        Component_prop_t properties;
        BluePrint_t default_BluePrint; 
        // custom events
        eventBody_t eventBody;
    };

    struct grid_prop_t {
        int rows, columns;
        float layout_width, layout_height; // whole surface area of the grid
        int border_line_thickness = 1;
    };

    // grid space for gui and entities
    class Grid {
        public:
        Grid(grid_prop_t grid_properties); //define calculate grid & draw on texture 
        ~Grid(); // unload textures
        std::vector<Rectangle> gridBuffer; // contains each cells rects dimensions
        void drawGridTexture(const Vector2& pos);

        private:
        RenderTexture2D gridRenderTexture;
    };
    
    struct text_prop_t {
        const char* text;
        // managed by container system
        Vector2 position = zeroVector2;
        int font_size = 0;
        Color color = BLANK;
    };

    // properties of a container to be sent to IRS
    struct container_prop_t {
        Rectangle m_rect_d; // container dimensions
        // inside the rect
        int rect_padding = 5, text_padding = 10;
        int font_size = 30;
        Color color = WHITE;
        const float m_next_posY = static_cast<float>(font_size + text_padding);
        bool isOutlineVisible = false;
        int m_text_stack_size = 0;
        std::vector<text_prop_t> text_prop_stack;
        Vector2 m_text_pos_buffer = zeroVector2;
        // used for live texts' positions
        // resets every frame by DrawGUI
        Vector2 m_text_pos_live = zeroVector2;
        float m_pos_layout = {0.0f};

        void drawText(){
            if (this->isOutlineVisible) DrawRectangleLinesEx(this->m_rect_d,1,WHITE);
            for (auto& i : text_prop_stack){
                DrawText(i.text,i.position.x,i.position.y,i.font_size,i.color);
            }
        }
    };

    //todo: positions of elements will be relative to container dimensions
    //can contain texts,
    struct container_t {
        container_t(Rectangle rect){
            send_container(rect);
        }

        // for containers that use the layout system
        // 'offset' is the offset position of the position layout
        // container_t(float posLayout, float Width, float Height, Vector2 offset = zeroVector2){
        //     switch (posLayout){

        //     }
        //     this->properties.m_pos_layout = posLayout;
        //     Rectangle rect;
        //     send_container(rect);
        // }

        void send_container(Rectangle rect){
            this->properties.m_rect_d = rect;
            this->properties.m_text_pos_buffer = (Vector2){
                this->properties.m_rect_d.x + this->properties.text_padding,
                this->properties.m_rect_d.y + this->properties.text_padding
            };

            INTERNAL_RENDERING_INSTANCE.push_to_container_stack(&properties);
        }

        container_prop_t properties;
        
        // static texts only
        // NOTE: must only be used before game loop
        void add_text(text_prop_t text_properties)
        {
            text_properties.position = this->properties.m_text_pos_buffer;
            if (compare_Colors(text_properties.color,Comparison::isEqual,BLANK)) text_properties.color = this->properties.color;
            if (text_properties.font_size == 0) text_properties.font_size = this->properties.font_size;
            this->properties.text_prop_stack.push_back(text_properties);
            this->properties.m_text_pos_buffer.y += this->properties.m_next_posY;
            this->properties.m_text_stack_size = static_cast<int>(this->properties.text_prop_stack.size());
        }

        // texts that can update its value and called on game loop
        // NOTE: must be called before drawGUI
        void live_text(const char* text)
        {    
            DrawText(
                text,
                this->properties.m_text_pos_live.x,
                this->properties.m_text_pos_live.y,
                this->properties.font_size,WHITE
            );

            this->properties.m_text_pos_live.y += this->properties.m_next_posY;
        }
    };

    // ---------------- parser -------------------

    //basic design language interface
    class BDL {
        public:
        BDL(const char *bdl_file_path, bool editorMode = false);
        private:
        vm_blueprint_t vm;
    };
}

namespace EditorLayer {
    void editor();
    // receives the vmbp (virtual model blueprint) from bdl parser
    void push_vmbp_to_editorLayer(BGUI::vm_blueprint_t& vmbp);
    inline BGUI::vm_blueprint_t* vmbp;
}

#endif