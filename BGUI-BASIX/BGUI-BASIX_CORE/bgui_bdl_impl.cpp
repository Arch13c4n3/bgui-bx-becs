#include "basicGUI.hpp"

BGUI::BDL::BDL(const char* bdl_file_path, bool editorMode)
{
    enum class syntax : uint8_t {
        none = 0u,
        define , background, font_size,begin_body,end_body, begin_segment,
        end_segment, container,number,
        //colors
        blank = 10u, red,white,blue,orange,purple,skyblue,brown, yellow, black,
        // shapes
        rectangle,
    };

    using enum syntax;

    std::unordered_map<syntax, Color> syntax_color_map = {
        {blank, BLANK},{red, RED}, {white, WHITE}, {blue,BLUE},
        {orange, ORANGE}, {purple, PURPLE}, {skyblue,SKYBLUE},
        {yellow,YELLOW}, {brown, BROWN}, {black , BLACK}
    };

    std::unordered_map<std::string, syntax> syntax_map = {
        {"#",define}, {"background",background},{"container", container}, {"font_size", font_size},
        {"{",begin_body}, {"}", end_body},{"[", begin_segment}, {"]", end_segment},
        // colors
        {"blank", blank},{"red", red}, {"white", white}, {"blue",blue},
        {"orange", orange}, {"purple", purple}, {"skyblue",skyblue},
        {"yellow", yellow}, {"brown", brown}, {"black", black},
        // shapes
        {"rectangle", rectangle}
    };

    std::vector<syntax> define_relations = {background,rectangle,container,font_size};

    std::unordered_map<syntax, std::vector<syntax>*> syntax_relations = {
        {define,&define_relations}
    };

    auto is_syntax_color = [](syntax Syntax){
        auto num = static_cast<uint8_t>(Syntax);
        if (num >= 10u && num <= 18u){
            return true;
        }
        else return false;
    };
    
    // used to keep track of used syntax
    std::vector<syntax> syntax_used_list;
    std::vector<std::string> word_stack;

    std::ifstream file(bdl_file_path);
    if (!file.is_open()) LOG.log_Text(TextFormat("error opening file: %s",bdl_file_path));

    char c;
    int newline = 0;
    std::string word = "";
    const char* debug_wbuffer;
    int word_count = 0;

    while(file.get(c))
    {    
        word += c;
        if (word ==" ") word.erase();
        else if (syntax_map.count(word)){
            debug_wbuffer = word.c_str();
            LOG.log_Text(TextFormat("pushed[%d]: (%s)",word_count, debug_wbuffer));
            word_stack.push_back(word);
            syntax_used_list.push_back(syntax_map.at(word));
            word.erase();
            word_count++;
        }

        if (word == "\n") newline++;
    }
    
    // push last word
    if (syntax_map.count(word)){
        debug_wbuffer = word.c_str();
        LOG.log_Text(TextFormat("pushed[%d]: (%s)",word_count, debug_wbuffer));
        word_stack.push_back(word);
        word.erase();
    }
    LOG.log_Text(TextFormat("new lines: %d", newline));
    
    int syntax_used_list_size = static_cast<int>(syntax_used_list.size());

    for (int i = 0;i<syntax_used_list_size; i++)
    {
        switch(syntax_used_list[i]){
            case define:
            if (i < word_count){
                int next_index1 = i+1;
                int next_index2 = i+2;
                bool isNextIndexValid = next_index1 < word_count;
                
                if (syntax_used_list[next_index1] == background){
                    if (isNextIndexValid && is_syntax_color(syntax_used_list[next_index2])){
                        this->vm.backgroundColor = syntax_color_map.at(syntax_used_list[next_index2]);
                        LOG.log_Text("background color change triggered");
                        i += 3;
                    }
                    else LOG.log_Text("[bgui::bdl] error: defined background has no valid color value assigned");
                }
                else if (syntax_used_list[next_index1] == rectangle){
                    //if (isNextIndexValid && )
                }
            }
            else LOG.log_Text("[bgui::bdl] error: invalid syntax after (#)");
            break;
            // conditional rendering / animations
            case begin_segment:
            case end_segment:
            break;
            default: LOG.log_Text("[bgui::bdl] error: invalid syntax");

        }
    }

    file.close();

    if (!editorMode) {
        INTERNAL_RENDERING_INSTANCE.push_vm_blueprint(vm);
        LOG.log_Text("INTERNAL_RENDERING_INSTANCE: virtual model blueprint has been pushed to IRS");
    }
    else {
        EditorLayer::push_vmbp_to_editorLayer(vm);
    }

}