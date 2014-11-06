/*
NFont v4.0.0: A font class for SDL
by Jonathan Dearborn
Dedicated to the memory of Florian Hufsky

License:
    The short:
    Use it however you'd like, but keep the copyright and license notice 
    whenever these files or parts of them are distributed in uncompiled form.
    
    The long:
Copyright (c) 2014 Jonathan Dearborn

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
#include <cstring>
#include <list>
using std::string;
using std::list;

#ifndef NFONT_USE_SDL_HELPERS
#define NFont_Rectf NFont::Rectf
#endif

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

static inline void drawPixel(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color, Uint8 alpha)
{
	if(x > surface->clip_rect.x + surface->clip_rect.w || x < surface->clip_rect.x || y > surface->clip_rect.y + surface->clip_rect.h || y < surface->clip_rect.y)
        return;

    switch (surface->format->BytesPerPixel)
    {
        case 1: { /* Assuming 8-bpp */

                Uint8 *pixel = (Uint8 *)surface->pixels + y*surface->pitch + x;

                Uint8 dR = surface->format->palette->colors[*pixel].r;
                Uint8 dG = surface->format->palette->colors[*pixel].g;
                Uint8 dB = surface->format->palette->colors[*pixel].b;
                Uint8 sR = surface->format->palette->colors[color].r;
                Uint8 sG = surface->format->palette->colors[color].g;
                Uint8 sB = surface->format->palette->colors[color].b;

                dR = dR + ((sR-dR)*alpha >> 8);
                dG = dG + ((sG-dG)*alpha >> 8);
                dB = dB + ((sB-dB)*alpha >> 8);

                *pixel = SDL_MapRGB(surface->format, dR, dG, dB);

        }
        break;

        case 2: { /* Probably 15-bpp or 16-bpp */

                Uint32 Rmask = surface->format->Rmask, Gmask = surface->format->Gmask, Bmask = surface->format->Bmask, Amask = surface->format->Amask;
                Uint16 *pixel = (Uint16 *)surface->pixels + y*surface->pitch/2 + x;
                Uint32 dc = *pixel;
                Uint32 R,G,B,A=0;

                R = ((dc & Rmask) + (( (color & Rmask) - (dc & Rmask) ) * alpha >> 8)) & Rmask;
                G = ((dc & Gmask) + (( (color & Gmask) - (dc & Gmask) ) * alpha >> 8)) & Gmask;
                B = ((dc & Bmask) + (( (color & Bmask) - (dc & Bmask) ) * alpha >> 8)) & Bmask;
                if( Amask )
                    A = ((dc & Amask) + (( (color & Amask) - (dc & Amask) ) * alpha >> 8)) & Amask;

                *pixel= R | G | B | A;

        }
        break;

        case 3: { /* Slow 24-bpp mode, usually not used */
            Uint8 *pix = (Uint8 *)surface->pixels + y * surface->pitch + x*3;
            Uint8 rshift8=surface->format->Rshift/8;
            Uint8 gshift8=surface->format->Gshift/8;
            Uint8 bshift8=surface->format->Bshift/8;
            Uint8 ashift8=surface->format->Ashift/8;



                Uint8 dR, dG, dB, dA;
                Uint8 sR, sG, sB, sA;

                pix = (Uint8 *)surface->pixels + y * surface->pitch + x*3;

                dR = *((pix)+rshift8);
                dG = *((pix)+gshift8);
                dB = *((pix)+bshift8);
                dA = *((pix)+ashift8);

                sR = (color>>surface->format->Rshift)&0xff;
                sG = (color>>surface->format->Gshift)&0xff;
                sB = (color>>surface->format->Bshift)&0xff;
                sA = (color>>surface->format->Ashift)&0xff;

                dR = dR + ((sR-dR)*alpha >> 8);
                dG = dG + ((sG-dG)*alpha >> 8);
                dB = dB + ((sB-dB)*alpha >> 8);
                dA = dA + ((sA-dA)*alpha >> 8);

                *((pix)+rshift8) = dR;
                *((pix)+gshift8) = dG;
                *((pix)+bshift8) = dB;
                *((pix)+ashift8) = dA;

        }
        break;

        case 4: { /* Probably 32-bpp */
            Uint32 Rmask = surface->format->Rmask, Gmask = surface->format->Gmask, Bmask = surface->format->Bmask, Amask = surface->format->Amask;
            Uint32* pixel = (Uint32*)surface->pixels + y*surface->pitch/4 + x;
            Uint32 source = *pixel;
            Uint32 R,G,B,A;
            R = color & Rmask;
            G = color & Gmask;
            B = color & Bmask;
            A = 0;  // keep this as 0 to avoid corruption of non-alpha surfaces

            // Blend and keep dest alpha
            if( alpha != SDL_ALPHA_OPAQUE ){
                R = ((source & Rmask) + (( R - (source & Rmask) ) * alpha >> 8)) & Rmask;
                G = ((source & Gmask) + (( G - (source & Gmask) ) * alpha >> 8)) & Gmask;
                B = ((source & Bmask) + (( B - (source & Bmask) ) * alpha >> 8)) & Bmask;
            }
            if(Amask)
                A = (source & Amask);

            *pixel = R | G | B | A;
        }
        break;
    }
}

static inline GPU_Rect rectUnion(const GPU_Rect& A, const GPU_Rect& B)
{
    float x,x2,y,y2;
    x = MIN(A.x, B.x);
    y = MIN(A.y, B.y);
    x2 = MAX(A.x+A.w, B.x+B.w);
    y2 = MAX(A.y+A.h, B.y+B.h);
    GPU_Rect result = {x, y, MAX(0, x2 - x), MAX(0, y2 - y)};
    return result;
}

