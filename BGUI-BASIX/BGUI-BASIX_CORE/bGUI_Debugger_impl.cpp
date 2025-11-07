#include "basicGUI.hpp"
// contains impl essential for debugging, like text file logger etc..

std::string BGUI::getCurrentTime(){
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm *localTime = std::localtime(&now);
    std::ostringstream ossBuffer;
    ossBuffer << std::put_time(localTime,"[%H:%M:%S]");
    return ossBuffer.str();
}

const char* BGUI::formatText_RectDimensions(Rectangle rect)
{
   return TextFormat("rect: %f,%f,%f,%f", rect.x,rect.y,rect.width,rect.height);
}

BGUI::Text_File_Logger::Text_File_Logger()
{
    this->clearFile();
    const char *boolstr = alwaysClearFile ? "TRUE" : "FALSE";
    log_Text("DEBUG MODE STARTED , FILE LOGGING IS ON");
    log_Text(TextFormat("Always Clear File: %s",boolstr));
    this->set_doTimeKeeping(false);
}

void BGUI::Text_File_Logger::log_Text(const char *text)
{ 
    // try open/create
    {
        std::ofstream ofs(fileName,std::ios::app);
        if (!ofs) {
            std::cerr << "failed to create / open file for writing : "<<fileName<<"\n";
        }
        ofs.close();
    }
    
    std::fstream file(fileName, std::ios::app);

    if (!file){
        std::cerr << "file failed to open for read and write mode : "<<fileName<<"\n";
    }
    else {
        if (doTimeKeeping) file << getCurrentTime() << text << "\n";
        else file << text <<"\n";
        file.close();
    }
}

void BGUI::Text_File_Logger::log_Text(std::string str_text)
{
    const char* c = str_text.c_str();
    this->log_Text(c);
}

void BGUI::Text_File_Logger::clearFile()
{
    std::ofstream file(fileName,std::ios::out | std::ios::trunc);
    file.close();
}

void BGUI::Text_File_Logger::set_AlwaysClearFile(bool t)
{
    this->alwaysClearFile = t;
    if(alwaysClearFile) clearFile();
}

void BGUI::Text_File_Logger::set_doTimeKeeping(bool t)
{
    this->doTimeKeeping = t;
}


