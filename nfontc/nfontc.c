/*
NFontC v1.71: A bitmap font struct for SDL
by Jonathan Dearborn 2-11-09
*/
#include "nfontc.h"
#include "stdarg.h"

void NF_DrawToSurfacePos(int x, int y, NFontAnim_Fn posFn);

typedef struct NF_Node NF_Node;

struct NF_Node
{
    NFont* font;
    NF_Node* next;
};

NF_Node* NFont_current = NULL;

void NF_Push(NFont* font)
{
    NF_Node* n = (NF_Node*)malloc(sizeof(NF_Node));
    n->font = font;
    n->next = NFont_current;
    NFont_current = n;
}

NFont* NF_Pop()
{
    if(NFont_current == NULL)
        return NULL;
    NF_Node* n = NFont_current;
    NFont_current = NFont_current->next;
    NFont* result = n->font;
    free(n);
    return result;
}

NFont* NF_Current()
{
    if(NFont_current == NULL)
        return NULL;
    return NFont_current->font;
}


void NF_Init(NFont* font)
{
    if(font == NULL)
        return;
    font->src = NULL;
    font->dest = NULL;
    font->cleanUp = 0;

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
}


NFont* NF_New()
{
    NFont* font = (NFont*)malloc(sizeof(NFont));
    font->src = NULL;
    font->dest = NULL;
    font->cleanUp = 0;

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
    if(font->cleanUp)
        SDL_FreeSurface(font->src);
    free(font);
}


SDL_Surface* NF_GetDest()
{
    if(NFont_current == NULL)
        return NULL;
    return NFont_current->font->dest;
}

void NF_SetSpacing(int LetterSpacing)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return;
    NFont_current->font->letterSpacing = LetterSpacing;
}

int NF_GetSpacing()
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return -1;
    return NFont_current->font->letterSpacing;
}

void NF_SetLineSpacing(int LineSpacing)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return;
    NFont_current->font->lineSpacing = LineSpacing;
}

int NF_GetLineSpacing()
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return -1;
    return NFont_current->font->lineSpacing;
}

int NF_GetBaseline()
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return -1;
    return NFont_current->font->baseline;
}

int NF_GetMaxWidth()
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return -1;
    return NFont_current->font->maxWidth;
}

void NF_SetBuffer(unsigned int size)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return;
    NFont* font = NFont_current->font;
    free(font->buffer);
    if(size > 0)
        font->buffer = (char*)malloc(size);
    else
        font->buffer = (char*)malloc(1024);
}

void NF_SetCleanUp(Uint8 enable)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return;
    NFont* font = NFont_current->font;
    font->cleanUp = enable;
}

void NF_SetDest(SDL_Surface* Dest)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return;
    NFont* font = NFont_current->font;
    font->dest = Dest;
}

Uint8 NF_ResetFont(SDL_Surface* Dest, SDL_Surface* FontSurface, Uint8 CleanUp)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return 0;
    NFont* font = NFont_current->font;
    font->dest = Dest;

    return NF_SetFont(FontSurface, CleanUp);
}

Uint32 NF_GetPixel(SDL_Surface *Surface, int x, int y)  // No Alpha?
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


