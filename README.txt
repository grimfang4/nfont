NFont: A font class for SDL
by Jonathan Dearborn
[https://github.com/grimfang4/nfont]
Dedicated to the memory of Florian Hufsky


NFont is a C++ library that makes it easy to add bitmap or TrueType fonts to your programs.  Using SDL_gpu or SDL_Renderer, NFont can load and display bitmap text.  Using SDL_ttf, you can load TrueType font files.

Cool features:
 * Multiline (\n) rendering
 * Text alignment (left, center, right)
 * Boxed drawing
 * Text position animations
 * Unicode (UTF-8) support
 * Coloring and scaling
 * Permissive license



Requires:
    SDL ("SDL.h") [www.libsdl.org]
    SDL_ttf ("SDL_ttf.h") [www.libsdl.org]
    
Optional:
    SDL_gpu ("SDL_gpu.h") [https://github.com/grimfang4/sdl-gpu]

Notes:
    NFont is a font class with text-block alignment, full
    support for the newline character ('\n'), position animation,
    and UTF-8 support.
	
	There are two versions of NFont.  NFont uses SDL 2.0's built-in SDL_Renderer API by default.  If you want to use the SDL_gpu rendering path, then #define FC_USE_SDL_GPU project-wide.  This will affect both SDL_FontCache (the core of NFont's caching system) and NFont itself.
    
    NFont natively loads and caches TrueType fonts with SDL_ttf via SDL_FontCache.  If you use SDL_Renderer, SDL version 2.0.4 is the first version to fully support clipping (e.g. for NFont::drawBox()).

    If you come up with something cool using NFont, I'd love to hear about it.
    Any comments can be sent to GrimFang4 [at] gmail [dot] com



License:
	NFont is licensed to you under the terms of the liberal MIT license.  See NFont.h for further information.