// Adapted from SDL_IntersectRect
static inline GPU_Rect rectIntersect(const GPU_Rect& A, const GPU_Rect& B)
{
    GPU_Rect result;
	float Amin, Amax, Bmin, Bmax;

	// Horizontal intersection
	Amin = A.x;
	Amax = Amin + A.w;
	Bmin = B.x;
	Bmax = Bmin + B.w;
	if(Bmin > Amin)
	        Amin = Bmin;
	result.x = Amin;
	if(Bmax < Amax)
	        Amax = Bmax;
	result.w = Amax - Amin > 0 ? Amax - Amin : 0;

	// Vertical intersection
	Amin = A.y;
	Amax = Amin + A.h;
	Bmin = B.y;
	Bmax = Bmin + B.h;
	if(Bmin > Amin)
	        Amin = Bmin;
	result.y = Amin;
	if(Bmax < Amax)
	        Amax = Bmax;
	result.h = Amax - Amin > 0 ? Amax - Amin : 0;

	return result;
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
NFont::Color::Color(const SDL_Color& color)
    : r(color.r), g(color.g), b(color.b), a(color.a)
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

NFont::Color& NFont::Color::color(const SDL_Color& color)
{
    r = color.r;
    g = color.g;
    b = color.b;
    a = color.a;

    return *this;
}

SDL_Color NFont::Color::to_SDL_Color() const
{
    SDL_Color c = {r, g, b, a};
    return c;
}



#ifndef NFONT_USE_SDL_HELPERS

NFont::Rectf::Rectf()
    : x(0), y(0), w(0), h(0)
{}

NFont::Rectf::Rectf(float x, float y)
    : x(x), y(y), w(0), h(0)
{}

NFont::Rectf::Rectf(float x, float y, float w, float h)
    : x(x), y(y), w(w), h(h)
{}

NFont::Rectf::Rectf(const SDL_Rect& rect)
    : x(rect.x), y(rect.y), w(rect.w), h(rect.h)
{}

NFont::Rectf::Rectf(const GPU_Rect& rect)
    : x(rect.x), y(rect.y), w(rect.w), h(rect.h)
{}

SDL_Rect NFont::Rectf::to_SDL_Rect() const
{
    SDL_Rect r = {int(x), int(y), int(w), int(h)};
    return r;
}

GPU_Rect NFont::Rectf::to_GPU_Rect() const
{
    return GPU_MakeRect(x, y, w, h);
}

#endif







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

    SDL_BlendMode blendMode;
    SDL_GetSurfaceBlendMode(surface, &blendMode);
    bool useCK = (blendMode == SDL_BLENDMODE_NONE);  // colorkey if no alpha
    Uint32 colorkey;
    SDL_GetColorKey(surface, &colorkey);

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
    load(copySurface(font.srcSurface));
}

NFont::NFont(SDL_Surface* src)
{
    init();
    load(src);
}

#ifndef NFONT_NO_TTF
NFont::NFont(TTF_Font* ttf)
{
    init();
    load(ttf, default_color);
}
NFont::NFont(TTF_Font* ttf, const NFont::Color& color)
{
    init();
    load(ttf, color);
}
NFont::NFont(const char* filename_ttf, Uint32 pointSize, const NFont::Color& fg, int style)
{
    init();
    load(filename_ttf, pointSize, fg, style);
}

#endif

NFont::~NFont()
{
    free();
}


NFont& NFont::operator=(const NFont& font)
{
    load(copySurface(font.srcSurface));
    return *this;
}

void NFont::init()
{
    ttf_source = NULL;
    owns_ttf_source = false;
    ttf_source_color = GPU_MakeColor(255, 255, 255, 255);
    src = NULL;
    srcSurface = NULL;
    
    default_color.rgba(0, 0, 0, 255);

    height = 0; // ascent+descent

    maxWidth = 0;
    baseline = 0;
    ascent = 0;
    descent = 0;

    lineSpacing = 0;
    letterSpacing = 0;

    if(buffer == NULL)
        buffer = new char[1024];
    
    lastGlyph.x = 0;
    lastGlyph.y = 0;
    lastGlyph.w = 0;
    lastGlyph.h = 0;
    
    glyphs.clear();
}


static void makeColorTransparent(GPU_Image* image, SDL_Color color)
{
    SDL_Surface* surface = GPU_CopySurfaceFromImage(image);
    Uint8* pixels = (Uint8*)surface->pixels;
    
    int x,y,i;
    for(y = 0; y < surface->h; y++)
    {
        for(x = 0; x < surface->w; x++)
        {
            i = y*surface->pitch + x*surface->format->BytesPerPixel;
            if(pixels[i] == color.r && pixels[i+1] == color.g && pixels[i+2] == color.b)
                pixels[i+3] = 0;
        }
    }
    
    GPU_UpdateImage(image, surface, NULL);
    SDL_FreeSurface(surface);
}

