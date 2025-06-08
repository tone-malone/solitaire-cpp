// include/CardRenderer.h
#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "Card.h"

// Draws cards & text
class CardRenderer {
public:
    CardRenderer(SDL_Renderer* renderer, TTF_Font* font);
    ~CardRenderer();
    void drawCard(int x,int y,const Card& card);
    void renderText(const std::string& text,int x,int y);
private:
    void drawPips(int x,int y,int w,int h,int value,int suit);
    void drawPipTexture(SDL_Texture* tex,int cx,int cy,int scale);

    SDL_Renderer* mRenderer;
    TTF_Font*     mFont;
    SDL_Texture*  mSpadeTexture;
    SDL_Texture*  mHeartTexture;
    SDL_Texture*  mDiamondTexture;
    SDL_Texture*  mClubTexture;
    SDL_Texture*  mCardBackTexture;
    SDL_Texture*  mJackTexture;
    SDL_Texture*  mQueenTexture;
    SDL_Texture*  mKingTexture;
};
