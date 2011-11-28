#include "SDL.h"
#include "../NFont.h"


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
	    font.drawAlign(screen, rightHalf.x, 70, NFont::LEFT, "drawAlign(LEFT)");
	    font.drawAlign(screen, rightHalf.x, 90, NFont::CENTER, "drawAlign(CENTER)");
	    font.drawAlign(screen, rightHalf.x, 110, NFont::RIGHT, "drawAlign(RIGHT)");
	    
	    SDL_Flip(screen);
	    
	    SDL_Delay(1);
	}
	
}

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Surface* screen = SDL_SetVideoMode(800, 600, 0, SDL_SWSURFACE);
	
	loop_drawSomeText(screen);
	
	SDL_Quit();
	return 0;
}
