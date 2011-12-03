/*
NFontC v3.0.0: A bitmap font struct for SDL
by Jonathan Dearborn 12-03-11
*/
#include "nfontc.h"
#include "stdarg.h"
#include <math.h>

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
static inline SDL_Rect rectUnion(SDL_Rect* A, SDL_Rect* B)
{
    Sint16 x,x2,y,y2;
    x = MIN(A->x, B->x);
    y = MIN(A->y, B->y);
    x2 = MAX(A->x+A->w, B->x+B->w);
    y2 = MAX(A->y+A->h, B->y+B->h);
    SDL_Rect result = {x, y, (Uint16)(x2 - x), (Uint16)(y2 - y)};
    return result;
}

// Adapted from SDL_IntersectRect
static inline SDL_Rect rectIntersect(SDL_Rect A, SDL_Rect B)
{
    SDL_Rect result;
	int Amin, Amax, Bmin, Bmax;

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

static inline SDL_Rect makeRect(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
    SDL_Rect r = {x, y, w, h};
    return r;
}

static inline SDL_Surface* copySurface(SDL_Surface *Surface)
{
    return SDL_ConvertSurface(Surface, Surface->format, Surface->flags);
}

static Uint32 _GetPixel(SDL_Surface *Surface, int x, int y)  // No Alpha?
{
    Uint8* bits;
    Uint32 bpp;

    if(x < 0 || x >= Surface->w)
        return 0;  // Best I could do for errors

    bpp = Surface->format->BytesPerPixel;
    bits = ((Uint8*)Surface->pixels) + y*Surface->pitch + x*bpp;

    Uint8 r, g, b;
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
            r = *((bits)+Surface->format->Rshift/8);
            g = *((bits)+Surface->format->Gshift/8);
            b = *((bits)+Surface->format->Bshift/8);
            return SDL_MapRGB(Surface->format, r, g, b);
            break;
        case 4:
            return *((Uint32*)Surface->pixels + y * Surface->pitch/4 + x);
            break;
    }

    return 0;  // Best I could do for errors
}

static void _SetPixel(SDL_Surface* surface, int x, int y, Uint32 color)
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

static char* _CopyString(const char* c)
{
    if(c == NULL)
        return NULL;
    
    char* result = (char*)malloc(strlen(c)+1);
    strcpy(result, c);

    return result;
}


NFontAnim_Params NF_AnimParams(float t, float amplitudeX, float frequencyX, float amplitudeY, float frequencyY)
{
    NFontAnim_Params p;
    p.t = t;
    p.amplitudeX = amplitudeX;
    p.amplitudeY = amplitudeY;
    p.frequencyX = frequencyX;
    p.frequencyY = frequencyY;
    return p;
}




NFont* NF_New()
{
    NFont* font = (NFont*)malloc(sizeof(NFont));
    font->src = NULL;

    font->maxPos = 0;

    font->height = 0; // ascent+descent

    font->maxWidth = 0;
    font->baseline = 0;
    font->ascent = 0;
    font->descent = 0;

    font->lineSpacing = 0;
    font->letterSpacing = 0;

    font->buffer = (char*)malloc(1024);
    
    
    font->data.font = font;
    font->data.dest = NULL;
    font->data.maxX = 0;
    font->data.src = NULL;
    font->data.height = 0;
    font->data.text = NULL;

    return font;
}

void NF_Free(NFont* font)
{
    if(font == NULL)
        return;
    free(font->buffer);
    SDL_FreeSurface(font->src);
    free(font);
}

void NF_SetBuffer(NFont* font, unsigned int size)
{
    if(font == NULL)
        return;
    
    free(font->buffer);
    if(size > 0)
        font->buffer = (char*)malloc(size);
    else
        font->buffer = (char*)malloc(1024);
}

