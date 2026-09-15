#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <format>
#include <filesystem>

#include <dirent.h>
#include <sys/types.h>

#include <fcntl.h>

namespace hestiaUtils {
    enum class checkResult{
        FOUND,
        NOT_FOUND,
        READ_ERROR
    };
    
    std::vector<std::string> split_args(const std::string cmd);
    checkResult search_command(const std::string& cmd, std::string& path);
    void handle_command(const std::string& cmd, std::vector<char*> argv, bool wait_for = false);
    void launch_command(const std::string& cmd, std::vector<char*> argv, bool wait_for = false, bool background = false); 
    void process_command(const std::string& voice_text);
    std::string get_voice_input(); 

    void load_modules(std::string module_folder_path);
} // namespace hestiaUtils