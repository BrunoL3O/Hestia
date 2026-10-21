#include "../include/hestiaAudio.hpp"
#include "miniaudio.h"
#include <cstdio>

namespace hestiaAudio {
    static ma_device g_device;
    static AudioCallback g_callback;
    static bool g_initialized = false;

    void data_callback(ma_device *pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
        if (g_callback)
            g_callback(static_cast<const float*>(pInput), frameCount);

        (void)pDevice;
        (void)pOutput;
    }

    bool initialize() {
        ma_device_config cfg = ma_device_config_init(ma_device_type_capture);
        cfg.capture.format = ma_format_f32;
        cfg.capture.channels = 1;

        // Human speech tops up at about 8KHz. 
        // Did you know you need the sample rate to be double the frequency of your audio, to get it all?
        cfg.sampleRate = 16000;
        cfg.dataCallback = data_callback;
    
        if (ma_device_init(NULL, &cfg, &g_device) != MA_SUCCESS) {
            printf("[Audio] Failed to initialize capture device.\n");
            return false;
        }

        g_initialized = true;
        return true;
    }

    // We do this to easily change listening for a wake-word to recording for Whisper
    void set_callback(AudioCallback callback) {
        g_callback = callback;
    }

    void start() { 
        if (g_initialized && ma_device_start(&g_device) != MA_SUCCESS)
            printf("[Audio] Failed to start device.\n");
    }

    void shutdown() {
        if (g_initialized) {
            ma_device_uninit(&g_device);
            g_initialized = false;
        }
    }



}