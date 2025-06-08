// include/Animation.h
#pragma once

#include <vector>
#include <functional>
#include <SDL2/SDL.h>
#include "Card.h"

enum class AnimType { MoveCard, FlipCard };

struct Animation {
    AnimType type;
    Card card;
    int srcX, srcY, dstX, dstY;
    Uint32 startTime, duration;
    std::function<void()> onComplete;
};

// Active animations
extern std::vector<Animation> animations;


// Enqueue a move‐card animation
void animateCardMove(const Card& c,
                     int fromX,int fromY,
                     int toX,int toY,
                     Uint32 ms,
                     std::function<void()> commit);

// Render & advance animations
void updateAnimations(class CardRenderer& renderer);