// Loading
bool NFont::load(SDL_Surface* FontSurface)
{
    free();

    srcSurface = FontSurface;
    if(srcSurface == NULL)
    {
        GPU_LogError("\n ERROR: NFont given a NULL surface\n");
        return false;
    }


    int x = 1, i = 0;
    
    // Data for referencing the surface
    Uint16 charWidth[256];
    int charPos[256];
    // memset would be faster
    for(int j = 0; j < 256; j++)
    {
        charWidth[j] = 0;
        charPos[j] = 0;
    }

    SDL_LockSurface(srcSurface);

    Uint32 pixel = SDL_MapRGB(srcSurface->format, 255, 0, 255); // pink pixel

    // Get the character positions and widths
    while (x < srcSurface->w)
    {
        if(getPixel(srcSurface, x, 0) != pixel)
        {
            charPos[i] = x;
            charWidth[i] = x;
            while(x < srcSurface->w && getPixel(srcSurface, x, 0) != pixel)
                x++;
            charWidth[i] = x - charWidth[i];
            i++;
        }

        x++;
    }


    pixel = getPixel(srcSurface, 0, srcSurface->h - 1);
    int j;
    //setBaseline(); // FIXME: This needs to work!
    
    // Get the baseline by checking a, b, and c and averaging their lowest y-value.
    {
        Uint32 pixel = getPixel(srcSurface, 0, srcSurface->h - 1);
        int heightSum = 0;
        int x, i, j;
        for(unsigned char avgChar = 64; avgChar < 67; avgChar++)
        {
            x = charPos[avgChar];

            j = srcSurface->h - 1;
            while(j > 0)
            {
                i = x;
                while(i - x < charWidth[64])
                {
                    if(getPixel(srcSurface, i, j) != pixel)
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


    // Get the max ascent
    j = 1;
    while(j < baseline && j < srcSurface->h)
    {
        x = 0;
        while(x < srcSurface->w)
        {
            if(getPixel(srcSurface, x, j) != pixel)
            {
                ascent = baseline - j;
                j = srcSurface->h;
                break;
            }
            x++;
        }
        j++;
    }

    // Get the max descent
    j = srcSurface->h - 1;
    while(j > 0 && j > baseline)
    {
        x = 0;
        while(x < srcSurface->w)
        {
            if(getPixel(srcSurface, x, j) != pixel)
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

    SDL_BlendMode blendMode;
    SDL_GetSurfaceBlendMode(srcSurface, &blendMode);
    if(blendMode == SDL_BLENDMODE_NONE)
    {
        pixel = getPixel(srcSurface, 0, srcSurface->h - 1);
        SDL_UnlockSurface(srcSurface);
        SDL_SetColorKey(srcSurface, SDL_TRUE, pixel);
    }
    else
    {
        SDL_UnlockSurface(srcSurface);
    }
    
    // Copy glyphs from the surface to the font texture and store the position data
    // Pack row by row into a square texture
    SDL_Surface* dest = createSurface32(2048, 2048);
    SDL_SetSurfaceBlendMode(srcSurface, SDL_BLENDMODE_NONE);
    lastGlyph.x = 0;
    lastGlyph.y = 0;
    lastGlyph.w = charWidth[0];
    lastGlyph.h = height;
    // Space
    glyphs.insert(std::make_pair(32, GlyphData(0, lastGlyph)));
    lastGlyph.w = 0;
    for(j = 0; j < i; j++)
    {
        SDL_Rect srcRect = {charPos[j], 0, charWidth[j], srcSurface->h};
        if(!addGlyph(j+33, charWidth[j], dest->w, dest->h))
            break;
        
        SDL_Rect destrect = lastGlyph;
        SDL_BlitSurface(srcSurface, &srcRect, dest, &destrect);
    }
    
    
    src = GPU_CopyImageFromSurface(dest);
    SDL_FreeSurface(dest);
    SDL_Color pink = {255, 0, 255, 255};
    makeColorTransparent(src, pink);

    SDL_SetSurfaceBlendMode(srcSurface, blendMode);
    return true;
}

#ifndef NFONT_NO_TTF


bool NFont::addGlyph(Uint32 codepoint, Uint16 width, Uint16 maxWidth, Uint16 maxHeight)
{
    if(lastGlyph.x + lastGlyph.w + width >= maxWidth)
    {
        if(lastGlyph.y + height + height >= maxHeight)
        {
            // Ran out of room on texture
            GPU_LogError("Error: NFont ran out of packing space for glyphs!\n");
            return false;
        }
        
        // Go to next row
        lastGlyph.x = 0;
        lastGlyph.y += height;
        lastGlyph.w = 0;
    }
    
    // Move to next space
    lastGlyph.x += lastGlyph.w + 1;
    lastGlyph.w = width;
    
    glyphs.insert(std::make_pair(codepoint, GlyphData(0, lastGlyph)));
    return true;
}


bool NFont::load(TTF_Font* ttf)
{
    return load(ttf, default_color);
}

bool NFont::load(TTF_Font* ttf, const NFont::Color& color)
{
    free();

    if(ttf == NULL)
        return false;
    
    ttf_source = ttf;
    
    height = TTF_FontHeight(ttf);
    ascent = TTF_FontAscent(ttf);
    descent = -TTF_FontDescent(ttf);
    
    baseline = height - descent;

    default_color = color;
    ttf_source_color = color.to_SDL_Color();
    
    SDL_Color white = {255, 255, 255, 255};

    SDL_Surface* surf;
    
    // Copy glyphs from the surface to the font texture and store the position data
    // Pack row by row into a square texture
    // Try figuring out dimensions that make sense for the font size.
    unsigned int dim = height*8;
    SDL_Surface* dest = createSurface32(dim, dim);
    lastGlyph.x = 0;
    lastGlyph.y = 0;
    lastGlyph.w = 0;
    lastGlyph.h = height;

    char buff[2];
    buff[1] = '\0';
    for(int i = 0; i < 127 - 32; i++)
    {
        buff[0] = i + 32;
        surf = TTF_RenderUTF8_Blended(ttf, buff, white);
        SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_NONE);
        
        SDL_Rect srcRect = {0, 0, surf->w, surf->h};
        if(!addGlyph(buff[0], surf->w, dest->w, dest->h))
        {
            SDL_FreeSurface(surf);
            break;
        }
        
        SDL_Rect destrect = lastGlyph;
        SDL_BlitSurface(surf, &srcRect, dest, &destrect);
        SDL_FreeSurface(surf);
    }
    
    src = GPU_CopyImageFromSurface(dest);
    SDL_FreeSurface(dest);
    
    return true;
}


bool NFont::load(const char* filename_ttf, Uint32 pointSize, const NFont::Color& color, int style)
{
    if(!TTF_WasInit() && TTF_Init() < 0)
    {
        GPU_LogError("Unable to initialize SDL_ttf: %s \n", TTF_GetError());
        return false;
    }

    TTF_Font* ttf = TTF_OpenFont(filename_ttf, pointSize);

    if(ttf == NULL)
    {
        GPU_LogError("Unable to load TrueType font: %s \n", TTF_GetError());
        return false;
    }
    
    bool outline = false;
    outline = (style & TTF_STYLE_OUTLINE);
    if(outline)
    {
        style &= ~TTF_STYLE_OUTLINE;
        TTF_SetFontOutline(ttf, 1);
    }
    TTF_SetFontStyle(ttf, style);
    bool result = load(ttf, color);
    owns_ttf_source = true;
    return result;
}

#endif

void NFont::free()
{
    if(owns_ttf_source)
    {
        TTF_CloseFont(ttf_source);
        ttf_source = NULL;
    }
    SDL_FreeSurface(srcSurface);
    GPU_FreeImage(src);

    srcSurface = NULL;
    src = NULL;
    
    init();
}



NFont_Rectf scaleAndBlit(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float xscale, float yscale)
{
    float w = srcrect->w * xscale;
    float h = srcrect->h * yscale;
    
    // FIXME: Why does the scaled offset look so wrong?
    GPU_BlitScale(src, srcrect, dest, x + xscale*srcrect->w/2.0f, y + srcrect->h/2.0f, xscale, yscale);

    NFont_Rectf result(x, y, w, h);
    return result;
}

void getUTF8FromCodepoint(char* result, Uint32 codepoint)
{
    char a, b, c, d;
    
    a = (codepoint >> 24) & 0xFF;
    b = (codepoint >> 16) & 0xFF;
    c = (codepoint >> 8) & 0xFF;
    d = codepoint & 0xFF;
    
    memset(result, 0, 4);
    
    if(a == 0)
    {
        if(b == 0)
        {
            if(c == 0)
            {
                result[0] = d;
            }
            else
            {
                result[0] = c;
                result[1] = d;
            }
        }
        else
        {
            result[0] = b;
            result[1] = c;
            result[2] = d;
        }
    }
    else
    {
        result[0] = a;
        result[1] = b;
        result[2] = c;
        result[3] = d;
    }
}

Uint32 getCodepointFromUTF8(const char*& c)
{
    Uint32 result = 0;
    if((unsigned char)*c <= 0x7F)
        result = *c;
    else if((unsigned char)*c < 0xE0)
    {
        result |= (unsigned char)(*c) << 8;
        c++;
        result |= (unsigned char)*c;
    }
    else if((unsigned char)*c < 0xF0)
    {
        result |= (unsigned char)(*c) << 16;
        c++;
        result |= (unsigned char)(*c) << 8;
        c++;
        result |= (unsigned char)*c;
    }
    else
    {
        result |= (unsigned char)(*c) << 24;
        c++;
        result |= (unsigned char)(*c) << 16;
        c++;
        result |= (unsigned char)(*c) << 8;
        c++;
        result |= (unsigned char)*c;
    }
    return result;
}

bool NFont::getGlyphData(NFont::GlyphData* result, Uint32 codepoint)
{
    std::map<Uint32, GlyphData>::const_iterator e = glyphs.find(codepoint);
    if(e == glyphs.end() || result == NULL)
    {
        if(ttf_source == NULL)
            return false;
        
        GPU_Target* dest = GPU_LoadTarget(src);
        char buff[5];
        getUTF8FromCodepoint(buff, codepoint);
        
        SDL_Surface* surf = TTF_RenderUTF8_Blended(ttf_source, buff, ttf_source_color);
        SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_NONE);
        
        if(!addGlyph(codepoint, surf->w, dest->w, dest->h))
        {
            SDL_FreeSurface(surf);
            return false;
        }
        
        GPU_Image* img = GPU_CopyImageFromSurface(surf);
        SDL_FreeSurface(surf);
        
        SDL_Rect destrect = lastGlyph;
        GPU_Blit(img, NULL, dest, destrect.x + destrect.w/2, destrect.y + destrect.h/2);
        GPU_FreeImage(img);
        
        GPU_FreeTarget(dest);
    }
    *result = e->second;
    return true;
}

// Drawing
GPU_Rect NFont::render_left(GPU_Target* dest, float x, float y, const Scale& scale, const char* text)
{
    const char* c = text;
    Rectf srcRect, dstRect, copyS, copyD;
    data.dirtyRect = GPU_MakeRect(x, y, 0, 0);

    if(c == NULL || src == NULL || dest == NULL)
        return data.dirtyRect;

    srcRect.y = baseline - ascent;
    srcRect.h = dstRect.h = height;
    //dstRect.h *= scale.y;
    float destH = dstRect.h * scale.y;
    //dstRect.x = x;
    //dstRect.y = y;
    float destX = x;
    float destY = y;
    float destLineSpacing = lineSpacing*scale.y;
    float destLetterSpacing = letterSpacing*scale.x;

    int newlineX = x;

    for(; *c != '\0'; c++)
    {
        if(*c == '\n')
        {
            destX = newlineX;
            destY += destH + destLineSpacing;
            continue;
        }
        
        GlyphData glyph;
        Uint32 codepoint = getCodepointFromUTF8(c);  // Increments 'c' to skip the extra UTF-8 bytes
        if(!getGlyphData(&glyph, codepoint))
            continue;  // Skip bad characters

        if (codepoint == ' ')
        {
            destX += glyph.rect.w*scale.x + destLetterSpacing;
            continue;
        }
        /*if(destX >= dest->w)
            continue;
        if(destY >= dest->h)
            continue;*/

        float destW = glyph.rect.w*scale.x;
        copyS = glyph.rect;
        copyD = dstRect;
        //SDL_BlitSurface(src, &srcRect, dest, &dstRect);
        GPU_Rect sr = copyS.to_GPU_Rect();
        dstRect = scaleAndBlit(src, &sr, dest, destX, destY, scale.x, scale.y);
        if(data.dirtyRect.w == 0 || data.dirtyRect.h == 0)
            data.dirtyRect = dstRect.to_GPU_Rect();
        else
            data.dirtyRect = rectUnion(data.dirtyRect, dstRect.to_GPU_Rect());
        //srcRect = copyS;
        //dstRect = copyD;

        destX += destW + destLetterSpacing;
    }

    return data.dirtyRect;
}

GPU_Rect NFont::render_left(GPU_Target* dest, float x, float y, const char* text)
{
    return render_left(dest, x, y, Scale(1.0f), text);
}

GPU_Rect NFont::render_animated(GPU_Target* dest, float x, float y, const NFont::AnimParams& params, NFont::AnimFn posFn, NFont::AlignEnum align)
{
    const char* c = buffer;
    Scale scale;
    Rectf srcRect, dstRect, copyS, copyD;
    data.dirtyRect = GPU_MakeRect(x, y, 0, 0);

    if(c == NULL || src == NULL || dest == NULL)
        return data.dirtyRect;

    srcRect.y = baseline - ascent;
    srcRect.h = dstRect.h = height;
    //dstRect.h *= scale.y;
    float destH = dstRect.h * scale.y;
    //dstRect.x = x;
    //dstRect.y = y;
    float destLineSpacing = lineSpacing*scale.y;
    float destLetterSpacing = letterSpacing*scale.x;
    
    
    int preFnX = x;
    int preFnY = y;
    data.font = this;
    data.dest = dest;
    data.src = src;
    data.text = buffer;  // Buffer for efficient drawing
    //data.charPos = charPos;
    //data.charWidth = charWidth;
    //data.maxX = maxPos;
    data.dirtyRect = GPU_MakeRect(x,y,0,0);

    data.index = -1;
    data.letterNum = 0;
    data.wordNum = 1;
    data.lineNum = 1;
    data.startX = x;  // used as reset value for line feed
    data.startY = y;

    data.align = align;

    for(; *c != '\0'; c++)
    {
        data.index++;
        data.letterNum++;
        
        if(*c == '\n')
        {
            data.letterNum = 1;
            data.wordNum = 1;
            data.lineNum++;
            
            x = data.startX;
            y += destH + destLineSpacing;
            continue;
        }
        
        GlyphData glyph;
        Uint32 codepoint = getCodepointFromUTF8(c);  // Increments 'c' to skip the extra UTF-8 bytes
        if(!getGlyphData(&glyph, codepoint))
            continue;  // Skip bad characters

        if (codepoint == ' ')
        {
            data.letterNum = 1;
            data.wordNum++;
            
            x += glyph.rect.w*scale.x + destLetterSpacing;
            continue;
        }
        /*if(x >= dest->w)
            continue;
        if(y >= dest->h)
            continue;*/

        preFnX = x;  // Save real position
        preFnY = y;

        // Use function pointer to get final x, y values
        posFn(x, y, params, data);

        dstRect.x = x;
        dstRect.y = y;
        
        float destW = glyph.rect.w*scale.x;
        copyS = glyph.rect;
        copyD = dstRect;
        
        GPU_Rect sr = copyS.to_GPU_Rect();
        dstRect = scaleAndBlit(src, &sr, dest, dstRect.x + srcRect.w/2, dstRect.y + srcRect.h/2, scale.x, scale.y);
        if(data.dirtyRect.w == 0 || data.dirtyRect.h == 0)
            data.dirtyRect = dstRect.to_GPU_Rect();
        else
            data.dirtyRect = rectUnion(data.dirtyRect, dstRect.to_GPU_Rect());
        
        x = preFnX;  // Restore real position
        y = preFnY;

        x += destW + destLetterSpacing;
    }
    
    return data.dirtyRect;
}

GPU_Rect NFont::draw(GPU_Target* dest, float x, float y, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return GPU_MakeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    GPU_SetRGBA(src, default_color.r, default_color.g, default_color.b, default_color.a);
    return render_left(dest, x, y, buffer);
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

    size_t oldPos = 0;
    size_t pos = str.find_first_of(delimiter);
    while(pos != string::npos)
    {
        result.push_back(str.substr(oldPos, pos - oldPos));
        oldPos = pos+1;
        pos = str.find_first_of(delimiter, oldPos);
    }

    result.push_back(str.substr(oldPos, string::npos));

    return result;
}

GPU_Rect NFont::drawBox(GPU_Target* dest, const Rectf& box, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return GPU_MakeRect(box.x, box.y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    bool useClip = dest->use_clip_rect;
    GPU_Rect oldclip, newclip;
    oldclip = dest->clip_rect;
    newclip = rectIntersect(oldclip, box.to_GPU_Rect());
    GPU_SetClipRect(dest, newclip);
    
    GPU_SetRGBA(src, default_color.r, default_color.g, default_color.b, default_color.a);

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
            line = *f + " ";
            f++;
            while(f != words.end())
            {
                if(getWidth((line + *f).c_str()) > box.w)
                {
                    render_left(dest, box.x, y, line.c_str());
                    y += getHeight();
                    line = *f + " ";
                }
                else
                    line += *f + " ";

                f++;
            }
        }

        render_left(dest, box.x, y, line.c_str());
        y += getHeight();
    }
    if(useClip)
        GPU_SetClipRect(dest, oldclip);
    else
        GPU_UnsetClip(dest);

    return box.to_GPU_Rect();
}

GPU_Rect NFont::drawBox(GPU_Target* dest, const Rectf& box, AlignEnum align, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return GPU_MakeRect(box.x, box.y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    bool useClip = dest->use_clip_rect;
    GPU_Rect oldclip, newclip;
    oldclip = dest->clip_rect;
    newclip = rectIntersect(oldclip, box.to_GPU_Rect());
    GPU_SetClipRect(dest, newclip);
    
    GPU_SetRGBA(src, default_color.r, default_color.g, default_color.b, default_color.a);

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
            line = *f + " ";
            f++;
            while(f != words.end())
            {
                if(getWidth((line + *f).c_str()) > box.w)
                {
                    switch(align)
                    {
                        case LEFT:
                            render_left(dest, box.x, y, line.c_str());
                            break;
                        case CENTER:
                            render_center(dest, box.x + box.w/2, y, line.c_str());
                            break;
                        case RIGHT:
                            render_right(dest, box.x + box.w, y, line.c_str());
                            break;
                    }
                    y += getHeight();
                    line = *f + " ";
                }
                else
                    line += *f + " ";

                f++;
            }
        }

        switch(align)
        {
            case LEFT:
                render_left(dest, box.x, y, line.c_str());
                break;
            case CENTER:
                render_center(dest, box.x + box.w/2, y, line.c_str());
                break;
            case RIGHT:
                render_right(dest, box.x + box.w, y, line.c_str());
                break;
        }
        y += getHeight();
    }

    if(useClip)
        GPU_SetClipRect(dest, oldclip);
    else
        GPU_UnsetClip(dest);

    return box.to_GPU_Rect();
}

GPU_Rect NFont::drawColumn(GPU_Target* dest, float x, float y, Uint16 width, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return GPU_MakeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    GPU_SetRGBA(src, default_color.r, default_color.g, default_color.b, default_color.a);

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
            line = *f + " ";
            f++;
            while(f != words.end())
            {
                if(getWidth((line + *f).c_str()) > width)
                {
                    render_left(dest, x, y, line.c_str());
                    y += getHeight();
                    line = *f + " ";
                }
                else
                    line += *f + " ";

                f++;
            }
        }

        render_left(dest, x, y, line.c_str());
        y += getHeight();
    }

    return GPU_MakeRect(x, y0, width, y-y0);
}

