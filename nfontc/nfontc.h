/*
NFontC v3.0.0: A bitmap font struct for SDL
by Jonathan Dearborn 12-03-11

Requires:
    SDL ("SDL.h") [www.libsdl.org]
    SDL_ttf ("SDL_ttf.h") [www.libsdl.org]

Notes:
    NFont is a bitmap font library with text-block alignment, full
    support for the newline character ('\n'), and position animation.  
    It accepts SDL_Surfaces so that any image format you can load 
    can be used as an NFont.
    
    NFont natively loads SFont bitmaps and TrueType fonts with SDL_ttf.  The 
    standard bitmaps have the following characters (ASCII 33-126) separated by 
    pink (255, 0, 255) pixels in the topmost row:
    ! " # $ % & ' ( ) * + , - . / 0 1 2 3 4 5 6 7 8 9 : ; < = > ? @ A B C D E F G H I J K L M N O P Q R S T U V W X Y Z [ \ ] ^ _ ` a b c d e f g h i j k l m n o p q r s t u v w x y z { | } ~
    
    Define NFONT_NO_TTF before including NFont.h to disable TrueType fonts.

    If you come up with something cool using NFont, I'd love to hear about it.
    Any comments can be sent to GrimFang4 [at] gmail [dot] com

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

#ifndef _NFONT_H__
#define _NFONT_H__

#include "SDL.h"

#ifndef NFONT_NO_TTF
    #include "SDL_ttf.h"
#endif

typedef struct NFont NFont;

typedef enum NF_AlignEnum {NF_LEFT, NF_CENTER, NF_RIGHT} NF_AlignEnum;

typedef struct NFontAnim_Data
{
    struct NFont* font;
    
    SDL_Surface* dest;
    SDL_Surface* src;
    char* text;  // Buffer for efficient drawing
    Uint16 height;
    int* charPos;
    Uint16* charWidth;
    int maxX;
    
    int index;
    int letterNum;
    int wordNum;
    int lineNum;
    int startX;
    int startY;
    NF_AlignEnum align;
    
    void* userVar;
    
    SDL_Rect dirtyRect;
} NFontAnim_Data;

typedef struct NFontAnim_Params
{
    float t;
    float amplitudeX;
    float amplitudeY;
    float frequencyX;
    float frequencyY;
} NFontAnim_Params;



struct NFont
{
    // The following can be modified directly
    
    Uint16 height;

    Uint16 baseline;
    int ascent;
    int descent;

    int lineSpacing;
    int letterSpacing;

    struct NFontAnim_Data data;  // Data is wrapped in a struct so it can all be passed to 
                          // the function pointers
    
    
    // Don't mess with the following directly.  Use the NF_* functions.
    
    SDL_Surface* src;  // bitmap source of characters
    
    Uint16 maxWidth;
    int charPos[256];
    Uint16 charWidth[256];
    int maxPos;

    char* buffer;  // Buffer for efficient drawing

};





// Function pointer
typedef void (*NFontAnim_Fn)(int*, int*, NFontAnim_Params, NFontAnim_Data*);

#ifdef __cplusplus
extern "C" {
#endif

// Loading
NFont* NF_New();
void NF_Free(NFont* font);
Uint8 NF_Load(NFont* font, SDL_Surface* FontSurface);
void NF_SetBuffer(NFont* font, unsigned int size);
SDL_Surface* NF_VerticalGradient(SDL_Surface* targetSurface, Uint32 top, Uint32 bottom, int heightAdjust);

#ifndef NFONT_NO_TTF
void NF_LoadTTF(NFont* font, const char* filename_ttf, Uint32 pointSize, SDL_Color fg, SDL_Color* bg, int style);
void NF_LoadTTF_Font(NFont* font, TTF_Font* ttf, SDL_Color fg, SDL_Color* bg);
#endif

// Drawing
SDL_Rect NF_Draw(NFont* font, SDL_Surface* dest, int x, int y, const char* formatted_text, ...);
SDL_Rect NF_DrawAlign(NFont* font, SDL_Surface* dest, int x, int y, NF_AlignEnum align, const char* formatted_text, ...);
SDL_Rect NF_DrawColumn(NFont* font, SDL_Surface* dest, int x, int y, Uint16 width, const char* formatted_text, ...);
SDL_Rect NF_DrawColumnAlign(NFont* font, SDL_Surface* dest, int x, int y, Uint16 width, NF_AlignEnum align, const char* formatted_text, ...);
SDL_Rect NF_DrawBox(NFont* font, SDL_Surface* dest, SDL_Rect box, const char* formatted_text, ...);
SDL_Rect NF_DrawBoxAlign(NFont* font, SDL_Surface* dest, SDL_Rect box, NF_AlignEnum align, const char* formatted_text, ...);


// Metrics
int NF_GetHeight(NFont* font, const char* formatted_text, ...);
int NF_GetWidth(NFont* font, const char* formatted_text, ...);

int NF_SetBaseline(NFont* font);
int NF_GetAscentChar(NFont* font, const char character);
int NF_GetAscent(NFont* font, const char* formatted_text, ...);
int NF_GetDescentChar(NFont* font, const char character);
int NF_GetDescent(NFont* font, const char* formatted_text, ...);


// Animation
NFontAnim_Params NF_AnimParams(float t, float amplitudeX, float frequencyX, float amplitudeY, float frequencyY);

void NF_DrawPos(NFont* font, SDL_Surface* dest, int x, int y, NFontAnim_Params params, NFontAnim_Fn posFn, const char* text, ...);
void NF_DrawPosAlign(NFont* font, SDL_Surface* dest, int x, int y, NFontAnim_Params params, NFontAnim_Fn posFn, NF_AlignEnum align, const char* text, ...);

// Built-in animations
void NF_bounce(int* x, int* y, NFontAnim_Params params, NFontAnim_Data* data);
void NF_wave(int* x, int* y, NFontAnim_Params params, NFontAnim_Data* data);
void NF_stretch(int* x, int* y, NFontAnim_Params params, NFontAnim_Data* data);
void NF_shake(int* x, int* y, NFontAnim_Params params, NFontAnim_Data* data);
void NF_circle(int* x, int* y, NFontAnim_Params params, NFontAnim_Data* data);


#ifdef __cplusplus
}
#endif


#endif // _NFONT_H__
