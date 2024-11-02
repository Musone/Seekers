#pragma once

#include <string>
#include <iostream>
#include <unordered_map>
#include <utils/Common.hpp>
#include <ecs/Registry.hpp>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

class AudioSystem {
public:
    static AudioSystem& get_instance() {
        static AudioSystem instance;
        return instance;
    }

    // initialize sound system
    bool initialize() {
        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            std::cerr << "Failed to initialize SDL Audio: " << SDL_GetError() << std::endl;
            return false;
        }
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
            std::cerr << "Failed to open audio device: " << Mix_GetError() << std::endl;
            return false;
        }
        return true;
    }

    // load background music
    void load_music(const std::string& file_path) {
        background_music = Mix_LoadMUS(file_path.c_str());
        if (background_music == nullptr) {
            std::cerr << "Failed to load music: " << Mix_GetError() << std::endl;
        }
    }

    // load sfx
    void load_sound_effect(const std::string& file_path) {
        Mix_Chunk* sound_effect = Mix_LoadWAV(file_path.c_str());
        if (sound_effect == nullptr) {
            std::cerr << "Failed to load sound effect: " << Mix_GetError() << std::endl;
        }
        else {
            sound_effects[file_path] = sound_effect;
        }
    }

    // load all sound
    void load_all_sound() {
        load_music(audio_path("music.wav"));
        load_sound_effect(audio_path("footstep.wav"));
        load_sound_effect(audio_path("dodge.wav"));
        load_sound_effect(audio_path("bowshot.wav"));
        load_sound_effect(audio_path("swordslash.wav"));
    }

    // play music
    void start_music() {
        set_music_volume(4);
        play_music(-1);
    }

    // Set music volume (0 to 128)
    void set_music_volume(int volume) {
        if (volume < 0) volume = 0;
        if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
        Mix_VolumeMusic(volume);
    }

    void handle_audio_per_frame() {
        Registry& registry = Registry::get_instance();
        InputState& input_state = registry.input_state;

        handle_footstep(input_state);
    }

    // play background music
    void play_music(int loops = -1) {
        if (background_music != nullptr) {
            Mix_PlayMusic(background_music, loops);
        }
    }

    // -1 to infinitely loop
    int play_sound_effect(const std::string& file_path, int loop, int volume = MIX_MAX_VOLUME) {
        if (sound_effects.find(file_path) != sound_effects.end()) {
            int channel = Mix_PlayChannel(-1, sound_effects[file_path], loop);
            if (channel != -1) {
                Mix_Volume(channel, volume); // Set volume for the specific channel
            }
            return channel;
        }
        else {
            std::cerr << "Sound effect not found: " << file_path << std::endl;
            return -1;
        }
    }

    // Handle footstep sound
    void handle_footstep(const InputState& input_state) {
        if (input_state.w_down || input_state.s_down || input_state.a_down || input_state.d_down) {
            if (!footstep_playing) {
                footstep_channel = play_sound_effect(audio_path("footstep.wav"), -1);
                footstep_playing = true;
            }
        }
        else if (footstep_playing) {
            stop_footstep(footstep_channel);
        }
    }

    // Stop footstep sound effect
    void stop_footstep(int channel) {
        if (footstep_playing) {
            // Stop sound on the channel
            Mix_HaltChannel(channel);
            footstep_playing = false;
        }
    }


    // Play sword sound effect
    void play_attack_sword() {
        play_sound_effect(audio_path("swordslash.wav"), 0, MIX_MAX_VOLUME / 4);
    }

    // Play bow sound effect
    void play_attack_bow() {
        play_sound_effect(audio_path("bowshot.wav"), 0, MIX_MAX_VOLUME / 4);
    }

    // Play dodge sound effect
    void play_dodge() {
        play_sound_effect(audio_path("dodge.wav"), 0, MIX_MAX_VOLUME / 6);
    }

    // Stop a sound effect playing on a specific channel
    void stop_sound_effect(int channel) {
        Mix_HaltChannel(channel);
    }

    void clean_up() {
        for (auto& pair : sound_effects) {
            Mix_FreeChunk(pair.second);
        }
        sound_effects.clear();

        if (background_music != nullptr) {
            Mix_FreeMusic(background_music);
            background_music = nullptr;
        }
        Mix_CloseAudio();
        SDL_Quit();
    }

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

private:
    AudioSystem() : background_music(nullptr) {}
    ~AudioSystem() { clean_up(); }

    Mix_Music* background_music;
    std::unordered_map<std::string, Mix_Chunk*> sound_effects;

    bool footstep_playing = false;
    int footstep_channel = -1;
};