SDL_Surface* NF_VerticalGradient(SDL_Surface* targetSurface, Uint32 top, Uint32 bottom, int heightAdjust)
{
    SDL_Surface* surface = targetSurface;
    if(surface == NULL)
        return NULL;
    
    Uint8 tr, tg, tb;

    SDL_GetRGB(top, surface->format, &tr, &tg, &tb);

    Uint8 br, bg, bb;

    SDL_GetRGB(bottom, surface->format, &br, &bg, &bb);

    Uint8 useCK = (surface->flags & SDL_SRCALPHA) != SDL_SRCALPHA;  // colorkey if no alpha
    Uint32 colorkey = surface->format->colorkey;

    Uint8 r, g, b, a;
    float ratio;
    Uint32 color;
    int temp;
    
    int x, y;
    for (x = 0, y = 0; y < surface->h; x++)
    {
        if (x >= surface->w)
        {
            x = 0;
            y++;

            if (y >= surface->h)
                break;
        }

        ratio = (y - 2)/(float)(surface->h - heightAdjust);  // the neg 2 is for full color at top?

        if(!useCK)
        {
            color = _GetPixel(surface, x, y);
            SDL_GetRGBA(color, surface->format, &r, &g, &b, &a);  // just getting alpha
        }
        else
            a = SDL_ALPHA_OPAQUE;

        // Get and clamp the new values
        temp = (int)(tr*(1-ratio) + br*ratio);
        r = temp < 0? 0 : temp > 255? 255 : temp;

        temp = (int)(tg*(1-ratio) + bg*ratio);
        g = temp < 0? 0 : temp > 255? 255 : temp;

        temp = (int)(tb*(1-ratio) + bb*ratio);
        b = temp < 0? 0 : temp > 255? 255 : temp;


        color = SDL_MapRGBA(surface->format, r, g, b, a);


        if(useCK)
        {
            if(_GetPixel(surface, x, y) == colorkey)
                continue;
            if(color == colorkey)
                color == 0? color++ : color--;
        }

        // make sure it isn't pink
        if(color == SDL_MapRGBA(surface->format, 0xFF, 0, 0xFF, a))
            color--;
        if(_GetPixel(surface, x, y) == SDL_MapRGBA(surface->format, 0xFF, 0, 0xFF, SDL_ALPHA_OPAQUE))
            continue;

        _SetPixel(surface, x, y, color);

    }
    return surface;
}

#ifndef NFONT_NO_TTF

void NF_LoadTTF(NFont* font, const char* filename_ttf, Uint32 pointSize, SDL_Color fg, SDL_Color* bg, int style)
{
    if(font == NULL)
        return;
    
    SDL_FreeSurface(font->src);
    font->src = NULL;
    
    if(!TTF_WasInit() && TTF_Init() < 0)
    {
        printf("Unable to initialize SDL_ttf: %s \n", TTF_GetError());
        return;
    }
    
    TTF_Font* ttf = TTF_OpenFont(filename_ttf, pointSize);
    
    if(ttf == NULL)
    {
        printf("Unable to load TrueType font: %s \n", TTF_GetError());
        return;
    }
    TTF_SetFontStyle(ttf, style);
    NF_LoadTTF_Font(font, ttf, fg, bg);
    TTF_CloseFont(ttf);
}

