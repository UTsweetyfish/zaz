/*
 * Zaz
 * Copyright (C) Remigiusz Dybka 2009 <remigiusz.dybka@gmail.com>
 *
 Zaz is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Zaz is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common.h"
#include <GL/gl.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

// ugly hack for windows
#ifdef WIN32
#define GL_BGR GL_RGB
#define GL_BGRA GL_RGBA
#endif


int LoadTextureFile( const char *filename )
{
    GLuint texture;                     // This is a handle to our texture object
    SDL_Surface *surface;       // This surface will tell us the details of the image
    GLenum texture_format;
    GLint  nOfColors;

    surface = IMG_Load(filename);

    if (strlen(filename) == 0)
    {
        ERR(string("Empty filename"));
    }

    if (!surface)
    { // try jpg instead
        char phname[256];

        strcpy(phname, filename);
        while (phname[strlen(phname) - 1] != '.')
            phname[strlen(phname) - 1] = 0;

        strcat(phname, "jpg");

        surface = IMG_Load(phname);
    }

    if (surface)
    {

        // get the number of channels in the SDL surface
        nOfColors = surface->format->BytesPerPixel;
        if (nOfColors == 4)     // contains an alpha channel
        {
#ifdef  __amigaos4__
            texture_format = GL_RGBA;
#else
            if (surface->format->Rmask == 0x000000ff)
                texture_format = GL_RGBA;
            else
                texture_format = GL_BGRA;
#endif
        }
        else if (nOfColors == 3)     // no alpha channel
        {
#ifdef  __amigaos4__
            texture_format = GL_RGB;
#else
            if (surface->format->Rmask == 0x000000ff)
                texture_format = GL_RGB;
            else
                texture_format = GL_BGR;
#endif
        }
        else
        {
            printf("warning: the image is not truecolor..  this will probably break\n");
            // this error should not go unhandled
        }

        // Have OpenGL generate a texture object handle for us
        glGenTextures( 1, &texture );

        // Bind the texture object
        glBindTexture( GL_TEXTURE_2D, texture );

        // Set the texture's stretching properties
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

        // Edit the texture object's image data using the information SDL_Surface gives us
        glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
                      texture_format, GL_UNSIGNED_BYTE, surface->pixels );
    }
    else
    {
        ERR(string("Could not load image ") + filename);
    }

// Free the SDL_Surface only if it was successfully created
    if ( surface )
    {
        SDL_FreeSurface( surface );
    }

    return texture;
}

int LoadBallsTextureFile(const char *filename, unsigned char r, unsigned char g, unsigned char b, int cb)
{
    GLuint texture;                     // This is a handle to our texture object
    SDL_Surface *surface;       // This surface will tell us the details of the image
    GLenum texture_format;
    GLint  nOfColors;

    surface = IMG_Load(filename);

    if (surface)
    {

        // get the number of channels in the SDL surface
        nOfColors = surface->format->BytesPerPixel;
#ifdef  __amigaos4__
        texture_format = GL_RGBA;
#else
        if (surface->format->Rmask == 0x000000ff)
            texture_format = GL_RGBA;
        else
            texture_format = GL_BGRA;
#endif

        Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
#endif

        SDL_Surface *temp = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w, surface->h, 32, rmask, gmask, bmask, amask);

        // copy the pixels
        for (int y = 0; y < surface->h; y++)
            for (int x = 0; x < surface->w; x++)
            {
                unsigned char *pixsrc = ((unsigned char *)surface->pixels) + (surface->pitch * y) + (x * nOfColors);
                unsigned char *pixdest = ((unsigned char *)temp->pixels) + (temp->pitch * y) + (x * 4);
                unsigned char rr = pixsrc[0];
                unsigned char gg = pixsrc[1];
                unsigned char bb = pixsrc[2];

                rr = (unsigned char)((double)rr) * ((double)r / 255.0);
                gg = (unsigned char)((double)gg) * ((double)g / 255.0);
                bb = (unsigned char)((double)bb) * ((double)b / 255.0);

                pixdest[0] = rr;
                pixdest[1] = gg;
                pixdest[2] = bb;
                pixdest[3] = pixsrc[3];
            }

        SDL_Surface *ol = 0;

        if (cb)
            ol = IMG_Load(settings.getCFilename("cb.png"));

        // apply overlay image
        if (ol)
        {
            int olx = cb * 64;
            int oly = 0;

            for (int y = 0; y < 8; y++)
                for (int x = 0; x < 8; x++)
                    for (int yy = 0; yy < 64; yy++)
                        for (int xx = 0; xx < 64; xx++)
                        {
                            unsigned char *pixsrc = ((unsigned char *)ol->pixels) + (ol->pitch * yy) + ((xx + (olx)) * nOfColors);
                            unsigned char *pixdest = ((unsigned char *)temp->pixels) + (temp->pitch * (yy + (64 * y))) + ((xx + (64 * x)) * 4);

                            if (pixsrc[3] > 0)
                            {
                                pixdest[0] = 255 - pixdest[0];
                                pixdest[1] = 255 - pixdest[1];
                                pixdest[2] = 255 - pixdest[2];
                            }

                        }

            SDL_FreeSurface(ol);
        }

        // Have OpenGL generate a texture object handle for us
        glGenTextures( 1, &texture );

        // Bind the texture object
        glBindTexture( GL_TEXTURE_2D, texture );

        // Set the texture's stretching properties
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

        // Edit the texture object's image data using the information SDL_Surface gives us
        glTexImage2D( GL_TEXTURE_2D, 0, 4, temp->w, temp->h, 0,
                      texture_format, GL_UNSIGNED_BYTE, temp->pixels );

        SDL_FreeSurface(temp);
    }
    else
    {
        ERR(string("Could not load image ") + filename);
    }

    if ( surface )
    {
        SDL_FreeSurface( surface );
    }

    return texture;
}

int LoadTexture(const char *filename)
{
    return LoadTextureFile(settings.getCFilename(filename));
};

int LoadBallsTexture(const char *filename, unsigned char r, unsigned char g, unsigned char b, int cb)
{
    return LoadBallsTextureFile(settings.getCFilename(filename), r, g, b, cb);
};