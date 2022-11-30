#define _CRT_SECURE_NO_WARNINGS
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<SDL.h>
#define FPS 60
#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	600
#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 50
#define HEALTH_X 700
#define HEALTH_Y 300
#define LOBBY_WIDTH 200
#define PLAYER_X 300
#define PLAYER_Y 200
#define LEVEL_WIDTH 1400
#define LEVEL_HEIGHT 1200
#define PI 3.142857
struct Things {
	SDL_Surface* screen=NULL, * charset=NULL, * player = NULL, * health_img = NULL, *game_over=NULL,*win=NULL;
	SDL_Surface* first_enemy = NULL, * bullet = NULL,* second_enemy=NULL,*third_enemy=NULL,*shot_bmp=NULL,*grade=NULL;
	SDL_Texture* scrtex=NULL;
	SDL_Window* window=NULL;
	SDL_Renderer* renderer=NULL;
};
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};
// draw a surface sprite on a surface screen in point (x, y)
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;// (x, y) is the center of sprite on screen
	dest.y = y - sprite->w / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};
// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};
// draw a rectangle of size l by k
void DrawRectangle(struct Things* SDL_Things, int x, int y, int l, int k,int fillColor) {
	int outlineColor = SDL_MapRGB(SDL_Things->screen->format, 220, 20, 60);
	int i;
	DrawLine(SDL_Things->screen, x, y, k, 0, 1, outlineColor);
	DrawLine(SDL_Things->screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(SDL_Things->screen, x, y, l, 1, 0, outlineColor);
	DrawLine(SDL_Things->screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(SDL_Things->screen, x + 1, i, l - 2, 1, 0, fillColor);
};
//moving player acroos the board
void MovingAcrossBoard(SDL_Surface* screen, int* x, int* y, int* player_x_ptr, int* player_y_ptr, int xBuff, int yBuff) {
	int l = LEVEL_WIDTH, k = LEVEL_HEIGHT;
	int outlineColor = SDL_MapRGB(screen->format, 220, 20, 60);
	if (*player_x_ptr < PLAYER_X)*player_x_ptr += xBuff;
	if (*player_y_ptr < PLAYER_Y)*player_y_ptr += yBuff;
	if (*player_x_ptr > PLAYER_X)*player_x_ptr += xBuff;
	if (*player_y_ptr > PLAYER_Y)*player_y_ptr += yBuff;
	if (*y < PLAYER_Y) {
		*player_y_ptr = *y + PLAYER_HEIGHT / 2;
		if (*y < 0)*y = 0;
		if (*player_y_ptr < PLAYER_WIDTH / 2) { *player_y_ptr = PLAYER_WIDTH / 2; }
	}
	if (*x < PLAYER_X) {
		*player_x_ptr = *x+PLAYER_WIDTH/2;
		if (*x < 0)*x = 0;
		if (*player_x_ptr < PLAYER_HEIGHT / 2) { *player_x_ptr = PLAYER_HEIGHT / 2; }
	}
	if (*y > k - (SCREEN_HEIGHT - PLAYER_Y)) {
		*player_y_ptr = *y - SCREEN_HEIGHT - PLAYER_HEIGHT / 2;
		if (*y > k)*y = k ;
		if (*player_y_ptr > SCREEN_HEIGHT - PLAYER_HEIGHT / 2) { *player_y_ptr = SCREEN_HEIGHT - PLAYER_HEIGHT / 2; }
		if (*player_y_ptr < PLAYER_Y) { *player_y_ptr = PLAYER_Y; }
	}
	if (*x > l-PLAYER_X) {
		*player_x_ptr = *x-SCREEN_WIDTH;
		if (*x > l)*x = l;
		if (*player_x_ptr > SCREEN_WIDTH - PLAYER_HEIGHT / 2 - LOBBY_WIDTH) { *player_x_ptr = SCREEN_WIDTH  - LOBBY_WIDTH-PLAYER_WIDTH/2; }
	}
	if (*x <= PLAYER_X+SCREEN_WIDTH && *x >PLAYER_X-PLAYER_WIDTH/2) {*player_x_ptr = PLAYER_X;}
	if (*y <= SCREEN_HEIGHT-PLAYER_Y &&*y>=PLAYER_Y - PLAYER_HEIGHT / 2) {*player_y_ptr = PLAYER_Y;}
	DrawLine(screen, 0, 0, k, 0, 1, outlineColor);//vertical line
	DrawLine(screen, l, 0, k, 0, 1, outlineColor);
	DrawLine(screen, 0, 0, l, 1, 0, outlineColor);//horizontal
	DrawLine(screen, 0, k - 1, l, 1, 0, outlineColor);
};
//translates coordinates to view them properly (calculator for coordinates)
void translation(int x, int y, int *object_x,int *object_y ) {
	if (y <= PLAYER_Y)*object_y = *object_y;
	if (y >= SCREEN_HEIGHT + PLAYER_Y)*object_y = *object_y - SCREEN_HEIGHT;
	if (y<SCREEN_HEIGHT + PLAYER_Y && y>PLAYER_Y)*object_y = *object_y + PLAYER_Y - y;
	if (x <= PLAYER_X)*object_x = *object_x + PLAYER_X - x;
	if (x<SCREEN_WIDTH + PLAYER_X && x>PLAYER_X)*object_x = *object_x + PLAYER_X - x;
	if (x >= SCREEN_WIDTH + PLAYER_X)*object_x = *object_x-x+PLAYER_X;
}
//polling event is for taking users input from the keayboard and closing with mouse
void pollEventsRect(int* quit, int* new_game_ptr, int* x_buff, int* y_buff) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) { *quit = 1; *new_game_ptr = 1; }
			else if (event.key.keysym.sym == SDLK_UP)  *y_buff = -5;
			else if (event.key.keysym.sym == SDLK_DOWN)*y_buff = +5;
			else if (event.key.keysym.sym == SDLK_LEFT)*x_buff = -5;
			else if (event.key.keysym.sym == SDLK_RIGHT)*x_buff = +5;
			else if (event.key.keysym.sym == SDLK_n) *quit = 1;
			break;
		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_UP)  *y_buff = 0;
			else if (event.key.keysym.sym == SDLK_DOWN)*y_buff = 0;
			else if (event.key.keysym.sym == SDLK_LEFT)*x_buff = 0;
			else if (event.key.keysym.sym == SDLK_RIGHT)*x_buff = 0;
			break;
		case SDL_QUIT:
			*quit = 1;
			*new_game_ptr = 1;
			break;
		}
	}
}
//drawing player onto the board
void drawPlayer(struct Things* SDL_Things, int x, int y, int* quit_ptr, int* new_game_ptr,int immunity) {
	SDL_Things->player = SDL_LoadBMP("./eti.bmp");
	if (SDL_Things->player == NULL) {
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		*quit_ptr = 1;
		*new_game_ptr = 1;
	};
	SDL_SetColorKey(SDL_Things->player, true, 0x000000);
	if(immunity%10==0)DrawSurface(SDL_Things->screen, SDL_Things->player, x, y);
}
typedef struct {
	int xBull;
	int yBull;
	int health;
	int pattern;
}bullet;
typedef struct {
	bullet* bb;
	int count;
	int max_count;
}bullet_array;
bullet_array* bullet_array_ptr;
//alocation pointer structure
bullet_array* bulletArrayInit(bullet_array* pointer, int size) {
	if (pointer == NULL) {
		pointer = (bullet_array*)malloc(sizeof(bullet_array));
		memset(pointer, 0, sizeof(bullet_array));
	}
	if (pointer->bb) {
		free(pointer->bb);
	}
	if (size > 0) {
		pointer->bb = (bullet*)malloc(sizeof(bullet) * size);
	}
	pointer->count = 0;
	pointer->max_count = size;
	return pointer;
}
//adding enemy bullet and givinf basic pattern and 
void addBullet(bullet_array* bullet_ptr, int x, int y, int remainder) {
	int first_pattern_time = 400,second_pattern_time = 500;
	if (bullet_ptr->count == bullet_ptr->max_count) {
		bullet* new_bb = (bullet*)malloc(sizeof(bullet) * bullet_ptr->max_count * 2);
		memcpy(new_bb, bullet_ptr->bb, sizeof(bullet) * bullet_ptr->max_count);
		free(bullet_ptr->bb);
		bullet_ptr->bb = new_bb;
		bullet_ptr->max_count = bullet_ptr->max_count * 2;
	}
	bullet_ptr->bb[bullet_ptr->count].xBull = x;
	bullet_ptr->bb[bullet_ptr->count].yBull = y;
	bullet_ptr->bb[bullet_ptr->count].health = 1;
	if(remainder <= first_pattern_time)bullet_ptr->bb[bullet_ptr->count].pattern = 1;
	else if(remainder>first_pattern_time&&remainder<second_pattern_time){bullet_ptr->bb[bullet_ptr->count].pattern = 2;}
	else bullet_ptr->bb[bullet_ptr->count].pattern =3;
	bullet_ptr->count += 1;
}
typedef struct {
	int xBull;
	int yBull;
	int health;
}shot;
typedef struct {
	shot* ss;
	int count;
	int max_count;
}shooting_array;
shooting_array* shooting_array_ptr;
///alocating pointer of structures
shooting_array* shootingArrayInit(shooting_array* pointer, int size) {
	if (pointer == NULL) {
		pointer = (shooting_array*)malloc(sizeof(shooting_array));
		memset(pointer, 0, sizeof(shooting_array));
	}
	if (pointer->ss) {
		free(pointer->ss);
	}
	if (size > 0) {
		pointer->ss = (shot*)malloc(sizeof(shot) * size);
	}
	pointer->count = 0;
	pointer->max_count = size;
	return pointer;
}
//adding shots to the struct and giving them basic values next functions have similar structure but diffrent patterns
void addShots(shooting_array* shooting_ptr, int x, int y) {
	if (shooting_ptr->count == shooting_ptr->max_count) {
		shot* new_ss = (shot*)malloc(sizeof(shot) * shooting_ptr->max_count * 2);
		memcpy(new_ss, shooting_ptr->ss, sizeof(shot) * shooting_ptr->max_count);
		free(shooting_ptr->ss);
		shooting_ptr->ss = new_ss;
		shooting_ptr->max_count = shooting_ptr->max_count * 2;
	}
	shooting_ptr->ss[shooting_ptr->count].xBull = x;
	shooting_ptr->ss[shooting_ptr->count].yBull = y;
	shooting_ptr->ss[shooting_ptr->count].health = 1;
	shooting_ptr->count += 1;
}
void drawFirstBullet(SDL_Surface* bullet, SDL_Surface* screen, int* quit_ptr, int* new_game_ptr,int distance, int move_x, int move_y,int enemy_x, int enemy_y) {
	int bullet_vel = 6, bullets_per_round = 3, start_angle = 0, end_angle = 360;
	float angle_step = (end_angle - start_angle) / bullets_per_round, angle = start_angle;
	bullet = SDL_LoadBMP("./pokeball.bmp");
	if (bullet == NULL) {
		printf("SDL_LoadBMP(pokeball.bmp) error: %s\n", SDL_GetError());
		*quit_ptr = 1;
		*new_game_ptr = 1;
	};
	int remainder = distance % 1000;
	SDL_SetColorKey(bullet, true, 0x000000);
	if (distance % 30 == 0) {
		for (int i = 0; i < bullets_per_round; i++)
			addBullet(bullet_array_ptr, enemy_x, enemy_y, remainder);
	}
	for (int i = 0; i < bullet_array_ptr->count; i++) {
		if (bullet_array_ptr->bb[i].pattern == 1) {//first pattern
			if (i % bullets_per_round == 0)bullet_array_ptr->bb[i].yBull -= bullet_vel;
			if (i % bullets_per_round == 1) { bullet_array_ptr->bb[i].yBull -= bullet_vel*2; bullet_array_ptr->bb[i].xBull -= (bullet_vel / 2+ sin(distance/4 ) *bullet_vel); }
			if (i % bullets_per_round == 2) { bullet_array_ptr->bb[i].yBull -= bullet_vel*2; bullet_array_ptr->bb[i].xBull += (bullet_vel / 2 + sin(distance/4) *bullet_vel); }
		}
		else if (bullet_array_ptr->bb[i].pattern == 2) {
			bullet_vel = 10;//second pattern
			if (i % bullets_per_round == 0)bullet_array_ptr->bb[i].yBull -= bullet_vel;
			if (i % bullets_per_round == 1) { bullet_array_ptr->bb[i].yBull -= bullet_vel; bullet_array_ptr->bb[i].xBull -= bullet_vel / 4; }
			if (i % bullets_per_round == 2) { bullet_array_ptr->bb[i].yBull -= bullet_vel; bullet_array_ptr->bb[i].xBull += bullet_vel / 4; }
		}
		else if (bullet_array_ptr->bb[i].pattern == 3) {
			bullet_vel = 10;//third pattern
			if (i % bullets_per_round == 0)bullet_array_ptr->bb[i].yBull -= bullet_vel, bullet_array_ptr->bb[i].xBull+=sin(distance/5)*bullet_vel*2;
			if (i % bullets_per_round == 1) { bullet_array_ptr->bb[i].yBull -= bullet_vel; bullet_array_ptr->bb[i].xBull -= bullet_vel / 4; }
			if (i % bullets_per_round == 2) { bullet_array_ptr->bb[i].yBull -= bullet_vel; bullet_array_ptr->bb[i].xBull += bullet_vel / 4; }
		}

		int bullet_x = bullet_array_ptr->bb[i].xBull;
		int bullet_y = bullet_array_ptr->bb[i].yBull;
		int* bullet_x_ptr = &bullet_x, * bullet_y_ptr = &bullet_y;
		if (bullet_array_ptr->bb[i].health == 1) {
			translation(move_x, move_y, bullet_x_ptr, bullet_y_ptr);
			DrawSurface(screen, bullet, bullet_x, bullet_y);
		}
		angle += angle_step;
	}
}
void drawFirstEnemy(SDL_Surface* first_enemy, SDL_Surface* screen, int* quit_ptr, int* new_game_ptr,int x,int y, int*enemy_x_ptr, int*enemy_y_ptr) {
	translation(x, y, enemy_x_ptr, enemy_y_ptr);
	first_enemy = SDL_LoadBMP("./enemy1.bmp");
	if (first_enemy == NULL) {
		printf("SDL_LoadBMP(enemy1.bmp) error: %s\n", SDL_GetError());
		*quit_ptr = 1;
		*new_game_ptr = 1;
	};
	SDL_SetColorKey(first_enemy, true, 0x000000);
	DrawSurface(screen, first_enemy, *enemy_x_ptr, *enemy_y_ptr);
}
void drawSecondBullet(SDL_Surface* bullet, SDL_Surface* screen, int* quit_ptr, int* new_game_ptr,int distance, int move_x, int move_y,int enemy_x, int enemy_y) {
	int bullet_vel = 6, bullets_per_round = 20, start_angle = 0, end_angle = 360,per_sec=30;
	float angle_step = (end_angle - start_angle) / bullets_per_round, angle = start_angle;
	bullet = SDL_LoadBMP("./pokeball.bmp");
	if (bullet == NULL) {
		printf("SDL_LoadBMP(pokeball.bmp) error: %s\n", SDL_GetError());
		*quit_ptr = 1;
		*new_game_ptr = 1;
	};
	int remainder = distance % 1000;
	SDL_SetColorKey(bullet, true, 0x000000);
	if (distance % per_sec == 0) {
		for (int i = 0; i < bullets_per_round; i++)
			addBullet(bullet_array_ptr, enemy_x, enemy_y, remainder);
	}
	for (int i = 0; i < bullet_array_ptr->count; i++) {
		if (bullet_array_ptr->bb[i].pattern == 1) {
			bullet_array_ptr->bb[i].yBull -= cos((angle * PI) / 180) * bullet_vel;
			bullet_array_ptr->bb[i].xBull += sin((angle * PI) / 180) * bullet_vel;
		}
		else if (bullet_array_ptr->bb[i].pattern == 2) {
			bullet_vel = 5;
			if (i % bullets_per_round== 0)bullet_array_ptr->bb[i].yBull -= bullet_vel;
			if (i % bullets_per_round > 0 && i % bullets_per_round < 10) { bullet_array_ptr->bb[i].yBull -= bullet_vel; bullet_array_ptr->bb[i].xBull -= (i % bullets_per_round)*2; }
			if (i % bullets_per_round >= 10 && i % bullets_per_round < 20) { bullet_array_ptr->bb[i].yBull -= bullet_vel; bullet_array_ptr->bb[i].xBull += ((i-10) % bullets_per_round)*2; }
		}	else if (bullet_array_ptr->bb[i].pattern == 3) {
			bullet_vel = 5;
			if (i % bullets_per_round== 0)bullet_array_ptr->bb[i].yBull -= bullet_vel;
			if (i % bullets_per_round > 0 && i % bullets_per_round < 10) { bullet_array_ptr->bb[i].yBull -= bullet_vel; bullet_array_ptr->bb[i].xBull -= (i % bullets_per_round); }
			if (i % bullets_per_round >= 10 && i % bullets_per_round < 20) { bullet_array_ptr->bb[i].yBull -= bullet_vel; bullet_array_ptr->bb[i].xBull += ((i-10) % bullets_per_round); }
		}
		int bullet_x = bullet_array_ptr->bb[i].xBull;
		int bullet_y = bullet_array_ptr->bb[i].yBull;
		int* bullet_x_ptr = &bullet_x, * bullet_y_ptr = &bullet_y;
		if (bullet_array_ptr->bb[i].health == 1) {
			translation(move_x, move_y, bullet_x_ptr, bullet_y_ptr);
			DrawSurface(screen, bullet, bullet_x, bullet_y);
		}
		angle += angle_step;
	}
}
void drawSecondEnemy(SDL_Surface* second_enemy, SDL_Surface* screen, int* quit_ptr, int* new_game_ptr, int x, int y, int* enemy_x_ptr, int* enemy_y_ptr) {
	translation(x, y, enemy_x_ptr, enemy_y_ptr);
	second_enemy = SDL_LoadBMP("./enemy2.bmp");
	if (second_enemy == NULL) {
		printf("SDL_LoadBMP(enemy2.bmp) error: %s\n", SDL_GetError());
		*quit_ptr = 1;
		*new_game_ptr = 1;
	};
	SDL_SetColorKey(second_enemy, true, 0x000000);
	DrawSurface(screen,second_enemy, *enemy_x_ptr, *enemy_y_ptr);
}
//drawing and adding pattern for 3rd enemy
void drawThirdBullet(SDL_Surface* bullet, SDL_Surface* screen, int* quit_ptr, int* new_game_ptr,int distance, int move_x, int move_y,int enemy_x, int enemy_y) {
	int bullet_vel = 6, bullets_per_round =12, start_angle = 0, end_angle=360;
	float angle_step = (end_angle - start_angle) / bullets_per_round, angle = start_angle;
	bullet = SDL_LoadBMP("./pokeball.bmp");
	if (bullet == NULL) {
		printf("SDL_LoadBMP(pokeball.bmp) error: %s\n", SDL_GetError());
		*quit_ptr = 1;
		*new_game_ptr = 1;
	};
	int remainder=distance%1000;
	SDL_SetColorKey(bullet, true, 0x000000);
	if (distance%30==0) { 
		for(int i=0; i< bullets_per_round; i++)
		addBullet(bullet_array_ptr, enemy_x,enemy_y, remainder);
	}
	for (int i = 0; i < bullet_array_ptr->count; i++) {
		if (bullet_array_ptr->bb[i].pattern==1) {
			bullet_array_ptr->bb[i].yBull -= cos((angle * PI) / 180) * bullet_vel;
			bullet_array_ptr->bb[i].xBull += sin((angle * PI) / 180) * bullet_vel;
		}
		else if(bullet_array_ptr->bb[i].pattern == 2) {
			if (i % bullets_per_round >=0&& i % bullets_per_round<5)bullet_array_ptr->bb[i].yBull -= bullet_vel;
			if (i % bullets_per_round >= 5 && i % bullets_per_round < 10) { bullet_array_ptr->bb[i].yBull -= bullet_vel; bullet_array_ptr->bb[i].xBull -= bullet_vel / 2; }
			if (i % bullets_per_round >= 10 && i % bullets_per_round < 15) { bullet_array_ptr->bb[i].yBull -= bullet_vel; bullet_array_ptr->bb[i].xBull += bullet_vel / 2; }
		}else if(bullet_array_ptr->bb[i].pattern == 3) {
			bullet_array_ptr->bb[i].yBull -= cos((angle * PI) / 180) * bullet_vel*2;
			bullet_array_ptr->bb[i].xBull += sin((angle * PI) / 180) * bullet_vel*2;
		}
	int bullet_x = bullet_array_ptr->bb[i].xBull;
	int bullet_y = bullet_array_ptr->bb[i].yBull;
	int* bullet_x_ptr = &bullet_x, *bullet_y_ptr = &bullet_y;
	if (bullet_array_ptr->bb[i].health == 1) {
	translation(move_x,move_y, bullet_x_ptr, bullet_y_ptr);
	DrawSurface(screen, bullet, bullet_x, bullet_y);
	    }
	angle += angle_step;
	}
}
//drawing and moving third enemy
void drawThirdEnemy(SDL_Surface* third_enemy, SDL_Surface* screen, int* quit_ptr, int* new_game_ptr, int x, int y, int* enemy_x_ptr, int* enemy_y_ptr) {
	translation(x, y, enemy_x_ptr, enemy_y_ptr);
	int enemy_y = *enemy_y_ptr,enemy_x = *enemy_x_ptr;
	third_enemy = SDL_LoadBMP("./enemy3.bmp");
	if (third_enemy == NULL) {
		printf("SDL_LoadBMP(enemy3.bmp) error: %s\n", SDL_GetError());
		*quit_ptr = 1;
		*new_game_ptr = 1;
	};
	
	SDL_SetColorKey(third_enemy, true, 0x000000);
	DrawSurface(screen,third_enemy, enemy_x, enemy_y);
}
//funciton which combines two functions about enemy
void firstEnemy(struct Things* SDL_Things, int* quit_ptr, int* new_game_ptr, int move_x, int move_y, int distance) {
	int enemy_x =LEVEL_WIDTH/2,*enemy_x_ptr=&enemy_x, enemy_y =LEVEL_HEIGHT-100,* enemy_y_ptr = &enemy_y;
	int bullet_x = enemy_x, bullet_y=enemy_y;
	drawFirstEnemy(SDL_Things->first_enemy, SDL_Things->screen, quit_ptr, new_game_ptr, move_x, move_y, enemy_x_ptr, enemy_y_ptr);
	drawFirstBullet(SDL_Things->bullet, SDL_Things->screen, quit_ptr, new_game_ptr, distance, move_x, move_y,bullet_x, bullet_y);
}
void secondEnemy(struct Things* SDL_Things, int* quit_ptr, int* new_game_ptr, int move_x, int move_y, int distance) {
	int enemy_x = LEVEL_WIDTH / 2, * enemy_x_ptr = &enemy_x, enemy_y = LEVEL_HEIGHT - 100, * enemy_y_ptr = &enemy_y;
	int bullet_x = enemy_x, bullet_y = enemy_y;
	drawSecondBullet(SDL_Things->bullet, SDL_Things->screen, quit_ptr, new_game_ptr, distance, move_x, move_y, bullet_x, bullet_y);
	drawSecondEnemy(SDL_Things->first_enemy, SDL_Things->screen, quit_ptr, new_game_ptr, move_x, move_y, enemy_x_ptr, enemy_y_ptr);
}
void thirdEnemy(struct Things* SDL_Things, int* quit_ptr, int* new_game_ptr, int move_x, int move_y, int distance, int*main_x, int *main_y){
	int enemy_x = *main_x, * enemy_x_ptr = &enemy_x, enemy_y = *main_y, * enemy_y_ptr = &enemy_y;
	int bullet_x = enemy_x, bullet_y = enemy_y, enemy_radius = 200;
	float vel=0.02;
	drawThirdEnemy(SDL_Things->first_enemy, SDL_Things->screen, quit_ptr, new_game_ptr, move_x, move_y, enemy_x_ptr, enemy_y_ptr);
	drawThirdBullet(SDL_Things->bullet, SDL_Things->screen, quit_ptr, new_game_ptr, distance, move_x, move_y, bullet_x, bullet_y);
	*main_x =LEVEL_WIDTH/2 + sin(distance*vel)* enemy_radius;
	*main_y=LEVEL_HEIGHT -300 + cos(distance*vel)* enemy_radius;
}
//adding shots to the struct
void drawShot(struct Things* SDL_Things, int* quit_ptr, int* new_game_ptr,  int distance,int player_x,int player_y,int level) {
	int shots_vel = 8, shots_per_round = 1, start_angle = 0, end_angle = 360,two_per_sec=30;
	if(level==2)shots_per_round = 3;
	if(level==3)shots_per_round = 10;
	float angle_step = (end_angle - start_angle) /shots_per_round, angle = start_angle;
	SDL_Things->shot_bmp = SDL_LoadBMP("./shot.bmp");
	if (SDL_Things->shot_bmp == NULL) {
		printf("SDL_LoadBMP(shot.bmp) error: %s\n", SDL_GetError());
		*quit_ptr = 1;
		*new_game_ptr = 1;
	};
	SDL_SetColorKey(SDL_Things->shot_bmp, true, 0x000000);
	if (distance % two_per_sec == 0) {
		for (int i = 0; i <shots_per_round; i++)
			addShots(shooting_array_ptr, player_x,player_y);
	}
	for (int i = 0; i < shooting_array_ptr->count; i++) {
		if(level==1){shooting_array_ptr->ss[i].yBull += shots_vel;}
		else if (level == 2) {
			if (i % shots_per_round == 0)shooting_array_ptr->ss[i].yBull += shots_vel;
			if (i % shots_per_round == 1) { shooting_array_ptr->ss[i].yBull+= shots_vel; shooting_array_ptr->ss[i].xBull -= shots_vel / 4; }
			if (i % shots_per_round == 2) { shooting_array_ptr->ss[i].yBull += shots_vel; shooting_array_ptr->ss[i].xBull += shots_vel / 4; }
		}else if (level == 3) {
			shooting_array_ptr->ss[i].yBull -= cos((angle * PI) / 180) * shots_vel;
			shooting_array_ptr->ss[i].xBull += sin((angle * PI) / 180) * shots_vel;
		}
		int shot_x = shooting_array_ptr->ss[i].xBull;
		int shot_y = shooting_array_ptr->ss[i].yBull;
		if (shooting_array_ptr->ss[i].health == 1) {
			DrawSurface(SDL_Things->screen, SDL_Things->shot_bmp, shot_x, shot_y);
		}
		angle += angle_step;
	}
}
void drawHealth(struct Things* SDL_Things, int health, int* quit_ptr, int* new_game_ptr,int enemy_hp, int*level) {
	SDL_Things->health_img = SDL_LoadBMP("health1.bmp");
	if (health >= 5) SDL_Things->health_img = SDL_LoadBMP("health1.bmp");
	else if (health == 4)SDL_Things->health_img = SDL_LoadBMP("health2.bmp");
	else if (health == 3)SDL_Things->health_img = SDL_LoadBMP("health3.bmp");
	else if (health == 2) SDL_Things->health_img = SDL_LoadBMP("health4.bmp");//diffrent pictures of health
	else if (health == 1) SDL_Things->health_img = SDL_LoadBMP("health5.bmp");
	else if (health == 0)SDL_Things->health_img = SDL_LoadBMP("health6.bmp");;
	if (SDL_Things->health_img == NULL) {
		printf("SDL_LoadBMP(health.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(SDL_Things->health_img);
		*quit_ptr = 1;
		*new_game_ptr = 1;
	}
	if (health < 0) {
		*quit_ptr = 1;
		*new_game_ptr = 1;
	}
	if (enemy_hp < 0) {
		*quit_ptr = 1;
		*level += 1;
	}
	else {
		SDL_SetColorKey(SDL_Things->health_img, true, 0x000000);
		DrawSurface(SDL_Things->screen, SDL_Things->health_img, HEALTH_X, HEALTH_Y);
	}
	int white_len = 160 - 8 * enemy_hp;
	int red_len = 8 * enemy_hp;
	int red = SDL_MapRGB(SDL_Things->screen->format, 220, 20, 60);
	int white = SDL_MapRGB(SDL_Things->screen->format, 255, 255, 255);
	DrawRectangle(SDL_Things, 620, 500, 160, 20, red);
	DrawRectangle(SDL_Things, 620, 500, white_len, 20,white);
	SDL_UpdateTexture(SDL_Things->scrtex, NULL, SDL_Things->screen->pixels, SDL_Things->screen->pitch);
	SDL_RenderCopy(SDL_Things->renderer, SDL_Things->scrtex, NULL, NULL);
	SDL_RenderPresent(SDL_Things->renderer);
}
void drawGrade(struct Things* SDL_Things, int score, int* quit_ptr, int* new_game_ptr ) {
	int first = 2000, second = 30000, third = 50000, fourth = 60000, fifth = 100000, six = 150000,grade_x=700, grade_y= 450;
	SDL_Things->grade = SDL_LoadBMP("F.bmp");
	if (score <first) SDL_Things->grade = SDL_LoadBMP("F.bmp");
	else if (score < second&&score>first)SDL_Things->grade = SDL_LoadBMP("E.bmp");
	else if (score <third && score>second)SDL_Things->grade = SDL_LoadBMP("D.bmp");
	else if (score <fourth && score>third) SDL_Things->grade = SDL_LoadBMP("C.bmp");//diffrent pictures of grade
	else if (score < fifth && score>fourth) SDL_Things->grade = SDL_LoadBMP("B.bmp");
	else if ( score>fifth)SDL_Things->grade = SDL_LoadBMP("A.bmp");
	if (SDL_Things->grade == NULL) {
		printf("SDL_LoadBMP(grade.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(SDL_Things->grade);
		*quit_ptr = 1;
		*new_game_ptr = 1;
	}
	else {
		SDL_SetColorKey(SDL_Things->grade, true, 0x000000);
		DrawSurface(SDL_Things->screen, SDL_Things->grade, grade_x, grade_y);
	}
}
//resening statistics after finishign level
void stats_restet(int *move_x_ptr, int*move_y_ptr,int *distance,int*player_x,int *player_y,int*enemy_x,int*enemy_y,int*player_hp, int*enemy_hp) {
	*move_x_ptr = SCREEN_WIDTH, * move_y_ptr = SCREEN_HEIGHT; 
	*distance = 0;
	*player_x = PLAYER_X, *player_y = PLAYER_Y;
	*enemy_x = LEVEL_WIDTH / 2; *enemy_y = LEVEL_HEIGHT - 100;
	*player_hp = 5; *enemy_hp = 20;
}
//main menu(switching levels)
int showMenu(struct Things SDL_Things,int*new_game, int*quit,int* level) {
	Uint32 time;
	int square_x=SCREEN_WIDTH/2-30, square_y=100, level_one=100, level_two=150, level_three =200,timedd=0,mili_sec=1000;
	char text[128];
	int fillColor = SDL_MapRGB(SDL_Things.screen->format, 0, 128, 128);
	SDL_Things.bullet = SDL_LoadBMP("./pokeball.bmp");
			if (SDL_Things.bullet == NULL) {
				printf("SDL_LoadBMP(enemy2.bmp) error: %s\n", SDL_GetError());
				*quit = 1;
				*new_game = 1;
			};
	SDL_SetColorKey(SDL_Things.charset, true, 0x000000);
	SDL_SetColorKey(SDL_Things.bullet, true, 0x000000);
	while (1)
	{	
	DrawRectangle(&SDL_Things,0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,fillColor);/// info text
	sprintf(text, "level:1");	
	DrawString(SDL_Things.screen, SCREEN_WIDTH /2, level_one, text, SDL_Things.charset);
	sprintf(text, "level:2");	
	DrawString(SDL_Things.screen, SCREEN_WIDTH/2, level_two, text, SDL_Things.charset);
	sprintf(text, "level:3");	
	DrawString(SDL_Things.screen, SCREEN_WIDTH /2, level_three, text, SDL_Things.charset);
	sprintf(text, "Esc- menu N - new game");//	      "Esc - exit, N new game
	DrawString(SDL_Things.screen, SCREEN_WIDTH /2, 250, text, SDL_Things.charset);
	DrawSurface(SDL_Things.screen, SDL_Things.bullet ,square_x,square_y);
	SDL_UpdateTexture(SDL_Things.scrtex, NULL, SDL_Things.screen->pixels, SDL_Things.screen->pitch);
	SDL_RenderClear(SDL_Things.renderer);	
	SDL_RenderCopy(SDL_Things.renderer, SDL_Things.scrtex, NULL, NULL);
	SDL_RenderPresent(SDL_Things.renderer);
	SDL_Event event;
		time = SDL_GetTicks();
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				*quit = 1;
				*new_game = 1;
				return 1;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) { *quit = 1; *new_game = 1; return 1; }
				if (event.key.keysym.sym == SDLK_n) { *level = 1; return 1; }
				if (event.key.keysym.sym == SDLK_UP) {
					if (square_y == level_two)square_y = level_one;
					else if (square_y == level_three)square_y = level_two;
				}
				if (event.key.keysym.sym == SDLK_DOWN) {
					if (square_y == level_two)square_y = level_three;
					else if (square_y == level_one)square_y = level_two;
				}
				if (event.key.keysym.sym == SDLK_RETURN) {
					if (square_y == level_one) { *level = 1; return 1; }
					if (square_y == level_two) { *level = 2; return 1; }
					if (square_y == level_three) { *level = 3; return 1; }
				}
				break;
			}
		}
		if (mili_sec / FPS/2 > (SDL_GetTicks() - time))
			SDL_Delay(mili_sec / FPS/2 - (SDL_GetTicks() - time));
	}
}
//menu displpayed after death;
int deathMenu(int*new_game,int*quit, struct Things SDL_Things,int *menu_ptr) {
	Uint32 time;
	int square_x = SCREEN_WIDTH / 2, square_y = 250, mili_sec = 1000;
	const int size = 128;
	char text[size];
	int fillColor = SDL_MapRGB(SDL_Things.screen->format, 0, 128, 128);
	SDL_Things.game_over = SDL_LoadBMP("./gameover.bmp");
	if (SDL_Things.game_over == NULL) {
		printf("SDL_LoadBMP(game_over.bmp) error: %s\n", SDL_GetError());
		*quit = 1;
		*new_game = 1;
	};
	SDL_SetColorKey(SDL_Things.charset, true, 0x000000);
	SDL_SetColorKey(SDL_Things.game_over, true, 0x000000);
	while (1)
	{
		DrawRectangle(&SDL_Things, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,fillColor);/// info text
		sprintf(text, "Esc- menu N - new game");//	      "Esc - exit, N new game
		DrawString(SDL_Things.screen, SCREEN_WIDTH / 2, 250, text, SDL_Things.charset);
		DrawSurface(SDL_Things.screen, SDL_Things.game_over, square_x, square_y);
		SDL_UpdateTexture(SDL_Things.scrtex, NULL, SDL_Things.screen->pixels, SDL_Things.screen->pitch);
		SDL_RenderClear(SDL_Things.renderer);
		SDL_RenderCopy(SDL_Things.renderer, SDL_Things.scrtex, NULL, NULL);
		SDL_RenderPresent(SDL_Things.renderer);
		SDL_Event event;
		time = SDL_GetTicks();
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				*quit = 1;
				*new_game = 1;
				return 1;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) { *quit = 1; *new_game = 1; return 1; }
				if (event.key.keysym.sym == SDLK_n) { *quit = 0; *menu_ptr = 0; *new_game = 0; return 1; }
				break;
			}
		}
		if (mili_sec / FPS/2 > (SDL_GetTicks() - time))
			SDL_Delay(mili_sec / FPS/2 - (SDL_GetTicks() - time));
	}
	SDL_FreeSurface(SDL_Things.game_over);
}
int winningMenu(int*new_game_ptr ,struct Things SDL_Things,int*menu_ptr) {
	Uint32 time;
	const int size = 128;
	char text[size];
	int fillColor = SDL_MapRGB(SDL_Things.screen->format, 0, 128, 128), mili_sec = 1000,grade_x=SCREEN_WIDTH/2+100,grade_y=120;
	int square_x = SCREEN_WIDTH / 2, square_y = SCREEN_HEIGHT/2+200;
	int pok_y = 400, new_game = 150, exit = 605 , pok_x=new_game;
	SDL_Things.win = SDL_LoadBMP("./win.bmp");
	if (SDL_Things.win == NULL) {
		printf("SDL_LoadBMP(win.bmp) error: %s\n", SDL_GetError());
		*new_game_ptr = 1;
	};
	SDL_Things.bullet = SDL_LoadBMP("./pokeball.bmp");
	if (SDL_Things.bullet == NULL) {
		printf("SDL_LoadBMP(enemy2.bmp) error: %s\n", SDL_GetError());
		*new_game_ptr = 1;
	};
	SDL_SetColorKey(SDL_Things.bullet, true, 0x000000);
	SDL_SetColorKey(SDL_Things.win, true, 0x000000);
	while (1)
	{
		DrawRectangle(&SDL_Things, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,fillColor);
		DrawSurface(SDL_Things.screen, SDL_Things.win, square_x, square_y);
		DrawSurface(SDL_Things.screen, SDL_Things.bullet,pok_x, pok_y);
		DrawSurface(SDL_Things.screen, SDL_Things.grade, grade_x, grade_y);
		SDL_UpdateTexture(SDL_Things.scrtex, NULL, SDL_Things.screen->pixels, SDL_Things.screen->pitch);
		SDL_RenderClear(SDL_Things.renderer);
		SDL_RenderCopy(SDL_Things.renderer, SDL_Things.scrtex, NULL, NULL);
		SDL_RenderPresent(SDL_Things.renderer);
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				*new_game_ptr = 1;
				return 1;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {*new_game_ptr = 1; return 1; }
				if (event.key.keysym.sym == SDLK_n) {  *menu_ptr = 0; *new_game_ptr = 0; return 1; }
			
			if (event.key.keysym.sym == SDLK_RIGHT) {
				if (pok_x == new_game)pok_x =pok_x=exit;
			}
			if (event.key.keysym.sym == SDLK_LEFT) {
				if (pok_x == exit)pok_x=new_game;
			}
			if (event.key.keysym.sym == SDLK_RETURN) {
				//if (pok_x == save_score) {  }
				if (pok_x == exit) { *new_game_ptr = 1; return 1; }
				if (pok_x == new_game) { *menu_ptr = 0; *new_game_ptr = 0; return 1; }
			}
			break;
			}
		}
		time = SDL_GetTicks();
		if (mili_sec/ FPS/2 > (SDL_GetTicks() - time))
			SDL_Delay(mili_sec /FPS/2 - (SDL_GetTicks() - time));
	}
	SDL_FreeSurface(SDL_Things.win);
}
//closes the SDL and destroys the window
void destroyEverything(struct Things* SDL_Things);
//frees the surfaces;
void freeEverything(struct Things* SDL_Things);
//to check if any corner of rectangle is not inside other
bool collision(int topB, int bottomB, int leftB, int rightB,int bottomA, int topA, int leftA, int rightA) {
			//If any of the sides from A are outside of B
			if (bottomA <= topB){return false;}
			if (topA >= bottomB){return false;}
			if (rightA <= leftB){return false;}
			if (leftA >= rightB){return false;}
			return true;//If none of the sides from A are outside B
}
//colision of enemy bullets
void collsionDetection(int player_x,int player_y, int *player_health, int move_x, int move_y,int*immunity) {
	int leftA, leftB, rightA, rightB, topA, topB, bottomA, bottomB,A_w=22, B_w=5;
	for (int i = 0; i < bullet_array_ptr->count; i++) {
		if (bullet_array_ptr->bb[i].health == 1) {
			//Calculate the sides of rect A
			leftA = player_x - A_w;
			rightA = player_x + A_w;
			topA = player_y - A_w;
			bottomA = player_y + A_w;
			//Calculate the sides of rect B
			int bullet_x = bullet_array_ptr->bb[i].xBull;
			int bullet_y = bullet_array_ptr->bb[i].yBull;
			int* bullet_x_ptr = &bullet_x, * bullet_y_ptr = &bullet_y;
			translation(move_x, move_y, bullet_x_ptr, bullet_y_ptr);
			leftB = bullet_x - B_w;
			rightB = bullet_x + B_w;
			topB = bullet_y -B_w;
			bottomB = bullet_y + B_w;
			if (collision(topB,bottomB,leftB,rightB,bottomA,topA,leftA,rightA)) {
				if (*immunity == 0) {
					*player_health -= 1;
					bullet_array_ptr->bb[i].health = 0;
					*immunity = 100;
				}
			}
		}
	}
}
//colision of players bullets with enemy
void enenmyCollision(int* enemy_hp,int enemy_x, int enemy_y,int move_x,int move_y,int& score,int &hits_num,int& score_bonus) {
	int leftA, leftB, rightA, rightB, topA, topB, bottomA, bottomB,A_w=27,B_w=5;
	int score_bonus_inc = 50,score_inc=25;
	for (int i = 0; i < shooting_array_ptr->count; i++) {
		if (shooting_array_ptr->ss[i].health == 1) {
			int shot_x = shooting_array_ptr->ss[i].xBull;
			int shot_y = shooting_array_ptr->ss[i].yBull;
			leftB = shot_x - B_w;//Calculate the sides of rect B
			rightB = shot_x + B_w;
			topB =shot_y - B_w;
			bottomB =shot_y + B_w;
			int enemy_x_buff = enemy_x, enemy_y_buff = enemy_y;
			int* enemy_x_ptr = &enemy_x_buff, *enemy_y_ptr = &enemy_y_buff;
			translation(move_x, move_y,enemy_x_ptr,enemy_y_ptr);
			leftA = enemy_x_buff - A_w;
			rightA = enemy_x_buff + A_w;//Calculate the sides of rect A
			topA = enemy_y_buff - A_w;
			bottomA = enemy_y_buff + A_w;
			if (collision(topB, bottomB, leftB, rightB, bottomA, topA, leftA, rightA)) {
				shooting_array_ptr->ss[i].health = 0;
				*enemy_hp -= 1;
				hits_num += 1;
				score += score_inc*score_bonus;
				score_bonus += score_bonus_inc;
			}
		}
	}
}
void printingInfo(double worldTime, struct Things* SDL_Things,int hits_num,int score,int level) {
	const int size_of_text = 128;
	char text[size_of_text];
	sprintf(text, "Bullet HELL = %.1lf s", worldTime);//section for printing informations
	DrawString(SDL_Things->screen, SCREEN_WIDTH - strlen(text) * 8 - 10, 10, text, SDL_Things->charset);
	sprintf(text, "Esc- menu N - new game");//	      "Esc - exit, N new game
	DrawString(SDL_Things->screen, SCREEN_WIDTH - strlen(text) * 8 - 10, 50, text, SDL_Things->charset);
	sprintf(text, "Number of hits = %i", hits_num);//	      "number of hits"
	DrawString(SDL_Things->screen, SCREEN_WIDTH - strlen(text) * 8 - 10,90, text, SDL_Things->charset);
	sprintf(text, "PLAYER SCORE = %i", score);//	      "player score"
	DrawString(SDL_Things->screen, SCREEN_WIDTH - strlen(text) * 8 - 10,130, text, SDL_Things->charset);
	sprintf(text, "LEVEL = %i", level);//	      "level in info"
	DrawString(SDL_Things->screen, SCREEN_WIDTH - strlen(text) * 8 - 10,170, text, SDL_Things->charset);
}
bool colorChanging( int color) {
	if (color == 255) {
		return false;
	}
	return true;
}
int main(int argc, char** argv) {
	int frame_start, t1, quit, * quit_ptr = &quit, rc, new_game = 0, * new_game_ptr = &new_game, frames = 0, health = 5, * health_ptr = &health, player_x = PLAYER_X, player_y = PLAYER_Y;
	int	l = SCREEN_WIDTH * 2, k = SCREEN_HEIGHT * 2, * player_x_ptr = &player_x, * player_y_ptr = &player_y, xBuff = 0, yBuff = 0, bullet_num = 1, * bullet_num_ptr = &bullet_num, distance = 0;
	int move_x = SCREEN_WIDTH, * move_x_ptr = &move_x, move_y = SCREEN_HEIGHT, * move_y_ptr = &move_y, * x_buff = &xBuff, * y_buff = &yBuff,frame_time, frame_delay=1000/FPS,*distance_ptr=&distance;
	int enemy_x = LEVEL_WIDTH / 2, * enemy_x_ptr = &enemy_x, enemy_y = LEVEL_HEIGHT - 100, * enemy_y_ptr = &enemy_y,level=1, *level_ptr=&level,basic_size=20,enemy_hp=20,*enemyhp_ptr=&enemy_hp;
	int menu = 0, * menu_ptr = &menu, immunity = 0,*immunity_ptr=&immunity,score=0,hits_num,score_bonus=1, changing_col=0;
	double delta, worldTime;
	bool isChanging = true;
	 struct Things SDL_Things;
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		SDL_RenderSetLogicalSize(SDL_Things.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
		SDL_SetRenderDrawColor(SDL_Things.renderer, 0, 0, 0, 255);
		SDL_SetWindowTitle(SDL_Things.window, "POKEMON HELL");
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
			printf("SDL_Init error: %s\n", SDL_GetError());
			return 1;
		}
		rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &SDL_Things.window, &SDL_Things.renderer);
		if (rc != 0) {
			SDL_Quit();
			printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
			return 1;
		};
	while (!new_game) {
		health = 5;
		quit = 0;
		SDL_Things.screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH * 3, SCREEN_HEIGHT * 4, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		SDL_Things.scrtex = SDL_CreateTexture(SDL_Things.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
		SDL_Things.charset = SDL_LoadBMP("cs8x8.bmp");
		if (SDL_Things.charset == NULL) {printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());return 1;};
		if (menu == 0) { 
			showMenu(SDL_Things, new_game_ptr, quit_ptr, level_ptr);
			score = 0, hits_num = 0, score_bonus=1;
		}
		bullet_array_ptr=bulletArrayInit(bullet_array_ptr,basic_size);
		shooting_array_ptr = shootingArrayInit(shooting_array_ptr, basic_size);
		SDL_SetColorKey(SDL_Things.charset, true, 0x000000);
		
		int fillColor = SDL_MapRGB(SDL_Things.screen->format, 0, 128, 128);
		t1 = SDL_GetTicks();
		worldTime = 0;
		menu = 0;
		while (!quit) {
			isChanging = colorChanging(changing_col);
			if (isChanging)	changing_col += 1;
			else changing_col -= 1;
			int czarny = SDL_MapRGB(SDL_Things.screen->format,125 , changing_col, changing_col);
			frame_start = SDL_GetTicks();
			delta = (frame_start-t1) * 0.001;
			t1 = frame_start;
			worldTime += delta;
			distance += 1;
			pollEventsRect(quit_ptr, new_game_ptr, x_buff, y_buff);
			SDL_FillRect(SDL_Things.screen, NULL, czarny); frames++;
			move_x += xBuff;
			move_y += yBuff;
			MovingAcrossBoard(SDL_Things.screen, move_x_ptr, move_y_ptr, player_x_ptr, player_y_ptr, xBuff, yBuff);
			if (level == 1) firstEnemy(&SDL_Things, quit_ptr, new_game_ptr, move_x, move_y, distance);
			if (level == 2) secondEnemy(&SDL_Things, quit_ptr, new_game_ptr, move_x, move_y, distance);
			if (level == 3) thirdEnemy(&SDL_Things, quit_ptr, new_game_ptr, move_x, move_y, distance,enemy_x_ptr,enemy_y_ptr);
			drawPlayer(&SDL_Things, player_x, player_y, quit_ptr, new_game_ptr,immunity);
			drawShot(&SDL_Things, quit_ptr, new_game_ptr, distance, player_x, player_y,level);
			collsionDetection(player_x, player_y, health_ptr,move_x,move_y,immunity_ptr);
			enenmyCollision(enemyhp_ptr, enemy_x, enemy_y,move_x,move_y,score,hits_num,score_bonus);
			DrawRectangle(&SDL_Things, SCREEN_WIDTH - LOBBY_WIDTH, 0, LOBBY_WIDTH, SCREEN_HEIGHT,fillColor);/// info text
			printingInfo(worldTime, &SDL_Things, hits_num,score,level);
			drawGrade(&SDL_Things, score, quit_ptr, new_game_ptr);
			drawHealth(&SDL_Things, health, quit_ptr, new_game_ptr,enemy_hp,level_ptr);
			SDL_UpdateTexture(SDL_Things.scrtex, NULL, SDL_Things.screen->pixels, SDL_Things.screen->pitch);
			SDL_RenderClear(SDL_Things.renderer);
			SDL_RenderCopy(SDL_Things.renderer, SDL_Things.scrtex, NULL, NULL);
			SDL_RenderPresent(SDL_Things.renderer);
			frame_time = SDL_GetTicks() - frame_start;
			if (immunity > 0)immunity-=1;//decreasing immunity
			if (score_bonus > 1)score_bonus -= 1;//decreasing score bonus
			if (frame_delay > frame_time) { SDL_Delay(frame_delay - frame_time); }
		};
		menu = 1, score+=(health*1000-worldTime*50);
		if (health < 0){ deathMenu(new_game_ptr, quit_ptr, SDL_Things,menu_ptr); }
		if (level > 3) { winningMenu(new_game_ptr,SDL_Things,menu_ptr); }
		stats_restet(move_x_ptr, move_y_ptr, distance_ptr, player_x_ptr, player_y_ptr, enemy_x_ptr, enemy_y_ptr, health_ptr, enemyhp_ptr);
		freeEverything(&SDL_Things);
	}
	destroyEverything(&SDL_Things);
	return 0;
};
void freeEverything(struct Things* SDL_Things) {
	free(shooting_array_ptr->ss);
	free(bullet_array_ptr->bb);
	bullet_array_ptr = NULL;
	shooting_array_ptr = NULL;
	SDL_FreeSurface(SDL_Things->bullet);
	SDL_FreeSurface(SDL_Things->first_enemy);
	SDL_FreeSurface(SDL_Things->player);
	SDL_FreeSurface(SDL_Things->screen);
	SDL_FreeSurface(SDL_Things->health_img);
	SDL_FreeSurface(SDL_Things->grade);
	SDL_FreeSurface(SDL_Things->second_enemy);
	SDL_FreeSurface(SDL_Things->third_enemy);
	SDL_FreeSurface(SDL_Things->shot_bmp);
}
void destroyEverything(struct Things* SDL_Things) {
	SDL_FreeSurface(SDL_Things->charset);// freeing all surfaces
	SDL_DestroyTexture(SDL_Things->scrtex);
	SDL_DestroyRenderer(SDL_Things->renderer);
	SDL_DestroyWindow(SDL_Things->window);
	SDL_Quit();
}