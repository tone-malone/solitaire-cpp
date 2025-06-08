// src/main.cpp
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include "../include/Constants.h"
#include "../include/GameEngine.h"

int main(int argc,char*argv[]){
  if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0){
    std::cerr<<"SDL_Init: "<<SDL_GetError()<<"\n"; return 1;
  }
  if(TTF_Init()<0){
    std::cerr<<"TTF_Init: "<<TTF_GetError()<<"\n"; SDL_Quit(); return 1;
  }
  if(!(IMG_Init(IMG_INIT_PNG)&IMG_INIT_PNG)){
    std::cerr<<"IMG_Init: "<<IMG_GetError()<<"\n"; TTF_Quit(); SDL_Quit(); return 1;
  }

  SDL_Window*   win=SDL_CreateWindow("Solitaire",
                      SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
                      WINDOW_WIDTH,WINDOW_HEIGHT,0);
  if(!win){ std::cerr<<"CreateWindow: "<<SDL_GetError()<<"\n"; IMG_Quit(); TTF_Quit(); SDL_Quit(); return 1; }
  SDL_Renderer* ren=SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED);
  if(!ren){ std::cerr<<"CreateRenderer: "<<SDL_GetError()<<"\n"; SDL_DestroyWindow(win); IMG_Quit(); TTF_Quit(); SDL_Quit(); return 1; }

  TTF_Font* font=TTF_OpenFont(FONT_FILE,FONT_SIZE);
  if(!font){
    std::cerr<<"OpenFont: "<<TTF_GetError()<<"\n"; SDL_DestroyRenderer(ren); SDL_DestroyWindow(win);
    IMG_Quit(); TTF_Quit(); SDL_Quit(); return 1;
  }

  GameEngine engine(ren,font);
  SDL_Event e;
  while(!engine.quit()){
    while(SDL_PollEvent(&e)) engine.handleEvent(e);
    engine.update();
    SDL_SetRenderDrawColor(ren,0,100,0,255);
    SDL_RenderClear(ren);
    engine.render();
    SDL_RenderPresent(ren);
    SDL_Delay(16);
  }

  TTF_CloseFont(font);
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  IMG_Quit(); TTF_Quit(); SDL_Quit();
  return 0;
}