GPU_Rect NFont::drawColumn(GPU_Target* dest, float x, float y, Uint16 width, AlignEnum align, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return GPU_MakeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    GPU_SetRGBA(src, default_color.r, default_color.g, default_color.b, default_color.a);

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
            line = *f + " ";
            f++;
            while(f != words.end())
            {
                if(getWidth((line + *f).c_str()) > width)
                {
                    switch(align)
                    {
                        case LEFT:
                            render_left(dest, x, y, line.c_str());
                            break;
                        case CENTER:
                            render_center(dest, x, y, line.c_str());
                            break;
                        case RIGHT:
                            render_right(dest, x, y, line.c_str());
                            break;
                    }
                    y += getHeight();
                    line = *f + " ";
                }
                else
                    line += *f + " ";

                f++;
            }
        }

        switch(align)
        {
            case LEFT:
                render_left(dest, x, y, line.c_str());
                break;
            case CENTER:
                render_center(dest, x, y, line.c_str());
                break;
            case RIGHT:
                render_right(dest, x, y, line.c_str());
                break;
        }
        y += getHeight();
    }

    return GPU_MakeRect(x, y0, width, y-y0);
}

GPU_Rect NFont::render_center(GPU_Target* dest, float x, float y, const char* text)
{
    if(text == NULL)
        return GPU_MakeRect(x, y, 0, 0);

    GPU_Rect result = GPU_MakeRect(x,y,0,0);
    char* str = copyString(text);
    char* del = str;

    // Go through str, when you find a \n, replace it with \0 and print it
    // then move down, back, and continue.
    for(char* c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            result = rectUnion(render_left(dest, x - getWidth("%s", str)/2.0f, y, str), result);
            *c = '\n';
            c++;
            str = c;
            y += height;
        }
        else
            c++;
    }

    result = rectUnion(render_left(dest, x - getWidth("%s", str)/2.0f, y, str), result);

    delete[] del;
    return result;
}

