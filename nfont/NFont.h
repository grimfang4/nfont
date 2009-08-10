//Comment out the define for NFontAnim if you don't want to use it.
#define NF_USE_ANIM

// Uncomment this if you want to use the SDL_ttf load feature (loadTTF).
//#define NF_USE_TTF

#define NF_USE_OPENGL


#define NF_VERSION_MAJOR 2
#define NF_VERSION_MINOR 0
#define NF_VERSION_BUGFIX 0

void NF_GetVersion(char* result);

// Uncomment this if you want to use the OpenGL features.
//#define NF_USE_OPENGL

/*
NFont: A bitmap font class for SDL
by Jonathan Dearborn 8-6-09
(class originally adapted from Florian Hufsky, jnrdev)

Requires:
    SDL ("SDL.h") [www.libsdl.org]

Optionally Requires:
    SDL_ttf ("SDL_ttf.h") [www.libsdl.org]
    OpenGL ("gl.h")

Notes:
    NFont improves on the SFont formula by having less typing in each function
    call (due to destination pointer), text-block alignment, full
    support for the newline character ('\n'), color-changing, and extended ASCII 
    support.  It accepts only SDL_Surfaces so that any image format you can load 
    can be used as an NFont.

    NFont uses a pointer (SDL_Surface*) to handle the destination
    surface.  Be aware that you will need to use setDest() if you replace the
    memory that it points to (like when using screen = SDL_SetVideoMode()).
    
    NFont can use standard SFont bitmaps or extended bitmaps.  The standard bitmaps
    have the following characters (ASCII 33-126) separated by pink (255, 0, 255) pixels in the topmost
    row:
    ! " # $ % & ' ( ) * + , - . / 0 1 2 3 4 5 6 7 8 9 : ; < = > ? @ A B C D E F G H I J K L M N O P Q R S T U V W X Y Z [ \ ] ^ _ ` a b c d e f g h i j k l m n o p q r s t u v w x y z { | } ~
    
    And the extended bitmaps have these (ASCII 161-255):
    ! " # $ % & ' ( ) * + , - . / 0 1 2 3 4 5 6 7 8 9 : ; < = > ? @ A B C D E F G H I J K L M N O P Q R S T U V W X Y Z [ \ ] ^ _ ` a b c d e f g h i j k l m n o p q r s t u v w x y z { | } ~ � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � �
    
    NFont can also load SDL_ttf fonts.  Uncomment the define at the 
    top of this file and call loadTTF() to use the flexibility of NFont with
    TrueType fonts.

    If you come up with something cool using NFont, I'd love to hear about it.
    Any comments can be sent to GrimFang4@hotmail.com

License:
    The short:
    Use it however you'd like, but give me credit if you share this code
    or a compiled version of the code, whether or not it is modified.
    
    The long:
    Copyright (c) 2009, Jonathan Dearborn
    All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted 
provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list 
      of conditions and the following disclaimer.
    * Redistributions in binary form, excluding commercial executables, must reproduce 
      the above copyright notice, this list of conditions and the following disclaimer 
      in the documentation and/or other materials provided with the distribution.
    * The names of its contributors may not be used to endorse or promote products 
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS 
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY 
WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Constructors:
    NFont();

Methods:
    static char* copyString(const char* c);
    void         draw(int x, int y, const char* formatted_text, ...);
    void         drawCenter(int x, int y, const char* formatted_text, ...);
    void         drawRight(int x, int y, const char* formatted_text, ...);
    SDL_Surface* getDest();
    int          getHeight(const char* formatted_text = NULL, ...);
    Uint32       getPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y);  // No Alpha?
    int          getWidth(const char* formatted_text, ...);
    SDL_Surface* newColorSurface(Uint32 top, Uint32 bottom, int heightAdjust = 0);
    void         loadTTF(TTF_Font* ttf, SDL_Color fg)
    void         loadTTF(TTF_Font* ttf, SDL_Color fg, SDL_Color bg)
    void         setBuffer(unsigned int size);
    void         setCleanUp(bool enable);
    void         setDest(SDL_Surface* Dest);
    bool         setFont(SDL_Surface* FontSurface, bool cleanUp = 0);
    bool         resetFont(SDL_Surface* Dest, SDL_Surface* FontSurface, bool cleanUp = 0);
    void         setSpacing(int LetterSpacing);
    int          getSpacing();
    void         setLineSpacing(int LineSpacing);
    int          getLineSpacing();
    int          getBaseline();
    int          getAscent();
    int          getDescent()
    int          getMaxWidth();

Method Descriptions:
    char* copyString(const char* c);
        Built-in c-string copier.  It is used internally by NFont, so why
        not make it public?

    void draw(int x, int y, const char* formatted_text, ...);
        Draws a left-aligned text block.

    void drawCenter(int x, int y, const char* formatted_text, ...);
        Draws a center-aligned text block.

    void drawRight(int x, int y, const char* formatted_text, ...);
        Draws a right-aligned text block.

    SDL_Surface* getDest();
        Returns a pointer to the surface that the font is currently drawing to.

    int getHeight(const char* formatted_text = NULL, ...);
        Returns the height of the given text.

    Uint32 getPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y);  // No Alpha
        Built-in pixel reading routine.  Ditto with copyString().

    int getWidth(const char* formatted_text, ...);
        Returns the width of the given text.

    SDL_Surface* newColorSurface(Uint32 top, Uint32 bottom, int heightAdjust = 0);
        Returns a pointer to a new NFont font surface with the specified colors.
        
    void loadTTF(TTF_Font* ttf, SDL_Color fg)
        Creates and loads a new font surface from the given SDL_ttf font.  Uses a 
        32-bit surface with alpha.
        
    void loadTTF(TTF_Font* ttf, SDL_Color fg, SDL_Color bg)
        Creates and loads a new font surface from the given SDL_ttf font.  Uses a
        24-bit RGB surface.

    void setBuffer(unsigned int size);
        Changes the size of the internal buffer so the user can control some
        of the memory consumption of the class or extend the maximum string
        length.

    void setCleanUp(bool enable);
        Enables or disables the auto-freeing of the current font source surface.

    void setDest(SDL_Surface* Dest);
        Sets the surface to draw on.

    bool setFont(SDL_Surface* FontSurface, bool CleanUp = 0);
        Initializes the font to use the given preloaded font surface.  If
        cleanUp is true, the surface will be freed when either the font is
        deleted or setFont() is called again.

    bool resetFont(SDL_Surface* Dest, SDL_Surface* FontSurface, bool CleanUp = 0);
        Sets the destination surface and calls setFont().
    
    void setSpacing(int LetterSpacing);
        Sets the extra spacing between letters.

    int getSpacing();
        Returns the letter spacing.

    void setLineSpacing(int LineSpacing)
        Sets the extra spacing between lines.

    int getLineSpacing();
        Returns the line spacing.

    int getBaseline();
        Returns the baseline of the letters.

    int getAscent();
        Returns how high above the baseline the letters extend.

    int getDescent()
        Returns how far below the baseline the letters extend.

    int getMaxWidth();
        Returns the width of the widest character.

Changes:
    v2.0.0 - Added the old constructor and initializer lists.  Cleaned up some
           various code.  Moved newColorSurface() and getPixel() out of the class.
    v1.71 - Added alpha-enabled SDL_TTF loader.
    v1.7 - A bugfix in getWidth() and added SDL_ttf support.
    v1.61 - Fixed animated drawing.
    v1.6 - Moved newColorSurface() into public access to replace newColor().
           Made getWidth() and getHeight() accept formatted strings.  Combined NFontAnim 
           into NFont.h.  Added new font metrics functions.
    v1.5 - Changed 'dest' to a standard pointer for simplicity's sake, removed
           a constructor (too error-prone when global) and added resetFont().
           Moved big functions to a new source file.
    v1.41 - Added copyS and copyD in write() as a simple solution to
            SDL_BlitSurface's destructive clipping.  This had caused the rects
            to permanently change when one character was clipped.
    v1.4  - newColor() and some functionality changes.
    v1.3  - Big API changes including removal of dependence on SDL_image.
*/

