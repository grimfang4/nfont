NFont v4.0.0: A font class for SDL
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
    
Optional (though recommended!):
    SDL_gpu ("SDL_gpu.h") [https://github.com/grimfang4/sdl-gpu] - Not required by NFontR, see below
    SDL_ttf ("SDL_ttf.h") [www.libsdl.org] - Can be disabled, see below

Notes:
    NFont is a font class with text-block alignment, full
    support for the newline character ('\n'), position animation,
    and UTF-8 support.
	
	There are two versions of NFont.  NFont uses SDL_gpu for rendering.  An
	alternative, NFontR, uses SDL 2.0's built-in SDL_Renderer API.  To use
	NFontR, use the NFont.h header in the NFontR directory.
    
    NFont natively loads SFont bitmaps and TrueType fonts with SDL_ttf.  The 
    standard bitmaps have the following characters (ASCII 33-126) separated by 
    pink (255, 0, 255) pixels in the topmost row:
    ! " # $ % & ' ( ) * + , - . / 0 1 2 3 4 5 6 7 8 9 : ; < = > ? @ A B C D E F G H I J K L M N O P Q R S T U V W X Y Z [ \ ] ^ _ ` a b c d e f g h i j k l m n o p q r s t u v w x y z { | } ~
    
    Define NFONT_NO_TTF before including NFont.h (or project-wide) to disable TrueType fonts.

    If you come up with something cool using NFont, I'd love to hear about it.
    Any comments can be sent to GrimFang4 [at] gmail [dot] com



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