GPU_Rect NFont::render_right(GPU_Target* dest, float x, float y, const char* text)
{
    if(text == NULL)
        return GPU_MakeRect(x, y, 0, 0);

    GPU_Rect result = GPU_MakeRect(x,y,0,0);
    char* str = copyString(text);
    char* del = str;

    for(char* c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            result = rectUnion(render_left(dest, x - getWidth("%s", str), y, str), result);
            *c = '\n';
            c++;
            str = c;
            y += height;
        }
        else
            c++;
    }

    result = rectUnion(render_left(dest, x - getWidth("%s", str), y, str), result);

    delete[] del;
    return result;
}

GPU_Rect NFont::render_center(GPU_Target* dest, float x, float y, const Scale& scale, const char* text)
{
    if(text == NULL)
        return GPU_MakeRect(x, y, 0, 0);

    GPU_Rect result = GPU_MakeRect(x,y,0,0);
    char* str = copyString(text);
    char* del = str;

    // Go through str, when you find a \n, replace it with \0 and print it
    // then move down, back, and continue.
    for(char* c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            result = rectUnion(render_left(dest, x - scale.x*getWidth("%s", str)/2.0f, y, scale, str), result);
            *c = '\n';
            c++;
            str = c;
            y += scale.y*height;
        }
        else
            c++;
    }

    result = rectUnion(render_left(dest, x - scale.x*getWidth("%s", str)/2.0f, y, scale, str), result);

    delete[] del;
    return result;
}

