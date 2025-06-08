// src/CardRenderer.cpp
#include "../include/CardRenderer.h"
#include "../include/Constants.h"
#include "../include/Utility.h"
#include <SDL2/SDL_image.h>
#include <iostream>

CardRenderer::CardRenderer(SDL_Renderer* R,TTF_Font* F)
 : mRenderer(R),mFont(F)
{
  mSpadeTexture    = IMG_LoadTexture(R,SPADE_IMG);
  mHeartTexture    = IMG_LoadTexture(R,HEART_IMG);
  mDiamondTexture  = IMG_LoadTexture(R,DIAMOND_IMG);
  mClubTexture     = IMG_LoadTexture(R,CLUB_IMG);
  mCardBackTexture = IMG_LoadTexture(R,CARDBACK_IMG);
  mJackTexture     = IMG_LoadTexture(R,JACK_IMG);
  mQueenTexture    = IMG_LoadTexture(R,QUEEN_IMG);
  mKingTexture     = IMG_LoadTexture(R,KING_IMG);
  if(!mSpadeTexture||!mHeartTexture||!mDiamondTexture||!mClubTexture)
    std::cerr<<"Suit texture error: "<<IMG_GetError()<<"\n";
  if(!mCardBackTexture)
    std::cerr<<"Cardback texture error: "<<IMG_GetError()<<"\n";
}

CardRenderer::~CardRenderer(){
  SDL_DestroyTexture(mSpadeTexture);
  SDL_DestroyTexture(mHeartTexture);
  SDL_DestroyTexture(mDiamondTexture);
  SDL_DestroyTexture(mClubTexture);
  SDL_DestroyTexture(mCardBackTexture);
  SDL_DestroyTexture(mJackTexture);
  SDL_DestroyTexture(mQueenTexture);
  SDL_DestroyTexture(mKingTexture);
}

void CardRenderer::drawPipTexture(SDL_Texture* tex,int cx,int cy,int scale){
  SDL_Rect d{cx-scale/2,cy-scale/2,scale,scale};
  SDL_RenderCopy(mRenderer,tex,nullptr,&d);
}

void CardRenderer::drawPips(int x,int y,int w,int h,int val,int suit){
  const int m=2;
  int iw=w-2*m, ih=h-2*m;
  int cx=x+m+iw/2, cy=y+6+m+ih/2;
  int lx=x+m+iw/4, rx=x+m+3*iw/4;
  int ty=y+8+m+ih/4, by=y+6+m+3*ih/4;
  int ps=16;
  SDL_Texture* pt=nullptr;
  switch(suit){case 0:pt=mSpadeTexture;break;
               case 1:pt=mHeartTexture;break;
               case 2:pt=mDiamondTexture;break;
               case 3:pt=mClubTexture;break;}
  if(!pt) return;
switch (val)
        {
        case 1:
            drawPipTexture(pt, cx, cy, ps);
            break;
        case 2:
            drawPipTexture(pt, cx, ty, ps);
            drawPipTexture(pt, cx, by, ps);
            break;
        case 3:
            drawPipTexture(pt, cx, ty, ps);
            drawPipTexture(pt, cx, cy, ps);
            drawPipTexture(pt, cx, by, ps);
            break;
        case 4:
            drawPipTexture(pt, lx, ty, ps);
            drawPipTexture(pt, rx, ty, ps);
            drawPipTexture(pt, lx, by, ps);
            drawPipTexture(pt, rx, by, ps);
            break;
        case 5:
            drawPipTexture(pt, lx, ty, ps);
            drawPipTexture(pt, rx, ty, ps);
            drawPipTexture(pt, cx, cy, ps);
            drawPipTexture(pt, lx, by, ps);
            drawPipTexture(pt, rx, by, ps);
            break;
        case 6:
            drawPipTexture(pt, lx, ty, ps);
            drawPipTexture(pt, rx, ty, ps);
            drawPipTexture(pt, lx, cy, ps);
            drawPipTexture(pt, rx, cy, ps);
            drawPipTexture(pt, lx, by, ps);
            drawPipTexture(pt, rx, by, ps);
            break;
        case 7:
            drawPipTexture(pt, lx, ty, ps);
            drawPipTexture(pt, rx, ty, ps);
            drawPipTexture(pt, cx, ty, ps);
            drawPipTexture(pt, lx, cy, ps);
            drawPipTexture(pt, rx, cy, ps);
            drawPipTexture(pt, lx, by, ps);
            drawPipTexture(pt, rx, by, ps);
            break;
        case 8:
            drawPipTexture(pt, lx, ty, ps);
            drawPipTexture(pt, rx, ty, ps);
            drawPipTexture(pt, cx, ty, ps);
            drawPipTexture(pt, lx, cy, ps);
            drawPipTexture(pt, rx, cy, ps);
            drawPipTexture(pt, cx, by, ps);
            drawPipTexture(pt, lx, by, ps);
            drawPipTexture(pt, rx, by, ps);
            break;
        case 9:
            drawPipTexture(pt, lx, ty, ps);
            drawPipTexture(pt, rx, ty, ps);
            drawPipTexture(pt, cx, ty, ps);
            drawPipTexture(pt, lx, cy, ps);
            drawPipTexture(pt, rx, cy, ps);
            drawPipTexture(pt, cx, cy, ps);
            drawPipTexture(pt, lx, by, ps);
            drawPipTexture(pt, rx, by, ps);
            drawPipTexture(pt, cx, by, ps);
            break;
        case 10:
        {
            int rows = 5;
            int pipSpacingY = (by - ty) / (rows - 1);
            for (int i = 0; i < rows; i++)
            {
                int posY = ty + i * pipSpacingY;
                drawPipTexture(pt, lx, posY, ps);
                drawPipTexture(pt, rx, posY, ps);
            }
            break;
        }
        default:
            break;
        }
}

