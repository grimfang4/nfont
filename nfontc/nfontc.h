/*
NFontC v3.0.0: A bitmap font struct for SDL
by Jonathan Dearborn 11-28-11

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

Control:
    void         NF_Init(NFont* font);
    NFont*       NF_New();
    void         NF_Free(NFont* font);
    void         NF_Push(NFont* font);
    NFont*       NF_Pop();
    bool         NF_SetFont(SDL_Surface* FontSurface, bool cleanUp);
    bool         NF_ResetFont(SDL_Surface* Dest, SDL_Surface* FontSurface, bool cleanUp);
    void         NF_SetBuffer(unsigned int size);
    void         NF_SetCleanUp(bool enable);
    SDL_Surface* NF_GetDest();
    void         NF_SetDest(SDL_Surface* Dest);

Drawing:
    void         NF_Draw(int x, int y, const char* formatted_text, ...);
    void         NF_DrawCenter(int x, int y, const char* formatted_text, ...);
    void         NF_DrawRight(int x, int y, const char* formatted_text, ...);
    
    SDL_Surface* NF_NewColorSurface(Uint32 top, Uint32 bottom, int heightAdjust);
    void         NF_LoadTTF(TTF_Font* ttf, SDL_Color fg);
    void         NF_LoadTTF(TTF_Font* ttf, SDL_Color fg, SDL_Color bg);

Metrics:
    int          NF_GetHeight(const char* formatted_text = NULL, ...);
    int          NF_GetWidth(const char* formatted_text, ...);
    int          NF_GetMaxWidth();
    void         NF_SetSpacing(int LetterSpacing);
    int          NF_GetSpacing();
    void         NF_SetLineSpacing(int LineSpacing);
    int          NF_GetLineSpacing();
    int          NF_GetBaseline();
    int          NF_GetAscent();
    int          NF_GetDescent()
    
Misc:
    char*        NF_CopyString(const char* c);
    Uint32       NF_GetPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y);  // No Alpha?
    

Animation:
    void         NF_DrawPos(int x, int y, NFontAnim_Fn posFn, const char* text, ...);
    void         NF_DrawPosX(int x, int y, NFontAnim_Fn posFn, void* userVar, const char* text, ...);
    void         NF_DrawAll(int x, int y, NFontAnim_Fn allFn, const char* text, ...);
    void         NF_DrawAllX(int x, int y, NFontAnim_Fn allFn, void* userVar, const char* text, ...);

Method Descriptions:

    void NF_Init(NFont* font);
        Resets an NFont to a usable state.  This is the first call that must be 
        made for an NFont that was not created using NF_New().  After this, 
        NF_ResetFont() should be called.
        
    NFont* NF_New();
        Returns a newly allocated and initialized NFont.  NF_ResetFont() should 
        be called next.
        
    void NF_Free(NFont* font);
        Frees the memory held by an NFont that was created using NF_New().
        
    void NF_Push(NFont* font);
        Pushes the given font onto the font stack, making it the current font.
        
    NFont* NF_Pop();
        Returns the current font and makes the next font in the stack to be the
        new current font.
        
    char* NF_CopyString(const char* c);
        Built-in c-string copier.  It is used internally by NFont, so why
        not make it public?

    void NF_Draw(int x, int y, const char* formatted_text, ...);
        Draws a left-aligned text block.

    void NF_DrawCenter(int x, int y, const char* formatted_text, ...);
        Draws a center-aligned text block.

    void NF_DrawRight(int x, int y, const char* formatted_text, ...);
        Draws a right-aligned text block.

    SDL_Surface* NF_GetDest();
        Returns a pointer to the surface that the font is currently drawing to.

    int NF_GetHeight(const char* formatted_text = NULL, ...);
        Returns the height of the given text.

    Uint32 NF_GetPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y);  // No Alpha
        Built-in pixel reading routine.  Ditto with copyString().

    int NF_GetWidth(const char* formatted_text, ...);
        Returns the width of the given text.

    SDL_Surface* NF_NewColorSurface(Uint32 top, Uint32 bottom, int heightAdjust);
        Returns a pointer to a new SDL_Surface with the specified colors.
        
    void NF_LoadTTF(TTF_Font* ttf, SDL_Color fg, SDL_Color* bg);
        Creates and loads a new font surface from the given SDL_ttf font.  If 
        the bg color is NULL, it uses a 32-bit surface with alpha.  Otherwise, 
        it uses a 24-bit RGB surface.

    void NF_SetBuffer(unsigned int size);
        Changes the size of the internal buffer so the user can control some
        of the memory consumption of the class or extend the maximum string
        length.

    void NF_SetCleanUp(bool enable);
        Enables or disables the auto-freeing of the current font source surface.

    void NF_SetDest(SDL_Surface* Dest);
        Sets the surface to draw on.

    bool NF_SetFont(SDL_Surface* FontSurface, bool CleanUp);
        Initializes the font to use the given preloaded font surface.  If
        cleanUp is true, the surface will be freed when either the font is
        deleted or setFont() is called again.

    bool NF_ResetFont(SDL_Surface* Dest, SDL_Surface* FontSurface, bool CleanUp);
        Sets the destination surface and calls setFont().
    
    void NF_SetSpacing(int LetterSpacing);
        Sets the extra spacing between letters.

    int NF_GetSpacing();
        Returns the letter spacing.

    void NF_SetLineSpacing(int LineSpacing)
        Sets the extra spacing between lines.

    int NF_GetLineSpacing();
        Returns the line spacing.

    int NF_GetBaseline();
        Returns the baseline of the letters.

    int NF_GetAscent();
        Returns how high above the baseline the letters extend.

    int NF_GetDescent()
        Returns how far below the baseline the letters extend.

    int NF_GetMaxWidth();
        Returns the width of the widest character.

    void NF_DrawPos(int x, int y, NFontAnim_Fn posFn, const char* text, ...);
        This function accepts a special function pointer that allows the 
        final position of each drawn character to be determined by the user.
        
    void NF_DrawAll(int x, int y, NFontAnim_Fn allFn, const char* text, ...);
        This function accepts a function pointer that handles all of the 
        drawing of the font for the given text.  Check out write() for ideas
        of how to make such a function work.

    void NF_DrawPosX(int x, int y, NFontAnim_Fn posFn, void* userVar, const char* text, ...);
        Same as drawPos(), except you can pass in your own data.
        
    void NF_DrawAllX(int x, int y, NFontAnim_Fn allFn, void* userVar, const char* text, ...);
        Same as drawAll(), except you can pass in your own data.

Changes:
    v1.71 - Added alpha-enabled SDL_TTF loader.
    v1.7 - A bugfix in getWidth() and added SDL_ttf support.
    v1.611 - Added NF_Init() and C++ compatibility.
    v1.61 - Fixed animated drawing (vsprintf portability bug).
    v1.6 - Moved newColorSurface() into public access to replace newColor().
           Made getWidth() and getHeight() accept formatted strings.  Combined NFontAnim 
           into NFont.h.  Added new font metrics functions.
    v1.5 - Changed 'dest' to a standard pointer for simplicity's sake, removed
           a constructor (too error-prone when global) and added resetFont().
           Moved big functions to a new source file to avoid super-bloat inlining.
    v1.41 - Added copyS and copyD in write() as a simple solution to
            SDL_BlitSurface's destructive clipping.  This had caused the rects
            to permanently change when one character was clipped.
    v1.4  - newColor() and some functionality changes.
    v1.3  - Big API changes including removal of dependence on SDL_image.
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
    SDL_Surface* src;  // bitmap source of characters

    Uint16 height;

    Uint16 maxWidth;
    Uint16 baseline;
    int ascent;
    int descent;

    int lineSpacing;
    int letterSpacing;

    int charPos[256];
    Uint16 charWidth[256];
    int maxPos;

    char* buffer;  // Buffer for efficient drawing

    struct NFontAnim_Data data;  // Data is wrapped in a struct so it can all be passed to 
                          // the function pointers

};


// Function pointer
typedef void (*NFontAnim_Fn)(int*, int*, NFontAnim_Params, NFontAnim_Data*);

#ifdef __cplusplus
extern "C" {
#endif

void NF_Init(NFont* font);
NFont* NF_New();
void NF_Free(NFont* font);
void NF_Push(NFont* font);
NFont* NF_Pop();

void NF_SetSpacing(int LetterSpacing);
int NF_GetSpacing();
void NF_SetLineSpacing(int LineSpacing);
int NF_GetLineSpacing();
int NF_GetBaseline();
int NF_GetMaxWidth();
void NF_SetBuffer(unsigned int size);
void NF_SetCleanUp(Uint8 enable);
Uint8 NF_Load(SDL_Surface* FontSurface);

SDL_Surface* NF_NewColorSurface(Uint32 top, Uint32 bottom, int heightAdjust);

#ifndef NFONT_NO_TTF

void NF_LoadTTF(const char* filename_ttf, Uint32 pointSize, SDL_Color fg, SDL_Color* bg, int style);
void NF_LoadTTF_Font(TTF_Font* ttf, SDL_Color fg, SDL_Color* bg);

#endif

SDL_Rect NF_Draw(SDL_Surface* dest, int x, int y, const char* formatted_text, ...);
SDL_Rect NF_DrawAlign(SDL_Surface* dest, int x, int y, NF_AlignEnum align, const char* formatted_text, ...);
SDL_Rect NF_DrawColumn(SDL_Surface* dest, int x, int y, Uint16 width, const char* formatted_text, ...);
SDL_Rect NF_DrawColumnAlign(SDL_Surface* dest, int x, int y, Uint16 width, NF_AlignEnum align, const char* formatted_text, ...);
SDL_Rect NF_DrawBox(SDL_Surface* dest, SDL_Rect box, const char* formatted_text, ...);
SDL_Rect NF_DrawBoxAlign(SDL_Surface* dest, SDL_Rect box, NF_AlignEnum align, const char* formatted_text, ...);


int NF_GetHeight(const char* formatted_text, ...);
int NF_GetWidth(const char* formatted_text, ...);


int NF_SetBaseline(int Baseline);
int NF_GetAscentChar(const char character);
int NF_GetAscent(const char* formatted_text, ...);
int NF_GetDescentChar(const char character);
int NF_GetDescent(const char* formatted_text, ...);






NFontAnim_Params NF_AnimParams(float t, float amplitudeX, float frequencyX, float amplitudeY, float frequencyY);

void NF_DrawPos(SDL_Surface* dest, int x, int y, NFontAnim_Params params, NFontAnim_Fn posFn, const char* text, ...);
void NF_DrawPosAlign(SDL_Surface* dest, int x, int y, NFontAnim_Params params, NFontAnim_Fn posFn, NF_AlignEnum align, const char* text, ...);


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