void NF_LoadTTF_Font(NFont* font, TTF_Font* ttf, SDL_Color fg, SDL_Color* bg)
{
    if(font == NULL || ttf == NULL)
        return;
    
    SDL_FreeSurface(font->src);
    font->src = NULL;
    
    SDL_Surface* surfs[127 - 33];
    int width = 0;
    int height = 0;
    
    char buff[2];
    buff[1] = '\0';
    int i;
    for(i = 0; i < 127 - 33; i++)
    {
        buff[0] = i + 33;
        if(bg == NULL)
            surfs[i] = TTF_RenderText_Blended(ttf, buff, fg);
        else
            surfs[i] = TTF_RenderText_Shaded(ttf, buff, fg, *bg);
        width += surfs[i]->w;
        height = (height < surfs[i]->h)? surfs[i]->h : height;
    }
    SDL_Surface* result;
    if(bg == NULL)
    {
        result = createSurface32(width + 127 - 33 + 1,height);
    }
    else
    {
        result = createSurface24(width + 127 - 33 + 1,height);
    }
    Uint32 pink = SDL_MapRGBA(result->format, 255, 0, 255, SDL_ALPHA_OPAQUE);
    Uint32 bgcolor;
    if(bg != NULL)
        bgcolor = SDL_MapRGB(result->format, bg->r, bg->g, bg->b);
    
    SDL_SetAlpha(result, 0, SDL_ALPHA_OPAQUE);
    
    SDL_Rect pixel = {1, 0, 1, 1};
    SDL_Rect line = {1, 0, 1, result->h};
    
    int x = 1;
    SDL_Rect dest = {x, 0, 0, 0};
    for(i = 0; i < 127 - 33; i++)
    {
        pixel.x = line.x = x-1;
        if(bg != NULL)
            SDL_FillRect(result, &line, bgcolor);
        SDL_FillRect(result, &pixel, pink);
        
        SDL_SetAlpha(surfs[i], 0, SDL_ALPHA_OPAQUE);
        SDL_BlitSurface(surfs[i], NULL, result, &dest);
        
        x += surfs[i]->w + 1;
        dest.x = x;
        
        SDL_FreeSurface(surfs[i]);
    }
    pixel.x = line.x = x-1;
    if(bg != NULL)
        SDL_FillRect(result, &line, bgcolor);
    SDL_FillRect(result, &pixel, pink);
    if(bg == NULL)
        SDL_SetAlpha(result, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
    
    NF_Load(font, result);
}

#endif

static SDL_Rect NF_DrawToSurface(NFont* font, SDL_Surface* dest, int x, int y, const char* text)
{
    if(font == NULL || text == NULL || font->src == NULL)
        return makeRect(x, y, 0, 0);
    
    const char* c = text;
    unsigned char num;
    SDL_Rect srcRect, dstRect, copyS, copyD;
    
    srcRect.y = font->baseline - font->ascent;
    srcRect.h = dstRect.h = font->height;
    dstRect.x = x;
    dstRect.y = y;
    
    font->data.dirtyRect = makeRect(x,y,0,0);
    
    int newlineX = x;
    
    for(; *c != '\0'; c++)
    {
        if(*c == '\n')
        {
            dstRect.x = newlineX;
            dstRect.y += font->height + font->lineSpacing;
            continue;
        }
        
        if (*c == ' ')
        {
            dstRect.x += font->charWidth[0] + font->letterSpacing;
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
        srcRect.x = font->charPos[num];
        srcRect.w = dstRect.w = font->charWidth[num];
        copyS = srcRect;
        copyD = dstRect;
        SDL_BlitSurface(font->src, &srcRect, dest, &dstRect);
        if(font->data.dirtyRect.w == 0 || font->data.dirtyRect.h == 0)
            font->data.dirtyRect = dstRect;
        else
            font->data.dirtyRect = rectUnion(&font->data.dirtyRect, &dstRect);
        srcRect = copyS;
        dstRect = copyD;
        
        dstRect.x += dstRect.w + font->letterSpacing;
    }
    
    return font->data.dirtyRect;
}

static SDL_Rect NF_DrawCenter(NFont* font, SDL_Surface* dest, int x, int y, const char* text)
{
    if(font == NULL || text == NULL)
        return makeRect(x,y,0,0);

    SDL_Rect result = makeRect(x,y,0,0);
    char* str = _CopyString(text);
    char* del = str;

    // Go through str, when you find a \n, replace it with \0 and print it
    // then move down, back, and continue.
    char* c;
    for(c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            SDL_Rect temp = NF_DrawToSurface(font, dest, x - NF_GetWidth(font, "%s", str)/2, y, str);
            result = rectUnion(&temp, &result);
            *c = '\n';
            c++;
            str = c;
            y += font->height;
        }
        else
            c++;
    }
    SDL_Rect temp = NF_DrawToSurface(font, dest, x - NF_GetWidth(font, "%s", str)/2, y, str);
    result = rectUnion(&temp, &result);

    free(del);
    
    return result;
}

static SDL_Rect NF_DrawRight(NFont* font, SDL_Surface* dest, int x, int y, const char* text)
{
    if(font == NULL || text == NULL)
        return makeRect(x,y,0,0);

    SDL_Rect result = makeRect(x,y,0,0);
    char* str = _CopyString(text);
    char* del = str;

    char* c;
    for(c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            SDL_Rect temp = NF_DrawToSurface(font, dest, x - NF_GetWidth(font, "%s", str), y, str);
            result = rectUnion(&temp, &result);
            *c = '\n';
            c++;
            str = c;
            y += font->height;
        }
        else
            c++;
    }
    
    SDL_Rect temp = NF_DrawToSurface(font, dest, x - NF_GetWidth(font, "%s", str), y, str);
    result = rectUnion(&temp, &result);

    free(del);
    
    return result;
}

SDL_Rect NF_Draw(NFont* font, SDL_Surface* dest, int x, int y, const char* formatted_text, ...)
{
    if(font == NULL || formatted_text == NULL)
        return makeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(font->buffer, formatted_text, lst);
    va_end(lst);

    return NF_DrawToSurface(font, dest, x, y, font->buffer);
}

SDL_Rect NF_DrawAlign(NFont* font, SDL_Surface* dest, int x, int y, NF_AlignEnum align, const char* formatted_text, ...)
{
    if(font == NULL || formatted_text == NULL)
        return makeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(font->buffer, formatted_text, lst);
    va_end(lst);
    
    switch(align)
    {
        case NF_LEFT:
            return NF_DrawToSurface(font, dest, x, y, font->buffer);
        case NF_CENTER:
            return NF_DrawCenter(font, dest, x, y, font->buffer);
        case NF_RIGHT:
            return NF_DrawRight(font, dest, x, y, font->buffer);
    }
    return makeRect(x,y,0,0);
}