#ifndef _NFONT_H__
#define _NFONT_H__

#include "SDL.h"
#include "stdarg.h"

#ifdef NF_USE_TTF
    #include "SDL_ttf.h"
#endif

#ifdef NF_USE_OPENGL
	//FreeType Headers
	#include <ft2build.h>
	#include <freetype/freetype.h>
	#include <freetype/ftglyph.h>
	#include <freetype/ftoutln.h>
	#include <freetype/fttrigon.h>
	
	//OpenGL Headers 
	#include <GL/gl.h>
	#include <GL/glu.h>
	
	//Some STL headers
	#include <vector>
	#include <string>
	
	//Using the STL exception library increases the
	//chances that someone else using our code will corretly
	//catch any exceptions that we throw.
	#include <stdexcept>
#endif

class NFont
{
protected:
    SDL_Surface* src;  // bitmap source of characters
    SDL_Surface* dest; // Destination to blit to
    bool cleanUp;

    int height;

    int maxWidth;
    int baseline;
    int ascent;
    int descent;

    int lineSpacing;
    int letterSpacing;

    int charPos[256];
    int charWidth[256];
    int maxPos;

    char* buffer;  // Buffer for efficient drawing
    unsigned int buffSize;

    void drawToSurface(int x, int y, const char* text);

public:

