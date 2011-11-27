#include "SDL.h"
#include "../NFont.h"


void loop_drawSomeText(SDL_Surface* screen)
{
    NFont font;
    SDL_Color fontColor = {0, 0, 0, 255};
    font.load("../fonts/FreeSans.ttf", 20, fontColor);
    font.setDest(screen);
    
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
	    
	    SDL_FillRect(screen, NULL, 0xffffff);
	    
	    font.draw(50, 50, "Hey");
	    
	    SDL_Flip(screen);
	    
	    SDL_Delay(1);
	}
	
	font.freeSurface();
}

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Surface* screen = SDL_SetVideoMode(800, 600, 0, SDL_SWSURFACE);
	
	loop_drawSomeText(screen);
	
	SDL_Quit();
	return 0;
}
