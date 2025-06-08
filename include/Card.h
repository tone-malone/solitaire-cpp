// include/Card.h
#pragma once

#include <string>
#include <vector>

// Game modes
enum Mode { RANDOM, WINNING };
// Pile types
enum PileType { STOCK, WASTE, TABLEAU, FOUNDATION };
// Overall UI/game state
enum GameState { MENU, PLAYING, PAUSED, SETTINGS, STATISTICS };

// Single card
struct Card {
    int value; // 1–13
    int suit;  // 0=♠,1=♥,2=♦,3=♣
    bool faceUp;
};

// A pile of cards
struct Pile {
    PileType type;
    int x, y;
    std::vector<Card> cards;
};

// Convert 1→"A", 11→"J", etc.
std::string cardValueToString(const Card& card);
