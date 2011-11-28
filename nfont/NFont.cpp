/*
NFont v2.1.0: A bitmap font class for SDL
by Jonathan Dearborn 11-27-11
(class adapted from Florian Hufsky)

License:
    The short:
    Use it however you'd like, but keep the copyright and license notice 
    whenever these files or parts of them are distributed in uncompiled form.
    
    The long:
Copyright (c) 2011 Jonathan Dearborn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include "NFont.h"
#include <cmath>

#include <string>
#include <list>
using std::string;
using std::list;


#define MIN(a,b) ((a) < (b)? (a) : (b))
#define MAX(a,b) ((a) > (b)? (a) : (b))

static inline SDL_Surface* createSurface24(Uint32 width, Uint32 height)
{
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        return SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 24, 0xFF0000, 0x00FF00, 0x0000FF, 0);
    #else
        return SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 24, 0x0000FF, 0x00FF00, 0xFF0000, 0);
    #endif
}

static inline SDL_Surface* createSurface32(Uint32 width, Uint32 height)
{
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        return SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    #else
        return SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    #endif
}

static inline char* copyString(const char* c)
{
    if(c == NULL)
        return NULL;
    
    char* result = new char[strlen(c)+1];
    strcpy(result, c);

    return result;
}

static inline Uint32 getPixel(SDL_Surface *Surface, int x, int y)
{
    Uint8* bits;
    Uint32 bpp;

    if(x < 0 || x >= Surface->w)
        return 0;  // Best I could do for errors

    bpp = Surface->format->BytesPerPixel;
    bits = ((Uint8*)Surface->pixels) + y*Surface->pitch + x*bpp;

    switch (bpp)
    {
        case 1:
            return *((Uint8*)Surface->pixels + y * Surface->pitch + x);
            break;
        case 2:
            return *((Uint16*)Surface->pixels + y * Surface->pitch/2 + x);
            break;
        case 3:
            // Endian-correct, but slower
            Uint8 r, g, b;
            r = *((bits)+Surface->format->Rshift/8);
            g = *((bits)+Surface->format->Gshift/8);
            b = *((bits)+Surface->format->Bshift/8);
            return SDL_MapRGB(Surface->format, r, g, b);
            break;
        case 4:
            return *((Uint32*)Surface->pixels + y * Surface->pitch/4 + x);
            break;
    }

    return 0;  // FIXME: Handle errors better
}

static inline void setPixel(SDL_Surface* surface, int x, int y, Uint32 color)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8* bits = ((Uint8 *)surface->pixels) + y*surface->pitch + x*bpp;

    /* Set the pixel */
    switch(bpp)
    {
        case 1:
            *((Uint8 *)(bits)) = (Uint8)color;
            break;
        case 2:
            *((Uint16 *)(bits)) = (Uint16)color;
            break;
        case 3: { /* Format/endian independent */
            Uint8 r,g,b;
            r = (color >> surface->format->Rshift) & 0xFF;
            g = (color >> surface->format->Gshift) & 0xFF;
            b = (color >> surface->format->Bshift) & 0xFF;
            *((bits)+surface->format->Rshift/8) = r;
            *((bits)+surface->format->Gshift/8) = g;
            *((bits)+surface->format->Bshift/8) = b;
            }
            break;
        case 4:
            *((Uint32 *)(bits)) = (Uint32)color;
            break;
    }
}

static inline SDL_Rect rectUnion(const SDL_Rect& A, const SDL_Rect& B)
{
    Sint16 x,x2,y,y2;
    x = MIN(A.x, B.x);
    y = MIN(A.y, B.y);
    x2 = MAX(A.x+A.w, B.x+B.w);
    y2 = MAX(A.y+A.h, B.y+B.h);
    SDL_Rect result = {x, y, x2 - x, y2 - y};
    return result;
}

static inline SDL_Rect makeRect(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
    SDL_Rect r = {x, y, w, h};
    return r;
}

static inline SDL_Surface* copySurface(SDL_Surface *Surface)
{
    return SDL_ConvertSurface(Surface, Surface->format, Surface->flags);
}







char* NFont::buffer = NULL;
NFont::AnimData NFont::data;



NFont::Color::Color()
    : r(0), g(0), b(0), a(255)
{}
NFont::Color::Color(Uint8 r, Uint8 g, Uint8 b)
    : r(r), g(g), b(b), a(255)
{}
NFont::Color::Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
    : r(r), g(g), b(b), a(a)
{}

NFont::Color& NFont::Color::rgb(Uint8 R, Uint8 G, Uint8 B)
{
    r = R;
    g = G;
    b = B;
    
    return *this;
}

NFont::Color& NFont::Color::rgba(Uint8 R, Uint8 G, Uint8 B, Uint8 A)
{
    r = R;
    g = G;
    b = B;
    a = A;
    
    return *this;
}