SDL_Rect NF_DrawColumn(NFont* font, SDL_Surface* dest, int x, int y, Uint16 width, const char* formatted_text, ...)
{
    if(font == NULL || formatted_text == NULL)
        return makeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(font->buffer, formatted_text, lst);
    va_end(lst);
    
    int y0 = y;
    
    char buffer[1024];
    strcpy(buffer, font->buffer);
    char line[1024];
    char* word;
    char part[1024];
    int i = 0;
    char* l;
    for(l = buffer; 1; l++)
    {
        if(*l == '\n' || *l == '\0')
        {
            line[i] = '\0';
            
            // Draw line
            int j = 0;
            word = part;
            char* w;
            for(w = line; 1; w++)
            {
                if(*w == ' ' || *w == '\0')
                {
                    word[j] = '\0';
                    // If the new word is too long, draw the old stuff and continue with the new word
                    if(NF_GetWidth(font, "%s", part) > width)
                    {
                        char firstOfWord = *(word-1);  // Save to replace later
                        if(word != part)
                            *(word-1) = '\0';  // End 'part' for drawing
                        
                        NF_DrawToSurface(font, dest, x, y, part);
                        y += NF_GetHeight(font, NULL);
                        
                        // Restore 'word'
                        if(word != part)
                            *(word-1) = firstOfWord;
                        // Copy new word down to the start
                        word[j] = '\0';
                        strcpy(part, word);
                        part[j] = ' ';
                        word = &part[j+1];
                        
                        j = 0;
                    }
                    else
                    {
                        word[j] = ' ';
                        // Not too long, so restart 'word' here and keep going.
                        word = &word[j+1];
                        j = 0;
                    }
                    
                    if(*w == '\0')
                        break;
                }
                else
                {
                    word[j] = *w;
                    j++;
                }
            }
            
            word[j] = '\0';
            NF_DrawToSurface(font, dest, x, y, part);
            y += NF_GetHeight(font, NULL);
            
            i = 0;
            
            if(*l == '\0')
                break;
        }
        else
        {
            line[i] = *l;
            i++;
        }
    }
    
    return makeRect(x, y0, width, y-y0);
}

