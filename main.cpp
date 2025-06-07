#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <random>
#include <stack>
#include <string>
#include <vector>

// ---------------------------
// Global Constants
// ---------------------------
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;
const int CARD_WIDTH = 75;
const int CARD_HEIGHT = 110;
const int CARD_SPACING_Y = 30;

const char* FONT_FILE = "fonts/arial.ttf";
const int FONT_SIZE = 24;

const char* SPADE_IMG    = "textures/spade.png";
const char* HEART_IMG    = "textures/heart.png";
const char* DIAMOND_IMG  = "textures/diamond.png";
const char* CLUB_IMG     = "textures/club.png";
const char* CARDBACK_IMG = "textures/cardback.png";

const char* JACK_IMG   = "textures/jack.png";
const char* QUEEN_IMG  = "textures/queen.png";
const char* KING_IMG   = "textures/king.png";

const char* MOVE_SOUND_FILE = "sounds/move.wav";

// ---------------------------
// Enumerations
// ---------------------------
enum Mode { RANDOM, WINNING };
enum PileType { STOCK, WASTE, TABLEAU, FOUNDATION };
enum GameState { MENU, PLAYING, PAUSED, SETTINGS, STATISTICS };

// ---------------------------
// Data Structures
// ---------------------------
struct Card {
    int value;    // 1-13 (Ace to King)
    int suit;     // 0: spades, 1: hearts, 2: diamonds, 3: clubs
    bool faceUp;
};

struct Pile {
    PileType type;
    int x, y;
    std::vector<Card> cards;
};

// Animation structure now carries an onComplete callback.
struct Animation {
    Card card;
    int startX, startY;
    int targetX, targetY;
    Uint32 startTime;
    Uint32 duration;
    std::function<void()> onComplete;
};

struct DragState {
    bool dragging = false;
    std::vector<Card> draggedCards;
    int originPileIndex = -1;
    int originCardIndex = -1;
    int offsetX = 0, offsetY = 0;
    int mouseX = 0, mouseY = 0;
};

// Global containers.
std::vector<Animation> animations;
DragState dragState;

// ---------------------------
// Utility Functions
// ---------------------------
int lerp(int start, int end, float t) {
    return start + static_cast<int>(t * (end - start));
}

float easeOutQuad(float t) {
    return 1 - (1 - t) * (1 - t);
}

bool pointInRect(int px, int py, int rx, int ry, int rw, int rh) {
    return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
}

std::string cardValueToString(const Card &card) {
    if (card.value == 1) return "A";
    else if (card.value == 11) return "J";
    else if (card.value == 12) return "Q";
    else if (card.value == 13) return "K";
    else return std::to_string(card.value);
}

// ---------------------------
// Button Class
// ---------------------------
class Button {
public:
    Button(int x, int y, int w, int h, const std::string &label, std::function<void()> callback)
        : rect{ x, y, w, h }, label(label), callback(callback) {}
    
    void render(SDL_Renderer* renderer, TTF_Font* font) {
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);
        SDL_Color textColor = {0, 0, 0, 255};
        SDL_Surface* surface = TTF_RenderText_Blended(font, label.c_str(), textColor);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                int textX = rect.x + (rect.w - surface->w) / 2;
                int textY = rect.y + (rect.h - surface->h) / 2;
                SDL_Rect textRect = { textX, textY, surface->w, surface->h };
                SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
    
    bool isClicked(int x, int y) {
        return pointInRect(x, y, rect.x, rect.y, rect.w, rect.h);
    }
    
    void onClick() { callback(); }
    
private:
    SDL_Rect rect;
    std::string label;
    std::function<void()> callback;
};

// ---------------------------
// SoundManager Class
// ---------------------------
class SoundManager {
public:
    SoundManager() : soundOn(true) {
        if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
            std::cerr << "SDL_mixer could not initialize! Error: " << Mix_GetError() << "\n";
        moveSound = Mix_LoadWAV(MOVE_SOUND_FILE);
        if(!moveSound)
            std::cerr << "Failed to load move sound: " << Mix_GetError() << "\n";
    }
    ~SoundManager() {
        Mix_FreeChunk(moveSound);
        Mix_CloseAudio();
    }
    void playMoveSound() {
        if (soundOn)
            Mix_PlayChannel(-1, moveSound, 0);
    }
    void toggleSound() { soundOn = !soundOn; }
    bool isSoundOn() const { return soundOn; }
private:
    Mix_Chunk* moveSound = nullptr;
    bool soundOn;
};