SDL_Color NFont::Color::toSDL_Color() const
{
    SDL_Color c = {r, g, b, a};
    return c;
}











// Static setters
void NFont::setAnimData(void* data)
{
    NFont::data.userVar = data;
}

void NFont::setBuffer(unsigned int size)
{
    delete[] buffer;
    if(size > 0)
        buffer = new char[size];
    else
        buffer = new char[1024];
}

SDL_Surface* NFont::verticalGradient(SDL_Surface* targetSurface, Uint32 top, Uint32 bottom, int heightAdjust)
{
    SDL_Surface* surface = targetSurface;
    if(surface == NULL)
        return NULL;
    
    Uint8 tr, tg, tb;

    SDL_GetRGB(top, surface->format, &tr, &tg, &tb);

    Uint8 br, bg, bb;

    SDL_GetRGB(bottom, surface->format, &br, &bg, &bb);

    bool useCK = (surface->flags & SDL_SRCALPHA) != SDL_SRCALPHA;  // colorkey if no alpha
    Uint32 colorkey = surface->format->colorkey;

    Uint8 r, g, b, a;
    float ratio;
    Uint32 color;
    int temp;

    for (int x = 0, y = 0; y < surface->h; x++)
    {
        if (x >= surface->w)
        {
            x = 0;
            y++;

            if (y >= surface->h)
                break;
        }

        ratio = (y - 2)/float(surface->h - heightAdjust);  // the neg 2 is for full color at top?

        if(!useCK)
        {
            color = getPixel(surface, x, y);
            SDL_GetRGBA(color, surface->format, &r, &g, &b, &a);  // just getting alpha
        }
        else
            a = SDL_ALPHA_OPAQUE;

        // Get and clamp the new values
        temp = int(tr*(1-ratio) + br*ratio);
        r = temp < 0? 0 : temp > 255? 255 : temp;

        temp = int(tg*(1-ratio) + bg*ratio);
        g = temp < 0? 0 : temp > 255? 255 : temp;

        temp = int(tb*(1-ratio) + bb*ratio);
        b = temp < 0? 0 : temp > 255? 255 : temp;


        color = SDL_MapRGBA(surface->format, r, g, b, a);


        if(useCK)
        {
            if(getPixel(surface, x, y) == colorkey)
                continue;
            if(color == colorkey)
                color == 0? color++ : color--;
        }

        // make sure it isn't pink
        if(color == SDL_MapRGBA(surface->format, 0xFF, 0, 0xFF, a))
            color--;
        if(getPixel(surface, x, y) == SDL_MapRGBA(surface->format, 0xFF, 0, 0xFF, SDL_ALPHA_OPAQUE))
            continue;

        setPixel(surface, x, y, color);

    }
    return surface;
}


// Constructors
NFont::NFont()
{
    init();
}

NFont::NFont(const NFont& font)
{
    init();
    load(copySurface(font.src));
}

NFont::NFont(SDL_Surface* src)
{
    init();
    load(src);
}

#ifndef NFONT_NO_TTF
NFont::NFont(TTF_Font* ttf, const NFont::Color& fg)
{
    init();
    load(ttf, fg);
}
NFont::NFont(TTF_Font* ttf, const NFont::Color& fg, const NFont::Color& bg)
{
    init();
    load(ttf, fg, bg);
}
NFont::NFont(const char* filename_ttf, Uint32 pointSize, const NFont::Color& fg, int style)
{
    init();
    load(filename_ttf, pointSize, fg, style);
}
NFont::NFont(const char* filename_ttf, Uint32 pointSize, const NFont::Color& fg, const NFont::Color& bg, int style)
{
    init();
    load(filename_ttf, pointSize, fg, bg, style);
}

#endif

void NFont::init()
{
    src = NULL;

    maxPos = 0;

    height = 0; // ascent+descent

    maxWidth = 0;
    baseline = 0;
    ascent = 0;
    descent = 0;

    lineSpacing = 0;
    letterSpacing = 0;
    
    if(buffer == NULL)
        buffer = new char[1024];
}

NFont::~NFont()
{
    SDL_FreeSurface(src);
}


NFont& NFont::operator=(const NFont& font)
{
    load(copySurface(font.src));
    return *this;
}


