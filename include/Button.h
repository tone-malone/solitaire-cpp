// include/Button.h
#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <functional>

enum class ButtonState { Normal, Hovered, Pressed };
class Button {
public:
    Button(int x,int y,int w,int h,
           const std::string& label,
           std::function<void()> callback);

    void update(int mouseX,int mouseY,bool mouseDown);
    void render(SDL_Renderer* renderer, TTF_Font* font);
    bool isClicked(int x,int y) const;
    void onClick();

private:
    SDL_Rect rect;
    std::string label;
    std::function<void()> callback;
    ButtonState state = ButtonState::Normal;
};
