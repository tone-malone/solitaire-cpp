// src/Animation.cpp
#include "../include/Animation.h"
#include "../include/Utility.h"
#include "../include/CardRenderer.h"
#include "../include/Constants.h"
#include <SDL2/SDL.h>

std::vector<Animation> animations;

void animateCardMove(const Card& c,int fx,int fy,int tx,int ty,Uint32 ms,std::function<void()> cb){
  animations.push_back(Animation{AnimType::MoveCard,c,fx,fy,tx,ty,SDL_GetTicks(),ms,cb});
}

void updateAnimations(CardRenderer& R){
  Uint32 now=SDL_GetTicks();
  for(size_t i=0;i<animations.size();){
    auto& A=animations[i];
    float t=float(now-A.startTime)/float(A.duration);
    if(t>1.f) t=1.f;
    float e=easeOutQuad(t);
    if(A.type==AnimType::MoveCard){
      int x=lerp(A.srcX,A.dstX,e), y=lerp(A.srcY,A.dstY,e);
      R.drawCard(x,y,A.card);
    }
    if(t>=1.f){
      if(A.onComplete) A.onComplete();
      animations.erase(animations.begin()+i);
    } else ++i;
  }
}