SDL_Surface* NF_NewColorSurface(Uint32 top, Uint32 bottom, int heightAdjust)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return NULL;
    NFont* font = NFont_current->font;
    
    Uint8 tr, tg, tb;

    SDL_GetRGB(top, font->src->format, &tr, &tg, &tb);

    Uint8 br, bg, bb;

    SDL_GetRGB(bottom, font->src->format, &br, &bg, &bb);

    SDL_Surface* result = SDL_ConvertSurface(font->src, font->src->format, font->src->flags);

    Uint8 useCK = (result->flags & SDL_SRCALPHA) != SDL_SRCALPHA;  // colorkey if no alpha
    Uint32 colorkey = result->format->colorkey;

    Uint8 r, g, b, a;
    float ratio;
    Uint32 color;
    int temp;
    int x, y;
    
    for(x = 0, y = 0; y < result->h; x++)
    {
        if (x >= result->w)
        {
            x = 0;
            y++;

            if (y >= result->h)
                break;
        }

        ratio = (y - 2)/(float)(result->h - heightAdjust);  // the neg 3s are for full color at top and bottom

        if(!useCK)
        {
            color = NF_GetPixel(result, x, y);
            SDL_GetRGBA(color, result->format, &r, &g, &b, &a);  // just getting alpha
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


        color = SDL_MapRGBA(result->format, r, g, b, a);


        if(useCK)
        {
            if(NF_GetPixel(result, x, y) == colorkey)
                continue;
            if(color == colorkey)
                color == 0? color++ : color--;
        }

        // make sure it isn't pink
        if(color == SDL_MapRGBA(result->format, 0xFF, 0, 0xFF, a))
            color--;
        if(NF_GetPixel(result, x, y) == SDL_MapRGBA(result->format, 0xFF, 0, 0xFF, SDL_ALPHA_OPAQUE))
            continue;

        int bpp = result->format->BytesPerPixel;
        Uint8* bits = ((Uint8 *)result->pixels) + y*result->pitch + x*bpp;

        /* Set the pixel */
        switch(bpp) {
            case 1:
                *((Uint8 *)(bits)) = (Uint8)color;
                break;
            case 2:
                *((Uint16 *)(bits)) = (Uint16)color;
                break;
            case 3: { /* Format/endian independent */
                r = (color >> result->format->Rshift) & 0xFF;
                g = (color >> result->format->Gshift) & 0xFF;
                b = (color >> result->format->Bshift) & 0xFF;
                *((bits)+result->format->Rshift/8) = r;
                *((bits)+result->format->Gshift/8) = g;
                *((bits)+result->format->Bshift/8) = b;
                }
                break;
            case 4:
                *((Uint32 *)(bits)) = (Uint32)color;
                break;
        }

    }

    return result;

}


#ifdef NF_USE_TTF
void NF_LoadTTF(TTF_Font* ttf, SDL_Color fg, SDL_Color* bg)
{
    if(NFont_current == NULL || ttf == NULL)
        return;
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
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        result = SDL_CreateRGBSurface(SDL_SWSURFACE,width + 127 - 33 + 1,height,32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    #else
        result = SDL_CreateRGBSurface(SDL_SWSURFACE,width + 127 - 33 + 1,height,32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    #endif
    }
    else
    {
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        result = SDL_CreateRGBSurface(SDL_SWSURFACE,width + 127 - 33 + 1,height,24, 0xFF0000, 0x00FF00, 0x0000FF, 0);
    #else
        result = SDL_CreateRGBSurface(SDL_SWSURFACE,width + 127 - 33 + 1,height,24, 0x0000FF, 0x00FF00, 0xFF0000, 0);
    #endif
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
    
    NF_SetFont(result, 1);
}

#endif

void NF_DrawToSurface(int x, int y, const char* text)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return;
    NFont* font = NFont_current->font;
    
    const char* c = text;
    unsigned char num;
    SDL_Rect srcRect, dstRect, copyS, copyD;
    
    if(c == NULL || font->src == NULL || font->dest == NULL)
        return;
    
    srcRect.y = font->baseline - font->ascent;
    srcRect.h = dstRect.h = font->height;
    dstRect.x = x;
    dstRect.y = y;
    
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
        if(dstRect.x >= font->dest->w)
            continue;
        if(dstRect.y >= font->dest->h)
            continue;
        
        num = ctest - 33;  // Get array index
        if(num > 126) // shift the extended characters down to the correct index
            num -= 34;
        srcRect.x = font->charPos[num];
        srcRect.w = dstRect.w = font->charWidth[num];
        copyS = srcRect;
        copyD = dstRect;
        SDL_BlitSurface(font->src, &srcRect, font->dest, &dstRect);
        srcRect = copyS;
        dstRect = copyD;
        
        dstRect.x += dstRect.w + font->letterSpacing;
    }
    
}

char* NF_CopyString(const char* c)
{
    if(c == NULL) return NULL;

    int count = 0;
    for(; c[count] != '\0'; count++);

    char* result = (char*)malloc(count+1);
    
    int i;
    for(i = 0; i < count; i++)
    {
        result[i] = c[i];
    }

    result[count] = '\0';
    return result;
}

