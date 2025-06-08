// src/GameEngine.cpp
#include "../include/GameEngine.h"
#include "../include/Utility.h"
#include <SDL2/SDL.h>
    DragState dragState;

GameEngine::GameEngine(SDL_Renderer *R, TTF_Font *F)
    : mRenderer(R), mFont(F),
      mCardRenderer(R, F),
      mSoundManager(),
      mGame(),
      menuText("Welcome to Solitaire"),
      mStartTime(SDL_GetTicks())
{
    setupMenuButtons();
    setupSettingsButtons();
    setupStatisticsButtons();
    setupPlayingButtons();
}

void GameEngine::startNewGame()
{
    mGame.score = 0;
    mGame.initializeDeck();
    mGame.setupPiles();
    while (!undoStack.empty())
        undoStack.pop();
    undoStack.push(mGame);
    mStartTime = SDL_GetTicks();
    mDrawCount = 1;
    paused = false;
    win = false;
    hintActive = false;
}

void GameEngine::setupMenuButtons()
{
    mMenuButtons.clear();
    mMenuButtons.emplace_back(410, 400, 200, 50, "Start Game", [&]()
                              {
    startNewGame(); state=PLAYING; });
    mMenuButtons.emplace_back(410, 470, 200, 50, "Settings", [&]()
                              { state = SETTINGS; });
    mMenuButtons.emplace_back(410, 540, 200, 50, "Statistics", [&]()
                              { state = STATISTICS; });
    mMenuButtons.emplace_back(410, 610, 200, 50, "Quit", [&]()
                              { mQuit = true; });
}

void GameEngine::setupSettingsButtons()
{
    mSettingsButtons.clear();
    mSettingsButtons.push_back(Button(400, 400, 200, 50, "Toggle Sound", [this]()
                                      { mSoundManager.toggleSound(); }));
    mSettingsButtons.push_back(Button(400, 470, 200, 50, "Back", [this]()
                                      { state = MENU; }));
}

void GameEngine::setupStatisticsButtons()
{
    mStatisticsButtons.clear();
    mStatisticsButtons.push_back(Button(400, 400, 200, 50, "Reset Stats", [this]()
                                        {
            highScore = 999999;
            bestTime = 999999;
            bestMoves = 999999; }));
    mStatisticsButtons.push_back(Button(400, 470, 200, 50, "Back", [this]()
                                        { state = MENU; }));
}

void GameEngine::setupPlayingButtons()
{
    mPlayingButtons.clear();
    mPlayingButtons.push_back(Button(800, 150, 150, 40, "Restart", [this]()
                                     { startNewGame(); }));
    mPlayingButtons.push_back(Button(800, 200, 150, 40, "Undo", [this]()
                                     {
            if (undoStack.size() > 1) {
                undoStack.pop();
                mGame = undoStack.top();
            } }));
    mPlayingButtons.push_back(Button(800, 250, 150, 40, "Toggle Draw", [this]()
                                     { mDrawCount = (mDrawCount == 1) ? 3 : 1; }));
    mPlayingButtons.push_back(Button(800, 300, 150, 40, "Pause/Resume", [this]()
                                     { paused = !paused; }));
    mPlayingButtons.push_back(Button(800, 350, 150, 40, "Hint", [this]()
                                     {
            int hp, hc, dest;
            if (findHint(hp, hc, dest)) {
                hintPileIndex = hp;
                hintCardIndex = hc;
                hintStartTime = SDL_GetTicks();
                hintActive = true;
            } }));
    mPlayingButtons.push_back(Button(800, 400, 150, 40, "Auto-Complete", [this]()
                                     { autoComplete(); }));
}

void GameEngine::animateAutoMove(int sp, int ci, int dp, int sx, int sy, int dx, int dy)
{
    Card c = mGame.piles[sp].cards[ci];
    mGame.piles[sp].cards.erase(mGame.piles[sp].cards.begin() + ci);
    animateCardMove(c, sx, sy, dx, dy, 500, [&, c, sp, dp]()
                    {
    mGame.piles[dp].cards.push_back(c);
    mGame.score+=10; mGame.moveCount++;
    auto& o=mGame.piles[sp];
    if(!o.cards.empty()&&!o.cards.back().faceUp) o.cards.back().faceUp=true;
    mSoundManager.playMoveSound();
    checkWin(); });
}

