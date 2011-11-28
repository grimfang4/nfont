#include "SDL.h"
#include "../NFont.h"

#include <cmath>


void drawRect(SDL_Surface* surf, const SDL_Rect& rect, Uint32 color)
{
    SDL_Rect r;
    r.x = rect.x;
    r.y = rect.y;
    r.w = rect.w;
    r.h = 1;
    SDL_FillRect(surf, &r, color);
    r.y = rect.y + rect.h;
    SDL_FillRect(surf, &r, color);
    
    r.y = rect.y;
    r.w = 1;
    r.h = rect.h;
    SDL_FillRect(surf, &r, color);
    r.x = rect.x + rect.w;
    SDL_FillRect(surf, &r, color);
}

void loop_drawSomeText(SDL_Surface* screen)
{
    NFont font("../fonts/FreeSans.ttf", 20, NFont::Color(0,0,0,255));
    NFont font2("../fonts/FreeSans.ttf", 18, NFont::Color(0,200,0,255));
    NFont font3("../fonts/FreeSans.ttf", 22, NFont::Color(0,0,200,255));
    
    SDL_Rect leftHalf = {0,0,3*screen->w/4, screen->h};
    SDL_Rect rightHalf = {leftHalf.w,0,screen->w/4, screen->h};
    
    SDL_Rect box1 = {215, 50, 150, 150};
    SDL_Rect box2 = {215, box1.y + box1.h + 50, 150, 150};
    SDL_Rect box3 = {215, box2.y + box2.h + 50, 150, 150};
    
    int scroll = 0;
    
    Uint8* keystates = SDL_GetKeyState(NULL);
    
    bool done = false;
	SDL_Event event;
	while(!done)
	{
	    while(SDL_PollEvent(&event))
	    {
	        if(event.type == SDL_QUIT)
                done = true;
	        else if(event.type == SDL_KEYDOWN)
	        {
	            if(event.key.keysym.sym == SDLK_ESCAPE)
                    done = true;
	        }
	    }
	    
	    if(keystates[SDLK_UP])
            scroll--;
	    else if(keystates[SDLK_DOWN])
            scroll++;
	    
	    SDL_FillRect(screen, &leftHalf, 0xffffff);
	    SDL_FillRect(screen, &rightHalf, 0x777777);
	    
	    font.draw(screen, rightHalf.x, 50, "draw()");
	    font.draw(screen, rightHalf.x, 70, NFont::LEFT, "draw(LEFT)");
	    font.draw(screen, rightHalf.x, 90, NFont::CENTER, "draw(CENTER)");
	    font.draw(screen, rightHalf.x, 110, NFont::RIGHT, "draw(RIGHT)");
	    
	    float time = SDL_GetTicks()/1000.0f;
	    
	    font.draw(screen, rightHalf.x, 220, time, &NFontAnim::bounce, NFont::RIGHT, "bounce align RIGHT");
	    font2.draw(screen, rightHalf.x, 280, time, &NFontAnim::bounce, "bounce align LEFT");
	    font3.draw(screen, rightHalf.x, 250, time, &NFontAnim::bounce, NFont::CENTER, "bounce align CENTER");
	    
	    font.draw(screen, rightHalf.x, 310, time, &NFontAnim::wave, NFont::RIGHT, "wave align RIGHT");
	    font2.draw(screen, rightHalf.x, 340, time, &NFontAnim::wave, NFont::CENTER, "wave align CENTER");
	    font3.draw(screen, rightHalf.x, 370, time, &NFontAnim::wave, "wave align LEFT");
	    
	    font.draw(screen, rightHalf.x, 400, time, &NFontAnim::stretch, NFont::RIGHT, "stretch align RIGHT");
	    font2.draw(screen, rightHalf.x, 430, time, &NFontAnim::stretch, NFont::CENTER, "stretch align CENTER");
	    font3.draw(screen, rightHalf.x, 460, time, &NFontAnim::stretch, "stretch align LEFT");
	    
        font.draw(screen, rightHalf.x, 490, time, &NFontAnim::shake, NFont::RIGHT, "shake align RIGHT");
        font2.draw(screen, rightHalf.x, 520, time, &NFontAnim::shake, NFont::CENTER, "shake align CENTER");
        font3.draw(screen, rightHalf.x, 550, time, &NFontAnim::shake, "shake align LEFT");
        
        font.drawColumn(screen, 0, 50, 200, "column align LEFT\n\nColumn text wraps at the width of the column and has no maximum height.");
        font.drawColumn(screen, 100, 250, 200, NFont::CENTER, "column align CENTER\n\nColumn text wraps at the width of the column and has no maximum height.");
        font.drawColumn(screen, 200, 450, 200, NFont::RIGHT, "column align RIGHT\n\nColumn text wraps at the width of the column and has no maximum height.");
	    
	    
	    drawRect(screen, box1, 0x000000);
	    drawRect(screen, box2, 0x000000);
	    drawRect(screen, box3, 0x000000);
	    
	    SDL_SetClipRect(screen, &box1);
	    SDL_Rect box1a = {box1.x, box1.y - scroll, box1.w, box1.h + scroll};
        font.drawBox(screen, box1a, "box align LEFT\n\nBox text wraps at the width of the box and is clipped to the maximum height.");
        
	    SDL_SetClipRect(screen, &box2);
	    SDL_Rect box2a = {box2.x, box2.y - scroll, box2.w, box2.h + scroll};
        font.drawBox(screen, box2a, NFont::CENTER, "box align CENTER\n\nBox text wraps at the width of the box and is clipped to the maximum height.");
        
	    SDL_SetClipRect(screen, &box3);
	    SDL_Rect box3a = {box3.x, box3.y - scroll, box3.w, box3.h + scroll};
        font.drawBox(screen, box3a, NFont::RIGHT, "box align RIGHT\n\nBox text wraps at the width of the box and is clipped to the maximum height.");
        
        SDL_SetClipRect(screen, NULL);
	    
	    SDL_Flip(screen);
	    
	    SDL_Delay(1);
	}
	
}

int main(int argc, char* argv[])
{
	if(SDL_Init(SDL_INIT_VIDEO ) < 0)
	{
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return 1;
    }
    
    int w = 800;
    int h = 600;
	SDL_Surface* screen = SDL_SetVideoMode(w, h, 0, SDL_SWSURFACE);
	
    if(screen == NULL)
    {
        printf("Couldn't set video mode %dx%d: %s\n", w, h, SDL_GetError());
		return 1;
    }
    
	loop_drawSomeText(screen);
	
	SDL_Quit();
	return 0;
}
