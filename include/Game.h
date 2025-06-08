// include/Game.h
#pragma once

#include <vector>
#include "Card.h"
#include "Constants.h"

// Core solitaire logic
class Game {
public:
    Game();
    Mode mode;
    int score, moveCount;
    std::vector<Card> deck;
    std::vector<Pile> piles;

    void initializeDeck();
    void setupPiles();
    bool canPlaceOnFoundation(const Card& c,const Pile& f) const;
    bool moveCardToFoundation(int fromPile,int cardIdx);
    void handleStockClick(int drawCount);
};