// Loading
bool NFont::load(SDL_Surface* FontSurface)
{
    SDL_FreeSurface(src);
    
    src = FontSurface;
    if(src == NULL)
    {
        printf("\n ERROR: NFont given a NULL surface\n");
        return false;
    }

    int x = 1, i = 0;
    
    // memset would be faster
    for(int j = 0; j < 256; j++)
    {
        charWidth[j] = 0;
        charPos[j] = 0;
    }

    SDL_LockSurface(src);

    Uint32 pixel = SDL_MapRGB(src->format, 255, 0, 255); // pink pixel
    
    maxWidth = 0;
    
    // Get the character positions and widths
    while (x < src->w)
    {
        if(getPixel(src, x, 0) != pixel)
        {
            charPos[i] = x;
            charWidth[i] = x;
            while(x < src->w && getPixel(src, x, 0) != pixel)
                x++;
            charWidth[i] = x - charWidth[i];
            if(charWidth[i] > maxWidth)
                maxWidth = charWidth[i];
            i++;
        }

        x++;
    }

    maxPos = x - 1;


    pixel = getPixel(src, 0, src->h - 1);
    int j;
    setBaseline();
    
    // Get the max ascent
    j = 1;
    while(j < baseline && j < src->h)
    {
        x = 0;
        while(x < src->w)
        {
            if(getPixel(src, x, j) != pixel)
            {
                ascent = baseline - j;
                j = src->h;
                break;
            }
            x++;
        }
        j++;
    }
    
    // Get the max descent
    j = src->h - 1;
    while(j > 0 && j > baseline)
    {
        x = 0;
        while(x < src->w)
        {
            if(getPixel(src, x, j) != pixel)
            {
                descent = j - baseline+1;
                j = 0;
                break;
            }
            x++;
        }
        j--;
    }
    
    
    height = ascent + descent;
    
    optimizeForVideoSurface();

    if((src->flags & SDL_SRCALPHA) != SDL_SRCALPHA)
    {
        pixel = getPixel(src, 0, src->h - 1);
        SDL_UnlockSurface(src);
        SDL_SetColorKey(src, SDL_SRCCOLORKEY, pixel);
    }
    else
        SDL_UnlockSurface(src);
    
    return true;
}

#ifndef NFONT_NO_TTF
bool NFont::load(TTF_Font* ttf, const NFont::Color& fg, const NFont::Color& bg)
{
    SDL_FreeSurface(src);
    src = NULL;
    
    if(ttf == NULL)
        return false;
    
    SDL_Color fgc = fg.toSDL_Color();
    SDL_Color bgc = bg.toSDL_Color();
    
    SDL_Surface* surfs[127 - 33];
    int width = 0;
    int height = 0;
    
    char buff[2];
    buff[1] = '\0';
    for(int i = 0; i < 127 - 33; i++)
    {
        buff[0] = i + 33;
        surfs[i] = TTF_RenderText_Shaded(ttf, buff, fgc, bgc);
        width += surfs[i]->w;
        height = (height < surfs[i]->h)? surfs[i]->h : height;
    }
    
    SDL_Surface* result = createSurface24(width + 127 - 33 + 1,height);
    
    Uint32 pink = SDL_MapRGB(result->format, 255, 0, 255);
    Uint32 bgcolor = SDL_MapRGB(result->format, bg.r, bg.g, bg.b);
    
    SDL_Rect pixel = {1, 0, 1, 1};
    SDL_Rect line = {1, 0, 1, result->h};
    
    int x = 1;
    SDL_Rect dest = {x, 0, 0, 0};
    for(int i = 0; i < 127 - 33; i++)
    {
        pixel.x = line.x = x-1;
        SDL_FillRect(result, &line, bgcolor);
        SDL_FillRect(result, &pixel, pink);
        
        SDL_BlitSurface(surfs[i], NULL, result, &dest);
        
        x += surfs[i]->w + 1;
        dest.x = x;
        
        SDL_FreeSurface(surfs[i]);
    }
    pixel.x = line.x = x-1;
    SDL_FillRect(result, &line, bgcolor);
    SDL_FillRect(result, &pixel, pink);
    
    return load(result);
}


