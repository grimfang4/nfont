/*
NFont: A bitmap font class for SDL
by Jonathan Dearborn 8-6-09
Based on SFont (class adapted from Florian Hufsky, jnrdev)
*/
#include "NFont.h"

#ifdef NF_USE_OPENGL
///This function gets the first power of 2 >= the
///int that we pass it.
inline int next_p2 ( int a )
{
	int rval=1;
	while(rval<a) rval<<=1;
	return rval;
}

///Create a display list coresponding to the give character.
void make_dlist ( FT_Face face, char ch, GLuint list_base, GLuint * tex_base ) {

	//The first thing we do is get FreeType to render our character
	//into a bitmap.  This actually requires a couple of FreeType commands:

	//Load the Glyph for our character.
	if(FT_Load_Glyph( face, FT_Get_Char_Index( face, ch ), FT_LOAD_DEFAULT ))
		throw std::runtime_error("FT_Load_Glyph failed");

	//Move the face's glyph into a Glyph object.
    FT_Glyph glyph;
    if(FT_Get_Glyph( face->glyph, &glyph ))
		throw std::runtime_error("FT_Get_Glyph failed");

	//Convert the glyph to a bitmap.
	FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

	//This reference will make accessing the bitmap easier
	FT_Bitmap& bitmap=bitmap_glyph->bitmap;

	//Use our helper function to get the widths of
	//the bitmap data that we will need in order to create
	//our texture.
	int width = next_p2( bitmap.width );
	int height = next_p2( bitmap.rows );

	//Allocate memory for the texture data.
	GLubyte* expanded_data = new GLubyte[ 2 * width * height];

	//Here we fill in the data for the expanded bitmap.
	//Notice that we are using two channel bitmap (one for
	//luminocity and one for alpha), but we assign
	//both luminocity and alpha to the value that we
	//find in the FreeType bitmap. 
	//We use the ?: operator so that value which we use
	//will be 0 if we are in the padding zone, and whatever
	//is the the Freetype bitmap otherwise.
	for(int j=0; j <height;j++) {
		for(int i=0; i < width; i++){
			expanded_data[2*(i+j*width)]= expanded_data[2*(i+j*width)+1] = 
				(i>=bitmap.width || j>=bitmap.rows) ?
				0 : bitmap.buffer[i + bitmap.width*j];
		}
	}


	//Now we just setup some texture paramaters.
    glBindTexture( GL_TEXTURE_2D, tex_base[ch]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	//Here we actually create the texture itself, notice
	//that we are using GL_LUMINANCE_ALPHA to indicate that
	//we are using 2 channel data.
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height,
		  0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );

	//With the texture created, we don't need to expanded data anymore
    delete [] expanded_data;

	//So now we can create the display list
	glNewList(list_base+ch,GL_COMPILE);

	glBindTexture(GL_TEXTURE_2D,tex_base[ch]);

	//first we need to move over a little so that
	//the character has the right amount of space
	//between it and the one before it.
	glTranslatef(bitmap_glyph->left,0,0);

	//Now we move down a little in the case that the
	//bitmap extends past the bottom of the line 
	//(this is only true for characters like 'g' or 'y'.
	glPushMatrix();
	glTranslatef(0,bitmap_glyph->top-bitmap.rows,0);

	//Now we need to account for the fact that many of
	//our textures are filled with empty padding space.
	//We figure what portion of the texture is used by 
	//the actual character and store that information in 
	//the x and y variables, then when we draw the
	//quad, we will only reference the parts of the texture
	//that we contain the character itself.
	float	x=(float)bitmap.width / (float)width,
			y=(float)bitmap.rows / (float)height;

	//Here we draw the texturemaped quads.
	//The bitmap that we got from FreeType was not 
	//oriented quite like we would like it to be,
	//so we need to link the texture to the quad
	//so that the result will be properly aligned.
	glBegin(GL_QUADS);
	glTexCoord2d(0,0); glVertex2f(0,bitmap.rows);
	glTexCoord2d(0,y); glVertex2f(0,0);
	glTexCoord2d(x,y); glVertex2f(bitmap.width,0);
	glTexCoord2d(x,0); glVertex2f(bitmap.width,bitmap.rows);
	glEnd();
	glPopMatrix();
	glTranslatef(face->glyph->advance.x >> 6 ,0,0);


	//increment the raster position as if we were a bitmap font.
	//(only needed if you want to calculate text length)
	//glBitmap(0,0,0,0,face->glyph->advance.x >> 6,0,NULL);

	//Finnish the display list
	glEndList();
}