SDL_Rect NF_DrawColumnAlign(NFont* font, SDL_Surface* dest, int x, int y, Uint16 width, NF_AlignEnum align, const char* formatted_text, ...)
{
    if(font == NULL || formatted_text == NULL)
        return makeRect(x, y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(font->buffer, formatted_text, lst);
    va_end(lst);
    
    int y0 = y;
    
    char buffer[1024];
    strcpy(buffer, font->buffer);  // font->buffer is overwritten by NF_GetWidth()
    char line[1024];
    char* word;
    char part[1024];
    int i = 0;
    char* l;
    for(l = buffer; 1; l++)
    {
        if(*l == '\n' || *l == '\0')
        {
            line[i] = '\0';
            
            // Draw line
            int j = 0;
            word = part;
            char* w;
            for(w = line; 1; w++)
            {
                if(*w == ' ' || *w == '\0')
                {
                    word[j] = '\0';
                    // If the new word is too long, draw the old stuff and continue with the new word
                    if(NF_GetWidth(font, "%s", part) > width)
                    {
                        char firstOfWord = *(word-1);  // Save to replace later
                        if(word != part)
                            *(word-1) = '\0';  // End 'part' for drawing
                        
                        switch(align)
                        {
                            case NF_LEFT:
                                NF_DrawToSurface(font, dest, x, y, part);
                                break;
                            case NF_CENTER:
                                NF_DrawCenter(font, dest, x, y, part);
                                break;
                            case NF_RIGHT:
                                NF_DrawRight(font, dest, x, y, part);
                                break;
                        }
                        y += NF_GetHeight(font, NULL);
                        
                        // Restore 'word'
                        if(word != part)
                            *(word-1) = firstOfWord;
                        // Copy new word down to the start
                        word[j] = '\0';
                        strcpy(part, word);
                        part[j] = ' ';
                        word = &part[j+1];
                        
                        j = 0;
                    }
                    else
                    {
                        word[j] = ' ';
                        // Not too long, so restart 'word' here and keep going.
                        word = &word[j+1];
                        j = 0;
                    }
                    
                    if(*w == '\0')
                        break;
                }
                else
                {
                    word[j] = *w;
                    j++;
                }
            }
            
            word[j] = '\0';
            switch(align)
            {
                case NF_LEFT:
                    NF_DrawToSurface(font, dest, x, y, part);
                    break;
                case NF_CENTER:
                    NF_DrawCenter(font, dest, x, y, part);
                    break;
                case NF_RIGHT:
                    NF_DrawRight(font, dest, x, y, part);
                    break;
            }
            y += NF_GetHeight(font, NULL);
            
            i = 0;
            
            if(*l == '\0')
                break;
        }
        else
        {
            line[i] = *l;
            i++;
        }
    }
    
    
    return makeRect(x, y0, width, y-y0);
}



SDL_Rect NF_DrawBox(NFont* font, SDL_Surface* dest, SDL_Rect box, const char* formatted_text, ...)
{
    if(font == NULL || formatted_text == NULL)
        return makeRect(box.x, box.y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(font->buffer, formatted_text, lst);
    va_end(lst);
    
    SDL_Rect oldclip, newclip;
    SDL_GetClipRect(dest, &oldclip);
    newclip = rectIntersect(oldclip, box);
    SDL_SetClipRect(dest, &newclip);
    
    int x = box.x;
    int y = box.y;
    Uint16 width = box.w;
    
    char buffer[1024];
    strcpy(buffer, font->buffer);
    char line[1024];
    char* word;
    char part[1024];
    int i = 0;
    char* l;
    for(l = buffer; 1; l++)
    {
        if(*l == '\n' || *l == '\0')
        {
            line[i] = '\0';
            
            // Draw line
            int j = 0;
            word = part;
            char* w;
            for(w = line; 1; w++)
            {
                if(*w == ' ' || *w == '\0')
                {
                    word[j] = '\0';
                    // If the new word is too long, draw the old stuff and continue with the new word
                    if(NF_GetWidth(font, "%s", part) > width)
                    {
                        char firstOfWord = *(word-1);  // Save to replace later
                        if(word != part)
                            *(word-1) = '\0';  // End 'part' for drawing
                        
                        NF_DrawToSurface(font, dest, x, y, part);
                        y += NF_GetHeight(font, NULL);
                        
                        // Restore 'word'
                        if(word != part)
                            *(word-1) = firstOfWord;
                        // Copy new word down to the start
                        word[j] = '\0';
                        strcpy(part, word);
                        part[j] = ' ';
                        word = &part[j+1];
                        
                        j = 0;
                    }
                    else
                    {
                        word[j] = ' ';
                        // Not too long, so restart 'word' here and keep going.
                        word = &word[j+1];
                        j = 0;
                    }
                    
                    if(*w == '\0')
                        break;
                }
                else
                {
                    word[j] = *w;
                    j++;
                }
            }
            
            word[j] = '\0';
            NF_DrawToSurface(font, dest, x, y, part);
            y += NF_GetHeight(font, NULL);
            
            i = 0;
            
            if(*l == '\0')
                break;
        }
        else
        {
            line[i] = *l;
            i++;
        }
    }
    
    SDL_SetClipRect(dest, &oldclip);
    
    return box;
}

SDL_Rect NF_DrawBoxAlign(NFont* font, SDL_Surface* dest, SDL_Rect box, NF_AlignEnum align, const char* formatted_text, ...)
{
    if(font == NULL || formatted_text == NULL)
        return makeRect(box.x, box.y, 0, 0);

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(font->buffer, formatted_text, lst);
    va_end(lst);
    
    SDL_Rect oldclip, newclip;
    SDL_GetClipRect(dest, &oldclip);
    newclip = rectIntersect(oldclip, box);
    SDL_SetClipRect(dest, &newclip);
    
    int x = box.x;
    int y = box.y;
    Uint16 width = box.w;
    switch(align)
    {
        case NF_LEFT:
            break;
        case NF_CENTER:
            x += width/2;
            break;
        case NF_RIGHT:
            x += width;
            break;
    }
    
    char buffer[1024];
    strcpy(buffer, font->buffer);  // font->buffer is overwritten by NF_GetWidth()
    char line[1024];
    char* word;
    char part[1024];
    int i = 0;
    char* l;
    for(l = buffer; 1; l++)
    {
        if(*l == '\n' || *l == '\0')
        {
            line[i] = '\0';
            
            // Draw line
            int j = 0;
            word = part;
            char* w;
            for(w = line; 1; w++)
            {
                if(*w == ' ' || *w == '\0')
                {
                    word[j] = '\0';
                    // If the new word is too long, draw the old stuff and continue with the new word
                    if(NF_GetWidth(font, "%s", part) > width)
                    {
                        char firstOfWord = *(word-1);  // Save to replace later
                        if(word != part)
                            *(word-1) = '\0';  // End 'part' for drawing
                        
                        switch(align)
                        {
                            case NF_LEFT:
                                NF_DrawToSurface(font, dest, x, y, part);
                                break;
                            case NF_CENTER:
                                NF_DrawCenter(font, dest, x, y, part);
                                break;
                            case NF_RIGHT:
                                NF_DrawRight(font, dest, x, y, part);
                                break;
                        }
                        y += NF_GetHeight(font, NULL);
                        
                        // Restore 'word'
                        if(word != part)
                            *(word-1) = firstOfWord;
                        // Copy new word down to the start
                        word[j] = '\0';
                        strcpy(part, word);
                        part[j] = ' ';
                        word = &part[j+1];
                        
                        j = 0;
                    }
                    else
                    {
                        word[j] = ' ';
                        // Not too long, so restart 'word' here and keep going.
                        word = &word[j+1];
                        j = 0;
                    }
                    
                    if(*w == '\0')
                        break;
                }
                else
                {
                    word[j] = *w;
                    j++;
                }
            }
            
            word[j-1] = '\0';
            switch(align)
            {
                case NF_LEFT:
                    NF_DrawToSurface(font, dest, x, y, part);
                    break;
                case NF_CENTER:
                    NF_DrawCenter(font, dest, x, y, part);
                    break;
                case NF_RIGHT:
                    NF_DrawRight(font, dest, x, y, part);
                    break;
            }
            y += NF_GetHeight(font, NULL);
            
            i = 0;
            
            if(*l == '\0')
                break;
        }
        else
        {
            line[i] = *l;
            i++;
        }
    }
    
    SDL_SetClipRect(dest, &oldclip);
    
    return box;
}







int NF_GetHeight(NFont* font, const char* formatted_text, ...)
{
    if(font == NULL)
        return -1;
        
    if(formatted_text == NULL)
        return font->height;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(font->buffer, formatted_text, lst);
    va_end(lst);

    int numLines = 1;
    const char* c;

    for (c = font->buffer; *c != '\0'; c++)
    {
        if(*c == '\n')
            numLines++;
    }

    //   Actual height of letter region + line spacing
    return font->height*numLines + font->lineSpacing*(numLines - 1);  //height*numLines;
}

int NF_GetWidth(NFont* font, const char* formatted_text, ...)
{
    if(font == NULL)
        return -1;
        
    if (formatted_text == NULL)
        return 0;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(font->buffer, formatted_text, lst);
    va_end(lst);

    const char* c;
    int charnum = 0;
    int width = 0;
    int bigWidth = 0;  // Allows for multi-line strings

    for (c = font->buffer; *c != '\0'; c++)
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
            width += font->charWidth[0];
            continue;
        }

        width += font->charWidth[charnum];
    }
    bigWidth = bigWidth >= width? bigWidth : width;

    return bigWidth;
}

