/*
NFontR v4.0.0: A font class for SDL and SDL_Renderer
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

#ifndef _NFONTR_H__
#define _NFONTR_H__

#define NFONTR

#include "SDL.h"
#include "stdarg.h"
#include <map>
#include <vector>

#ifndef NFONT_NO_TTF
    #include "SDL_ttf.h"
#endif

// Let's pretend this exists...
#define TTF_STYLE_OUTLINE	16


class NFont
{
  public:

    class Color
    {
        public:
        
        Uint8 r, g, b, a;
        
        Color();
        Color(Uint8 r, Uint8 g, Uint8 b);
        Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
        Color(const SDL_Color& color);
        
        Color& rgb(Uint8 R, Uint8 G, Uint8 B);
        Color& rgba(Uint8 R, Uint8 G, Uint8 B, Uint8 A);
        Color& color(const SDL_Color& color);
        
        SDL_Color to_SDL_Color() const;
    };
    
    class Rectf
    {
        public:
        float x, y;
        float w, h;
        
        Rectf();
        Rectf(float x, float y);
        Rectf(float x, float y, float w, float h);
        Rectf(const SDL_Rect& rect);
        
        SDL_Rect to_SDL_Rect() const;
    };

    
    enum AlignEnum {LEFT, CENTER, RIGHT};
    
    class Scale
    {
        public:
        
        float x;
        float y;
        
        enum ScaleTypeEnum {NEAREST};
        ScaleTypeEnum type;
        
        Scale()
            : x(1.0f), y(1.0f), type(NEAREST)
        {}
        Scale(float xy)
            : x(xy), y(xy), type(NEAREST)
        {}
        Scale(float xy, ScaleTypeEnum type)
            : x(xy), y(xy), type(type)
        {}
        Scale(float x, float y)
            : x(x), y(y), type(NEAREST)
        {}
        Scale(float x, float y, ScaleTypeEnum type)
            : x(x), y(y), type(type)
        {}
    };
    
    class Effect
    {
        public:
        AlignEnum alignment;
        Scale scale;
        bool use_color;
        Color color;
        
        Effect()
            : alignment(LEFT), use_color(false), color(255, 255, 255, 255)
        {}
        
        Effect(const Scale& scale)
            : alignment(LEFT), scale(scale), use_color(false), color(255, 255, 255, 255)
        {}
        Effect(AlignEnum alignment)
            : alignment(alignment), use_color(false), color(255, 255, 255, 255)
        {}
        Effect(const Color& color)
            : alignment(LEFT), use_color(true), color(color)
        {}
        
        Effect(AlignEnum alignment, const Scale& scale)
            : alignment(alignment), scale(scale), use_color(false), color(255, 255, 255, 255)
        {}
        Effect(AlignEnum alignment, const Color& color)
            : alignment(alignment), use_color(true), color(color)
        {}
        Effect(const Scale& scale, const Color& color)
            : alignment(LEFT), scale(scale), use_color(true), color(color)
        {}
        Effect(AlignEnum alignment, const Scale& scale, const Color& color)
            : alignment(alignment), scale(scale), use_color(true), color(color)
        {}
    };
    
    class GlyphData
    {
    public:
        SDL_Rect rect;
        int cacheIndex;
        
        GlyphData()
            : cacheIndex(0)
        {
            rect.x = 0;
            rect.y = 0;
            rect.w = 0;
            rect.h = 0;
        }
        GlyphData(int cacheIndex, Sint16 x, Sint16 y, Uint16 w, Uint16 h)
            : cacheIndex(cacheIndex)
        {
            rect.x = x;
            rect.y = y;
            rect.w = w;
            rect.h = h;
        }
        GlyphData(int cacheIndex, const SDL_Rect& rect)
            : rect(rect), cacheIndex(cacheIndex)
        {}
    };
    
    struct AnimData
    {
        NFont* font;
        
        SDL_Renderer* dest;
        SDL_Texture* src;
        char* text;  // Buffer for efficient drawing
        const int* charPos;
        const Uint16* charWidth;
        float maxX;
        
        int index;
        int letterNum;
        int wordNum;
        int lineNum;
        float startX;
        float startY;
        
        NFont::AlignEnum align;
        
        void* userVar;
        
        Rectf dirtyRect;
    };
    
    class AnimParams
    {
        public:
        
        float t;
        float amplitudeX;
        float amplitudeY;
        float frequencyX;
        float frequencyY;
        
        AnimParams()
            : t(0.0f), amplitudeX(20.0f), amplitudeY(20.0f), frequencyX(1.0f), frequencyY(1.0f)
        {}
        AnimParams(float t)
            : t(t), amplitudeX(20.0f), amplitudeY(20.0f), frequencyX(1.0f), frequencyY(1.0f)
        {}
        AnimParams(float t, float amplitude, float frequency)
            : t(t), amplitudeX(amplitude), amplitudeY(20.0f), frequencyX(frequency), frequencyY(1.0f)
        {}
        AnimParams(float t, float amplitudeX, float frequencyX, float amplitudeY, float frequencyY)
            : t(t), amplitudeX(amplitudeX), amplitudeY(amplitudeY), frequencyX(frequencyX), frequencyY(frequencyY)
        {}
    };
    
    // Function pointer
    typedef void (*AnimFn)(float&, float&, const AnimParams&, AnimData&);
    
    
    
    // Static functions
    
    /*!
    Returns the Uint32 codepoint parsed from the given UTF-8 string.
    \param c A string of proper UTF-8 character values.
    \param advance_pointer If true, the source pointer will be incremented to skip the extra bytes from multibyte codepoints.
    */
    static Uint32 getCodepointFromUTF8(const char*& c, bool advance_pointer = false);
    
    /*!
    Parses the given codepoint and stores the UTF-8 bytes in 'result'.  The result is NULL terminated.
    \param result A memory buffer for the UTF-8 values.  Must be at least 5 bytes long.
    \param codepoint The Uint32 codepoint to parse.
    */
    static void getUTF8FromCodepoint(char* result, Uint32 codepoint);
    
    // Static accessors
    static void setAnimData(void* data);
    static void setBuffer(unsigned int size);
    
    // Constructors
    NFont();
    NFont(const NFont& font);
    NFont(SDL_Renderer* renderer, SDL_Surface* src);
    #ifndef NFONT_NO_TTF
        NFont(SDL_Renderer* renderer, TTF_Font* ttf);
        NFont(SDL_Renderer* renderer, TTF_Font* ttf, const NFont::Color& color);
        NFont(SDL_Renderer* renderer, const char* filename_ttf, Uint32 pointSize);
        NFont(SDL_Renderer* renderer, const char* filename_ttf, Uint32 pointSize, const NFont::Color& color, int style = TTF_STYLE_NORMAL);
        NFont(SDL_Renderer* renderer, SDL_RWops* file_rwops_ttf, Uint8 own_rwops, Uint32 pointSize, const NFont::Color& color, int style = TTF_STYLE_NORMAL);
    #endif

    ~NFont();
    
    NFont& operator=(const NFont& font);

    // Loading
    bool load(SDL_Renderer* renderer, SDL_Surface* FontSurface);
    #ifndef NFONT_NO_TTF
        bool load(SDL_Renderer* renderer, TTF_Font* ttf);
        bool load(SDL_Renderer* renderer, TTF_Font* ttf, const NFont::Color& color);
        bool load(SDL_Renderer* renderer, const char* filename_ttf, Uint32 pointSize);
        bool load(SDL_Renderer* renderer, const char* filename_ttf, Uint32 pointSize, const NFont::Color& color, int style = TTF_STYLE_NORMAL);
        bool load(SDL_Renderer* renderer, SDL_RWops* file_rwops_ttf, Uint8 own_rwops, Uint32 pointSize, const NFont::Color& color, int style = TTF_STYLE_NORMAL);
    #endif
    
    void free();

    // Drawing
    Rectf draw(SDL_Renderer* dest, float x, float y, const char* formatted_text, ...);
    Rectf draw(SDL_Renderer* dest, float x, float y, AlignEnum align, const char* formatted_text, ...);
    Rectf draw(SDL_Renderer* dest, float x, float y, const Scale& scale, const char* formatted_text, ...);
    Rectf draw(SDL_Renderer* dest, float x, float y, const Color& color, const char* formatted_text, ...);
    Rectf draw(SDL_Renderer* dest, float x, float y, const Effect& effect, const char* formatted_text, ...);
    Rectf draw(SDL_Renderer* dest, float x, float y, const AnimParams& params, NFont::AnimFn posFn, const char* text, ...);
    Rectf draw(SDL_Renderer* dest, float x, float y, const AnimParams& params, NFont::AnimFn posFn, NFont::AlignEnum align, const char* text, ...);
    
    Rectf drawBox(SDL_Renderer* dest, const Rectf& box, const char* formatted_text, ...);
    Rectf drawBox(SDL_Renderer* dest, const Rectf& box, AlignEnum align, const char* formatted_text, ...);
    Rectf drawColumn(SDL_Renderer* dest, float x, float y, Uint16 width, const char* formatted_text, ...);
    Rectf drawColumn(SDL_Renderer* dest, float x, float y, Uint16 width, AlignEnum align, const char* formatted_text, ...);
    
    // Getters
    SDL_Texture* getImage() const;
    SDL_Surface* getSurface() const;
    Uint16 getHeight() const;
    Uint16 getHeight(const char* formatted_text, ...) const;
    Uint16 getWidth(const char* formatted_text, ...);
    Uint16 getColumnPosWidth(Uint16 width, Uint16 pos, const char* formatted_text, ...);
    Uint16 getColumnPosHeight(Uint16 width, Uint16 pos, const char* formatted_text, ...);
    Uint16 getColumnHeight(Uint16 width, const char* formatted_text, ...);
    int getSpacing() const;
    int getLineSpacing() const;
    Uint16 getBaseline() const;
    int getAscent() const;
    int getAscent(const char character);
    int getAscent(const char* formatted_text, ...);
    int getDescent() const;
    int getDescent(const char character);
    int getDescent(const char* formatted_text, ...);
    Uint16 getMaxWidth() const;
    Color getDefaultColor() const;
    
    // Setters
    void setSpacing(int LetterSpacing);
    void setLineSpacing(int LineSpacing);
    void setBaseline();
    void setBaseline(Uint16 Baseline);
    void setDefaultColor(const Color& color);
    
    #ifndef NFONT_NO_TTF
    void enableTTFOwnership();
    #endif
    
  private:
    
    SDL_Renderer* renderer;
    #ifndef NFONT_NO_TTF
    TTF_Font* ttf_source;  // TTF_Font source of characters
    bool owns_ttf_source;  // Can we delete the TTF_Font ourselves?
    #endif
    SDL_Surface* srcSurface;  // bitmap source of characters
    SDL_Texture* src;  // bitmap source of characters
    
    Color default_color;
    Uint16 height;

    Uint16 maxWidth;
    Uint16 baseline;
    int ascent;
    int descent;

    int lineSpacing;
    int letterSpacing;
    
    // Uses 32-bit (4-byte) Unicode codepoints to refer to each glyph
    // Codepoints are little endian (reversed from UTF-8) so that something like 0x00000005 is ASCII 5 and the map can be indexed by ASCII values
    // Glyph needs to store it's blitting rect
    std::map<Uint32, GlyphData> glyphs;
    SDL_Rect lastGlyph;  // Texture packing cursor
    std::vector<SDL_Texture*> glyphCache;  // TODO: Use this to store the glyphs (instead of 'src').  The issue is getting SDL_Surface glyphs blitted here.
    
    //int maxPos;
    bool addGlyph(Uint32 codepoint, Uint16 width, Uint16 maxWidth, Uint16 maxHeight);
    bool getGlyphData(GlyphData* result, Uint32 codepoint);
    
    void init();  // Common constructor

    Rectf render_animated(SDL_Renderer* dest, float x, float y, const NFont::AnimParams& params, NFont::AnimFn posFn, NFont::AlignEnum align);
    
    // Static variables
    static char* buffer;  // Shared buffer for efficient drawing
    static AnimData data;  // Data is wrapped in a struct so it can all be passed to 
                                 // the function pointers for animation
    
    Rectf render_left(SDL_Renderer* dest, float x, float y, const Scale& scale, const char* text);
    Rectf render_left(SDL_Renderer* dest, float x, float y, const char* text);
    Rectf render_center(SDL_Renderer* dest, float x, float y, const char* text);
    Rectf render_center(SDL_Renderer* dest, float x, float y, const Scale& scale, const char* text);
    Rectf render_right(SDL_Renderer* dest, float x, float y, const char* text);
    Rectf render_right(SDL_Renderer* dest, float x, float y, const Scale& scale, const char* text);
    
};


namespace NFontAnim
{
    void bounce(float& x, float& y, const NFont::AnimParams& params, NFont::AnimData& data);
    void wave(float& x, float& y, const NFont::AnimParams& params, NFont::AnimData& data);
    void stretch(float& x, float& y, const NFont::AnimParams& params, NFont::AnimData& data);
    void shake(float& x, float& y, const NFont::AnimParams& params, NFont::AnimData& data);
    void circle(float& x, float& y, const NFont::AnimParams& params, NFont::AnimData& data);
}



#endif // _NFONT_H__