    #ifdef NF_USE_OPENGL
	float h;			///< Holds the height of the font.
	GLuint * textures;	///< Holds the texture id's 
	GLuint list_base;	///< Holds the first display list id

	//The init function will create a font of
	//of the height h from the file fname.
	void loadTex(const char * fname, unsigned int h);

	//The flagship function of the library - this thing will print
	//out text at window coordinates x,y, using the font ft_font.
	//The current modelview matrix will also be applied to the text. 
	void drawf(float x, float y, const char *fmt, ...);
    
    #endif

    NFont();
    NFont(SDL_Surface* Dest, SDL_Surface* FontSurface, bool CleanUp = 0);

    ~NFont();


    static char* copyString(const char* c);

    void draw(int x, int y, const char* formatted_text, ...);

    void drawCenter(int x, int y, const char* formatted_text, ...);

    void drawRight(int x, int y, const char* formatted_text, ...);

    SDL_Surface* getDest()
    {
        return dest;
    }
    
    SDL_Surface* getSurface()
    {
        return src;
    }

    int getHeight(const char* formatted_text = NULL, ...);

    int getWidth(const char* formatted_text, ...);

    void setSpacing(int LetterSpacing)
    {
        letterSpacing = LetterSpacing;
    }

    int getSpacing()
    {
        return letterSpacing;
    }

    void setLineSpacing(int LineSpacing)
    {
        lineSpacing = LineSpacing;
    }

    int getLineSpacing()
    {
        return lineSpacing;
    }

    int getBaseline()
    {
        return baseline;
    }

    int setBaseline(int Baseline = -1);
    
    int getAscent(const char character);
    
    int getAscent(const char* formatted_text = NULL, ...);
    
    int getDescent(const char character);

    int getDescent(const char* formatted_text = NULL, ...);

    int getMaxWidth()
    {
        return maxWidth;
    }


    
    #ifdef NF_USE_TTF
    
    void loadTTF(TTF_Font* ttf, SDL_Color fg);
    
    void loadTTF(TTF_Font* ttf, SDL_Color fg, SDL_Color bg);
    
    #endif

    #ifdef NF_USE_OPENGL
    
    /*void loadTex(GLuint textureID, float charPosf[256])
    {
        tex = textureID;
        isBound = false;
        usingTex = true;
        for(int i = 0; i < 256; i++)
        {
            this->charPosf[i] = charPosf[i];
            if(i < 255)
                charWidthf[i] = charPosf[i+1] - charPosf[i];
            else
                charWidthf[i] = 20;
        }
    }*/
    
    #endif

