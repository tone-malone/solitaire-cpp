// src/Button.cpp
#include "../include/Button.h"
#include "../include/Utility.h"
#include <SDL2/SDL_ttf.h>

Button::Button(int x,int y,int w,int h,const std::string& lbl,std::function<void()> cb)
 : rect{x,y,w,h},label(lbl),callback(cb){}

void Button::update(int mx,int my,bool down){
  bool over=pointInRect(mx,my,rect.x,rect.y,rect.w,rect.h);
  state= over ? (down?ButtonState::Pressed:ButtonState::Hovered)
              : ButtonState::Normal;
}

void Button::render(SDL_Renderer* R,TTF_Font* F){
  SDL_Color bg = (state==ButtonState::Pressed)?SDL_Color{100,100,250,255}
                 :(state==ButtonState::Hovered)?SDL_Color{180,180,255,255}
                                               :SDL_Color{200,200,200,255};
  SDL_SetRenderDrawColor(R,bg.r,bg.g,bg.b,bg.a);
  SDL_RenderFillRect(R,&rect);
  SDL_SetRenderDrawColor(R,0,0,0,255);
  SDL_RenderDrawRect(R,&rect);

  SDL_Surface* s=TTF_RenderText_Blended(F,label.c_str(),{0,0,0,255});
  if(s){
    SDL_Texture* t=SDL_CreateTextureFromSurface(R,s);
    int tx=rect.x+(rect.w-s->w)/2, ty=rect.y+(rect.h-s->h)/2;
    SDL_Rect dst = {tx, ty, s->w, s->h};
    SDL_RenderCopy(R,t,nullptr,&dst);
    SDL_DestroyTexture(t);
    SDL_FreeSurface(s);
  }
}

bool Button::isClicked(int x,int y) const {
  return pointInRect(x,y,rect.x,rect.y,rect.w,rect.h);
}

void Button::onClick(){ if(callback) callback(); }
