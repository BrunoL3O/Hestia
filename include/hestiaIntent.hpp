#pragma once

#include <string>
#include <vector>
#include <map>

namespace hestiaIntent {

    struct IntentMapping {
        std::vector<std::string> intents;
        std::map<std::string, int> intent_to_id;
        std::map<int, std::string> id_to_intent;
    };

    bool initialize(const std::string& model_path, const std::string& mapping_path);
    int predict_intent(const std::string& text);
    std::string get_intent_name(int intent_id);
    std::vector<std::string> get_intents();
    bool is_ready();

} // namespace hestiaIntent