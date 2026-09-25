#!/bin/bash

# Hestia Setup Script
# Builds CrispEmbed and whisper.cpp. Others will follow at later versions
# See the end for model instructions.

set -e

# -----------------------------
# Config
# -----------------------------
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXTERNAL_DIR="$PROJECT_ROOT/external"
MODELS_DIR="$PROJECT_ROOT/models"
LIB_DIR="$PROJECT_ROOT/lib"

# -----------------------------
# miniaudio
# -----------------------------
download_miniaudio() {
    log "Downloading miniaudio..."
    cd "$EXTERNAL_DIR"

    if [ ! -d "miniaudio" ]; then
        mkdir miniaudio-repo 
        git clone --depth 1 https://github.com/mackron/miniaudio.git miniaudio-repo
        mkdir miniaudio
        cp miniaudio-repo/miniaudio.c miniaudio/
        cp miniaudio-repo/miniaudio.h miniaudio/
        rm -rf miniaudio-repo
        log "Downloaded miniaudio.c and miniaudio.h."
    else
        log "miniaudio present, skipping"
    fi

    
}

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log()  { echo -e "${GREEN}[Setup]${NC} $1"; }
warn() { echo -e "${YELLOW}[Setup]${NC} $1"; }
err()  { echo -e "${RED}[Setup]${NC} $1" >&2; }

# -----------------------------
# Check required tools
# -----------------------------
log "Checking required tools..."
for tool in git cmake make curl; do
    if ! command -v "$tool" &>/dev/null; then
        err "$tool is not installed. Please install it with your favorite package manager."
        exit 1
    fi
done
log "All tools present."

# -----------------------------
# Create directories
# -----------------------------
mkdir -p "$EXTERNAL_DIR" "$MODELS_DIR" "$LIB_DIR"

# -----------------------------
# Build CrispEmbed (intent router)
# -----------------------------
build_crispembed() {
    log "Building CrispEmbed..."
    cd "$EXTERNAL_DIR"

    if [ ! -d "CrispEmbed" ]; then
        git clone --recursive https://github.com/CrispStrobe/CrispEmbed.git
    else
        log "CrispEmbed already cloned, updating..."
        cd CrispEmbed
        git pull
        git submodule update --init --recursive
        cd ..
    fi

    cd CrispEmbed
    cmake -S . -B build \
        -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
        -DGGML_CUDA=OFF \
        -DGGML_VULKAN=OFF
    cmake --build build -j "$(nproc)"

    cp build/libcrispembed-static.a "$LIB_DIR/libcrispembed.a"
    log "CrispEmbed built and copied to lib/."
}

# -----------------------------
# Build whisper.cpp (speech-to-text)
# -----------------------------
build_whisper() {
    log "Building whisper.cpp..."
    cd "$EXTERNAL_DIR"

    if [ ! -d "whisper.cpp" ]; then
        git clone https://github.com/ggml-org/whisper.cpp.git
    else
        log "whisper.cpp already cloned, updating..."
        cd whisper.cpp
        git pull
        cd ..
    fi

    cd whisper.cpp
    cmake -B build \
        -DBUILD_SHARED_LIBS=OFF \
        -DWHISPER_BUILD_TESTS=OFF \
        -DWHISPER_BUILD_EXAMPLES=OFF
    cmake --build build -j "$(nproc)" --config Release

    cp build/src/libwhisper.a "$LIB_DIR/libwhisper.a" 2>/dev/null || \
    cp build/libwhisper.a "$LIB_DIR/libwhisper.a"
    log "whisper.cpp built and copied to lib/."
}

# -----------------------------
# Create intent_mapping.json
# -----------------------------
create_intent_mapping() {
    log "Creating intent_mapping.json..."
    cd "$MODELS_DIR"

    if [ -f "intent_mapping.json" ]; then
        log "intent_mapping.json already exists, skipping..."
        return
    fi

    cat > intent_mapping.json << 'EOF'
{
    "intents": ["get_weather", "set_timer", "play_music", "list_files", "open_app", "conversation"],
    "examples": {
        "get_weather": ["what is the weather like", "tell me the weather", "is it raining", "should i wear a jacket", "what's the temperature", "weather forecast"],
        "set_timer": ["set a timer", "remind me in", "set an alarm", "wake me up", "timer for", "countdown"],
        "play_music": ["play some music", "play jazz", "play songs", "start playing", "turn on music"],
        "list_files": ["list files", "show me files", "what files are in", "list all documents"],
        "open_app": ["open firefox", "launch chrome", "start spotify", "open the terminal"],
        "conversation": ["tell me a joke", "how are you", "tell me something interesting", "who are you"]
    },
    "intent_to_id": {
        "get_weather": 0, "set_timer": 1, "play_music": 2,
        "list_files": 3, "open_app": 4, "conversation": 5
    },
    "id_to_intent": {
        "0": "get_weather", "1": "set_timer", "2": "play_music",
        "3": "list_files", "4": "open_app", "5": "conversation"
    }
}
EOF
    log "intent_mapping.json created."
}

# -----------------------------
# Run
# -----------------------------
log "Starting Hestia setup..."
download_miniaudio
build_crispembed
build_whisper
create_intent_mapping
log "Setup complete!"

# -----------------------------
# Where models go
# -----------------------------
echo ""
log "=================================================="
log "  MODEL PLACEMENT INSTRUCTIONS"
log "=================================================="
echo ""
echo -e "${YELLOW}Please download the models from appropriate sources based on your computer's hardware."
echo -e "${RED}Remember to change the model's path in main.cpp before building, otherwise it will not work."
echo -e "${YELLOW}Place all model files in:${NC} $MODELS_DIR/"
echo ""
echo "Required models:"
echo ""
echo -e "  ${GREEN}- Intent Router (CrispEmbed)${NC}"
echo ""
echo -e "  ${GREEN}- Speech-to-Text (whisper.cpp) (TBA)${NC}"
echo ""
echo -e "  ${GREEN}- LLM (llama.cpp) (TBA)${NC}"
echo ""
echo -e "  ${GREEN}- Text-to-Speech (Piper) (TBA)${NC}"
echo ""
log "=================================================="
echo ""
log "Next: cmake -B build && cmake --build build -j\$(nproc)"