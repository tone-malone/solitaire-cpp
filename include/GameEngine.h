// include/GameEngine.h
#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stack>
#include <vector>
#include <string>
#include "Game.h"
#include "CardRenderer.h"
#include "SoundManager.h"
#include "Button.h"
#include "Animation.h"

struct DragState
{
    bool dragging = false;
    std::vector<Card> draggedCards;
    int originPileIndex = -1;
    int originCardIndex = -1;
    int offsetX = 0, offsetY = 0;
    int mouseX = 0, mouseY = 0;
};


class GameEngine {
public:
    GameEngine(SDL_Renderer* ren, TTF_Font* f);
    void update();
    void render();
    void handleEvent(SDL_Event& e);
    bool quit() const;

private:
    void startNewGame();
    void setupMenuButtons();
    void setupSettingsButtons();
    void setupStatisticsButtons();
    void setupPlayingButtons();

    void animateAutoMove(int srcPile,int cardIdx,int destPile,
                         int sx,int sy,int dx,int dy);
    void checkWin();
    bool findHint(int& hp,int& hc,int& dest);
    void autoComplete();

    SDL_Renderer* mRenderer;
    TTF_Font*     mFont;
    CardRenderer  mCardRenderer;
    SoundManager  mSoundManager;
    Game          mGame;
    std::stack<Game> undoStack;

    bool           mQuit     = false;
    bool           paused    = false;
    int            mDrawCount= 1;
    GameState      state     = MENU;
    std::string    menuText;
    std::vector<Button> mMenuButtons,
                       mSettingsButtons,
                       mStatisticsButtons,
                       mPlayingButtons;

    Uint32 mStartTime=0;
    int    highScore=999999,
           bestTime =999999,
           bestMoves=999999;

    bool   hintActive=false, win=false;
    int    hintPileIndex=-1, hintCardIndex=-1;
    Uint32 hintStartTime=0;
};
