// include/SoundManager.h
#pragma once
#include <SDL2/SDL_mixer.h>

// Plays move‐sound, toggles on/off
class SoundManager {
public:
    SoundManager();
    ~SoundManager();
    void playMoveSound();
    void toggleSound();
    bool isSoundOn() const;
private:
    Mix_Chunk* moveSound = nullptr;
    bool soundOn;
};
