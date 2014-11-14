#include "SDL.h"

// Which rendering API?  NFont uses SDL_gpu.  NFontR uses SDL_Renderer.
#ifdef NFONT_USE_NFONTR
    #include "../NFontR/NFont.h"
#else
    #include "../NFont/NFont.h"
#endif

#include <cmath>
#include <string>

// UTF-8 Sample from http://www.columbia.edu/~fdc/utf8/


#ifdef SDL_GPU_VERSION_MAJOR
GPU_Target* screen;
#else
SDL_Window* window;
SDL_Renderer* renderer;
#endif


void draw_rect(const NFont::Rectf& rect, const SDL_Color& color)
{
    #ifdef SDL_GPU_VERSION_MAJOR
    GPU_Rectangle(screen, rect.x, rect.y, rect.x + rect.w - 1, rect.y + rect.h - 1, color);
    #else
    SDL_Rect r = rect.to_SDL_Rect();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &r);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    #endif
}

void fill_rect(const NFont::Rectf& rect, const SDL_Color& color)
{
    #ifdef SDL_GPU_VERSION_MAJOR
    GPU_RectangleFilled(screen, rect.x, rect.y, rect.x + rect.w - 1, rect.y + rect.h - 1, color);
    #else
    SDL_Rect r = rect.to_SDL_Rect();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    #endif
}

void set_clip(const NFont::Rectf& rect)
{
    #ifdef SDL_GPU_VERSION_MAJOR
    GPU_SetClipRect(screen, rect.to_GPU_Rect());
    #else
    //SDL_Rect r = rect.to_SDL_Rect();
    //SDL_RenderSetClipRect(renderer, &r);
    #endif
}