Uint8 NF_Load(NFont* font, SDL_Surface* FontSurface)
{
    if(font == NULL)
        return 0;
        
    SDL_FreeSurface(font->src);
    font->src = FontSurface;
    
    if (font->src == NULL)
    {
        printf("\n ERROR: NFont given a NULL surface\n");
        return 0;
    }

    int x = 1, i = 0;
    
    // memset would be faster
    int j;
    for(j = 0; j < 256; j++)
    {
        font->charWidth[j] = 0;
        font->charPos[j] = 0;
    }

    SDL_LockSurface(font->src);

    Uint32 pixel = SDL_MapRGB(font->src->format, 255, 0, 255); // pink pixel
    
    font->maxWidth = 0;
    
    // Get the character positions and widths
    while(x < font->src->w)
    {
        if(_GetPixel(font->src, x, 0) != pixel)
        {
            font->charPos[i] = x;
            font->charWidth[i] = x;
            while(x < font->src->w && _GetPixel(font->src, x, 0) != pixel)
                x++;
            font->charWidth[i] = x - font->charWidth[i];
            if(font->charWidth[i] > font->maxWidth)
                font->maxWidth = font->charWidth[i];
            i++;
        }

        x++;
    }

    font->maxPos = x - 1;


    pixel = _GetPixel(font->src, 0, font->src->h - 1);

    NF_SetBaseline(font);
    
    // Get the max ascent
    j = 1;
    while(j < font->baseline && j < font->src->h)
    {
        x = 0;
        while(x < font->src->w)
        {
            if(_GetPixel(font->src, x, j) != pixel)
            {
                font->ascent = font->baseline - j;
                j = font->src->h;
                break;
            }
            x++;
        }
        j++;
    }
    
    // Get the max descent
    j = font->src->h - 1;
    while(j > 0 && j > font->baseline)
    {
        x = 0;
        while(x < font->src->w)
        {
            if(_GetPixel(font->src, x, j) != pixel)
            {
                font->descent = j - font->baseline;
                j = 0;
                break;
            }
            x++;
        }
        j--;
    }
    
    
    font->height = font->ascent + font->descent;
    

    if((font->src->flags & SDL_SRCALPHA) != SDL_SRCALPHA)
    {
        pixel = _GetPixel(font->src, 0, font->src->h - 1);
        SDL_UnlockSurface(font->src);
        SDL_SetColorKey(font->src, SDL_SRCCOLORKEY, pixel);
    }
    else
        SDL_UnlockSurface(font->src);

    return 1;
}