void NFont::loadTex(const char * fname, unsigned int h) {
	//Allocate some memory to store the texture ids.
	textures = new GLuint[128];

	this->h=h;

	//Create and initilize a freetype font library.
	FT_Library library;
	if (FT_Init_FreeType( &library )) 
		throw std::runtime_error("FT_Init_FreeType failed");

	//The object in which Freetype holds information on a given
	//font is called a "face".
	FT_Face face;

	//This is where we load in the font information from the file.
	//Of all the places where the code might die, this is the most likely,
	//as FT_New_Face will die if the font file does not exist or is somehow broken.
	if (FT_New_Face( library, fname, 0, &face )) 
		throw std::runtime_error("FT_New_Face failed (there is probably a problem with your font file)");

	//For some twisted reason, Freetype measures font size
	//in terms of 1/64ths of pixels.  Thus, to make a font
	//h pixels high, we need to request a size of h*64.
	//(h << 6 is just a prettier way of writting h*64)
	FT_Set_Char_Size( face, h << 6, h << 6, 96, 96);

	//Here we ask opengl to allocate resources for
	//all the textures and displays lists which we
	//are about to create.  
	list_base=glGenLists(128);
	glGenTextures( 128, textures );

	//This is where we actually create each of the fonts display lists.
	for(unsigned char i=0;i<128;i++)
		make_dlist(face,i,list_base,textures);

	//We don't need the face information now that the display
	//lists have been created, so we free the assosiated resources.
	FT_Done_Face(face);

	//Ditto for the library.
	FT_Done_FreeType(library);
}


NFont::~NFont()
{
	delete[] buffer;
	if(cleanUp)
	{
		SDL_FreeSurface(src);
		
		#ifdef NF_USE_OPENGL
		glDeleteLists(list_base,128);
		glDeleteTextures(128,textures);
		delete [] textures;
		#endif
	}
}

/// A fairly straight forward function that pushes
/// a projection matrix that will make object world 
/// coordinates identical to window coordinates.
inline void pushScreenCoordinateMatrix() {
	glPushAttrib(GL_TRANSFORM_BIT);
	GLint	viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]);
	glPopAttrib();
}

/// Pops the projection matrix without changing the current
/// MatrixMode.
inline void pop_projection_matrix() {
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}

///Much like Nehe's glPrint function, but modified to work
///with freetype fonts.
void NFont::drawf(float x, float y, const char *fmt, ...)  {
	using namespace std;
	// We want a coordinate system where things coresponding to window pixels.
	pushScreenCoordinateMatrix();					
	
	GLuint font=this->list_base;
	float h=this->h/.63f;						//We make the height about 1.5* that of
	
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		*text=0;											// Do Nothing

	else {
	va_start(ap, fmt);									// Parses The String For Variables
	    vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text
	}


	//Here is some code to split the text that we have been
	//given into a set of lines.
	//This could be made much neater by using
	//a regular expression library such as the one avliable from
	//boost.org (I've only done it out by hand to avoid complicating
	//this tutorial with unnecessary library dependencies).
	const char *start_line=text;
	vector<string> lines;

	const char * c = text;;

	//for(const char *c=text;*c;c++) {
	for(;*c;c++) {
		if(*c=='\n') {
			string line;
			for(const char *n=start_line;n<c;n++) line.append(1,*n);
			lines.push_back(line);
			start_line=c+1;
		}
	}
	if(start_line) {
		string line;
		for(const char *n=start_line;n<c;n++) line.append(1,*n);
		lines.push_back(line);
	}

	glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);	
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	glListBase(font);

	float modelview_matrix[16];	
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);

	//This is where the text display actually happens.
	//For each line of text we reset the modelview matrix
	//so that the line's text will start in the correct position.
	//Notice that we need to reset the matrix, rather than just translating
	//down by h. This is because when each character is
	//draw it modifies the current matrix so that the next character
	//will be drawn immediatly after it.  
	for(unsigned int i=0;i<lines.size();i++) {
		

		glPushMatrix();
		glLoadIdentity();
		glTranslatef(x,y-h*i,0);
		glMultMatrixf(modelview_matrix);

	//  The commented out raster position stuff can be useful if you need to
	//  know the length of the text that you are creating.
	//  If you decide to use it make sure to also uncomment the glBitmap command
	//  in make_dlist().
	//	glRasterPos2f(0,0);
		glCallLists(lines[i].length(), GL_UNSIGNED_BYTE, lines[i].c_str());
	//	float rpos[4];
	//	glGetFloatv(GL_CURRENT_RASTER_POSITION ,rpos);
	//	float len=x-rpos[0];

		glPopMatrix();

		

	}


	glPopAttrib();		

	pop_projection_matrix();
}