bool NFont::load(TTF_Font* ttf, const NFont::Color& fg)
{
    SDL_FreeSurface(src);
    src = NULL;
    
    if(ttf == NULL)
        return false;
    
    SDL_Color fgc = fg.toSDL_Color();
    
    SDL_Surface* surfs[127 - 33];
    int width = 0;
    int height = 0;
    
    char buff[2];
    buff[1] = '\0';
    for(int i = 0; i < 127 - 33; i++)
    {
        buff[0] = i + 33;
        surfs[i] = TTF_RenderText_Blended(ttf, buff, fgc);
        width += surfs[i]->w;
        height = (height < surfs[i]->h)? surfs[i]->h : height;
    }
    
    SDL_Surface* result = createSurface32(width + 127 - 33 + 1,height);
    
    Uint32 pink = SDL_MapRGBA(result->format, 255, 0, 255, SDL_ALPHA_OPAQUE);
    
    SDL_SetAlpha(result, 0, SDL_ALPHA_OPAQUE);
    
    SDL_Rect pixel = {1, 0, 1, 1};
    
    int x = 1;
    SDL_Rect dest = {x, 0, 0, 0};
    for(int i = 0; i < 127 - 33; i++)
    {
        pixel.x = x-1;
        SDL_FillRect(result, &pixel, pink);
        
        SDL_SetAlpha(surfs[i], 0, SDL_ALPHA_OPAQUE);
        SDL_BlitSurface(surfs[i], NULL, result, &dest);
        
        x += surfs[i]->w + 1;
        dest.x = x;
        
        SDL_FreeSurface(surfs[i]);
    }
    pixel.x = x-1;
    SDL_FillRect(result, &pixel, pink);
    
    SDL_SetAlpha(result, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
    
    return load(result);
}


bool NFont::load(const char* filename_ttf, Uint32 pointSize, const NFont::Color& fg, int style)
{
    SDL_FreeSurface(src);
    src = NULL;
    
    if(!TTF_WasInit() && TTF_Init() < 0)
    {
        printf("Unable to initialize SDL_ttf: %s \n", TTF_GetError());
        return false;
    }
    
    TTF_Font* ttf = TTF_OpenFont(filename_ttf, pointSize);
    
    if(ttf == NULL)
    {
        printf("Unable to load TrueType font: %s \n", TTF_GetError());
        return false;
    }
    TTF_SetFontStyle(ttf, style);
    bool result = load(ttf, fg);
    TTF_CloseFont(ttf);
    return result;
}

bool NFont::load(const char* filename_ttf, Uint32 pointSize, const NFont::Color& fg, const NFont::Color& bg, int style)
{
    SDL_FreeSurface(src);
    src = NULL;
    
    if(!TTF_WasInit() && TTF_Init() < 0)
    {
        printf("Unable to initialize SDL_ttf: %s \n", TTF_GetError());
        return false;
    }
    
    TTF_Font* ttf = TTF_OpenFont(filename_ttf, pointSize);
    
    if(ttf == NULL)
    {
        printf("Unable to load TrueType font: %s \n", TTF_GetError());
        return false;
    }
    TTF_SetFontStyle(ttf, style);
    bool result = load(ttf, fg, bg);
    TTF_CloseFont(ttf);
    return result;
}

#endif



// Drawing
SDL_Rect NFont::drawLeft(SDL_Surface* dest, int x, int y, const char* text) const
{
    const char* c = text;
    unsigned char num;
    SDL_Rect srcRect, dstRect, copyS, copyD;
    data.dirtyRect = makeRect(x, y, 0, 0);
    
    if(c == NULL || src == NULL || dest == NULL)
        return data.dirtyRect;
    
    srcRect.y = baseline - ascent;
    srcRect.h = dstRect.h = height;
    dstRect.x = x;
    dstRect.y = y;
    
    int newlineX = x;
    
    for(; *c != '\0'; c++)
    {
        if(*c == '\n')
        {
            dstRect.x = newlineX;
            dstRect.y += height + lineSpacing;
            continue;
        }
        
        if (*c == ' ')
        {
            dstRect.x += charWidth[0] + letterSpacing;
            continue;
        }
        unsigned char ctest = (unsigned char)(*c);
        // Skip bad characters
        if(ctest < 33 || (ctest > 126 && ctest < 161))
            continue;
        if(dstRect.x >= dest->w)
            continue;
        if(dstRect.y >= dest->h)
            continue;
        
        num = ctest - 33;  // Get array index
        if(num > 126) // shift the extended characters down to the correct index
            num -= 34;
        srcRect.x = charPos[num];
        srcRect.w = dstRect.w = charWidth[num];
        copyS = srcRect;
        copyD = dstRect;
        SDL_BlitSurface(src, &srcRect, dest, &dstRect);
        if(data.dirtyRect.w == 0 || data.dirtyRect.h == 0)
            data.dirtyRect = dstRect;
        else
            data.dirtyRect = rectUnion(data.dirtyRect, dstRect);
        srcRect = copyS;
        dstRect = copyD;
        
        dstRect.x += dstRect.w + letterSpacing;
    }
    
    return data.dirtyRect;
}

SDL_Rect NFont::drawAnimated(SDL_Surface* dest, int x, int y, float t, NFont::AnimFn posFn, NFont::AlignEnum align) const
{
    data.font = this;
    data.dest = dest;
    data.src = src;
    data.text = buffer;  // Buffer for efficient drawing
    data.charPos = charPos;
    data.charWidth = charWidth;
    data.maxX = maxPos;
    data.dirtyRect = makeRect(x,y,0,0);

    data.index = -1;
    data.letterNum = 0;
    data.wordNum = 1;
    data.lineNum = 1;
    data.startX = x;  // used as reset value for line feed
    data.startY = y;
    
    data.align = align;
    
    data.t = t;
    
    int preFnX = x;
    int preFnY = y;
    
    const char* c = buffer;
    unsigned char num;
    SDL_Rect srcRect, dstRect, copyS, copyD;
    
    if(c == NULL || src == NULL || dest == NULL)
        return makeRect(x,y,0,0);
    
    srcRect.y = baseline - ascent;
    srcRect.h = dstRect.h = height;
    dstRect.x = x;
    dstRect.y = y;
    
    for(; *c != '\0'; c++)
    {
        data.index++;
        data.letterNum++;
        
        if(*c == '\n')
        {
            data.letterNum = 1;
            data.wordNum = 1;
            data.lineNum++;

            x = data.startX;  // carriage return
            y += height + lineSpacing;
            continue;
        }
        if (*c == ' ')
        {
            data.letterNum = 1;
            data.wordNum++;
            
            x += charWidth[0] + letterSpacing;
            continue;
        }
        unsigned char ctest = (unsigned char)(*c);
        // Skip bad characters
        if(ctest < 33 || (ctest > 126 && ctest < 161))
            continue;
        //if(x >= dest->w) // This shouldn't be used with position control
        //    continue;
        num = ctest - 33;
        if(num > 126) // shift the extended characters down to the array index
            num -= 34;
        srcRect.x = charPos[num];
        srcRect.w = dstRect.w = charWidth[num];
        
        preFnX = x;  // Save real position
        preFnY = y;

        // Use function pointer to get final x, y values
        posFn(x, y, data);
        
        dstRect.x = x;
        dstRect.y = y;
        
        copyS = srcRect;
        copyD = dstRect;
        SDL_BlitSurface(src, &srcRect, dest, &dstRect);
        if(data.dirtyRect.w == 0 || data.dirtyRect.h == 0)
            data.dirtyRect = dstRect;
        else
            data.dirtyRect = rectUnion(data.dirtyRect, dstRect);
        srcRect = copyS;
        dstRect = copyD;
        
        x = preFnX;  // Restore real position
        y = preFnY;
        
        x += dstRect.w + letterSpacing;
    }
    
    return data.dirtyRect;
}

SDL_Rect NFont::draw(SDL_Surface* dest, int x, int y, const char* formatted_text, ...) const
{
    if(formatted_text == NULL)
        return makeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    return drawLeft(dest, x, y, buffer);
}

/*static int getIndexPastWidth(const char* text, int width, const int* charWidth)
{
    int charnum;
    int len = strlen(text);
    
    for (int index = 0; index < len; index++)
    {
        char c = text[index];
        charnum = (unsigned char)(c) - 33;

        // spaces and nonprintable characters
        if (c == ' ' || charnum > 222)
        {
            width -= charWidth[0];
        }
        else
            width -= charWidth[charnum];
        
        if(width <= 0)
            return index;
    }
    return 0;
}*/



static list<string> explode(const string& str, char delimiter)
{
    list<string> result;
    
    unsigned int oldPos = 0;
    unsigned int pos = str.find_first_of(delimiter);
    while(pos != string::npos)
    {
        result.push_back(str.substr(oldPos, pos - oldPos));
        oldPos = pos+1;
        pos = str.find_first_of(delimiter, oldPos);
    }
    
    result.push_back(str.substr(oldPos, string::npos));
    
    return result;
}

SDL_Rect NFont::drawBox(SDL_Surface* dest, const SDL_Rect& box, const char* formatted_text, ...) const
{
    if(formatted_text == NULL)
        return makeRect(box.x, box.y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    SDL_Rect oldclip;
    SDL_GetClipRect(dest, &oldclip);
    SDL_SetClipRect(dest, &box);
    
    int y = box.y;
    
    using std::string;
    string text = buffer;
    list<string> ls = explode(text, '\n');
    for(list<string>::iterator e = ls.begin(); e != ls.end(); e++)
    {
        string line = *e;
        
        // If line is too long, then add words one at a time until we go over.
        if(getWidth(line.c_str()) > box.w)
        {
            list<string> words = explode(line, ' ');
            list<string>::iterator f = words.begin();
            line = *f;
            f++;
            while(f != words.end())
            {
                if(getWidth((line + " " + *f).c_str()) > box.w)
                {
                    drawLeft(dest, box.x, y, line.c_str());
                    y += getHeight();
                    line = *f;
                }
                else
                    line += " " + *f;
                
                f++;
            }
        }
        
        drawLeft(dest, box.x, y, line.c_str());
        y += getHeight();
    }
    
    SDL_SetClipRect(dest, &oldclip);
    
    return box;
}

SDL_Rect NFont::drawBox(SDL_Surface* dest, const SDL_Rect& box, AlignEnum align, const char* formatted_text, ...) const
{
    if(formatted_text == NULL)
        return makeRect(box.x, box.y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    SDL_Rect oldclip;
    SDL_GetClipRect(dest, &oldclip);
    SDL_SetClipRect(dest, &box);
    
    int y = box.y;
    
    using std::string;
    string text = buffer;
    list<string> ls = explode(text, '\n');
    for(list<string>::iterator e = ls.begin(); e != ls.end(); e++)
    {
        string line = *e;
        
        // If line is too long, then add words one at a time until we go over.
        if(getWidth(line.c_str()) > box.w)
        {
            list<string> words = explode(line, ' ');
            list<string>::iterator f = words.begin();
            line = *f;
            f++;
            while(f != words.end())
            {
                if(getWidth((line + " " + *f).c_str()) > box.w)
                {
                    switch(align)
                    {
                        case LEFT:
                            drawLeft(dest, box.x, y, line.c_str());
                            break;
                        case CENTER:
                            drawCenter(dest, box.x + box.w/2, y, line.c_str());
                            break;
                        case RIGHT:
                            drawRight(dest, box.x + box.w, y, line.c_str());
                            break;
                    }
                    y += getHeight();
                    line = *f;
                }
                else
                    line += " " + *f;
                
                f++;
            }
        }
        
        switch(align)
        {
            case LEFT:
                drawLeft(dest, box.x, y, line.c_str());
                break;
            case CENTER:
                drawCenter(dest, box.x + box.w/2, y, line.c_str());
                break;
            case RIGHT:
                drawRight(dest, box.x + box.w, y, line.c_str());
                break;
        }
        y += getHeight();
    }
    
    SDL_SetClipRect(dest, &oldclip);
    
    return box;
}

SDL_Rect NFont::drawColumn(SDL_Surface* dest, int x, int y, Uint16 width, const char* formatted_text, ...) const
{
    if(formatted_text == NULL)
        return makeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    int y0 = y;
    
    using std::string;
    string text = buffer;
    list<string> ls = explode(text, '\n');
    for(list<string>::iterator e = ls.begin(); e != ls.end(); e++)
    {
        string line = *e;
        
        // If line is too long, then add words one at a time until we go over.
        if(getWidth(line.c_str()) > width)
        {
            list<string> words = explode(line, ' ');
            list<string>::iterator f = words.begin();
            line = *f;
            f++;
            while(f != words.end())
            {
                if(getWidth((line + " " + *f).c_str()) > width)
                {
                    drawLeft(dest, x, y, line.c_str());
                    y += getHeight();
                    line = *f;
                }
                else
                    line += " " + *f;
                
                f++;
            }
        }
        
        drawLeft(dest, x, y, line.c_str());
        y += getHeight();
    }
    
    return makeRect(x, y0, width, y-y0);
}

SDL_Rect NFont::drawColumn(SDL_Surface* dest, int x, int y, Uint16 width, AlignEnum align, const char* formatted_text, ...) const
{
    if(formatted_text == NULL)
        return makeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    int y0 = y;
    
    using std::string;
    string text = buffer;
    list<string> ls = explode(text, '\n');
    for(list<string>::iterator e = ls.begin(); e != ls.end(); e++)
    {
        string line = *e;
        
        // If line is too long, then add words one at a time until we go over.
        if(getWidth(line.c_str()) > width)
        {
            list<string> words = explode(line, ' ');
            list<string>::iterator f = words.begin();
            line = *f;
            f++;
            while(f != words.end())
            {
                if(getWidth((line + " " + *f).c_str()) > width)
                {
                    switch(align)
                    {
                        case LEFT:
                            drawLeft(dest, x, y, line.c_str());
                            break;
                        case CENTER:
                            drawCenter(dest, x, y, line.c_str());
                            break;
                        case RIGHT:
                            drawRight(dest, x, y, line.c_str());
                            break;
                    }
                    y += getHeight();
                    line = *f;
                }
                else
                    line += " " + *f;
                
                f++;
            }
        }
        
        switch(align)
        {
            case LEFT:
                drawLeft(dest, x, y, line.c_str());
                break;
            case CENTER:
                drawCenter(dest, x, y, line.c_str());
                break;
            case RIGHT:
                drawRight(dest, x, y, line.c_str());
                break;
        }
        y += getHeight();
    }
    
    return makeRect(x, y0, width, y-y0);
}

SDL_Rect NFont::drawCenter(SDL_Surface* dest, int x, int y, const char* text) const
{
    if(text == NULL)
        return makeRect(x, y, 0, 0);

    char* str = copyString(text);
    char* del = str;

    // Go through str, when you find a \n, replace it with \0 and print it
    // then move down, back, and continue.
    for(char* c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            drawLeft(dest, x - getWidth("%s", str)/2, y, str);
            *c = '\n';
            c++;
            str = c;
            y += height;
        }
        else
            c++;
    }
    char s[strlen(str)+1];
    strcpy(s, str);
    delete[] del;
    
    return drawLeft(dest, x - getWidth("%s", s)/2, y, s);
}

SDL_Rect NFont::drawRight(SDL_Surface* dest, int x, int y, const char* text) const
{
    if(text == NULL)
        return makeRect(x, y, 0, 0);

    char* str = copyString(text);
    char* del = str;

    for(char* c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            drawLeft(dest, x - getWidth("%s", str), y, str);
            *c = '\n';
            c++;
            str = c;
            y += height;
        }
        else
            c++;
    }
    char s[strlen(str)+1];
    strcpy(s, str);
    delete[] del;
    
    return drawLeft(dest, x - getWidth("%s", s), y, s);
}



SDL_Rect NFont::draw(SDL_Surface* dest, int x, int y, AlignEnum align, const char* formatted_text, ...) const
{
    if(formatted_text == NULL)
        return makeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    switch(align)
    {
        case LEFT:
            return drawLeft(dest, x, y, buffer);
        case CENTER:
            return drawCenter(dest, x, y, buffer);
        case RIGHT:
            return drawRight(dest, x, y, buffer);
    }
    
    return makeRect(x, y, 0, 0);
}

SDL_Rect NFont::draw(SDL_Surface* dest, int x, int y, float t, NFont::AnimFn posFn, const char* text, ...) const
{
    va_list lst;
    va_start(lst, text);
    vsprintf(buffer, text, lst);
    va_end(lst);

    return drawAnimated(dest, x, y, t, posFn, NFont::LEFT);
}

SDL_Rect NFont::draw(SDL_Surface* dest, int x, int y, float t, NFont::AnimFn posFn, AlignEnum align, const char* text, ...) const
{
    va_list lst;
    va_start(lst, text);
    vsprintf(buffer, text, lst);
    va_end(lst);

    return drawAnimated(dest, x, y, t, posFn, align);
}




// Getters

SDL_Surface* NFont::getSurface() const
{
    return src;
}

Uint16 NFont::getHeight() const
{
    return height;
}

Uint16 NFont::getHeight(const char* formatted_text, ...) const
{
    if(formatted_text == NULL)
        return 0;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    Uint16 numLines = 1;
    const char* c;

    for (c = buffer; *c != '\0'; c++)
    {
        if(*c == '\n')
            numLines++;
    }

    //   Actual height of letter region + line spacing
    return height*numLines + lineSpacing*(numLines - 1);  //height*numLines;
}

Uint16 NFont::getWidth(const char* formatted_text, ...) const
{
    if (formatted_text == NULL)
        return 0;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    const char* c;
    int charnum = 0;
    Uint16 width = 0;
    Uint16 bigWidth = 0;  // Allows for multi-line strings

    for (c = buffer; *c != '\0'; c++)
    {
        charnum = (unsigned char)(*c) - 33;

        // skip spaces and nonprintable characters
        if(*c == '\n')
        {
            bigWidth = bigWidth >= width? bigWidth : width;
            width = 0;
        }
        else if (*c == ' ' || charnum > 222)
        {
            width += charWidth[0];
            continue;
        }

        width += charWidth[charnum];
    }
    bigWidth = bigWidth >= width? bigWidth : width;

    return bigWidth;
}


Uint16 NFont::getColumnHeight(Uint16 width, const char* formatted_text, ...) const
{
    if(formatted_text == NULL || width == 0)
        return height;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    int y = 0;
    
    using std::string;
    string text = buffer;
    list<string> ls = explode(text, '\n');
    for(list<string>::iterator e = ls.begin(); e != ls.end(); e++)
    {
        string line = *e;
        
        // If line is too long, then add words one at a time until we go over.
        if(getWidth(line.c_str()) > width)
        {
            list<string> words = explode(line, ' ');
            list<string>::iterator f = words.begin();
            line = *f;
            f++;
            while(f != words.end())
            {
                if(getWidth((line + " " + *f).c_str()) > width)
                {
                    y += getHeight();
                    line = *f;
                }
                else
                    line += " " + *f;
                
                f++;
            }
        }
        
        y += getHeight();
    }
    
    return y;
}

int NFont::getAscent(const char character) const
{
    unsigned char test = (unsigned char)character;
    if(test < 33 || test > 222 || (test > 126 && test < 161))
        return 0;
    unsigned char num = (unsigned char)character - 33;
    // Get the max ascent
    int x = charPos[num];
    int i, j = 1, result = 0;
    Uint32 pixel = getPixel(src, 0, src->h - 1); // bg pixel
    while(j < baseline && j < src->h)
    {
        i = charPos[num];
        while(i < x + charWidth[num])
        {
            if(getPixel(src, i, j) != pixel)
            {
                result = baseline - j;
                j = src->h;
                break;
            }
            i++;
        }
        j++;
    }
    return result;
}

int NFont::getAscent() const
{
    return ascent;
}

int NFont::getAscent(const char* formatted_text, ...) const
{
    if(formatted_text == NULL)
        return ascent;
    
    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    int max = 0;
    const char* c = buffer;
    
    for (; *c != '\0'; c++)
    {
        int asc = getAscent(*c);
        if(asc > max)
            max = asc;
    }
    return max;
}

int NFont::getDescent(const char character) const
{
    unsigned char test = (unsigned char)character;
    if(test < 33 || test > 222 || (test > 126 && test < 161))
        return 0;
    unsigned char num = (unsigned char)character - 33;
    // Get the max descent
    int x = charPos[num];
    int i, j = src->h - 1, result = 0;
    Uint32 pixel = getPixel(src, 0, src->h - 1); // bg pixel
    while(j > 0 && j > baseline)
    {
        i = charPos[num];
        while(i < x + charWidth[num])
        {
            if(getPixel(src, i, j) != pixel)
            {
                result = j - baseline;
                j = 0;
                break;
            }
            i++;
        }
        j--;
    }
    return result;
}

int NFont::getDescent() const
{
    return descent;
}

int NFont::getDescent(const char* formatted_text, ...) const
{
    if(formatted_text == NULL)
        return descent;
    
    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    int max = 0;
    const char* c = buffer;
    
    for (; *c != '\0'; c++)
    {
        int des = getDescent(*c);
        if(des > max)
            max = des;
    }
    return max;
}

int NFont::getSpacing() const
{
    return letterSpacing;
}

int NFont::getLineSpacing() const
{
    return lineSpacing;
}

Uint16 NFont::getBaseline() const
{
    return baseline;
}

Uint16 NFont::getMaxWidth() const
{
    return maxWidth;
}





// Setters
void NFont::setSpacing(int LetterSpacing)
{
    letterSpacing = LetterSpacing;
}

void NFont::setLineSpacing(int LineSpacing)
{
    lineSpacing = LineSpacing;
}

void NFont::setBaseline()
{
    // TODO: Find a better way
    // Get the baseline by checking a, b, and c and averaging their lowest y-value.
    Uint32 pixel = getPixel(src, 0, src->h - 1);
    int heightSum = 0;
    int x, i, j;
    for(unsigned char avgChar = 64; avgChar < 67; avgChar++)
    {
        x = charPos[avgChar];
        
        j = src->h - 1;
        while(j > 0)
        {
            i = x;
            while(i - x < charWidth[64])
            {
                if(getPixel(src, i, j) != pixel)
                {
                    heightSum += j;
                    j = 0;
                    break;
                }
                i++;
            }
            j--;
        }
    }
    baseline = int(heightSum/3.0f + 0.5f);  // Round up and cast
}

void NFont::setBaseline(Uint16 Baseline)
{
    baseline = Baseline;
}



void NFont::optimizeForVideoSurface()
{
    if(src == NULL)
        return;
    SDL_Surface* temp = src;
    
    if(src->format->Amask == 0)
        src = SDL_DisplayFormat(src);
    else
        src = SDL_DisplayFormatAlpha(src);
    
    SDL_FreeSurface(temp);
}



namespace NFontAnim
{

void bounce(int& x, int& y, NFont::AnimData& data)
{
    y -= int(20*fabs(sin(-4*data.t + (x - data.startX)/40.0)));
    
    if(data.align == NFont::CENTER)
    {
        x -= data.font->getWidth(data.text)/2;
    }
    else if(data.align == NFont::RIGHT)
    {
        x -= data.font->getWidth(data.text);
    }
}


void wave(int& x, int& y, NFont::AnimData& data)
{
    y += int(20*sin(-4*data.t + (x - data.startX)/40.0));
    
    if(data.align == NFont::CENTER)
    {
        x -= data.font->getWidth(data.text)/2;
    }
    else if(data.align == NFont::RIGHT)
    {
        x -= data.font->getWidth(data.text);
    }
}


void stretch(int& x, int& y, NFont::AnimData& data)
{
    float place = float(data.index) / strlen(data.text);
    if(data.align == NFont::CENTER)
    {
        place -= 0.5f;
        x -= data.font->getWidth(data.text)/2;
    }
    else if(data.align == NFont::RIGHT)
    {
        place -= 1.0f;
        x -= data.font->getWidth(data.text);
    }
    x += int(80*(place)*cos(2*data.t));
}


void shake(int& x, int& y, NFont::AnimData& data)
{
    if(data.align == NFont::CENTER)
    {
        x -= data.font->getWidth(data.text)/2;
    }
    else if(data.align == NFont::RIGHT)
    {
        x -= data.font->getWidth(data.text);
    }
    x += int(4*sin(40*data.t));
}

}



