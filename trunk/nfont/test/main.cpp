#include "SDL.h"
#include "../NFont.h"

#include <cmath>
#include <string>

// UTF-8 Sample from http://www.columbia.edu/~fdc/utf8/

void drawRect(GPU_Target* target, const GPU_Rect& rect, const SDL_Color& color)
{
    GPU_Rect r;
    r.x = rect.x;
    r.y = rect.y;
    r.w = rect.w;
    r.h = 1;
    GPU_RectangleFilled(target, r.x, r.y, r.x + r.w - 1, r.y + r.h - 1, color);
    r.y = rect.y + rect.h;
    GPU_RectangleFilled(target, r.x, r.y, r.x + r.w - 1, r.y + r.h - 1, color);
    
    r.y = rect.y;
    r.w = 1;
    r.h = rect.h;
    GPU_RectangleFilled(target, r.x, r.y, r.x + r.w - 1, r.y + r.h - 1, color);
    r.x = rect.x + rect.w;
    GPU_RectangleFilled(target, r.x, r.y, r.x + r.w - 1, r.y + r.h - 1, color);
}

std::string get_string_from_file(const std::string& filename)
{
    std::string result;
    SDL_RWops* rwops = SDL_RWFromFile(filename.c_str(), "r");
    
    char c;
    while(SDL_RWread(rwops, &c, 1, 1) > 0)
    {
        result += c;
    }
    
    SDL_RWclose(rwops);
    return result;
}

