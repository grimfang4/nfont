#include "SDL.h"
#include "../nfontc.h"

#include <math.h>


void drawRect(SDL_Surface* surf, SDL_Rect rect, Uint32 color)
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
    SDL_Color black = {0,0,0,255};
    SDL_Color green = {0,200,0,255};
    SDL_Color blue = {0,0,200,255};
    
    NF_Push(NF_New());
    NF_LoadTTF("../fonts/FreeSans.ttf", 20, black, NULL, TTF_STYLE_NORMAL);
    NFont* font = NF_Pop();
    
    NF_Push(NF_New());
    NF_LoadTTF("../fonts/FreeSans.ttf", 18, green, NULL, TTF_STYLE_NORMAL);
    NFont* font2 = NF_Pop();
    
    NF_Push(NF_New());
    NF_LoadTTF("../fonts/FreeSans.ttf", 22, blue, NULL, TTF_STYLE_NORMAL);
    NFont* font3 = NF_Pop();
    
    
    SDL_Rect leftHalf = {0,0,3*screen->w/4, screen->h};
    SDL_Rect rightHalf = {leftHalf.w,0,screen->w/4, screen->h};
    
    SDL_Rect box1 = {215, 50, 150, 150};
    SDL_Rect box2 = {215, box1.y + box1.h + 50, 150, 150};
    SDL_Rect box3 = {215, box2.y + box2.h + 50, 150, 150};
    
    int scroll = 0;
    
    Uint8* keystates = SDL_GetKeyState(NULL);
    
    Uint8 done = 0;
	SDL_Event event;
	while(!done)
	{
	    while(SDL_PollEvent(&event))
	    {
	        if(event.type == SDL_QUIT)
                done = 1;
	        else if(event.type == SDL_KEYDOWN)
	        {
	            if(event.key.keysym.sym == SDLK_ESCAPE)
                    done = 1;
	        }
	    }
	    
	    if(keystates[SDLK_UP])
            scroll--;
	    else if(keystates[SDLK_DOWN])
            scroll++;
	    
	    SDL_FillRect(screen, &leftHalf, 0xffffff);
	    SDL_FillRect(screen, &rightHalf, 0x777777);
	    
	    NF_Push(font);
	    NF_Draw(screen, rightHalf.x, 5, "draw align LEFT");
	    NF_DrawAlign(screen, rightHalf.x, 25, NF_CENTER, "draw align CENTER");
	    NF_DrawAlign(screen, rightHalf.x, 45, NF_RIGHT, "draw align RIGHT");
	    NF_Pop();
	    
	    float time = SDL_GetTicks()/1000.0f;
	    
	    NFontAnim_Params params = NF_AnimParams(time, 20, 1, 20, 1);
	    
	    NF_Push(font);
	    NF_DrawPosAlign(screen, rightHalf.x, 90, params, &NF_bounce, NF_RIGHT, "bounce align RIGHT");
	    NF_DrawPosAlign(screen, rightHalf.x, 180, params, &NF_wave, NF_RIGHT, "wave align RIGHT");
	    NF_DrawPosAlign(screen, rightHalf.x, 270, params, &NF_stretch, NF_RIGHT, "stretch align RIGHT");
        NF_DrawPosAlign(screen, rightHalf.x, 380, NF_AnimParams(time, 60, 0.2, 60, 0.1), &NF_circle, NF_RIGHT, "circle align RIGHT");
        NF_DrawPosAlign(screen, rightHalf.x, 490, NF_AnimParams(time, 5, 9, 5, 7), &NF_shake, NF_RIGHT, "shake align RIGHT");
        NF_Pop();
	    
	    NF_Push(font2);
	    NF_DrawPos(screen, rightHalf.x, 150, params, &NF_bounce, "bounce align LEFT");
	    NF_DrawPosAlign(screen, rightHalf.x, 210, params, &NF_wave, NF_CENTER, "wave align CENTER");
	    NF_DrawPosAlign(screen, rightHalf.x, 300, params, &NF_stretch, NF_CENTER, "stretch align CENTER");
        NF_DrawPosAlign(screen, rightHalf.x, 410, NF_AnimParams(time, 60, 0.2, 60, 0.1), &NF_circle, NF_CENTER, "circle align CENTER");
        NF_DrawPosAlign(screen, rightHalf.x, 520, NF_AnimParams(time, 5, 9, 5, 7), &NF_shake, NF_CENTER, "shake align CENTER");
        NF_Pop();
        
        NF_Push(font3);
	    NF_DrawPosAlign(screen, rightHalf.x, 120, params, &NF_bounce, NF_CENTER, "bounce align CENTER");
	    NF_DrawPos(screen, rightHalf.x, 240, params, &NF_wave, "wave align LEFT");
	    NF_DrawPos(screen, rightHalf.x, 330, params, &NF_stretch, "stretch align LEFT");
        NF_DrawPos(screen, rightHalf.x, 440, NF_AnimParams(time, 60, 0.2, 60, 0.1), &NF_circle, "circle align LEFT");
        NF_DrawPos(screen, rightHalf.x, 550, NF_AnimParams(time, 5, 9, 5, 7), &NF_shake, "shake align LEFT");
        NF_Pop();
        
        NF_Push(font);
        NF_DrawColumn(screen, 0, 50, 200, "column align LEFT\n\nColumn text wraps at the width of the column and has no maximum height.");
        NF_DrawColumnAlign(screen, 100, 250, 200, NF_CENTER, "column align CENTER\n\nColumn text wraps at the width of the column and has no maximum height.");
        NF_DrawColumnAlign(screen, 200, 450, 200, NF_RIGHT, "column align RIGHT\n\nColumn text wraps at the width of the column and has no maximum height.");
	    
	    drawRect(screen, box1, 0x000000);
	    drawRect(screen, box2, 0x000000);
	    drawRect(screen, box3, 0x000000);
	    
	    SDL_SetClipRect(screen, &box1);
	    SDL_Rect box1a = {box1.x, box1.y - scroll, box1.w, box1.h + scroll};
        NF_DrawBox(screen, box1a, "box align LEFT\n\nBox text wraps at the width of the box and is clipped to the maximum height.");
        
	    SDL_SetClipRect(screen, &box2);
	    SDL_Rect box2a = {box2.x, box2.y - scroll, box2.w, box2.h + scroll};
        NF_DrawBoxAlign(screen, box2a, NF_CENTER, "box align CENTER\n\nBox text wraps at the width of the box and is clipped to the maximum height.");
        
	    SDL_SetClipRect(screen, &box3);
	    SDL_Rect box3a = {box3.x, box3.y - scroll, box3.w, box3.h + scroll};
        NF_DrawBoxAlign(screen, box3a, NF_RIGHT, "box align RIGHT\n\nBox text wraps at the width of the box and is clipped to the maximum height.");
        
        SDL_SetClipRect(screen, NULL);
        NF_Pop();
	    
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
