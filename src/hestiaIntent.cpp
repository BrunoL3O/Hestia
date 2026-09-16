#include "../include/hestiaIntent.hpp"
#include "crispembed.h"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstring>

namespace hestiaIntent {

    static bool initialized = false;
    static crispembed_context* ctx = nullptr;
    static IntentMapping mapping;
    static int call_count = 0;
    static std::vector<std::vector<float>> intent_embeddings;
    static int embedding_dim = 0;

    float cosine_similarity(const std::vector<float>& a, const std::vector<float>& b) {
        float dot = 0.0f, norm_a = 0.0f, norm_b = 0.0f;
        for (size_t i = 0; i < a.size(); ++i) {
            dot += a[i] * b[i];
            norm_a += a[i] * a[i];
            norm_b += b[i] * b[i];
        }
        return dot / (std::sqrt(norm_a) * std::sqrt(norm_b) + 1e-8f);
    }

    bool initialize(const std::string& model_path, const std::string& mapping_path) {
        if (initialized) {
            std::cout << "[Intent] Already initialized." << std::endl;
            return true;
        }

        // Load CrispEmbed model
        ctx = crispembed_init(model_path.c_str(), 4);
        if (!ctx) {
            std::cerr << "[Intent] Failed to load model: " << model_path << std::endl;
            return false;
        }

        // Get embedding dimension by encoding a random string
        int test_dim = 0;
        const float* test_emb = crispembed_encode(ctx, "test", &test_dim);
        if (!test_emb || test_dim == 0) {
            std::cerr << "[Intent] Failed to get embedding dimension" << std::endl;
            crispembed_free(ctx);
            return false;
        }
        embedding_dim = test_dim;
        std::cout << "[Intent] Embedding dimension : " << embedding_dim << std::endl;

        // Load intent mapping
        std::ifstream mapping_file(mapping_path);
        if (!mapping_file.is_open()) {
            std::cerr << "[Intent] Failed to open mapping file : " << mapping_path << std::endl;
            crispembed_free(ctx);
            return false;
        }
        
        nlohmann::json mapping_json;
        mapping_file >> mapping_json;
        
        mapping.intents = mapping_json["intents"].get<std::vector<std::string>>();
        
        for (const auto& intent : mapping.intents) {
            int id = mapping_json["intent_to_id"][intent].get<int>();
            mapping.intent_to_id[intent] = id;
            mapping.id_to_intent[id] = intent;
        }
        
        std::cout << "[Intent] Loaded " << mapping.intents.size() << " intents" << std::endl;

        // Pre-compute intent embeddings by averaging examples
        intent_embeddings.clear();
        intent_embeddings.reserve(mapping.intents.size());

        for (const auto& intent : mapping.intents) {
            std::vector<float> intent_avg(embedding_dim, 0.0f);
            int example_count = 0;

            // Check if there are examples for this intent
            if (mapping_json.contains("examples") && mapping_json["examples"].contains(intent)) {
                auto examples = mapping_json["examples"][intent];
                for (const auto& example : examples) {
                    std::string example_text = example.get<std::string>();
                    int dim_out = 0;
                    const float* emb = crispembed_encode(ctx, example_text.c_str(), &dim_out);
                    if (emb && dim_out > 0) {
                        for (int i = 0; i < dim_out; ++i) {
                            intent_avg[i] += emb[i];
                        }
                        example_count++;
                    }
                }
            }

            // Fallback: use the intent name if no examples
            if (example_count == 0) {
                std::cerr << "[Intent] No examples for '" << intent << "', using intent name" << std::endl;
                int dim_out = 0;
                const float* emb = crispembed_encode(ctx, intent.c_str(), &dim_out);
                if (emb && dim_out > 0) {
                    intent_avg.assign(emb, emb + dim_out);
                    intent_embeddings.push_back(intent_avg);
                    std::cout << "[Intent] Computed embedding for : " << intent << " (from intent name)" << std::endl;
                }
                continue;
            }

            // Average the embeddings
            for (int i = 0; i < embedding_dim; ++i) {
                intent_avg[i] /= example_count;
            }

            // Normalize the averaged embedding (L2 normalization)
            float norm = 0.0f;
            for (int i = 0; i < embedding_dim; ++i) {
                norm += intent_avg[i] * intent_avg[i];
            }
            norm = std::sqrt(norm);
            if (norm > 1e-8f) {
                for (int i = 0; i < embedding_dim; ++i) {
                    intent_avg[i] /= norm;
                }
            }

            intent_embeddings.push_back(intent_avg);
            std::cout << "[Intent] Computed embedding for: " << intent << " (averaged over " << example_count << " examples)" << std::endl;
        }

        initialized = true;
        std::cout << "[Intent] System ready." << std::endl;
        return true;
    }

    int predict_intent(const std::string& text) {
        call_count++;
        std::cout << "[Intent] Query #" << call_count << std::endl;

        if (!initialized || !ctx) {
            std::cerr << "[Intent] System not initialized." << std::endl;
            return -1;
        }

        int dim_out = 0;
        const float* emb = crispembed_encode(ctx, text.c_str(), &dim_out);
        if (!emb || dim_out == 0) {
            std::cerr << "[Intent] Failed to encode text" << std::endl;
            return -1;
        }

        std::vector<float> query_embedding(emb, emb + dim_out);

        int best_intent = -1;
        float best_score = -1.0f;

        for (size_t i = 0; i < intent_embeddings.size(); ++i) {
            float score = cosine_similarity(query_embedding, intent_embeddings[i]);
            if (score > best_score) {
                best_score = score;
                best_intent = static_cast<int>(i);
            }
        }

        if (best_intent >= 0 && best_intent < (int)mapping.intents.size()) {
            std::cout << "[Intent] Best match: " << mapping.intents[best_intent] << " (score: " << best_score << ")" << std::endl;
        }

        return best_intent;
    }

    std::string get_intent_name(int intent_id) {
        if (mapping.id_to_intent.find(intent_id) != mapping.id_to_intent.end()) {
            return mapping.id_to_intent[intent_id];
        }
        return "unknown";
    }

    std::vector<std::string> get_intents() {
        return mapping.intents;
    }

    bool is_ready() {
        return initialized;
    }

} // namespace hestiaIntent