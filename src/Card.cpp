// src/Card.cpp
#include "../include/Card.h"

std::string cardValueToString(const Card& card) {
    switch(card.value) {
        case 1:  return "A";
        case 11: return "J";
        case 12: return "Q";
        case 13: return "K";
        default: return std::to_string(card.value);
    }
}
