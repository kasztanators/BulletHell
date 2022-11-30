#include "rect.h"
#include <SDL_image.h>
#include<stdio.h>
Rect::Rect(int w, int h, int x, int y, int r, int g, int b, int a) :
	_w(w), _h(h), _x(x), _y(y), _r(r), _g(g), _b(b), _a(a)
{
}
Rect::Rect(int w, int h, int x, int y, const char path[]) :
	_w(w), _h(h), _x(x), _y(y)
{
	auto surface = IMG_Load(path);
	if (!surface) {
		printf("Failed to create surface.\n");
	}
	_pikachu_texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!_pikachu_texture) {
		printf("Failed to create texture.\n");
	}
	SDL_FreeSurface(surface);

}
Rect::~Rect() {
	SDL_DestroyTexture(_pikachu_texture);
}
void Rect::draw()const {
	SDL_Rect rect = { _x, _y, _w, _h };
	if (_pikachu_texture) {
		SDL_RenderCopy(renderer, _pikachu_texture, nullptr, &rect);
	}
	else {
		SDL_SetRenderDrawColor(renderer, _r, _g, _b, _a);
		SDL_RenderFillRect(renderer, &rect);
	}
}
void Rect::pollEvents(SDL_Event& event) {
	if (event.type == SDL_KEYDOWN) {
		switch (event.key.keysym.sym) {
		case SDLK_LEFT:
			_x -= 10;
			break;
		case SDLK_RIGHT:
			_x += 10;
			break;
		case SDLK_UP:
			_y -= 10;
			break;
		case SDLK_DOWN:
			_y += 10;
			break;

		}
	}