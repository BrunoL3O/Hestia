#include "../include/hestiaUtils.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <vector>


namespace hestiaUtils {

    std::vector<std::string> split_args(const std::string cmd) {
        std::vector<std::string> args;
        std::stringstream ss(cmd);
        std::string arg;
        
        while (ss >> arg) {
            args.push_back(arg);
        }
        
        return args;
    }

    // checks /bin/ and $HOME/.local/share/Hestia for binaries
    checkResult search_command(const std::string& cmd, std::string& path) {
        path = "";

        const char* home = getenv("HOME");
        std::string modules_path = std::string(home) + "/.local/share/Hestia/modules/";

        DIR* bin_directory = opendir("/bin/");
        DIR* local_directory = opendir(modules_path.c_str());

        checkResult result = checkResult::NOT_FOUND;
        

        if (bin_directory) {
            errno = 0;
            dirent* dp;
            while ((dp = readdir(bin_directory)) != NULL) {
                if (std::strcmp(dp->d_name, cmd.c_str()) == 0){
                    path = "/bin/";
                    result = checkResult::FOUND;
                    break;
                }
            } 
            if (errno == 0 && path == "")
                result = checkResult::NOT_FOUND;
            else if (errno != 0)
                result = checkResult::READ_ERROR;
            closedir(bin_directory);
        }
        if (result != checkResult::FOUND) {
            if (local_directory){
                errno = 0;
                dirent* dp;
                while ((dp = readdir(local_directory)) != NULL) {
                    if (std::strcmp(dp->d_name, cmd.c_str()) == 0){
                        path = modules_path;
                        result = checkResult::FOUND;
                        break;
                    }
                } 
                if (errno == 0 && path == "")
                    result = checkResult::NOT_FOUND;
                else if (errno != 0)
                    result = checkResult::READ_ERROR;
                
                closedir(local_directory);
            }
        }
        return result;
    }

    void handle_command(std::string& cmd, std::vector<char*> argv, bool wait_for) {
        std::string path;

        // if followed by &, we remove it and mark as background execution
        bool background = false;
        if (!cmd.empty() && cmd.back() == '&') {
            background = true;
            cmd.pop_back(); // remove '&'
        }

        checkResult result = search_command(cmd, path);
        path = path.append(cmd);

        // std::cout << "Argv length : " << argv.size() << std::endl;
        // std::cout << "Args: " << std::endl;

        // for (char* p : argv)
        // {
        //     if (p == nullptr)
        //         break;
        //     printf("%s\n", p);
        // }


        // std::cout << std::format("Command: {}. Background: {}\n", path, background);

        switch (result) {
            case checkResult::FOUND:
                launch_command(path, argv, wait_for, background);
                break;
            case checkResult::NOT_FOUND:
                std::cerr << std::format("Command {} not found.\n", cmd);
                break;
            case checkResult::READ_ERROR:
                std::cerr << "Read error. Do you have permissions to look in /bin/ and ~/.local/Hestia/modules/?\n";
                break;
        }
    }

    ///
    /// cmd = path to the command
    /// argv = the arguments for execv
    ///
    /// moreover, this launches a command. should this be a normal script for launching apps with args?
    ///
    /// TODO : turn this into a normal module - 06.08.2026 ( 1:22 AM )
    ///
    void launch_command(const std::string& cmd, std::vector<char*> argv, bool wait_for, bool background) {
        /*std::cout << "Argv length : " << argv.size() << std::endl;
        std::cout << "Args: " << std::endl;

        for (char* p : argv)
        {
            if (p == nullptr)
                break;
            printf("%s\n", p);
        }*/                
        
        pid_t pid = fork();

        if (background == true)
            wait_for = false;

        if (pid == 0) {
            if (background == true){
                
                int fd = open("/dev/null", O_WRONLY);
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);

                close(fd);

                //execl(cmd.c_str(), cmd.c_str(),  (char*)nullptr);
                execv(cmd.c_str(), argv.data());
            }
            
            else
                execv(cmd.c_str(), argv.data());
                //execl(cmd.c_str(), cmd.c_str(), (char*)nullptr);
            exit(1);
        }
        else if (pid > 0) {
            if (wait_for) {
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status)) {
                    int code = WEXITSTATUS(status);
                    if (code != 0)
                    {
                        std::cerr << std::format("Command '{}' exited with exit code {}.\n", cmd, code);
                    }
                }
            }
        }
    }

    
    void process_command(const std::string& voice_text) {
        std::string command = voice_text;

        std::vector<std::string> args_list = split_args(command);
        std::vector<char*> argv;

        for (auto& arg : args_list)
            argv.push_back(arg.data());

        argv.push_back(nullptr);  

        std::string cmd = argv.front();
        
        handle_command(cmd, argv, true);

    }

    std::string get_voice_input() {
        std::string input;
        std::cout << "Hestia > ";
        std::getline(std::cin, input);
        std::vector<std::string> strlist;
        return input;
    }
}