int NF_SetBaseline(NFont* font)
{
    if(font == NULL)
        return -1;
    
    // Get the baseline by checking a, b, and c and averaging their lowest y-value.
    // Is there a better way?
    Uint32 pixel = _GetPixel(font->src, 0, font->src->h - 1);
    int heightSum = 0;
    int x, i, j;
    unsigned char avgChar;
    for(avgChar = 64; avgChar < 67; avgChar++)
    {
        x = font->charPos[avgChar];
        
        j = font->src->h - 1;
        while(j > 0)
        {
            i = x;
            while(i - x < font->charWidth[64])
            {
                if(_GetPixel(font->src, i, j) != pixel)
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
    
    font->baseline = (int)(heightSum/3.0f + 0.5f);  // Round up and cast
    
    return font->baseline;
}

int NF_GetAscentChar(NFont* font, const char character)
{
    if(font == NULL)
        return -1;
        
    unsigned char test = (unsigned char)character;
    if(test < 33 || test > 222 || (test > 126 && test < 161))
        return 0;
    unsigned char num = (unsigned char)character - 33;
    // Get the max ascent
    int x = font->charPos[num];
    int i, j = 1, result = 0;
    Uint32 pixel = _GetPixel(font->src, 0, font->src->h - 1); // bg pixel
    while(j < font->baseline && j < font->src->h)
    {
        i = font->charPos[num];
        while(i < x + font->charWidth[num])
        {
            if(_GetPixel(font->src, i, j) != pixel)
            {
                result = font->baseline - j;
                j = font->src->h;
                break;
            }
            i++;
        }
        j++;
    }
    return result;
}

int NF_GetAscent(NFont* font, const char* formatted_text, ...)
{
    if(font == NULL)
        return -1;
        
    if(formatted_text == NULL)
        return font->ascent;
    
    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(font->buffer, formatted_text, lst);
    va_end(lst);
    
    int max = 0;
    const char* c = font->buffer;
    
    for (; *c != '\0'; c++)
    {
        int asc = NF_GetAscentChar(font, *c);
        if(asc > max)
            max = asc;
    }
    return max;
}

int NF_GetDescentChar(NFont* font, const char character)
{
    if(font == NULL)
        return -1;
        
    unsigned char test = (unsigned char)character;
    if(test < 33 || test > 222 || (test > 126 && test < 161))
        return 0;
    unsigned char num = (unsigned char)character - 33;
    // Get the max descent
    int x = font->charPos[num];
    int i, j = font->src->h - 1, result = 0;
    Uint32 pixel = _GetPixel(font->src, 0, font->src->h - 1); // bg pixel
    while(j > 0 && j > font->baseline)
    {
        i = font->charPos[num];
        while(i < x + font->charWidth[num])
        {
            if(_GetPixel(font->src, i, j) != pixel)
            {
                result = j - font->baseline;
                j = 0;
                break;
            }
            i++;
        }
        j--;
    }
    return result;
}

int NF_GetDescent(NFont* font, const char* formatted_text, ...)
{
    if(font == NULL)
        return -1;
        
    if(formatted_text == NULL)
        return font->descent;
    
    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(font->buffer, formatted_text, lst);
    va_end(lst);
    
    int max = 0;
    const char* c = font->buffer;
    
    for (; *c != '\0'; c++)
    {
        int des = NF_GetDescentChar(font, *c);
        if(des > max)
            max = des;
    }
    return max;
}






static SDL_Rect _DrawToSurfacePos(NFont* font, SDL_Surface* dest, int x, int y, NFontAnim_Params params, NFontAnim_Fn posFn, NF_AlignEnum align)
{
    if(font == NULL)
        return makeRect(x,y,0,0);
        
    font->data.dest = dest;
    font->data.src = font->src;
    font->data.text = font->buffer;  // Buffer for efficient drawing
    font->data.height = font->height;
    font->data.charPos = font->charPos;
    font->data.charWidth = font->charWidth;
    font->data.maxX = font->maxPos;


    font->data.index = -1;
    font->data.letterNum = 0;
    font->data.wordNum = 1;
    font->data.lineNum = 1;
    font->data.startX = x;  // used as reset value for line feed
    font->data.startY = y;
    font->data.align = align;
    font->data.dirtyRect = makeRect(x,y,0,0);
    
    int preFnX = x;
    int preFnY = y;
    
    const char* c = font->buffer;
    unsigned char num;
    SDL_Rect srcRect, dstRect, copyS, copyD;
    
    if(c == NULL || font->src == NULL || dest == NULL)
        return makeRect(x,y,0,0);
    
    srcRect.y = font->baseline - font->ascent;
    srcRect.h = dstRect.h = font->height;
    dstRect.x = x;
    dstRect.y = y;
    
    for(; *c != '\0'; c++)
    {
        font->data.index++;
        font->data.letterNum++;
        
        if(*c == '\n')
        {
            font->data.letterNum = 1;
            font->data.wordNum = 1;
            font->data.lineNum++;

            x = font->data.startX;  // carriage return
            y += font->height + font->lineSpacing;
            continue;
        }
        if (*c == ' ')
        {
            font->data.letterNum = 1;
            font->data.wordNum++;
            
            x += font->charWidth[0] + font->letterSpacing;
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
        srcRect.x = font->charPos[num];
        srcRect.w = dstRect.w = font->charWidth[num];
        
        preFnX = x;  // Save real position
        preFnY = y;

        // Use function pointer to get final x, y values
        posFn(&x, &y, params, &(font->data));
        
        dstRect.x = x;
        dstRect.y = y;
        
        copyS = srcRect;
        copyD = dstRect;
        SDL_BlitSurface(font->src, &srcRect, dest, &dstRect);
        if(font->data.dirtyRect.w == 0 || font->data.dirtyRect.h == 0)
            font->data.dirtyRect = dstRect;
        else
            font->data.dirtyRect = rectUnion(&font->data.dirtyRect, &dstRect);
        srcRect = copyS;
        dstRect = copyD;
        
        x = preFnX;  // Restore real position
        y = preFnY;
        
        x += dstRect.w + font->letterSpacing;
    }
    
    return font->data.dirtyRect;
}

void NF_DrawPos(NFont* font, SDL_Surface* dest, int x, int y, NFontAnim_Params params, NFontAnim_Fn posFn, const char* text, ...)
{
    if(font == NULL)
        return;
    va_list lst;
    va_start(lst, text);
    vsprintf(font->buffer, text, lst);
    va_end(lst);

    font->data.userVar = NULL;
    _DrawToSurfacePos(font, dest, x, y, params, posFn, NF_LEFT);
}

void NF_DrawPosAlign(NFont* font, SDL_Surface* dest, int x, int y, NFontAnim_Params params, NFontAnim_Fn posFn, NF_AlignEnum align, const char* text, ...)
{
    if(font == NULL)
        return;
    va_list lst;
    va_start(lst, text);
    vsprintf(font->buffer, text, lst);
    va_end(lst);

    font->data.userVar = NULL;
    _DrawToSurfacePos(font, dest, x, y, params, posFn, align);
}






void NF_bounce(int* x, int* y, NFontAnim_Params params, NFontAnim_Data* data)
{
    *y -= (int)(params.amplitudeX*fabs(sin(-M_PI*params.frequencyX*params.t + (*x - data->startX)/40.0)));
    
    if(data->align == NF_CENTER)
    {
        *x -= NF_GetWidth(data->font, "%s", data->text)/2;
    }
    else if(data->align == NF_RIGHT)
    {
        *x -= NF_GetWidth(data->font, "%s", data->text);
    }
}


void NF_wave(int* x, int* y, NFontAnim_Params params, NFontAnim_Data* data)
{
    *y += (int)(params.amplitudeX*sin(-2*M_PI*params.frequencyX*params.t + (*x - data->startX)/40.0));
    
    if(data->align == NF_CENTER)
    {
        *x -= NF_GetWidth(data->font, "%s", data->text)/2;
    }
    else if(data->align == NF_RIGHT)
    {
        *x -= NF_GetWidth(data->font, "%s", data->text);
    }
}


void NF_stretch(int* x, int* y, NFontAnim_Params params, NFontAnim_Data* data)
{
    float place = (float)(data->index) / strlen(data->text);
    if(data->align == NF_CENTER)
    {
        place -= 0.5f;
        *x -= NF_GetWidth(data->font, "%s", data->text)/2;
    }
    else if(data->align == NF_RIGHT)
    {
        place -= 1.0f;
        *x -= NF_GetWidth(data->font, "%s", data->text);
    }
    *x += (int)(params.amplitudeX*(place)*cos(2*M_PI*params.frequencyX*params.t));
}


void NF_shake(int* x, int* y, NFontAnim_Params params, NFontAnim_Data* data)
{
    if(data->align == NF_CENTER)
    {
        *x -= NF_GetWidth(data->font, "%s", data->text)/2;
    }
    else if(data->align == NF_RIGHT)
    {
        *x -= NF_GetWidth(data->font, "%s", data->text);
    }
    *x += (int)(params.amplitudeX*sin(2*M_PI*params.frequencyX*params.t));
    *y += (int)(params.amplitudeY*sin(2*M_PI*params.frequencyY*params.t));
}

void NF_circle(int* x, int* y, NFontAnim_Params params, NFontAnim_Data* data)
{
    // Negate auto-placement
    *x = data->startX;
    *y = data->startY;
    
    if(data->align == NF_LEFT)
    {
        *x += NF_GetWidth(data->font, "%s", data->text)/2;
    }
    else if(data->align == NF_RIGHT)
    {
        *x -= NF_GetWidth(data->font, "%s", data->text)/2;
    }
    
    float place = (float)(data->index + 1) / strlen(data->text);
    *x += (int)(params.amplitudeX*cos(place * 2*M_PI - 2*M_PI*params.frequencyX*params.t));
    *y += (int)(params.amplitudeY*sin(place * 2*M_PI - 2*M_PI*params.frequencyY*params.t));
}