void GameEngine::checkWin()
{
    if (!mGame.piles[0].cards.empty() || !mGame.piles[1].cards.empty())
        return;
    for (size_t i = 6; i < mGame.piles.size(); ++i)
        if (!mGame.piles[i].cards.empty())
            return;
    win = true;
    Uint32 t = (SDL_GetTicks() - mStartTime) / 1000;
    if (t < bestTime)
        bestTime = t;
    if (mGame.moveCount < bestMoves)
        bestMoves = mGame.moveCount;
}

bool GameEngine::findHint(int &hp, int &hc, int &dest)
{
    auto &w = mGame.piles[1];
    if (!w.cards.empty() && w.cards.back().faceUp)
    {
        for (int f = 2; f < 6; ++f)
        {
            if (mGame.canPlaceOnFoundation(w.cards.back(), mGame.piles[f]))
            {
                hp = 1;
                hc = w.cards.size() - 1;
                dest = f;
                return true;
            }
        }
    }
    for (int i = 6; i < (int)mGame.piles.size(); ++i)
    {
        auto &p = mGame.piles[i];
        if (!p.cards.empty() && p.cards.back().faceUp)
        {
            for (int f = 2; f < 6; ++f)
            {
                if (mGame.canPlaceOnFoundation(p.cards.back(), mGame.piles[f]))
                {
                    hp = i;
                    hc = p.cards.size() - 1;
                    dest = f;
                    return true;
                }
            }
        }
    }
    return false;
}

void GameEngine::autoComplete()
{
    int hp, hc, d;
    if (findHint(hp, hc, d))
    {
        int sx = (hp == 1 ? mGame.piles[1].x : mGame.piles[hp].x);
        int sy = (hp == 1 ? mGame.piles[1].y : mGame.piles[hp].y + hc * CARD_SPACING_Y);
        int dx = mGame.piles[d].x, dy = mGame.piles[d].y;
        animateAutoMove(hp, hc, d, sx, sy, dx, dy);
    }
}

void GameEngine::update()
{
    if (state == PLAYING && !paused)
    {
        // …
    }
}

