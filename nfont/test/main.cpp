#include "SDL.h"
#include "../NFont.h"

#include <cmath>



void loop_drawSomeText(SDL_Surface* screen)
{
    NFont font("../fonts/FreeSans.ttf", 20, NFont::Color(0,0,0,255));
    
    SDL_Rect leftHalf = {0,0,screen->w/2, screen->h};
    SDL_Rect rightHalf = {leftHalf.w,0,screen->w/2, screen->h};
    
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
	    
	    SDL_FillRect(screen, &leftHalf, 0xffffff);
	    SDL_FillRect(screen, &rightHalf, 0x777777);
	    
	    font.draw(screen, rightHalf.x, 50, "draw()");
	    font.draw(screen, rightHalf.x, 70, NFont::LEFT, "draw(LEFT)");
	    font.draw(screen, rightHalf.x, 90, NFont::CENTER, "draw(CENTER)");
	    font.draw(screen, rightHalf.x, 110, NFont::RIGHT, "draw(RIGHT)");
	    
	    float time = SDL_GetTicks()/1000.0f;
	    
	    font.draw(screen, rightHalf.x, 220, time, &NFontAnim::bounce, NFont::RIGHT, "bounce align RIGHT");
	    font.draw(screen, rightHalf.x, 280, time, &NFontAnim::bounce, "bounce align LEFT");
	    font.draw(screen, rightHalf.x, 250, time, &NFontAnim::bounce, NFont::CENTER, "bounce align CENTER");
	    
	    font.draw(screen, rightHalf.x, 310, time, &NFontAnim::wave, NFont::RIGHT, "wave align RIGHT");
	    font.draw(screen, rightHalf.x, 340, time, &NFontAnim::wave, NFont::CENTER, "wave align CENTER");
	    font.draw(screen, rightHalf.x, 370, time, &NFontAnim::wave, "wave align LEFT");
	    
	    font.draw(screen, rightHalf.x, 400, time, &NFontAnim::stretch, NFont::RIGHT, "stretch align RIGHT");
	    font.draw(screen, rightHalf.x, 430, time, &NFontAnim::stretch, NFont::CENTER, "stretch align CENTER");
	    font.draw(screen, rightHalf.x, 460, time, &NFontAnim::stretch, "stretch align LEFT");
	    
        font.draw(screen, rightHalf.x, 490, time, &NFontAnim::shake, NFont::RIGHT, "shake align RIGHT");
        font.draw(screen, rightHalf.x, 520, time, &NFontAnim::shake, NFont::CENTER, "shake align CENTER");
        font.draw(screen, rightHalf.x, 550, time, &NFontAnim::shake, "shake align LEFT");
	    
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