void NF_Draw(int x, int y, const char* formatted_text, ...)
{
    if(NFont_current == NULL || NFont_current->font == NULL || formatted_text == NULL)
        return;
    NFont* font = NFont_current->font;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(font->buffer, formatted_text, lst);
    va_end(lst);

    NF_DrawToSurface(x, y, font->buffer);
}

void NF_DrawCenter(int x, int y, const char* formatted_text, ...)
{
    if(NFont_current == NULL || NFont_current->font == NULL || formatted_text == NULL)
        return;
    NFont* font = NFont_current->font;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(font->buffer, formatted_text, lst);
    va_end(lst);

    char* str = NF_CopyString(font->buffer);
    char* del = str;

    // Go through str, when you find a \n, replace it with \0 and print it
    // then move down, back, and continue.
    char* c;
    for(c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            NF_DrawToSurface(x - NF_GetWidth("%s", str)/2, y, str);
            *c = '\n';
            c++;
            str = c;
            y += font->height;
        }
        else
            c++;
    }
    NF_DrawToSurface(x - NF_GetWidth("%s", str)/2, y, str);

    free(del);
}

void NF_DrawRight(int x, int y, const char* formatted_text, ...)
{
    if(NFont_current == NULL || NFont_current->font == NULL || formatted_text == NULL)
        return;
    NFont* font = NFont_current->font;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(font->buffer, formatted_text, lst);
    va_end(lst);

    char* str = NF_CopyString(font->buffer);
    char* del = str;

    char* c;
    for(c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            NF_DrawToSurface(x - NF_GetWidth("%s", str), y, str);
            *c = '\n';
            c++;
            str = c;
            y += font->height;
        }
        else
            c++;
    }
    NF_DrawToSurface(x - NF_GetWidth("%s", str), y, str);

    free(del);
}

int NF_GetHeight(const char* formatted_text, ...)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return -1;
    NFont* font = NFont_current->font;
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

int NF_GetWidth(const char* formatted_text, ...)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return -1;
    if (formatted_text == NULL)
        return 0;
    NFont* font = NFont_current->font;

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

Uint8 NF_SetFont(SDL_Surface* FontSurface, Uint8 CleanUp)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return 0;
    NFont* font = NFont_current->font;
    if(font->cleanUp)
        SDL_FreeSurface(font->src);
    font->src = FontSurface;
    if (font->src == NULL)
    {
        printf("\n ERROR: NFont given a NULL surface\n");
        font->cleanUp = 0;
        return 0;
    }

    font->cleanUp = CleanUp;

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
        if(NF_GetPixel(font->src, x, 0) != pixel)
        {
            font->charPos[i] = x;
            font->charWidth[i] = x;
            while(x < font->src->w && NF_GetPixel(font->src, x, 0) != pixel)
                x++;
            font->charWidth[i] = x - font->charWidth[i];
            if(font->charWidth[i] > font->maxWidth)
                font->maxWidth = font->charWidth[i];
            i++;
        }

        x++;
    }

    font->maxPos = x - 1;


    pixel = NF_GetPixel(font->src, 0, font->src->h - 1);

    NF_SetBaseline(-1);
    
    // Get the max ascent
    j = 1;
    while(j < font->baseline && j < font->src->h)
    {
        x = 0;
        while(x < font->src->w)
        {
            if(NF_GetPixel(font->src, x, j) != pixel)
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
            if(NF_GetPixel(font->src, x, j) != pixel)
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
        pixel = NF_GetPixel(font->src, 0, font->src->h - 1);
        SDL_UnlockSurface(font->src);
        SDL_SetColorKey(font->src, SDL_SRCCOLORKEY, pixel);
    }
    else
        SDL_UnlockSurface(font->src);

    return 1;
}



int NF_SetBaseline(int Baseline)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return -1;
    NFont* font = NFont_current->font;
    if(Baseline >= 0)
        font->baseline = Baseline;
    else
    {
        // Get the baseline by checking a, b, and c and averaging their lowest y-value.
        // Is there a better way?
        Uint32 pixel = NF_GetPixel(font->src, 0, font->src->h - 1);
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
                    if(NF_GetPixel(font->src, i, j) != pixel)
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
    }
    return font->baseline;
}

