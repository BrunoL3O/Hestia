#include <functional>

namespace hestiaAudio {
    using AudioCallback = std::function<void(const float* samples, size_t n)>;
    bool initialize();
    void set_callback(AudioCallback callback);
    void start();
    void shutdown();

} // namespace hestiaAudio