void unset_clip()
{
    #ifdef SDL_GPU_VERSION_MAJOR
    GPU_UnsetClip(screen);
    #else
    //SDL_RenderSetClipRect(renderer, NULL);
    #endif
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

void loop_drawSomeText()
{
    #ifdef SDL_GPU_VERSION_MAJOR
    NFont font("fonts/FreeSans.ttf", 20);
    NFont font2("fonts/FreeSans.ttf", 18, NFont::Color(0,200,0,255));
    NFont font3("fonts/FreeSans.ttf", 22, NFont::Color(0,0,200,255));
    #else
    NFont font(renderer, "fonts/FreeSans.ttf", 20);
    NFont font2(renderer, "fonts/FreeSans.ttf", 18, NFont::Color(0,200,0,255));
    NFont font3(renderer, "fonts/FreeSans.ttf", 22, NFont::Color(0,0,200,255));
    #endif
    
    std::string utf8_string = get_string_from_file("utf8_sample.txt");
    
    float target_w, target_h;
    #ifdef SDL_GPU_VERSION_MAJOR
    GPU_Target* target = screen;
    target_w = target->w;
    target_h = target->h;
    #else
    SDL_Renderer* target = renderer;
    {
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        target_w = w;
        target_h = h;
    }
    #endif
    
    NFont::Rectf leftHalf(0, 0, 3*target_w/4.0f, target_h);
    NFont::Rectf rightHalf(leftHalf.w, 0, target_w/4.0f, target_h);
    
    NFont::Rectf box1(215, 50, 150, 150);
    NFont::Rectf box2(215, box1.y + box1.h + 50, 150, 150);
    NFont::Rectf box3(215, box2.y + box2.h + 50, 150, 150);
    
    SDL_Color black = {0, 0, 0, 255};
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray = {0x77, 0x77, 0x77, 255};
    
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
	    
	    fill_rect(leftHalf, white);
	    fill_rect(rightHalf, gray);
	    
	    
	    font.draw(target, rightHalf.x, 5, NFont::LEFT, "draw align LEFT");
	    font.draw(target, rightHalf.x, 25, NFont::CENTER, "draw align CENTER");
	    font.draw(target, rightHalf.x, 45, NFont::RIGHT, "draw align RIGHT");
	    
	    float time = SDL_GetTicks()/1000.0f;
	    
	    font.draw(target, rightHalf.x, 65, NFont::Color(128 + 127*sin(time), 128 + 127*sin(time/2), 128 + 127*sin(time/4), 128 + 127*sin(time/8)), "Dynamic colored text");
	    font.draw(target, 0, 0, NFont::Scale(0.85f), "UTF-8 text: %s", utf8_string.c_str());
	    
	    font.draw(target, rightHalf.x, 90, NFont::AnimParams(time), &NFontAnim::bounce, NFont::RIGHT, "bounce align RIGHT");
	    font2.draw(target, rightHalf.x, 120, NFont::AnimParams(time), &NFontAnim::bounce, NFont::CENTER, "bounce align CENTER");
	    font3.draw(target, rightHalf.x, 150, NFont::AnimParams(time), &NFontAnim::bounce, "bounce align LEFT");
	    
	    font.draw(target, rightHalf.x, 180, NFont::AnimParams(time), &NFontAnim::wave, NFont::RIGHT, "wave align RIGHT");
	    font2.draw(target, rightHalf.x, 210, NFont::AnimParams(time), &NFontAnim::wave, NFont::CENTER, "wave align CENTER");
	    font3.draw(target, rightHalf.x, 240, NFont::AnimParams(time), &NFontAnim::wave, "wave align LEFT");
	    
	    font.draw(target, rightHalf.x, 270, NFont::AnimParams(time), &NFontAnim::stretch, NFont::RIGHT, "stretch align RIGHT");
	    font2.draw(target, rightHalf.x, 300, NFont::AnimParams(time), &NFontAnim::stretch, NFont::CENTER, "stretch align CENTER");
	    font3.draw(target, rightHalf.x, 330, NFont::AnimParams(time), &NFontAnim::stretch, "stretch align LEFT");
        
        font.draw(target, rightHalf.x, 380, NFont::AnimParams(time, 60, 0.2, 60, 0.1), &NFontAnim::circle, NFont::RIGHT, "circle align RIGHT");
        font2.draw(target, rightHalf.x, 410, NFont::AnimParams(time, 60, 0.2, 60, 0.1), &NFontAnim::circle, NFont::CENTER, "circle align CENTER");
        font3.draw(target, rightHalf.x, 440, NFont::AnimParams(time, 60, 0.2, 60, 0.1), &NFontAnim::circle, "circle align LEFT");
	    
        font.draw(target, rightHalf.x, 490, NFont::AnimParams(time, 5, 9, 5, 7), &NFontAnim::shake, NFont::RIGHT, "shake align RIGHT");
        font2.draw(target, rightHalf.x, 520, NFont::AnimParams(time, 5, 9, 5, 7), &NFontAnim::shake, NFont::CENTER, "shake align CENTER");
        font3.draw(target, rightHalf.x, 550, NFont::AnimParams(time, 5, 9, 5, 7), &NFontAnim::shake, "shake align LEFT");
        
        font.drawColumn(target, 0, 50, 200, "column align LEFT\nColumn text wraps at the width of the column and has no maximum height.");
        font.drawColumn(target, 100, 250, 200, NFont::CENTER, "column align CENTER\nColumn text wraps at the width of the column and has no maximum height.");
        font.drawColumn(target, 200, 450, 200, NFont::RIGHT, "column align RIGHT\nColumn text wraps at the width of the column and has no maximum height.");
	    
	    
	    draw_rect(box1, black);
	    draw_rect(box2, black);
	    draw_rect(box3, black);
	    
	    set_clip(box1);
	    NFont::Rectf box1a(box1.x, box1.y - scroll, box1.w, box1.h + scroll);
        font.drawBox(target, box1a, "box align LEFT\nBox text wraps at the width of the box and is clipped to the maximum height.");
        
	    set_clip(box2);
	    NFont::Rectf box2a(box2.x, box2.y - scroll, box2.w, box2.h + scroll);
        font.drawBox(target, box2a, NFont::CENTER, "box align CENTER\nBox text wraps at the width of the box and is clipped to the maximum height.");
        
	    set_clip(box3);
	    NFont::Rectf box3a(box3.x, box3.y - scroll, box3.w, box3.h + scroll);
        font.drawBox(target, box3a, NFont::RIGHT, "box align RIGHT\nBox text wraps at the width of the box and is clipped to the maximum height.");
        
	    unset_clip();
	    
        #ifdef SDL_GPU_VERSION_MAJOR
	    GPU_Flip(screen);
	    #else
	    SDL_RenderPresent(renderer);
	    #endif
	    
	    SDL_Delay(1);
	}
	
}

int main(int argc, char* argv[])
{
    int w = 800;
    int h = 600;
    
    #ifdef SDL_GPU_VERSION_MAJOR
    screen = GPU_Init(w, h, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
	{
        GPU_LogError("Failed to initialize SDL_gpu.\n");
        return 1;
    }
    #else
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_Log("Failed to initialize SDL.\n");
        return 1;
    }
    
    if(SDL_CreateWindowAndRenderer(w, h, SDL_WINDOW_SHOWN, &window, &renderer) < 0)
    {
        SDL_Log("Failed to create window and renderer.\n");
        return 2;
    }
    #endif
    
	loop_drawSomeText();
	
    #ifdef SDL_GPU_VERSION_MAJOR
    GPU_Quit();
    #else
	SDL_Quit();
	#endif
	
	return 0;
}
