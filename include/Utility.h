// include/Utility.h
#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include "Card.h"

// Interpolation & easing
int   lerp(int start, int end, float t);
float easeOutQuad(float t);

// Geometry hit‐tests
bool pointInRect(int px,int py,int rx,int ry,int rw,int rh);

// Tableau rules
bool isRedSuit(int suit);
bool canPlaceOnTableau(const Card& c, const Pile& p);
bool canMoveSequence(const std::vector<Card>& seq, const Pile& p);

// Hit‐testing a pile
bool findCardAtPoint(const Pile& p, int mx,int my,int& cardIndex,int pileYOffset);