void CardRenderer::drawCard(int x,int y,const Card& card)    {
        SDL_Rect cardRect{x, y, CARD_WIDTH, CARD_HEIGHT};
        if (card.faceUp)
        {
            // Draw card face.
            SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
            SDL_RenderFillRect(mRenderer, &cardRect);
            SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(mRenderer, &cardRect);

            if (card.value >= 1 && card.value <= 10)
            {
                drawPips(x, y, CARD_WIDTH, CARD_HEIGHT, card.value, card.suit);
                std::string valueText = cardValueToString(card);
                SDL_Color textColor = {0, 0, 0, 255};
                if (card.suit == 1 || card.suit == 2)
                    textColor = {255, 0, 0, 255};
                else
                    textColor = {0, 0, 0, 255};

                SDL_Surface *textSurface = TTF_RenderText_Blended(mFont, valueText.c_str(), textColor);
                if (textSurface)
                {
                    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);
                    if (textTexture)
                    {
                        SDL_Rect textRect{x + 3, y + 3, textSurface->w, textSurface->h};
                        SDL_RenderCopy(mRenderer, textTexture, nullptr, &textRect);
                        SDL_DestroyTexture(textTexture);
                    }
                    SDL_FreeSurface(textSurface);
                }
            }
            else
            {

                // For face cards, draw special face textures if available.
                SDL_Texture *faceTexture = nullptr;
                switch (card.value)
                {
                case 11:
                    faceTexture = mJackTexture;
                    break;
                case 12:
                    faceTexture = mQueenTexture;
                    break;
                case 13:
                    faceTexture = mKingTexture;
                    break;
                default:
                    break;
                }
                if (faceTexture)
                {
                    int largeSize = CARD_WIDTH - 20;
                    SDL_Rect dest{x + 15, y + 20, 45, 70};
                    SDL_RenderCopy(mRenderer, faceTexture, nullptr, &dest);
                }
                // Fallback: use suit texture.
                SDL_Texture *suitTexture = nullptr;
                switch (card.suit)
                {
                case 0:
                    suitTexture = mSpadeTexture;
                    break;
                case 1:
                    suitTexture = mHeartTexture;
                    break;
                case 2:
                    suitTexture = mDiamondTexture;
                    break;
                case 3:
                    suitTexture = mClubTexture;
                    break;
                default:
                    break;
                }
                if (suitTexture)
                {
                    int largeSize = CARD_WIDTH - 20;
                    SDL_Rect dest{x + 15, y + 25, 16, 16};
                    SDL_RenderCopy(mRenderer, suitTexture, nullptr, &dest);
                }

                std::string valueText = cardValueToString(card);
                SDL_Color textColor = {0, 0, 0, 255};
                if (card.suit == 1 || card.suit == 2)
                    textColor = {255, 0, 0, 255};
                else
                    textColor = {0, 0, 0, 255};

                SDL_Surface *textSurface = TTF_RenderText_Blended(mFont, valueText.c_str(), textColor);
                if (textSurface)
                {
                    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);
                    if (textTexture)
                    {
                        SDL_Rect textRect{x + 3, y + 3, textSurface->w, textSurface->h};
                        SDL_RenderCopy(mRenderer, textTexture, nullptr, &textRect);
                        SDL_DestroyTexture(textTexture);
                    }
                    SDL_FreeSurface(textSurface);
                }
            }
        }
        else
        {
            // Draw card back image.
            if (mCardBackTexture)
                SDL_RenderCopy(mRenderer, mCardBackTexture, nullptr, &cardRect);
            else
            {
                SDL_SetRenderDrawColor(mRenderer, 0, 0, 255, 255);
                SDL_RenderFillRect(mRenderer, &cardRect);
                SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(mRenderer, &cardRect);
            }
        }
    }

void CardRenderer::renderText(const std::string& txt,int x,int y){
  SDL_Surface* s=TTF_RenderText_Blended(mFont,txt.c_str(),{255,255,255,255});
  if(s){
    SDL_Texture* t=SDL_CreateTextureFromSurface(mRenderer,s);
    SDL_Rect d{x,y,s->w,s->h};
    SDL_RenderCopy(mRenderer,t,nullptr,&d);
    SDL_DestroyTexture(t);
    SDL_FreeSurface(s);
  }
}