void GameEngine::render()
{

    int mx, my;
    Uint32 mb = SDL_GetMouseState(&mx, &my);
    bool down = (mb & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    for (auto &b : mMenuButtons)
        b.update(mx, my, down);
    for (auto &b : mSettingsButtons)
        b.update(mx, my, down);
    for (auto &b : mStatisticsButtons)
        b.update(mx, my, down);
    for (auto &b : mPlayingButtons)
        b.update(mx, my, down);

    if (state == MENU)
    {
        int textWidth = 0;
        TTF_SizeText(mFont, menuText.c_str(), &textWidth, nullptr);
        mCardRenderer.renderText(menuText, (WINDOW_WIDTH/2) - textWidth/2, 300);
        for (auto &b : mMenuButtons)
            b.render(mRenderer, mFont);
    }
    else if (state == SETTINGS)
    {
        mCardRenderer.renderText("Settings", 400, 200);
        mCardRenderer.renderText("Sound: " + std::string(mSoundManager.isSoundOn() ? "On" : "Off"), 400, 300);
        for (auto &b : mSettingsButtons)
            b.render(mRenderer, mFont);
    }
    else if (state == STATISTICS)
    {
        mCardRenderer.renderText("Statistics", 400, 200);
        mCardRenderer.renderText("Best Time: " + std::to_string(bestTime) + " sec", 400, 250);
        mCardRenderer.renderText("Fewest Moves: " + std::to_string(bestMoves), 400, 300);
        for (auto &b : mStatisticsButtons)
            b.render(mRenderer, mFont);
    }
    else if (state == PLAYING)
    {
        for (const auto &pile : mGame.piles)
        {
            SDL_Rect pileRect{pile.x, pile.y, CARD_WIDTH, CARD_HEIGHT};
            SDL_SetRenderDrawColor(mRenderer, 50, 50, 50, 255);
            SDL_RenderDrawRect(mRenderer, &pileRect);
            int offset = (pile.type == TABLEAU) ? CARD_SPACING_Y : 0;
            for (size_t i = 0; i < pile.cards.size(); i++)
            {
                int cardX = pile.x;
                int cardY = pile.y + i * offset;
                mCardRenderer.drawCard(cardX, cardY, pile.cards[i]);
            }
        }
        if (dragState.dragging)
        {
            int drawX = dragState.mouseX - dragState.offsetX;
            int drawY = dragState.mouseY - dragState.offsetY;
            for (size_t i = 0; i < dragState.draggedCards.size(); i++)
                mCardRenderer.drawCard(drawX, drawY + i * CARD_SPACING_Y, dragState.draggedCards[i]);
        }
        updateAnimations(mCardRenderer);
        mCardRenderer.renderText("Score: " + std::to_string(mGame.score), 800, 10);
        mCardRenderer.renderText("Moves: " + std::to_string(mGame.moveCount), 800, 30);
        mCardRenderer.renderText("Time: " + std::to_string((SDL_GetTicks() - mStartTime) / 1000) + " sec", 800, 50);
        mCardRenderer.renderText("Draw Count: " + std::to_string(mDrawCount), 800, 70);
        mCardRenderer.renderText("High Score: " + std::to_string(highScore), 800, 90);
        // draw winning or random mode.
        if (mGame.mode == WINNING)
            mCardRenderer.renderText("WINNING MODE", 800, 110);
        else
            mCardRenderer.renderText("RANDOM MODE", 800, 110);

        if (win)
            mCardRenderer.renderText("YOU WIN!", 450, 350);
        else if (paused)
            mCardRenderer.renderText("PAUSED", 450, 350);
        // If hint is active, draw red outline for 2 sec.
        if (hintActive)
        {
            Uint32 elapsed = SDL_GetTicks() - hintStartTime;
            if (elapsed < 2000)
            {
                Pile &hintPileRef = mGame.piles[hintPileIndex];
                int hx = hintPileRef.x;
                int hy = hintPileRef.y;
                if (hintPileRef.type == TABLEAU)
                    hy += hintCardIndex * CARD_SPACING_Y;
                SDL_Rect hintRect = {hx - 2, hy - 2, CARD_WIDTH + 4, CARD_HEIGHT + 4};
                SDL_SetRenderDrawColor(mRenderer, 255, 0, 0, 255);
                SDL_RenderDrawRect(mRenderer, &hintRect);
            }
            else
            {
                hintActive = false;
            }
        }
        for (auto &button : mPlayingButtons)
            button.render(mRenderer, mFont);
    }
}

void GameEngine::handleEvent(SDL_Event &event)
{
    const int tableauYOffset = CARD_SPACING_Y;
    if (state == MENU)
    {

        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
        {
            int mx = event.button.x, my = event.button.y;
            for (auto &button : mMenuButtons)
            {
                if (button.isClicked(mx, my))
                {
                    button.onClick();
                    return;
                }
            }
        }
    }
    else if (state == SETTINGS)
    {
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
        {
            int mx = event.button.x, my = event.button.y;
            for (auto &button : mSettingsButtons)
            {
                if (button.isClicked(mx, my))
                {
                    button.onClick();
                    return;
                }
            }
        }
    }
    else if (state == STATISTICS)
    {
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
        {
            int mx = event.button.x, my = event.button.y;
            for (auto &button : mStatisticsButtons)
            {
                if (button.isClicked(mx, my))
                {
                    button.onClick();
                    return;
                }
            }
        }
    }
    else if (state == PLAYING)
    {
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
        {
            int mx = event.button.x, my = event.button.y;
            for (auto &button : mPlayingButtons)
            {
                if (button.isClicked(mx, my))
                {
                    button.onClick();
                    return;
                }
            }
        }
        switch (event.type)
        {
        case SDL_QUIT:
            mQuit = true;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_p)
                paused = !paused;
            if (!paused)
            {
                if (event.key.keysym.sym == SDLK_w)
                {
                    mGame.mode = (mGame.mode == RANDOM) ? WINNING : RANDOM;
                    startNewGame();
                }
                if (event.key.keysym.sym == SDLK_d)
                {
                    mDrawCount = (mDrawCount == 1) ? 3 : 1;
                }
                if (event.key.keysym.sym == SDLK_u)
                {
                    if (undoStack.size() > 1)
                    {
                        undoStack.pop();
                        mGame = undoStack.top();
                    }
                }
                if (event.key.keysym.sym == SDLK_r)
                {
                    startNewGame();
                }
                if (event.key.keysym.sym == SDLK_h)
                {
                    int hp, hc, dest;
                    if (findHint(hp, hc, dest))
                    {
                        hintPileIndex = hp;
                        hintCardIndex = hc;
                        hintStartTime = SDL_GetTicks();
                        hintActive = true;
                    }
                }
                if (event.key.keysym.sym == SDLK_a)
                {
                    // 'A' key triggers auto–complete.
                    autoComplete();
                }
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (paused)
                break;
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                int mx = event.button.x, my = event.button.y;
                if (event.button.clicks > 1)
                {
                    // Check Waste.
                    Pile &waste = mGame.piles[1];
                    int cardIndex;
                    if (!waste.cards.empty() && pointInRect(mx, my, waste.x, waste.y, CARD_WIDTH, CARD_HEIGHT) &&
                        findCardAtPoint(waste, mx, my, cardIndex, 0))
                    {
                        if (waste.cards[cardIndex].faceUp)
                        {
                            int destIndex = -1;
                            for (int i = 2; i < 6; i++)
                            {
                                if (mGame.canPlaceOnFoundation(waste.cards[cardIndex], mGame.piles[i]))
                                {
                                    destIndex = i;
                                    break;
                                }
                            }
                            if (destIndex != -1)
                            {
                                int srcX = mGame.piles[1].x;
                                int srcY = mGame.piles[1].y;
                                int destX = mGame.piles[destIndex].x;
                                int destY = mGame.piles[destIndex].y;
                                animateAutoMove(1, cardIndex, destIndex, srcX, srcY, destX, destY);
                                return;
                            }
                        }
                    }
                    // Check Tableaus.
                    for (int i = 6; i < mGame.piles.size(); i++)
                    {
                        Pile &pile = mGame.piles[i];
                        int cardIndex;
                        if (!pile.cards.empty() && findCardAtPoint(pile, mx, my, cardIndex, tableauYOffset))
                        {
                            if (pile.cards[cardIndex].faceUp)
                            {
                                int destIndex = -1;
                                for (int j = 2; j < 6; j++)
                                {
                                    if (mGame.canPlaceOnFoundation(pile.cards[cardIndex], mGame.piles[j]))
                                    {
                                        destIndex = j;
                                        break;
                                    }
                                }
                                if (destIndex != -1)
                                {
                                    int srcX = pile.x;
                                    int srcY = pile.y + cardIndex * tableauYOffset;
                                    int destX = mGame.piles[destIndex].x;
                                    int destY = mGame.piles[destIndex].y;
                                    animateAutoMove(i, cardIndex, destIndex, srcX, srcY, destX, destY);
                                    return;
                                }
                            }
                        }
                    }
                }
                // Click on Stock.
                if (pointInRect(mx, my, mGame.piles[0].x, mGame.piles[0].y, CARD_WIDTH, CARD_HEIGHT))
                {
                    mGame.handleStockClick(mDrawCount);
                    undoStack.push(mGame);
                    return;
                }
                // Click on Waste for dragging.
                Pile &waste = mGame.piles[1];
                if (!waste.cards.empty() && pointInRect(mx, my, waste.x, waste.y, CARD_WIDTH, CARD_HEIGHT))
                {
                    dragState.dragging = true;
                    dragState.draggedCards.clear();
                    dragState.draggedCards.push_back(waste.cards.back());
                    waste.cards.pop_back();
                    dragState.originPileIndex = 1;
                    dragState.originCardIndex = waste.cards.size();
                    dragState.offsetX = mx - waste.x;
                    dragState.offsetY = my - waste.y;
                    dragState.mouseX = event.motion.x;
                    dragState.mouseY = event.motion.y;
                    return;
                }
                // Click on Tableaus.
                for (int i = 6; i < mGame.piles.size(); i++)
                {
                    Pile &pile = mGame.piles[i];
                    if (pile.type == TABLEAU && !pile.cards.empty())
                    {
                        int cardIndex = -1;
                        if (findCardAtPoint(pile, mx, my, cardIndex, tableauYOffset))
                        {
                            if (pile.cards[cardIndex].faceUp)
                            {
                                dragState.dragging = true;
                                dragState.draggedCards.clear();
                                for (size_t k = cardIndex; k < pile.cards.size(); k++)
                                    dragState.draggedCards.push_back(pile.cards[k]);
                                pile.cards.erase(pile.cards.begin() + cardIndex, pile.cards.end());

                                dragState.originPileIndex = i;
                                dragState.originCardIndex = cardIndex;
                                dragState.offsetX = mx - pile.x;
                                dragState.offsetY = my - (pile.y + cardIndex * tableauYOffset);
                                dragState.mouseX = event.motion.x;
                                dragState.mouseY = event.motion.y;
                                return;
                            }
                        }
                    }
                }
            }
            break;
        case SDL_MOUSEMOTION:
            if (dragState.dragging)
            {
                dragState.mouseX = event.motion.x;
                dragState.mouseY = event.motion.y;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (dragState.dragging)
            {
                int mx = event.button.x, my = event.button.y;
                bool placed = false;

                // --- 3a) Try Foundations if single card ---
                if (dragState.draggedCards.size() == 1)
                {
                    Card c = dragState.draggedCards[0];
                    for (int f = 2; f < 6; ++f)
                    {
                        Pile &dest = mGame.piles[f];
                        if (pointInRect(mx, my, dest.x, dest.y, CARD_WIDTH, CARD_HEIGHT) &&
                            mGame.canPlaceOnFoundation(c, dest))
                        {
                            dest.cards.push_back(c);
                            mGame.moveCount++;
                            placed = true;
                            break;
                        }
                    }
                }

                // --- 3b) Then your existing tableau logic ---
                if (!placed)
                {
                    for (int i = 6; i < (int)mGame.piles.size(); ++i)
                    {
                        Pile &dest = mGame.piles[i];
                        if (dest.type != TABLEAU)
                            continue;
                        int dropX = dest.x;
                        int dropY = dest.y + dest.cards.size() * CARD_SPACING_Y;
                        if (pointInRect(mx, my, dropX, dropY, CARD_WIDTH, CARD_HEIGHT) &&
                            canMoveSequence(dragState.draggedCards, dest))
                        {
                            dest.cards.insert(dest.cards.end(),
                                              dragState.draggedCards.begin(),
                                              dragState.draggedCards.end());
                            mGame.moveCount++;
                            placed = true;
                            break;
                        }
                    }
                }

                // --- flip underneath only if placed ---
                if (placed)
                {
                    auto &orig = mGame.piles[dragState.originPileIndex];
                    if (!orig.cards.empty() && !orig.cards.back().faceUp)
                        orig.cards.back().faceUp = true;
                    mSoundManager.playMoveSound();
                }
                else
                {
                    // bounce back
                    auto &orig = mGame.piles[dragState.originPileIndex];
                    orig.cards.insert(orig.cards.end(),
                                      dragState.draggedCards.begin(),
                                      dragState.draggedCards.end());
                }

                dragState.dragging = false;
                undoStack.push(mGame);
            }
            break;
        }
    }
}

bool GameEngine::quit() const { return mQuit; }