void loop_drawSomeText(GPU_Target* screen)
{
    NFont font("../fonts/FreeSans.ttf", 20, NFont::Color(0,0,0,255));
    NFont font2("../fonts/FreeSans.ttf", 18, NFont::Color(0,200,0,255));
    NFont font3("../fonts/FreeSans.ttf", 22, NFont::Color(0,0,200,255));
    NFont font4("../fonts/FreeSans.ttf", 20);
    
    std::string utf8_string = get_string_from_file("utf8_sample.txt");
    
    GPU_Rect leftHalf = {0,0,3*screen->w/4.0f, float(screen->h)};
    GPU_Rect rightHalf = {leftHalf.w,0,screen->w/4.0f, float(screen->h)};
    
    GPU_Rect box1 = {215, 50, 150, 150};
    GPU_Rect box2 = {215, box1.y + box1.h + 50, 150, 150};
    GPU_Rect box3 = {215, box2.y + box2.h + 50, 150, 150};
    
    int scroll = 0;
    
    Uint8* keystates = SDL_GetKeyboardState(NULL);
    
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
	    
	    if(keystates[SDL_SCANCODE_UP])
            scroll--;
	    else if(keystates[SDL_SCANCODE_DOWN])
            scroll++;
	    
	    GPU_RectangleFilled(screen, leftHalf.x, leftHalf.y, leftHalf.x + leftHalf.w - 1, leftHalf.y + leftHalf.h - 1, GPU_MakeColor(255, 255, 255, 255));
	    GPU_RectangleFilled(screen, rightHalf.x, rightHalf.y, rightHalf.x + rightHalf.w - 1, rightHalf.y + rightHalf.h - 1, GPU_MakeColor(0x77, 0x77, 0x77, 255));
	    
	    
	    font.draw(screen, rightHalf.x, 5, NFont::LEFT, "draw align LEFT");
	    font.draw(screen, rightHalf.x, 25, NFont::CENTER, "draw align CENTER");
	    font.draw(screen, rightHalf.x, 45, NFont::RIGHT, "draw align RIGHT");
	    
	    font4.draw(screen, rightHalf.x, 65, NFont::Color(255, 100, 100, 255), "Colored text");
	    font4.draw(screen, 0, 0, NFont::Scale(0.85f), "UTF-8 text: %s", utf8_string.c_str());
	    
	    float time = SDL_GetTicks()/1000.0f;
	    
	    font.draw(screen, rightHalf.x, 90, NFont::AnimParams(time), &NFontAnim::bounce, NFont::RIGHT, "bounce align RIGHT");
	    font3.draw(screen, rightHalf.x, 120, NFont::AnimParams(time), &NFontAnim::bounce, NFont::CENTER, "bounce align CENTER");
	    font2.draw(screen, rightHalf.x, 150, NFont::AnimParams(time), &NFontAnim::bounce, "bounce align LEFT");
	    
	    font.draw(screen, rightHalf.x, 180, NFont::AnimParams(time), &NFontAnim::wave, NFont::RIGHT, "wave align RIGHT");
	    font2.draw(screen, rightHalf.x, 210, NFont::AnimParams(time), &NFontAnim::wave, NFont::CENTER, "wave align CENTER");
	    font3.draw(screen, rightHalf.x, 240, NFont::AnimParams(time), &NFontAnim::wave, "wave align LEFT");
	    
	    font.draw(screen, rightHalf.x, 270, NFont::AnimParams(time), &NFontAnim::stretch, NFont::RIGHT, "stretch align RIGHT");
	    font2.draw(screen, rightHalf.x, 300, NFont::AnimParams(time), &NFontAnim::stretch, NFont::CENTER, "stretch align CENTER");
	    font3.draw(screen, rightHalf.x, 330, NFont::AnimParams(time), &NFontAnim::stretch, "stretch align LEFT");
        
        font.draw(screen, rightHalf.x, 380, NFont::AnimParams(time, 60, 0.2, 60, 0.1), &NFontAnim::circle, NFont::RIGHT, "circle align RIGHT");
        font2.draw(screen, rightHalf.x, 410, NFont::AnimParams(time, 60, 0.2, 60, 0.1), &NFontAnim::circle, NFont::CENTER, "circle align CENTER");
        font3.draw(screen, rightHalf.x, 440, NFont::AnimParams(time, 60, 0.2, 60, 0.1), &NFontAnim::circle, "circle align LEFT");
	    
        font.draw(screen, rightHalf.x, 490, NFont::AnimParams(time, 5, 9, 5, 7), &NFontAnim::shake, NFont::RIGHT, "shake align RIGHT");
        font2.draw(screen, rightHalf.x, 520, NFont::AnimParams(time, 5, 9, 5, 7), &NFontAnim::shake, NFont::CENTER, "shake align CENTER");
        font3.draw(screen, rightHalf.x, 550, NFont::AnimParams(time, 5, 9, 5, 7), &NFontAnim::shake, "shake align LEFT");
        
        font.drawColumn(screen, 0, 50, 200, "column align LEFT\n\nColumn text wraps at the width of the column and has no maximum height.");
        font.drawColumn(screen, 100, 250, 200, NFont::CENTER, "column align CENTER\n\nColumn text wraps at the width of the column and has no maximum height.");
        font.drawColumn(screen, 200, 450, 200, NFont::RIGHT, "column align RIGHT\n\nColumn text wraps at the width of the column and has no maximum height.");
	    
	    
	    drawRect(screen, box1, GPU_MakeColor(0, 0, 0, 255));
	    drawRect(screen, box2, GPU_MakeColor(0, 0, 0, 255));
	    drawRect(screen, box3, GPU_MakeColor(0, 0, 0, 255));
	    
	    GPU_SetClipRect(screen, box1);
	    GPU_Rect box1a = {box1.x, box1.y - scroll, box1.w, box1.h + scroll};
        font.drawBox(screen, box1a, "box align LEFT\n\nBox text wraps at the width of the box and is clipped to the maximum height.");
        
	    GPU_SetClipRect(screen, box2);
	    GPU_Rect box2a = {box2.x, box2.y - scroll, box2.w, box2.h + scroll};
        font.drawBox(screen, box2a, NFont::CENTER, "box align CENTER\n\nBox text wraps at the width of the box and is clipped to the maximum height.");
        
	    GPU_SetClipRect(screen, box3);
	    GPU_Rect box3a = {box3.x, box3.y - scroll, box3.w, box3.h + scroll};
        font.drawBox(screen, box3a, NFont::RIGHT, "box align RIGHT\n\nBox text wraps at the width of the box and is clipped to the maximum height.");
        
	    GPU_UnsetClip(screen);
	    
	    GPU_Flip(screen);
	    
	    SDL_Delay(1);
	}
	
}

int main(int argc, char* argv[])
{
    int w = 800;
    int h = 600;
    GPU_Target* screen = GPU_Init(w, h, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
	{
        GPU_LogError("Couldn't initialize SDL_gpu\n");
        return 1;
    }
    
	loop_drawSomeText(screen);
	
	SDL_Quit();
	return 0;
}