int NF_GetAscentChar(const char character)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return -1;
    NFont* font = NFont_current->font;
    unsigned char test = (unsigned char)character;
    if(test < 33 || test > 222 || (test > 126 && test < 161))
        return 0;
    unsigned char num = (unsigned char)character - 33;
    // Get the max ascent
    int x = font->charPos[num];
    int i, j = 1, result = 0;
    Uint32 pixel = NF_GetPixel(font->src, 0, font->src->h - 1); // bg pixel
    while(j < font->baseline && j < font->src->h)
    {
        i = font->charPos[num];
        while(i < x + font->charWidth[num])
        {
            if(NF_GetPixel(font->src, i, j) != pixel)
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

int NF_GetAscent(const char* formatted_text, ...)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return -1;
    NFont* font = NFont_current->font;
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
        int asc = NF_GetAscentChar(*c);
        if(asc > max)
            max = asc;
    }
    return max;
}

int NF_GetDescentChar(const char character)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return -1;
    NFont* font = NFont_current->font;
    unsigned char test = (unsigned char)character;
    if(test < 33 || test > 222 || (test > 126 && test < 161))
        return 0;
    unsigned char num = (unsigned char)character - 33;
    // Get the max descent
    int x = font->charPos[num];
    int i, j = font->src->h - 1, result = 0;
    Uint32 pixel = NF_GetPixel(font->src, 0, font->src->h - 1); // bg pixel
    while(j > 0 && j > font->baseline)
    {
        i = font->charPos[num];
        while(i < x + font->charWidth[num])
        {
            if(NF_GetPixel(font->src, i, j) != pixel)
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

int NF_GetDescent(const char* formatted_text, ...)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return -1;
    NFont* font = NFont_current->font;
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
        int des = NF_GetDescentChar(*c);
        if(des > max)
            max = des;
    }
    return max;
}






void NF_DrawToSurfacePos(int x, int y, NFontAnim_Fn posFn)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return;
    NFont* font = NFont_current->font;
        
    font->data.dest = font->dest;
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
    
    int preFnX = x;
    int preFnY = y;
    
    const char* c = font->buffer;
    unsigned char num;
    SDL_Rect srcRect, dstRect, copyS, copyD;
    
    if(c == NULL || font->src == NULL || font->dest == NULL)
        return;
    
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
        posFn(&x, &y, &(font->data));
        
        dstRect.x = x;
        dstRect.y = y;
        
        copyS = srcRect;
        copyD = dstRect;
        SDL_BlitSurface(font->src, &srcRect, font->dest, &dstRect);
        srcRect = copyS;
        dstRect = copyD;
        
        x = preFnX;  // Restore real position
        y = preFnY;
        
        x += dstRect.w + font->letterSpacing;
    }
    
}

void NF_DrawPos(int x, int y, NFontAnim_Fn posFn, const char* text, ...)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return;
    va_list lst;
    va_start(lst, text);
    vsprintf(NFont_current->font->buffer, text, lst);
    va_end(lst);

    NFont_current->font->data.userVar = NULL;
    NF_DrawToSurfacePos(x, y, posFn);
}

void NF_DrawPosX(int x, int y, NFontAnim_Fn posFn, void* userVar, const char* text, ...)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return;
    va_list lst;
    va_start(lst, text);
    vsprintf(NFont_current->font->buffer, text, lst);
    va_end(lst);

    NFont_current->font->data.userVar = userVar;
    NF_DrawToSurfacePos(x, y, posFn);
}

void NF_DrawAll(int x, int y, NFontAnim_Fn allFn, const char* text, ...)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return;
    va_list lst;
    va_start(lst, text);
    vsprintf(NFont_current->font->buffer, text, lst);
    va_end(lst);

    NFont_current->font->data.userVar = NULL;
    allFn(&x, &y, &(NFont_current->font->data));
}

void NF_DrawAllX(int x, int y, NFontAnim_Fn allFn, void* userVar, const char* text, ...)
{
    if(NFont_current == NULL || NFont_current->font == NULL)
        return;
    va_list lst;
    va_start(lst, text);
    vsprintf(NFont_current->font->buffer, text, lst);
    va_end(lst);

    NFont_current->font->data.userVar = userVar;
    allFn(&x, &y, &(NFont_current->font->data));
}



