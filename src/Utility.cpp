// src/Utility.cpp
#include "../include/Utility.h"
#include "../include/Constants.h"
#include <algorithm>

int lerp(int s,int e,float t){ return s + int(t*(e-s)); }
float easeOutQuad(float t){ return 1.f-(1.f-t)*(1.f-t); }
bool pointInRect(int px,int py,int rx,int ry,int rw,int rh){
  return px>=rx&&px<=rx+rw&&py>=ry&&py<=ry+rh;
}
bool isRedSuit(int suit)
{
    return suit == 1 || suit == 2; // hearts or diamonds
}

bool canPlaceOnTableau(const Card& c,const Pile& p){
  if(p.cards.empty()) return c.value==13;
  const Card& top=p.cards.back();
  return (isRedSuit(c.suit)!=isRedSuit(top.suit))&&(c.value==top.value-1);
}

bool canMoveSequence(const std::vector<Card>& seq,const Pile& p){
  for(size_t i=1;i<seq.size();++i){
    if(!(isRedSuit(seq[i-1].suit)!=isRedSuit(seq[i].suit)
       && seq[i-1].value==seq[i].value+1)) return false;
  }
  return canPlaceOnTableau(seq.front(),p);
}

bool findCardAtPoint(const Pile& p,int mx,int my,int& idx,int pileYOffset){
  for(int i=int(p.cards.size())-1;i>=0;--i){
    int x=p.x, y=p.y + i*pileYOffset;
    if(pointInRect(mx,my,x,y,CARD_WIDTH,CARD_HEIGHT)){
      idx=i; return true;
    }
  }
  return false;
}