GPU_Rect NFont::render_right(GPU_Target* dest, float x, float y, const Scale& scale, const char* text)
{
    if(text == NULL)
        return GPU_MakeRect(x, y, 0, 0);

    GPU_Rect result = GPU_MakeRect(x,y,0,0);
    char* str = copyString(text);
    char* del = str;

    for(char* c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            result = rectUnion(render_left(dest, x - scale.x*getWidth("%s", str), y, scale, str), result);
            *c = '\n';
            c++;
            str = c;
            y += scale.y*height;
        }
        else
            c++;
    }

    result = rectUnion(render_left(dest, x - scale.x*getWidth("%s", str), y, scale, str), result);

    delete[] del;
    return result;
}



GPU_Rect NFont::draw(GPU_Target* dest, float x, float y, const Scale& scale, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return GPU_MakeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    GPU_SetRGBA(src, default_color.r, default_color.g, default_color.b, default_color.a);
    return render_left(dest, x, y, scale, buffer);
}

GPU_Rect NFont::draw(GPU_Target* dest, float x, float y, AlignEnum align, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return GPU_MakeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    GPU_SetRGBA(src, default_color.r, default_color.g, default_color.b, default_color.a);

    switch(align)
    {
        case LEFT:
            return render_left(dest, x, y, buffer);
        case CENTER:
            return render_center(dest, x, y, buffer);
        case RIGHT:
            return render_right(dest, x, y, buffer);
    }

    return GPU_MakeRect(x, y, 0, 0);
}

