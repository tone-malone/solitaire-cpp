// src/Game.cpp
#include "../include/Game.h"
#include "../include/Utility.h"
#include <algorithm>
#include <random>

Game::Game():mode(RANDOM),score(0),moveCount(0){}

void Game::initializeDeck(){
  deck.clear();
  for(int s=0;s<4;++s)
    for(int v=1;v<=13;++v)
      deck.push_back({v,s,false});
  if(mode==RANDOM){
    std::mt19937 g(std::random_device{}());
    std::shuffle(deck.begin(),deck.end(),g);
  } else {
    std::sort(deck.begin(),deck.end(),[](auto&a,auto&b){
      return (a.suit==b.suit)?(a.value<b.value):(a.suit<b.suit);
    });
  }
}

void Game::setupPiles(){
  piles.clear();
  piles.push_back({STOCK,50,50,{}});
  piles.push_back({WASTE,130,50,{}});
  for(int i=0;i<4;++i)
    piles.push_back({FOUNDATION,400+i*(CARD_WIDTH+20),50,{}});
  for(int i=0;i<7;++i)
    piles.push_back({TABLEAU,50+i*(CARD_WIDTH+20),200,{}});
  int idx=0;
  for(int i=0;i<7;++i){
    for(int j=0;j<=i;++j){
      Card c=deck[idx++]; c.faceUp=(j==i);
      piles[6+i].cards.push_back(c);
    }
  }
  while(idx<(int)deck.size()){
    Card c=deck[idx++]; c.faceUp=false;
    piles[0].cards.push_back(c);
  }
  moveCount=0;
}

bool Game::canPlaceOnFoundation(const Card& c,const Pile& f) const {
  if(f.cards.empty()) return c.value==1;
  auto& top=f.cards.back();
  return c.suit==top.suit && c.value==top.value+1;
}

bool Game::moveCardToFoundation(int sp,int ci){
  Card c=piles[sp].cards[ci];
  for(int i=2;i<6;++i){
    if(canPlaceOnFoundation(c,piles[i])){
      piles[i].cards.push_back(c);
      piles[sp].cards.erase(piles[sp].cards.begin()+ci);
      score+=10; moveCount++;
      return true;
    }
  }
  return false;
}

void Game::handleStockClick(int drawCount){
  auto& stock=piles[0];
  auto& waste=piles[1];
  if(!stock.cards.empty()){
    for(int i=0;i<drawCount&& !stock.cards.empty();++i){
      Card c=stock.cards.back(); stock.cards.pop_back();
      c.faceUp=true; waste.cards.push_back(c);
    }
    moveCount++;
  } else if(!waste.cards.empty()){
    while(!waste.cards.empty()){
      Card c=waste.cards.back(); waste.cards.pop_back();
      c.faceUp=false; stock.cards.push_back(c);
    }
  }
}
