// src/SoundManager.cpp
#include "../include/SoundManager.h"
#include "../include/Constants.h"
#include <iostream>

SoundManager::SoundManager():soundOn(true){
  if(Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048)<0)
    std::cerr<<"SDL_mixer init error: "<<Mix_GetError()<<"\n";
  moveSound=Mix_LoadWAV(MOVE_SOUND_FILE);
  if(!moveSound) std::cerr<<"LoadWAV error: "<<Mix_GetError()<<"\n";
}

SoundManager::~SoundManager(){
  if(moveSound) Mix_FreeChunk(moveSound);
  Mix_CloseAudio();
}

void SoundManager::playMoveSound(){
  if(soundOn&&moveSound) Mix_PlayChannel(-1,moveSound,0);
}

void SoundManager::toggleSound(){ soundOn=!soundOn; }
bool SoundManager::isSoundOn()const{ return soundOn; }
