#include "include/hestiaUtils.hpp"
#include "include/hestiaIntent.hpp"
#include <iostream>
#include <string>
#include <fstream>

int main() {
    std::cout << "Initializing Hestia..." << std::endl;

    // Initialize intent system with CrispEmbed. Temporarily hardcoded for funsies
    std::cout << "[Main] Initializing intent system..." << std::endl;
    std::string model_path = "models/multilingual-MiniLM-L12-Q4.gguf";
    std::string mapping_path = "models/intent_mapping.json";

    if (!hestiaIntent::initialize(model_path, mapping_path)) {
        std::cerr << "Failed to initialize intent system!" << std::endl;
        return 1;
    }

    std::cout << "\nHestia ready! Type 'exit' to quit.\n" << std::endl;

    while (true) {
        std::string voice_input = hestiaUtils::get_voice_input();

        if (voice_input == "exit" || voice_input == "quit") {
            break;
        }

        if (!voice_input.empty()) {
            // Predict intent
            int intent_id = hestiaIntent::predict_intent(voice_input);

            if (intent_id >= 0) {
                std::string intent_name = hestiaIntent::get_intent_name(intent_id);
                std::cout << "Intent : " << intent_name << " (ID: " << intent_id << ")" << std::endl;

                // Execute the corresponding script
                std::string script_path = "scripts/modules/" + intent_name + ".sh";
                std::ifstream script_file(script_path);
                if (script_file.good()) {
                    script_file.close();
                    std::string cmd = script_path;
                    std::cout << "Executing : " << cmd << std::endl;
                    system(cmd.c_str());
                } else {
                    std::cout << "Script not found. Executing as command..." << std::endl;
                    hestiaUtils::process_command(voice_input);
                }
            } else {
                std::cout << "Could not determine intent. Executing as command..." << std::endl;
                hestiaUtils::process_command(voice_input);
            }
        }
    }

    return 0;
}