    void setBuffer(unsigned int size)
    {
        delete[] buffer;
        if(size > 0)
        {
            buffer = new char[size];
            buffSize = size;
        }
        else
        {
            buffer = new char[1024];
            buffSize = 1024;
        }
    }

    void setCleanUp(bool enable)
    {
        cleanUp = enable;
    }

    void setDest(SDL_Surface* Dest)
    {
        dest = Dest;
    }

    bool setFont(SDL_Surface* FontSurface, bool CleanUp = 0);

    bool resetFont(SDL_Surface* Dest, SDL_Surface* FontSurface, bool CleanUp = 0)
    {
        dest = Dest;

        return setFont(FontSurface, CleanUp);
    }

};





#ifdef NF_USE_ANIM

/*
NFontAnim: An animating bitmap font class for SDL
by Jonathan Dearborn

Notes:
    NFontAnim is a derived class of NFont that differentiates itself by presenting 
    the user (you, the programmer) with the ability to control the font in two 
    ways: It's position or it's everything.  By using drawPos() or drawPosX(), you can 
    use a function you create to handle the final positions of the drawn 
    characters.  With drawAll() and drawAllX(), you can handle everything that 
    the font does (please use my drawToSurface() function as a reference).
    
    
Constructors:
    NFontAnim();

New Methods:
    void drawPos(int x, int y, NFontAnim_Fn posFn, const char* text, ...);
    void drawAll(int x, int y, NFontAnim_Fn allFn, const char* text, ...);
    void drawPosX(int x, int y, NFontAnim_Fn posFn, void* userVar, const char* text, ...);
    void drawAllX(int x, int y, NFontAnim_Fn allFn, void* userVar, const char* text, ...);

New Method Descriptions:

    void drawPos(int x, int y, NFontAnim_Fn posFn, const char* text, ...);
        This function accepts a special function pointer that allows the 
        final position of each drawn character to be determined by the user.
        
    void drawAll(int x, int y, NFontAnim_Fn allFn, const char* text, ...);
        This function accepts a function pointer that handles all of the 
        drawing of the font for the given text.  Check out write() for ideas
        of how to make such a function work.

    void drawPosX(int x, int y, NFontAnim_Fn posFn, void* userVar, const char* text, ...);
        Same as drawPos(), except you can pass in your own data.
        
    void drawAllX(int x, int y, NFontAnim_Fn allFn, void* userVar, const char* text, ...);
        Same as drawAll(), except you can pass in your own data.
        
*/

class NFontAnim;


struct NFontAnim_Data
{
    NFontAnim* font;
    
    SDL_Surface* dest;
    SDL_Surface* src;
    char* text;  // Buffer for efficient drawing
    int height;
    int* charPos;
    int* charWidth;
    int maxX;
    
    int index;
    int letterNum;
    int wordNum;
    int lineNum;
    int startX;
    int startY;
    void* userVar;
};

// Function pointer
typedef void (*NFontAnim_Fn)(int&, int&, NFontAnim_Data&);


class NFontAnim : public NFont
{
protected:
    NFontAnim_Data data;  // Data is wrapped in a struct so it can all be passed to 
                          // the function pointers
                      
    void drawToSurfacePos(int x, int y, NFontAnim_Fn posFn);
    
public:
    NFontAnim();
    
    ~NFontAnim()
    {
        delete[] buffer;
        if(cleanUp)
            SDL_FreeSurface(src);
    }
    
    void drawPos(int x, int y, NFontAnim_Fn posFn, const char* text, ...);
    void drawPosX(int x, int y, NFontAnim_Fn posFn, void* userVar, const char* text, ...);
    void drawAll(int x, int y, NFontAnim_Fn allFn, const char* text, ...);
    void drawAllX(int x, int y, NFontAnim_Fn allFn, void* userVar, const char* text, ...);
};



#endif // NF_USE_ANIM



SDL_Surface* NF_NewColorSurface(SDL_Surface* font_surface, Uint32 top, Uint32 bottom, int heightAdjust = 0);

Uint32 NF_GetPixel(SDL_Surface *Surface, int x, int y);



#endif // _NFONT_H__