#endif

void NF_GetVersion(char* result)
{
    if(result != NULL)
        sprintf(result, "%d.%d.%d", NF_VERSION_MAJOR, NF_VERSION_MINOR, NF_VERSION_BUGFIX);
}


SDL_Surface* NF_NewColorSurface(SDL_Surface* font_surface, Uint32 top, Uint32 bottom, int heightAdjust)
{
    Uint8 tr, tg, tb;

    SDL_GetRGB(top, font_surface->format, &tr, &tg, &tb);

    Uint8 br, bg, bb;

    SDL_GetRGB(bottom, font_surface->format, &br, &bg, &bb);

    SDL_Surface* result = SDL_ConvertSurface(font_surface, font_surface->format, font_surface->flags);

    bool useCK = (result->flags & SDL_SRCALPHA) != SDL_SRCALPHA;  // colorkey if no alpha
    Uint32 colorkey = result->format->colorkey;

    Uint8 r, g, b, a;
    float ratio;
    Uint32 color;
    int temp;

    for (int x = 0, y = 0; y < result->h; x++)
    {
        if (x >= result->w)
        {
            x = 0;
            y++;

            if (y >= result->h)
                break;
        }

        ratio = (y - 2)/float(result->h - heightAdjust);  // the neg 3s are for full color at top and bottom

        if(!useCK)
        {
            color = NF_GetPixel(result, x, y);
            SDL_GetRGBA(color, result->format, &r, &g, &b, &a);  // just getting alpha
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


Uint32 NF_GetPixel(SDL_Surface *Surface, int x, int y)
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

    return 0;  // Best I could do for errors
}







NFont::NFont()
    : src(NULL)
    , dest(NULL)
    , cleanUp(0)
    , height(0)
    , maxWidth(0)
    , baseline(0)
    , ascent(0)
    , descent(0)
    , lineSpacing(0)
    , letterSpacing(0)
    , maxPos(0)
    , buffer(new char[1024])
    , buffSize(1024)
{
    
}


NFont::NFont(SDL_Surface* Dest, SDL_Surface* FontSurface, bool CleanUp)
    : src(NULL)
    , dest(NULL)
    , cleanUp(0)
    , height(0)
    , maxWidth(0)
    , baseline(0)
    , ascent(0)
    , descent(0)
    , lineSpacing(0)
    , letterSpacing(0)
    , maxPos(0)
    , buffer(new char[1024])
    , buffSize(1024)
{
    resetFont(Dest, FontSurface, CleanUp);
}



#ifdef NF_USE_TTF
void NFont::loadTTF(TTF_Font* ttf, SDL_Color fg, SDL_Color bg)
{
    if(ttf == NULL)
        return;
    SDL_Surface* surfs[127 - 33];
    int width = 0;
    int height = 0;
    
    char buff[2];
    buff[1] = '\0';
    for(int i = 0; i < 127 - 33; i++)
    {
        buff[0] = i + 33;
        surfs[i] = TTF_RenderText_Shaded(ttf, buff, fg, bg);
        width += surfs[i]->w;
        height = (height < surfs[i]->h)? surfs[i]->h : height;
    }
    
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        SDL_Surface* result = SDL_CreateRGBSurface(SDL_SWSURFACE,width + 127 - 33 + 1,height,24, 0xFF0000, 0x00FF00, 0x0000FF, 0);
    #else
        SDL_Surface* result = SDL_CreateRGBSurface(SDL_SWSURFACE,width + 127 - 33 + 1,height,24, 0x0000FF, 0x00FF00, 0xFF0000, 0);
    #endif
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
    
    setFont(result, true);
}


void NFont::loadTTF(TTF_Font* ttf, SDL_Color fg)
{
    if(ttf == NULL)
        return;
    SDL_Surface* surfs[127 - 33];
    int width = 0;
    int height = 0;
    
    char buff[2];
    buff[1] = '\0';
    for(int i = 0; i < 127 - 33; i++)
    {
        buff[0] = i + 33;
        surfs[i] = TTF_RenderText_Blended(ttf, buff, fg);
        width += surfs[i]->w;
        height = (height < surfs[i]->h)? surfs[i]->h : height;
    }
    
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        SDL_Surface* result = SDL_CreateRGBSurface(SDL_SWSURFACE,width + 127 - 33 + 1,height,32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    #else
        SDL_Surface* result = SDL_CreateRGBSurface(SDL_SWSURFACE,width + 127 - 33 + 1,height,32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    #endif
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
    
    setFont(result, true);
}
#endif

void NFont::drawToSurface(int x, int y, const char* text)
{
    SDL_Rect srcRect, dstRect, copyS, copyD;
    
    if(text == NULL)
        return;
    
    unsigned char c = (unsigned char)(*text);

    #ifdef NF_USE_OPENGL
    
    /*if(usingTex)
    {
        height = 50;
        float dh;
        float sy = 0;//baseline - ascent;
        float sw = 0;
        float sh = dh = height;
        float dx = x;
        float dy = y;
        float newlineX = x;
    
        while(c != '\0')
        {
            if(c == '\n')
            {
                dx = newlineX;
                dy += height + lineSpacing;
                
                text++;
                c = (unsigned char)(*text);
                continue;
            }
            
            if(c == ' ')
            {
                dx += charPos[0] + letterSpacing;
                
                text++;
                c = (unsigned char)(*text);
                continue;
            }
            // Skip bad characters
            if(c < 33 || (c > 126 && c < 161))
            {
                text++;
                c = (unsigned char)(*text);
                continue;
            }
            
            c -= 33;  // Get array index
            if(c > 126) // shift the extended characters down to the correct index
                c -= 34;
            sx = charPosf[c];
            sw = dw = charWidthf[c];
            
            // Draw
            
            glBegin(GL_QUADS);
            
            glTexCoord2f(sx, 0);
            glVertex3f(dx, dy, 0);
            
            glTexCoord2f(sx+sw, 0);
            glVertex3f(dx+dw, dy, 0);
            
            glTexCoord2f(sx+sw, sh);
            glVertex3f(dx+dw, dy, 0);
            
            glTexCoord2f(sx, sh);
            glVertex3f(dx, dy+dh, 0);
            
            glEnd();
            
            dx += dw + letterSpacing;
            
            text++;
            c = (unsigned char)(*text);
        }
        return;
    }*/
    
    #endif
    
    if(src == NULL || dest == NULL)
        return;
    
    srcRect.y = baseline - ascent;
    srcRect.h = dstRect.h = height;
    dstRect.x = x;
    dstRect.y = y;
    
    int newlineX = x;
    
    while(c != '\0')
    {
        if(c == '\n')
        {
            dstRect.x = newlineX;
            dstRect.y += height + lineSpacing;
            
            text++;
            c = (unsigned char)(*text);
            continue;
        }
        
        if(c == ' ')
        {
            dstRect.x += charWidth[0] + letterSpacing;
            
            text++;
            c = (unsigned char)(*text);
            continue;
        }
        // Skip bad characters
        if(c < 33 || (c > 126 && c < 161)
           || dstRect.x >= dest->w
           || dstRect.y >= dest->h)
        {
            text++;
            c = (unsigned char)(*text);
            continue;
        }
        
        c -= 33;  // Get array index
        if(c > 126) // shift the extended characters down to the correct index
            c -= 34;
        srcRect.x = charPos[c];
        srcRect.w = dstRect.w = charWidth[c];
        
        // Protect the rects from the destructive SDL_BlitSurface()...
        copyS = srcRect;
        copyD = dstRect;
        SDL_BlitSurface(src, &srcRect, dest, &dstRect);
        srcRect = copyS;
        dstRect = copyD;
        
        dstRect.x += dstRect.w + letterSpacing;
        
        text++;
        c = (unsigned char)(*text);
    }
    
}

char* NFont::copyString(const char* c)
{
    if(c == NULL) return NULL;

    int count = 0;
    for(; c[count] != '\0'; count++);

    char* result = new char[count+1];

    for(int i = 0; i < count; i++)
    {
        result[i] = c[i];
    }

    result[count] = '\0';
    return result;
}

void NFont::draw(int x, int y, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    drawToSurface(x, y, buffer);
}

void NFont::drawCenter(int x, int y, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    char* str = copyString(buffer);
    char* del = str;

    // Go through str, when you find a \n, replace it with \0 and print it
    // then move down, back, and continue.
    for(char* c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            drawToSurface(x - getWidth("%s", str)/2, y, str);
            *c = '\n';
            c++;
            str = c;
            y += height;
        }
        else
            c++;
    }
    drawToSurface(x - getWidth("%s", str)/2, y, str);

    delete[] del;
}

void NFont::drawRight(int x, int y, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    char* str = copyString(buffer);
    char* del = str;

    for(char* c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            drawToSurface(x - getWidth("%s", str), y, str);
            *c = '\n';
            c++;
            str = c;
            y += height;
        }
        else
            c++;
    }
    drawToSurface(x - getWidth("%s", str), y, str);

    delete[] del;
}

int NFont::getHeight(const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return height;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    int numLines = 1;
    const char* c;

    for (c = buffer; *c != '\0'; c++)
    {
        if(*c == '\n')
            numLines++;
    }

    //   Actual height of letter region + line spacing
    return height*numLines + lineSpacing*(numLines - 1);  //height*numLines;
}



int NFont::getWidth(const char* formatted_text, ...)
{
    if (formatted_text == NULL)
        return 0;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    const char* c;
    int charnum = 0;
    int width = 0;
    int bigWidth = 0;  // Allows for multi-line strings

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

bool NFont::setFont(SDL_Surface* FontSurface, bool CleanUp)
{
    if(cleanUp)
        SDL_FreeSurface(src);
    src = FontSurface;
    if (src == NULL)
    {
        printf("\n ERROR: NFont given a NULL surface\n");
        cleanUp = 0;
        return 0;
    }

    cleanUp = CleanUp;

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
        if(NF_GetPixel(src, x, 0) != pixel)
        {
            charPos[i] = x;
            charWidth[i] = x;
            while(x < src->w && NF_GetPixel(src, x, 0) != pixel)
                x++;
            charWidth[i] = x - charWidth[i];
            if(charWidth[i] > maxWidth)
                maxWidth = charWidth[i];
            i++;
        }

        x++;
    }

    maxPos = x - 1;


    pixel = NF_GetPixel(src, 0, src->h - 1);
    int j;
    setBaseline();
    
    // Get the max ascent
    j = 1;
    while(j < baseline && j < src->h)
    {
        x = 0;
        while(x < src->w)
        {
            if(NF_GetPixel(src, x, j) != pixel)
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
            if(NF_GetPixel(src, x, j) != pixel)
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
    

    if((src->flags & SDL_SRCALPHA) != SDL_SRCALPHA)
    {
        pixel = NF_GetPixel(src, 0, src->h - 1);
        SDL_UnlockSurface(src);
        SDL_SetColorKey(src, SDL_SRCCOLORKEY, pixel);
    }
    else
        SDL_UnlockSurface(src);

    return 1;
}



int NFont::setBaseline(int Baseline)
{
    if(Baseline >= 0)
        baseline = Baseline;
    else
    {
        // Get the baseline by checking a, b, and c and averaging their lowest y-value.
        // Is there a better way?
        Uint32 pixel = NF_GetPixel(src, 0, src->h - 1);
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
                    if(NF_GetPixel(src, i, j) != pixel)
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
    return baseline;
}

int NFont::getAscent(const char character)
{
    unsigned char test = (unsigned char)character;
    if(test < 33 || test > 222 || (test > 126 && test < 161))
        return 0;
    unsigned char num = (unsigned char)character - 33;
    // Get the max ascent
    int x = charPos[num];
    int i, j = 1, result = 0;
    Uint32 pixel = NF_GetPixel(src, 0, src->h - 1); // bg pixel
    while(j < baseline && j < src->h)
    {
        i = charPos[num];
        while(i < x + charWidth[num])
        {
            if(NF_GetPixel(src, i, j) != pixel)
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
    unsigned char test = (unsigned char)character;
    if(test < 33 || test > 222 || (test > 126 && test < 161))
        return 0;
    unsigned char num = (unsigned char)character - 33;
    // Get the max descent
    int x = charPos[num];
    int i, j = src->h - 1, result = 0;
    Uint32 pixel = NF_GetPixel(src, 0, src->h - 1); // bg pixel
    while(j > 0 && j > baseline)
    {
        i = charPos[num];
        while(i < x + charWidth[num])
        {
            if(NF_GetPixel(src, i, j) != pixel)
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




#ifdef USE_NFONTANIM

NFontAnim::NFontAnim()
{
    data.font = this;
    data.dest = NULL;
    data.maxX = 0;
    data.src = NULL;
    data.height = 0;
    cleanUp = 0;
    data.text = NULL;

    src = NULL;
    dest = NULL;
    maxPos = 0;
    height = 0;
    buffer = new char[1024];
}

void NFontAnim::drawToSurfacePos(int x, int y, NFontAnim_Fn posFn)
{
    data.dest = dest;
    data.src = src;
    data.text = buffer;  // Buffer for efficient drawing
    data.height = height;
    data.charPos = charPos;
    data.charWidth = charWidth;
    data.maxX = maxPos;


    data.index = -1;
    data.letterNum = 0;
    data.wordNum = 1;
    data.lineNum = 1;
    data.startX = x;  // used as reset value for line feed
    data.startY = y;
    
    int preFnX = x;
    int preFnY = y;
    
    const char* c = buffer;
    unsigned char num;
    SDL_Rect srcRect, dstRect, copyS, copyD;
    
    if(c == NULL || src == NULL || dest == NULL)
        return;
    
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
        srcRect = copyS;
        dstRect = copyD;
        
        x = preFnX;  // Restore real position
        y = preFnY;
        
        x += dstRect.w + letterSpacing;
    }
    
}

void NFontAnim::drawPos(int x, int y, NFontAnim_Fn posFn, const char* text, ...)
{
    va_list lst;
    va_start(lst, text);
    vsprintf(buffer, text, lst);
    va_end(lst);

    data.userVar = NULL;
    drawToSurfacePos(x, y, posFn);
}

void NFontAnim::drawPosX(int x, int y, NFontAnim_Fn posFn, void* userVar, const char* text, ...)
{
    va_list lst;
    va_start(lst, text);
    vsprintf(buffer, text, lst);
    va_end(lst);

    data.userVar = userVar;
    drawToSurfacePos(x, y, posFn);
}

void NFontAnim::drawAll(int x, int y, NFontAnim_Fn allFn, const char* text, ...)
{
    va_list lst;
    va_start(lst, text);
    vsprintf(buffer, text, lst);
    va_end(lst);

    data.userVar = NULL;
    allFn(x, y, data);
}

void NFontAnim::drawAllX(int x, int y, NFontAnim_Fn allFn, void* userVar, const char* text, ...)
{
    va_list lst;
    va_start(lst, text);
    vsprintf(buffer, text, lst);
    va_end(lst);

    data.userVar = userVar;
    allFn(x, y, data);
}

#endif