GPU_Rect NFont::draw(GPU_Target* dest, float x, float y, const Color& color, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return GPU_MakeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    GPU_SetRGBA(src, color.r, color.g, color.b, color.a);
    GPU_Rect result = render_left(dest, x, y, buffer);
    GPU_SetRGBA(src, 255, 255, 255, 255);
    return result;
}


GPU_Rect NFont::draw(GPU_Target* dest, float x, float y, const Effect& effect, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return GPU_MakeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    if(effect.use_color)
        GPU_SetRGBA(src, effect.color.r, effect.color.g, effect.color.b, effect.color.a);
    else
        GPU_SetRGBA(src, default_color.r, default_color.g, default_color.b, default_color.a);
    
    GPU_Rect result;
    switch(effect.alignment)
    {
        case LEFT:
            result = render_left(dest, x, y, effect.scale, buffer);
            break;
        case CENTER:
            result = render_center(dest, x, y, effect.scale, buffer);
            break;
        case RIGHT:
            result = render_right(dest, x, y, effect.scale, buffer);
            break;
        default:
            result = GPU_MakeRect(x, y, 0, 0);
            break;
    }
    GPU_SetRGBA(src, 255, 255, 255, 255);

    return result;
}

GPU_Rect NFont::draw(GPU_Target* dest, float x, float y, const NFont::AnimParams& params, NFont::AnimFn posFn, const char* text, ...)
{
    va_list lst;
    va_start(lst, text);
    vsprintf(buffer, text, lst);
    va_end(lst);

    GPU_SetRGBA(src, default_color.r, default_color.g, default_color.b, default_color.a);
    return render_animated(dest, x, y, params, posFn, NFont::LEFT);
}

GPU_Rect NFont::draw(GPU_Target* dest, float x, float y, const NFont::AnimParams& params, NFont::AnimFn posFn, AlignEnum align, const char* text, ...)
{
    va_list lst;
    va_start(lst, text);
    vsprintf(buffer, text, lst);
    va_end(lst);

    GPU_SetRGBA(src, default_color.r, default_color.g, default_color.b, default_color.a);
    return render_animated(dest, x, y, params, posFn, align);
}




// Getters

GPU_Image* NFont::getImage() const
{
    return src;
}

SDL_Surface* NFont::getSurface() const
{
    return srcSurface;
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

Uint16 NFont::getWidth(const char* formatted_text, ...)
{
    if (formatted_text == NULL)
        return 0;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    const char* c;
    Uint16 width = 0;
    Uint16 bigWidth = 0;  // Allows for multi-line strings

    for (c = buffer; *c != '\0'; c++)
    {

        // skip spaces and nonprintable characters
        if(*c == '\n')
        {
            bigWidth = bigWidth >= width? bigWidth : width;
            width = 0;
        }

        GlyphData glyph;
        getGlyphData(&glyph, *c);
        width += glyph.rect.w;
    }
    bigWidth = bigWidth >= width? bigWidth : width;

    return bigWidth;
}


Uint16 NFont::getColumnPosWidth(Uint16 width, Uint16 pos, const char* formatted_text, ...)
{
    if(formatted_text == NULL || width == 0 || pos == 0)
        return 0;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);


    using std::string;
    string text = buffer;
    list<string> ls = explode(text, '\n');


	string line;
    for(list<string>::iterator e = ls.begin(); e != ls.end(); e++)
    {
        line = *e;

        // If line is too long, then add words one at a time until we go over.
        if(getWidth("%s", line.c_str()) > width)
        {
            list<string> words = explode(line, ' ');
            list<string>::iterator f = words.begin();
            line = *f + " ";
            f++;
            while(f != words.end())
            {
                if(getWidth("%s", (line + *f).c_str()) > width)
                {
                    for(int i = 0; i < int(line.size()); i++)
                    {
                        pos--;
                        if(pos == 0)
                        {
                            line = line.substr(0, i+1);
                            return getWidth("%s", line.c_str());
                        }
                    }

                    line = *f + " ";
                }
                else
                    line += *f + " ";

                f++;
            }
        }

        for(int i = 0; i < int(line.size()); i++)
        {
            pos--;
            if(pos == 0)
            {
                line = line.substr(0, i+1);
                return getWidth("%s", line.c_str());
            }
        }

    }

    return getWidth("%s", line.c_str());
}


Uint16 NFont::getColumnPosHeight(Uint16 width, Uint16 pos, const char* formatted_text, ...)
{
    if(formatted_text == NULL || width == 0 || pos == 0)
        return 0;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    int y = 0;

    using std::string;
    string text = buffer;
    list<string> ls = explode(text, '\n');
    for(list<string>::iterator e = ls.begin(); e != ls.end();)
    {
        string line = *e;

        // If line is too long, then add words one at a time until we go over.
        if(getWidth("%s", line.c_str()) > width)
        {
            list<string> words = explode(line, ' ');
            list<string>::iterator f = words.begin();
            line = *f + " ";
            f++;
            while(f != words.end())
            {
                if(getWidth("%s", (line + *f).c_str()) > width)
                {
                    for(int i = 0; i < int(line.size()); i++)
                    {
                        pos--;
                        if(pos == 0)
                            return y;
                    }

                    y += getHeight();
                    line = *f + " ";
                }
                else
                    line += *f + " ";

                f++;
            }
        }

        e++;
        if(e != ls.end())
            y += getHeight();
    }


    return y;
}