// ---------------------------
// CardRenderer Class
// ---------------------------
class CardRenderer {
public:
    CardRenderer(SDL_Renderer* renderer, TTF_Font* font)
        : mRenderer(renderer), mFont(font)
    {
        mSpadeTexture   = IMG_LoadTexture(mRenderer, SPADE_IMG);
        mHeartTexture   = IMG_LoadTexture(mRenderer, HEART_IMG);
        mDiamondTexture = IMG_LoadTexture(mRenderer, DIAMOND_IMG);
        mClubTexture    = IMG_LoadTexture(mRenderer, CLUB_IMG);
        mCardBackTexture = IMG_LoadTexture(mRenderer, CARDBACK_IMG);
        mJackTexture    = IMG_LoadTexture(mRenderer, JACK_IMG);
        mQueenTexture   = IMG_LoadTexture(mRenderer, QUEEN_IMG);
        mKingTexture    = IMG_LoadTexture(mRenderer, KING_IMG);
        if (!mSpadeTexture || !mHeartTexture || !mDiamondTexture || !mClubTexture)
            std::cerr << "Failed to load suit textures: " << IMG_GetError() << "\n";
        if (!mCardBackTexture)
            std::cerr << "Failed to load card back texture: " << IMG_GetError() << "\n";
    }
    ~CardRenderer() {
        SDL_DestroyTexture(mSpadeTexture);
        SDL_DestroyTexture(mHeartTexture);
        SDL_DestroyTexture(mDiamondTexture);
        SDL_DestroyTexture(mClubTexture);
        if(mCardBackTexture) SDL_DestroyTexture(mCardBackTexture);
        if(mJackTexture) SDL_DestroyTexture(mJackTexture);
        if(mQueenTexture) SDL_DestroyTexture(mQueenTexture);
        if(mKingTexture) SDL_DestroyTexture(mKingTexture);
    }
    void drawPipTexture(SDL_Texture* texture, int cx, int cy, int scale) {
        SDL_Rect dest{ cx - scale / 2, cy - scale / 2, scale, scale };
        SDL_RenderCopy(mRenderer, texture, nullptr, &dest);
    }
    void drawPips(int x, int y, int width, int height, int cardValue, int suit) {
        const int margin = 2;
        int innerW = width - 2 * margin;
        int innerH = height - 2 * margin;
        int centerX = x + margin + innerW / 2;
        int centerY = y + 6 + margin + innerH / 2;
        int leftX = x + margin + innerW / 4;
        int rightX = x + margin + (3 * innerW) / 4;
        int topY = y + 8 + margin + innerH / 4;
        int bottomY = y + 6 + margin + (3 * innerH) / 4;
        int pipScale = 16;
        SDL_Texture* pipTexture = nullptr;
        switch(suit) {
            case 0: pipTexture = mSpadeTexture; break;
            case 1: pipTexture = mHeartTexture; break;
            case 2: pipTexture = mDiamondTexture; break;
            case 3: pipTexture = mClubTexture; break;
            default: break;
        }
        if (!pipTexture) return;
        switch(cardValue) {
            case 1:
                drawPipTexture(pipTexture, centerX, centerY, pipScale);
                break;
            case 2:
                drawPipTexture(pipTexture, centerX, topY, pipScale);
                drawPipTexture(pipTexture, centerX, bottomY, pipScale);
                break;
            case 3:
                drawPipTexture(pipTexture, centerX, topY, pipScale);
                drawPipTexture(pipTexture, centerX, centerY, pipScale);
                drawPipTexture(pipTexture, centerX, bottomY, pipScale);
                break;
            case 4:
                drawPipTexture(pipTexture, leftX, topY, pipScale);
                drawPipTexture(pipTexture, rightX, topY, pipScale);
                drawPipTexture(pipTexture, leftX, bottomY, pipScale);
                drawPipTexture(pipTexture, rightX, bottomY, pipScale);
                break;
            case 5:
                drawPipTexture(pipTexture, leftX, topY, pipScale);
                drawPipTexture(pipTexture, rightX, topY, pipScale);
                drawPipTexture(pipTexture, centerX, centerY, pipScale);
                drawPipTexture(pipTexture, leftX, bottomY, pipScale);
                drawPipTexture(pipTexture, rightX, bottomY, pipScale);
                break;
            case 6:
                drawPipTexture(pipTexture, leftX, topY, pipScale);
                drawPipTexture(pipTexture, rightX, topY, pipScale);
                drawPipTexture(pipTexture, leftX, centerY, pipScale);
                drawPipTexture(pipTexture, rightX, centerY, pipScale);
                drawPipTexture(pipTexture, leftX, bottomY, pipScale);
                drawPipTexture(pipTexture, rightX, bottomY, pipScale);
                break;
            case 7:
                drawPipTexture(pipTexture, leftX, topY, pipScale);
                drawPipTexture(pipTexture, rightX, topY, pipScale);
                drawPipTexture(pipTexture, centerX, topY, pipScale);
                drawPipTexture(pipTexture, leftX, centerY, pipScale);
                drawPipTexture(pipTexture, rightX, centerY, pipScale);
                drawPipTexture(pipTexture, leftX, bottomY, pipScale);
                drawPipTexture(pipTexture, rightX, bottomY, pipScale);
                break;
            case 8:
                drawPipTexture(pipTexture, leftX, topY, pipScale);
                drawPipTexture(pipTexture, rightX, topY, pipScale);
                drawPipTexture(pipTexture, centerX, topY, pipScale);
                drawPipTexture(pipTexture, leftX, centerY, pipScale);
                drawPipTexture(pipTexture, rightX, centerY, pipScale);
                drawPipTexture(pipTexture, centerX, bottomY, pipScale);
                drawPipTexture(pipTexture, leftX, bottomY, pipScale);
                drawPipTexture(pipTexture, rightX, bottomY, pipScale);
                break;
            case 9:
                drawPipTexture(pipTexture, leftX, topY, pipScale);
                drawPipTexture(pipTexture, rightX, topY, pipScale);
                drawPipTexture(pipTexture, centerX, topY, pipScale);
                drawPipTexture(pipTexture, leftX, centerY, pipScale);
                drawPipTexture(pipTexture, rightX, centerY, pipScale);
                drawPipTexture(pipTexture, centerX, centerY, pipScale);
                drawPipTexture(pipTexture, leftX, bottomY, pipScale);
                drawPipTexture(pipTexture, rightX, bottomY, pipScale);
                drawPipTexture(pipTexture, centerX, bottomY, pipScale);
                break;
            case 10: {
                int rows = 5;
                int pipSpacingY = (bottomY - topY) / (rows - 1);
                for (int i = 0; i < rows; i++) {
                    int posY = topY + i * pipSpacingY;
                    drawPipTexture(pipTexture, leftX, posY, pipScale);
                    drawPipTexture(pipTexture, rightX, posY, pipScale);
                }
                break;
            }
            default:
                break;
        }
    }
    void drawCard(int x, int y, const Card &card) {
        SDL_Rect cardRect{ x, y, CARD_WIDTH, CARD_HEIGHT };
        if (card.faceUp) {
            // Draw card face.
            SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
            SDL_RenderFillRect(mRenderer, &cardRect);
            SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(mRenderer, &cardRect);

            if (card.value >= 1 && card.value <= 10) {
                drawPips(x, y, CARD_WIDTH, CARD_HEIGHT, card.value, card.suit);
                std::string valueText = cardValueToString(card);
                SDL_Color textColor = {0, 0, 0, 255};
                if (card.suit == 1 || card.suit == 2)
                textColor = {255, 0, 0, 255}; else
                textColor = {0, 0, 0, 255};
                
                SDL_Surface* textSurface = TTF_RenderText_Blended(mFont, valueText.c_str(), textColor);
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);
                    if (textTexture) {
                        SDL_Rect textRect{ x + 3, y + 3, textSurface->w, textSurface->h };
                        SDL_RenderCopy(mRenderer, textTexture, nullptr, &textRect);
                        SDL_DestroyTexture(textTexture);
                    }
                    SDL_FreeSurface(textSurface);
                }
            } else {
                
                // For face cards, draw special face textures if available.
                SDL_Texture* faceTexture = nullptr;
                switch(card.value) {
                    case 11: faceTexture = mJackTexture; break;
                    case 12: faceTexture = mQueenTexture; break;
                    case 13: faceTexture = mKingTexture; break;
                    default: break;
                }
                if (faceTexture) {
                    int largeSize = CARD_WIDTH - 20;
                    SDL_Rect dest{ x + 15, y + 20, 45, 70};
                    SDL_RenderCopy(mRenderer, faceTexture, nullptr, &dest);
                } 
                    // Fallback: use suit texture.
                    SDL_Texture* suitTexture = nullptr;
                    switch(card.suit) {
                        case 0: suitTexture = mSpadeTexture; break;
                        case 1: suitTexture = mHeartTexture; break;
                        case 2: suitTexture = mDiamondTexture; break;
                        case 3: suitTexture = mClubTexture; break;
                        default: break;
                    }
                    if (suitTexture) {
                        int largeSize = CARD_WIDTH - 20;
                        SDL_Rect dest{ x + 15, y + 25, 16, 16 };
                        SDL_RenderCopy(mRenderer, suitTexture, nullptr, &dest);
                    }

                    std::string valueText = cardValueToString(card);
                    SDL_Color textColor = {0, 0, 0, 255};
                    if (card.suit == 1 || card.suit == 2)
                    textColor = {255, 0, 0, 255}; else
                    textColor = {0, 0, 0, 255};
                    
                    SDL_Surface* textSurface = TTF_RenderText_Blended(mFont, valueText.c_str(), textColor);
                    if (textSurface) {
                        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);
                        if (textTexture) {
                            SDL_Rect textRect{ x + 3, y + 3, textSurface->w, textSurface->h };
                            SDL_RenderCopy(mRenderer, textTexture, nullptr, &textRect);
                            SDL_DestroyTexture(textTexture);
                        }
                        SDL_FreeSurface(textSurface);
                    }
                
            }
        } else {
            // Draw card back image.
            if (mCardBackTexture)
                SDL_RenderCopy(mRenderer, mCardBackTexture, nullptr, &cardRect);
            else {
                SDL_SetRenderDrawColor(mRenderer, 0, 0, 255, 255);
                SDL_RenderFillRect(mRenderer, &cardRect);
                SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(mRenderer, &cardRect);
            }
        }
    }
    void renderText(const std::string &text, int x, int y) {
        SDL_Color color = {255, 255, 255, 255};
        SDL_Surface* surface = TTF_RenderText_Blended(mFont, text.c_str(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);
            if (texture) {
                SDL_Rect dest{ x, y, surface->w, surface->h };
                SDL_RenderCopy(mRenderer, texture, nullptr, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
private:
    SDL_Renderer* mRenderer;
    TTF_Font* mFont;
    SDL_Texture* mSpadeTexture = nullptr;
    SDL_Texture* mHeartTexture = nullptr;
    SDL_Texture* mDiamondTexture = nullptr;
    SDL_Texture* mClubTexture = nullptr;
    SDL_Texture* mCardBackTexture = nullptr;
    SDL_Texture* mJackTexture = nullptr;
    SDL_Texture* mQueenTexture = nullptr;
    SDL_Texture* mKingTexture = nullptr;
};

// ---------------------------
// Game Class
// ---------------------------
class Game {
public:
    Game() : mode(RANDOM), score(0), moveCount(0) {}
    Mode mode;
    int score;
    int moveCount;
    std::vector<Card> deck;
    std::vector<Pile> piles;
    void initializeDeck() {
        deck.clear();
        for (int s = 0; s < 4; s++)
            for (int v = 1; v <= 13; v++)
                deck.push_back({v, s, false});
        if (mode == RANDOM) {
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(deck.begin(), deck.end(), g);
        } else {
            std::sort(deck.begin(), deck.end(), [](const Card &a, const Card &b) {
                return (a.suit == b.suit) ? (a.value < b.value) : (a.suit < b.suit);
            });
        }
    }
    void setupPiles() {
        piles.clear();
        // Stock and Waste.
        piles.push_back({ STOCK, 50, 50, {} });
        piles.push_back({ WASTE, 130, 50, {} });
        // Four foundations.
        for (int i = 0; i < 4; i++)
            piles.push_back({ FOUNDATION, 400 + i * (CARD_WIDTH + 20), 50, {} });
        // Seven tableaus.
        for (int i = 0; i < 7; i++)
            piles.push_back({ TABLEAU, 50 + i * (CARD_WIDTH + 20), 200, {} });
        int deckIndex = 0;
        for (int i = 0; i < 7; i++) {
            int numCards = i + 1;
            for (int j = 0; j < numCards; j++) {
                Card card = deck[deckIndex++];
                card.faceUp = (j == numCards - 1);
                piles[6 + i].cards.push_back(card);
            }
        }
        while (deckIndex < deck.size()) {
            Card card = deck[deckIndex++];
            card.faceUp = false;
            piles[0].cards.push_back(card);
        }
        moveCount = 0;
    }
    bool canPlaceOnFoundation(const Card &card, const Pile &foundation) {
        if (foundation.cards.empty())
            return (card.value == 1);
        const Card &top = foundation.cards.back();
        return (card.suit == top.suit && card.value == top.value + 1);
    }
    bool moveCardToFoundation(int sourcePileIndex, int cardIndex) {
        Card card = piles[sourcePileIndex].cards[cardIndex];
        for (int i = 2; i < 6; i++) {
            if (canPlaceOnFoundation(card, piles[i])) {
                piles[i].cards.push_back(card);
                piles[sourcePileIndex].cards.erase(piles[sourcePileIndex].cards.begin() + cardIndex);
                score += 10;
                moveCount++;
                return true;
            }
        }
        return false;
    }
    void handleStockClick(int drawCount) {
        Pile &stock = piles[0];
        Pile &waste = piles[1];
        if (!stock.cards.empty()) {
            for (int i = 0; i < drawCount && !stock.cards.empty(); i++) {
                Card card = stock.cards.back();
                stock.cards.pop_back();
                card.faceUp = true;
                waste.cards.push_back(card);
            }
            moveCount++;
        } else if (!waste.cards.empty()) {
            while (!waste.cards.empty()) {
                Card card = waste.cards.back();
                waste.cards.pop_back();
                card.faceUp = false;
                stock.cards.push_back(card);
            }
        }
    }
};

// ---------------------------
// Animation Management
// ---------------------------
void addAnimation(const Card &card, int startX, int startY, int targetX, int targetY, Uint32 duration, std::function<void()> onComplete) {
    Animation anim;
    anim.card = card;
    anim.startX = startX;
    anim.startY = startY;
    anim.targetX = targetX;
    anim.targetY = targetY;
    anim.startTime = SDL_GetTicks();
    anim.duration = duration;
    anim.onComplete = onComplete;
    animations.push_back(anim);
}

void updateAnimations(CardRenderer &renderer) {
    Uint32 now = SDL_GetTicks();
    for (size_t i = 0; i < animations.size(); ) {
        Animation &anim = animations[i];
        float t = float(now - anim.startTime) / float(anim.duration);
        if (t > 1.0f) t = 1.0f;
        float easedT = easeOutQuad(t);
        int currentX = lerp(anim.startX, anim.targetX, easedT);
        int currentY = lerp(anim.startY, anim.targetY, easedT);
        renderer.drawCard(currentX, currentY, anim.card);
        if (t >= 1.0f) {
            if (anim.onComplete) anim.onComplete();
            animations.erase(animations.begin() + i);
        } else {
            ++i;
        }
    }
}

bool findCardAtPoint(const Pile &pile, int mx, int my, int &cardIndex, int pileYOffset) {
    for (int i = pile.cards.size() - 1; i >= 0; i--) {
        int x = pile.x, y = pile.y + i * pileYOffset;
        if (pointInRect(mx, my, x, y, CARD_WIDTH, CARD_HEIGHT)) {
            cardIndex = i;
            return true;
        }
    }
    return false;
}

// ---------------------------
// GameEngine: State Machine, UI Buttons, and Input/Render
// ---------------------------
class GameEngine {
public:
bool win;
    GameEngine(SDL_Renderer* renderer, TTF_Font* font)
        : mRenderer(renderer), mFont(font), mCardRenderer(renderer, font),
          mSoundManager(), state(MENU), highScore(999999), hintActive(false), win(false)
    {
       
        setupMenuButtons();
        setupSettingsButtons();
        setupPlayingButtons();
        setupStatisticsButtons();
    }
    
    // Animate auto–moves for double–click moves.
    void animateAutoMove(int sourcePileIndex, int cardIndex, int destPileIndex, int srcX, int srcY, int destX, int destY) {
        Card card = mGame.piles[sourcePileIndex].cards[cardIndex];
        mGame.piles[sourcePileIndex].cards.erase(mGame.piles[sourcePileIndex].cards.begin() + cardIndex);
        addAnimation(card, srcX, srcY, destX, destY, 500, [this, card, destPileIndex]() {
            mGame.piles[destPileIndex].cards.push_back(card);
            mGame.score += 10;
            mGame.moveCount++;
            mSoundManager.playMoveSound();
            checkWin();
        });
    }
    
    // Check win condition: if Stock, Waste, and Tableaus are empty.
    void checkWin() {
        if (!mGame.piles[0].cards.empty() || !mGame.piles[1].cards.empty())
            return;
        for (size_t i = 6; i < mGame.piles.size(); i++) {
            if (!mGame.piles[i].cards.empty())
                return;
        }
        win = true;
        // Update best (lowest) time and moves if needed.
        Uint32 timeTaken = (SDL_GetTicks() - mStartTime) / 1000;
        if (timeTaken < bestTime)
            bestTime = timeTaken;
        if (mGame.moveCount < bestMoves)
            bestMoves = mGame.moveCount;
    }
    
    // Find a hint move.
    bool findHint(int &hintPileIndex, int &hintCardIndex, int &destIndex) {
        // Check Waste.
        Pile &waste = mGame.piles[1];
        if (!waste.cards.empty()) {
            for (int i = 0; i < waste.cards.size(); i++) {
                if (waste.cards[i].faceUp) {
                    for (int j = 2; j < 6; j++) {
                        if (mGame.canPlaceOnFoundation(waste.cards[i], mGame.piles[j])) {
                            hintPileIndex = 1;
                            hintCardIndex = i;
                            destIndex = j;
                            return true;
                        }
                    }
                }
            }
        }
        // Check Tableaus.
        for (size_t i = 6; i < mGame.piles.size(); i++) {
            Pile &pile = mGame.piles[i];
            for (int k = 0; k < pile.cards.size(); k++) {
                if (pile.cards[k].faceUp) {
                    for (int j = 2; j < 6; j++) {
                        if (mGame.canPlaceOnFoundation(pile.cards[k], mGame.piles[j])) {
                            hintPileIndex = i;
                            hintCardIndex = k;
                            destIndex = j;
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }
    
    // Auto–complete function: processes only a single eligible move per call.
    void autoComplete() {
        // For each card in Waste and Tableaus, if it can be moved to foundation, animate the move.
        // For simplicity, only process one move per call.
        int hintPile, hintCard, dest;
        if (findHint(hintPile, hintCard, dest)) {
            int srcX, srcY;
            if (hintPile == 1) {
                srcX = mGame.piles[1].x;
                srcY = mGame.piles[1].y;
            } else {
                srcX = mGame.piles[hintPile].x;
                srcY = mGame.piles[hintPile].y + hintCard * CARD_SPACING_Y;
            }
            int destX = mGame.piles[dest].x;
            int destY = mGame.piles[dest].y;
            animateAutoMove(hintPile, hintCard, dest, srcX, srcY, destX, destY);
        }
    }
    
    void update() {
        if (state == PLAYING && !paused) {
            // Additional game logic (timers, win conditions, etc.) can be added here.
        }
    }
    
    void render() {
        if (state == MENU) {
            mCardRenderer.renderText(menuText, 300, 300);
            for (auto &button : mMenuButtons)
                button.render(mRenderer, mFont);
        } else if (state == SETTINGS) {
            mCardRenderer.renderText("Settings", 400, 200);
            mCardRenderer.renderText("High Score: " + std::to_string(highScore), 400, 250);
            mCardRenderer.renderText("Sound: " + std::string(mSoundManager.isSoundOn() ? "On" : "Off"), 400, 300);
            for (auto &button : mSettingsButtons)
                button.render(mRenderer, mFont);
        } else if (state == STATISTICS) {
            mCardRenderer.renderText("Statistics", 400, 200);
            mCardRenderer.renderText("Best Time: " + std::to_string(bestTime) + " sec", 400, 250);
            mCardRenderer.renderText("Fewest Moves: " + std::to_string(bestMoves), 400, 300);
            for (auto &button : mStatisticsButtons)
                button.render(mRenderer, mFont);
        } else if (state == PLAYING) {
            for (const auto &pile : mGame.piles) {
                SDL_Rect pileRect{ pile.x, pile.y, CARD_WIDTH, CARD_HEIGHT };
                SDL_SetRenderDrawColor(mRenderer, 50, 50, 50, 255);
                SDL_RenderDrawRect(mRenderer, &pileRect);
                int offset = (pile.type == TABLEAU) ? CARD_SPACING_Y : 0;
                for (size_t i = 0; i < pile.cards.size(); i++) {
                    int cardX = pile.x;
                    int cardY = pile.y + i * offset;
                    mCardRenderer.drawCard(cardX, cardY, pile.cards[i]);
                }
            }
            if (dragState.dragging) {
                int drawX = dragState.mouseX - dragState.offsetX;
                int drawY = dragState.mouseY - dragState.offsetY;
                for (size_t i = 0; i < dragState.draggedCards.size(); i++)
                    mCardRenderer.drawCard(drawX, drawY + i * CARD_SPACING_Y, dragState.draggedCards[i]);
            }
            updateAnimations(mCardRenderer);
            mCardRenderer.renderText("Score: " + std::to_string(mGame.score), 800, 10);
            mCardRenderer.renderText("Moves: " + std::to_string(mGame.moveCount), 800, 30);
            mCardRenderer.renderText("Time: " + std::to_string((SDL_GetTicks()-mStartTime)/1000) + " sec", 800, 50);
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
            if (hintActive) {
                Uint32 elapsed = SDL_GetTicks() - hintStartTime;
                if (elapsed < 2000) {
                    Pile &hintPileRef = mGame.piles[hintPileIndex];
                    int hx = hintPileRef.x;
                    int hy = hintPileRef.y;
                    if (hintPileRef.type == TABLEAU)
                        hy += hintCardIndex * CARD_SPACING_Y;
                    SDL_Rect hintRect = { hx - 2, hy - 2, CARD_WIDTH + 4, CARD_HEIGHT + 4 };
                    SDL_SetRenderDrawColor(mRenderer, 255, 0, 0, 255);
                    SDL_RenderDrawRect(mRenderer, &hintRect);
                } else {
                    hintActive = false;
                }
            }
            for (auto &button : mPlayingButtons)
                button.render(mRenderer, mFont);
        }
    }
    
    void handleEvent(SDL_Event &event) {
        const int tableauYOffset = CARD_SPACING_Y;
        if (state == MENU) {

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;
                for (auto &button : mMenuButtons) {
                    if (button.isClicked(mx, my)) {
                        button.onClick();
                        return;
                    }
                }
            }
        } else if (state == SETTINGS) {
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;
                for (auto &button : mSettingsButtons) {
                    if (button.isClicked(mx, my)) {
                        button.onClick();
                        return;
                    }
                }
            }
        } else if (state == STATISTICS) {
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;
                for (auto &button : mStatisticsButtons) {
                    if (button.isClicked(mx, my)) {
                        button.onClick();
                        return;
                    }
                }
            }
        } else if (state == PLAYING) {
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;
                for (auto &button : mPlayingButtons) {
                    if (button.isClicked(mx, my)) {
                        button.onClick();
                        return;
                    }
                }
            }
            switch(event.type) {
                case SDL_QUIT:
                    mQuit = true;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_p)
                        paused = !paused;
                    if (!paused) {
                        if (event.key.keysym.sym == SDLK_w) {
                            mGame.mode = (mGame.mode == RANDOM) ? WINNING : RANDOM;
                            startNewGame();
                        }
                        if (event.key.keysym.sym == SDLK_d) {
                            mDrawCount = (mDrawCount == 1) ? 3 : 1;
                        }
                        if (event.key.keysym.sym == SDLK_u) {
                            if (undoStack.size() > 1) {
                                undoStack.pop();
                                mGame = undoStack.top();
                            }
                        }
                        if (event.key.keysym.sym == SDLK_r) {
                            startNewGame();
                        }
                        if (event.key.keysym.sym == SDLK_h) {
                            int hp, hc, dest;
                            if (findHint(hp, hc, dest)) {
                                hintPileIndex = hp;
                                hintCardIndex = hc;
                                hintStartTime = SDL_GetTicks();
                                hintActive = true;
                            }
                        }
                        if (event.key.keysym.sym == SDLK_a) {
                            // 'A' key triggers auto–complete.
                            autoComplete();
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (paused) break;
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        int mx = event.button.x, my = event.button.y;
                        if (event.button.clicks > 1) {
                            // Check Waste.
                            Pile &waste = mGame.piles[1];
                            int cardIndex;
                            if (!waste.cards.empty() && pointInRect(mx, my, waste.x, waste.y, CARD_WIDTH, CARD_HEIGHT) &&
                                findCardAtPoint(waste, mx, my, cardIndex, 0))
                            {
                                if (waste.cards[cardIndex].faceUp) {
                                    int destIndex = -1;
                                    for (int i = 2; i < 6; i++) {
                                        if (mGame.canPlaceOnFoundation(waste.cards[cardIndex], mGame.piles[i])) {
                                            destIndex = i;
                                            break;
                                        }
                                    }
                                    if (destIndex != -1) {
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
                            for (int i = 6; i < mGame.piles.size(); i++) {
                                Pile &pile = mGame.piles[i];
                                int cardIndex;
                                if (!pile.cards.empty() && findCardAtPoint(pile, mx, my, cardIndex, tableauYOffset)) {
                                    if (pile.cards[cardIndex].faceUp) {
                                        int destIndex = -1;
                                        for (int j = 2; j < 6; j++) {
                                            if (mGame.canPlaceOnFoundation(pile.cards[cardIndex], mGame.piles[j])) {
                                                destIndex = j;
                                                break;
                                            }
                                        }
                                        if (destIndex != -1) {
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
                        if (pointInRect(mx, my, mGame.piles[0].x, mGame.piles[0].y, CARD_WIDTH, CARD_HEIGHT)) {
                            mGame.handleStockClick(mDrawCount);
                            undoStack.push(mGame);
                            return;
                        }
                        // Click on Waste for dragging.
                        Pile &waste = mGame.piles[1];
                        if (!waste.cards.empty() && pointInRect(mx, my, waste.x, waste.y, CARD_WIDTH, CARD_HEIGHT)) {
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
                        for (int i = 6; i < mGame.piles.size(); i++) {
                            Pile &pile = mGame.piles[i];
                            if (pile.type == TABLEAU && !pile.cards.empty()) {
                                int cardIndex = -1;
                                if (findCardAtPoint(pile, mx, my, cardIndex, tableauYOffset)) {
                                    if (pile.cards[cardIndex].faceUp) {
                                        dragState.dragging = true;
                                        dragState.draggedCards.clear();
                                        for (size_t k = cardIndex; k < pile.cards.size(); k++)
                                            dragState.draggedCards.push_back(pile.cards[k]);
                                        pile.cards.erase(pile.cards.begin() + cardIndex, pile.cards.end());
                                        if (!pile.cards.empty() && !pile.cards.back().faceUp)
                                            pile.cards.back().faceUp = true;
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
                    if (dragState.dragging) {
                        dragState.mouseX = event.motion.x;
                        dragState.mouseY = event.motion.y;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT && dragState.dragging) {
                        int mx = event.button.x, my = event.button.y;
                        bool placed = false;
                        for (int i = 6; i < mGame.piles.size(); i++) {
                            Pile &pile = mGame.piles[i];
                            if (pile.type == TABLEAU) {
                                int dropX = pile.x;
                                int dropY = pile.y + pile.cards.size() * tableauYOffset;
                                if (pointInRect(mx, my, dropX, dropY, CARD_WIDTH, CARD_HEIGHT)) {
                                    for (auto &card : dragState.draggedCards)
                                        pile.cards.push_back(card);
                                    placed = true;
                                    break;
                                }
                            }
                        }
                        if (!placed) {
                            mGame.piles[dragState.originPileIndex].cards.insert(
                                mGame.piles[dragState.originPileIndex].cards.end(),
                                dragState.draggedCards.begin(), dragState.draggedCards.end());
                        }
                        dragState.dragging = false;
                        undoStack.push(mGame);
                        mSoundManager.playMoveSound();
                    }
                    break;
                default:
                    break;
            }
        }
    }
    
    bool quit() const { return mQuit; }
    
private:
    void startNewGame() {
        
        mGame.score = 0;
        mGame.initializeDeck();
        mGame.setupPiles();
        while(!undoStack.empty()) undoStack.pop();
        undoStack.push(mGame);
        // Update high score if current score is lower.
        Uint32 timeTaken = (SDL_GetTicks() - mStartTime) / 1000;
        if (timeTaken < highScore)
            highScore = timeTaken;
        mStartTime = SDL_GetTicks();
        mDrawCount = 1;
        mGame.moveCount = 0;
        paused = false;
        win = false;
        hintActive = false;
    }
    
    void setupMenuButtons() {
        mMenuButtons.clear();
        mMenuButtons.push_back(Button(400, 400, 200, 50, "Start Game", [this]() {
            startNewGame();
            state = PLAYING;
        }));
        mMenuButtons.push_back(Button(400, 470, 200, 50, "Settings", [this]() {
            state = SETTINGS;
        }));
        mMenuButtons.push_back(Button(400, 540, 200, 50, "Statistics", [this]() {
            state = STATISTICS;
        }));
        mMenuButtons.push_back(Button(400, 610, 200, 50, "Quit", [this]() {
            mQuit = true;
        }));
    }
    
    void setupSettingsButtons() {
        mSettingsButtons.clear();
        mSettingsButtons.push_back(Button(400, 400, 200, 50, "Toggle Sound", [this]() {
            mSoundManager.toggleSound();
        }));
        mSettingsButtons.push_back(Button(400, 470, 200, 50, "Back", [this]() {
            state = MENU;
        }));
    }
    
    void setupStatisticsButtons() {
        mStatisticsButtons.clear();
        mStatisticsButtons.push_back(Button(400, 400, 200, 50, "Reset Stats", [this]() {
            highScore = 999999;
            bestTime = 999999;
            bestMoves = 999999;
        }));
        mStatisticsButtons.push_back(Button(400, 470, 200, 50, "Back", [this]() {
            state = MENU;
        }));
    }
    
    void setupPlayingButtons() {
        mPlayingButtons.clear();
        mPlayingButtons.push_back(Button(800, 150, 150, 40, "Restart", [this]() {
            startNewGame();
        }));
        mPlayingButtons.push_back(Button(800, 200, 150, 40, "Undo", [this]() {
            if (undoStack.size() > 1) {
                undoStack.pop();
                mGame = undoStack.top();
            }
        }));
        mPlayingButtons.push_back(Button(800, 250, 150, 40, "Toggle Draw", [this]() {
            mDrawCount = (mDrawCount == 1) ? 3 : 1;
        }));
        mPlayingButtons.push_back(Button(800, 300, 150, 40, "Pause/Resume", [this]() {
            paused = !paused;
        }));
        mPlayingButtons.push_back(Button(800, 350, 150, 40, "Hint", [this]() {
            int hp, hc, dest;
            if (findHint(hp, hc, dest)) {
                hintPileIndex = hp;
                hintCardIndex = hc;
                hintStartTime = SDL_GetTicks();
                hintActive = true;
            }
        }));
        mPlayingButtons.push_back(Button(800, 400, 150, 40, "Auto-Complete", [this]() {
            autoComplete();
        }));
    }
    

    
    SDL_Renderer* mRenderer;
    TTF_Font* mFont;
    CardRenderer mCardRenderer;
    SoundManager mSoundManager;
    Game mGame;
    Uint32 mStartTime = SDL_GetTicks();
    int mDrawCount = 1;
    GameState state;
    std::string menuText;
    std::vector<Button> mMenuButtons;
    std::vector<Button> mSettingsButtons;
    std::vector<Button> mStatisticsButtons;
    std::vector<Button> mPlayingButtons;
    std::stack<Game> undoStack;
    bool mQuit = false;
    bool paused = false;
    int highScore;
    // Statistics for best time and moves.
    int bestTime = 999999;
    int bestMoves = 999999;
    
    // Hint variables.
    bool hintActive;
    int hintPileIndex;
    int hintCardIndex;
    Uint32 hintStartTime;
};

// ---------------------------
// Main Function
// ---------------------------
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }
    if (TTF_Init() < 0) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << "\n";
        SDL_Quit();
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "IMG_Init failed: " << IMG_GetError() << "\n";
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("Enhanced Solitaire", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << "\n";
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    TTF_Font* font = TTF_OpenFont(FONT_FILE, FONT_SIZE);
    if (!font) {
        std::cerr << "Failed to load font: " << FONT_FILE << " Error: " << TTF_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    GameEngine engine(renderer, font);
    SDL_Event event;
    while (!engine.quit()) {
        while (SDL_PollEvent(&event)) {
            engine.handleEvent(event);
        }
        engine.update();
        SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
        SDL_RenderClear(renderer);
        engine.render();
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