Uint16 NFont::getColumnHeight(Uint16 width, const char* formatted_text, ...)
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
        if(getWidth("%s", line.c_str()) > width)
        {
            list<string> words = explode(line, ' ');
            list<string>::iterator f = words.begin();
            line = *f + " ";
            f++;
            while(f != words.end())
            {
                if(getWidth("%s", (line + *f).c_str()) > width)
                {
                    y += getHeight();
                    line = *f + " ";
                }
                else
                    line += *f + " ";

                f++;
            }
        }

        y += getHeight();
    }

    return y;
}

int NFont::getAscent(const char character)
{
    // FIXME
    GlyphData glyph;
    getGlyphData(&glyph, character);
    return glyph.rect.h;
    
    /*unsigned char test = (unsigned char)character;
    if(test < 33 || test > 222 || (test > 126 && test < 161))
        return 0;
    unsigned char num = (unsigned char)character - 33;
    // Get the max ascent
    int x = charPos[num];
    int i, j = 1, result = 0;
    Uint32 pixel = getPixel(srcSurface, 0, srcSurface->h - 1); // bg pixel
    while(j < baseline && j < srcSurface->h)
    {
        i = charPos[num];
        while(i < x + charWidth[num])
        {
            if(getPixel(srcSurface, i, j) != pixel)
            {
                result = baseline - j;
                j = srcSurface->h;
                break;
            }
            i++;
        }
        j++;
    }
    return result;*/
}

int NFont::getAscent() const
{
    return ascent;
}

int NFont::getAscent(const char* formatted_text, ...)
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

int NFont::getDescent(const char character)
{
    // FIXME
    GlyphData glyph;
    getGlyphData(&glyph, character);
    return glyph.rect.h;
    
    /*
    unsigned char test = (unsigned char)character;
    if(test < 33 || test > 222 || (test > 126 && test < 161))
        return 0;
    unsigned char num = (unsigned char)character - 33;
    // Get the max descent
    int x = charPos[num];
    int i, j = srcSurface->h - 1, result = 0;
    Uint32 pixel = getPixel(srcSurface, 0, srcSurface->h - 1); // bg pixel
    while(j > 0 && j > baseline)
    {
        i = charPos[num];
        while(i < x + charWidth[num])
        {
            if(getPixel(srcSurface, i, j) != pixel)
            {
                result = j - baseline;
                j = 0;
                break;
            }
            i++;
        }
        j--;
    }
    return result;*/
}

int NFont::getDescent() const
{
    return descent;
}

int NFont::getDescent(const char* formatted_text, ...)
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

NFont::Color NFont::getDefaultColor() const
{
    return default_color;
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
    /*
    // TODO: Find a better way
    // Get the baseline by checking a, b, and c and averaging their lowest y-value.
    Uint32 pixel = getPixel(srcSurface, 0, srcSurface->h - 1);
    int heightSum = 0;
    int x, i, j;
    for(unsigned char avgChar = 64; avgChar < 67; avgChar++)
    {
        x = charPos[avgChar];

        j = srcSurface->h - 1;
        while(j > 0)
        {
            i = x;
            while(i - x < charWidth[64])
            {
                if(getPixel(srcSurface, i, j) != pixel)
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
    */
}

void NFont::setBaseline(Uint16 Baseline)
{
    baseline = Baseline;
}

void NFont::setDefaultColor(const Color& color)
{
    default_color = color;
}

void NFont::enableTTFOwnership()
{
    owns_ttf_source = true;
}




namespace NFontAnim
{

void bounce(float& x, float& y, const NFont::AnimParams& params, NFont::AnimData& data)
{
    y -= params.amplitudeX*fabs(sin(-M_PI*params.frequencyX*params.t + (x - data.startX)/40.0));

    if(data.align == NFont::CENTER)
    {
        x -= data.font->getWidth("%s", data.text)/2.0f;
    }
    else if(data.align == NFont::RIGHT)
    {
        x -= data.font->getWidth("%s", data.text);
    }
}


void wave(float& x, float& y, const NFont::AnimParams& params, NFont::AnimData& data)
{
    y += params.amplitudeX*sin(-2*M_PI*params.frequencyX*params.t + (x - data.startX)/40.0);

    if(data.align == NFont::CENTER)
    {
        x -= data.font->getWidth("%s", data.text)/2.0f;
    }
    else if(data.align == NFont::RIGHT)
    {
        x -= data.font->getWidth("%s", data.text);
    }
}


void stretch(float& x, float& y, const NFont::AnimParams& params, NFont::AnimData& data)
{
    float place = float(data.index) / strlen(data.text);
    if(data.align == NFont::CENTER)
    {
        place -= 0.5f;
        x -= data.font->getWidth("%s", data.text)/2.0f;
    }
    else if(data.align == NFont::RIGHT)
    {
        place -= 1.0f;
        x -= data.font->getWidth("%s", data.text);
    }
    x += params.amplitudeX*(place)*cos(2*M_PI*params.frequencyX*params.t);
}


void shake(float& x, float& y, const NFont::AnimParams& params, NFont::AnimData& data)
{
    if(data.align == NFont::CENTER)
    {
        x -= data.font->getWidth("%s", data.text)/2.0f;
    }
    else if(data.align == NFont::RIGHT)
    {
        x -= data.font->getWidth("%s", data.text);
    }
    x += params.amplitudeX*sin(2*M_PI*params.frequencyX*params.t);
    y += params.amplitudeY*sin(2*M_PI*params.frequencyY*params.t);
}

void circle(float& x, float& y, const NFont::AnimParams& params, NFont::AnimData& data)
{
    // Negate auto-placement
    x = data.startX;
    y = data.startY;

    if(data.align == NFont::LEFT)
    {
        x += data.font->getWidth("%s", data.text)/2.0f;
    }
    else if(data.align == NFont::RIGHT)
    {
        x -= data.font->getWidth("%s", data.text)/2.0f;
    }

    float place = float(data.index + 1) / strlen(data.text);
    x += params.amplitudeX*cos(place * 2*M_PI - 2*M_PI*params.frequencyX*params.t);
    y += params.amplitudeY*sin(place * 2*M_PI - 2*M_PI*params.frequencyY*params.t);
}

}



