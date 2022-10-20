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

#include "game.h"
#include "splash.h"

Splash::Splash(SDL_Surface *surf, uint fps)
        : Scene(surf, fps), showOverlay(false), overX(0.0), overY(0.0)
{
    music = NULL;
    logoTex = LoadTexture("phbx_logo.png");
    logoOverlayTex = LoadTexture("phbx_logo_overlay.png");
}

Splash::~Splash()
{
    glDeleteTextures(1, &logoTex);
    glDeleteTextures(1, &logoOverlayTex);
};

void Splash::Logic(ulong frame)
{
    if (music == NULL)
    {
        music = (Scenes::Sample *)new Scenes::StreamingOggSample("phbx_snd.ogg");
        mixer->EnqueueSample(music, atoi(settings.get("musicVolume", "50").c_str()), 0, true);
    }

    if (!events.empty)
    {
        if (events.keyDown.size() > 0)
            for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
            {
                if (*i == SDLK_ESCAPE)
                {
                    quit = true;
                }
            }
    }

    if (frame%2 == 0)
    {
        int r = rand()%10;
        showOverlay = r > 5;

        overX = double(rand()%(overlayShift * 100))/100.0;
        overY = double(rand()%(overlayShift * 100))/100.0;

        overX = overX - (overlayShift / 2.0);
        overY = overY - (overlayShift / 2.0);

        overR = double(rand()%(overlayRot * 100))/100.0;
        overR = overR - (overlayRot / 2.0);
    }

    if (frame > showFrames)
        quit = true;

    if (quit)
    {
        mixer->DisposeSample(music);
    }
}

void Splash::Render(ulong frame)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );

    glPushMatrix();

    XY tl(0,0);
    XY tr(1,0);
    XY bl(0,1);
    XY br(1,1);

    glTranslatef(0.0, 0.0, 1.5);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, logoTex);

    double alpha = 1.0;

    if (frame < fadeInFrames)
    {
        alpha = (double)frame / (double) fadeInFrames;
    }

    if (frame > showFrames - fadeOutFrames)
    {
        ulong a = frame - (showFrames - fadeOutFrames);

        alpha = 1.0 - ((double)a / (double) fadeOutFrames);
    }

    glColor4d(1.0, 1.0, 1.0, alpha);
    glBegin(GL_QUADS);

    glTexCoord2d(tl.x, tl.y);
    glVertex3d(vleft, 100, 0);
    glTexCoord2d(bl.x, bl.y);
    glVertex3d(vleft, 0, 0);
    glTexCoord2d(br.x, br.y);
    glVertex3d(vleft + vwidth, 0, 0);
    glTexCoord2d(tr.x, tr.y);
    glVertex3d(vleft + vwidth, 100, 0);
    glEnd();
    glPopMatrix();

    if (showOverlay)
    {
        glBindTexture(GL_TEXTURE_2D, logoOverlayTex);
        glPushMatrix();
        glTranslatef(overX, overY, 2.0);
        glRotated(overR, 0.0, 0.0, 1.0);
        glColor4d(1.0, 1.0, 1.0, alpha);

        glBegin(GL_QUADS);

        glTexCoord2d(tl.x, tl.y);
        glVertex3d(vleft, 100, 0);
        glTexCoord2d(bl.x, bl.y);
        glVertex3d(vleft, 0, 0);
        glTexCoord2d(br.x, br.y);
        glVertex3d(vleft + vwidth, 0, 0);
        glTexCoord2d(tr.x, tr.y);
        glVertex3d(vleft + vwidth, 100, 0);
        glEnd();
        glPopMatrix();
    }

    glDisable(GL_TEXTURE_2D);
}

void Splash::GLSetup()
{
    int width = surface->w;
    int height = surface->h;
    SDL_ShowCursor(0);
    /* Our shading model--Gouraud (smooth). */
    glShadeModel( GL_SMOOTH );

    /* Culling. */
    glCullFace( GL_BACK );
    glFrontFace( GL_CCW );
    glEnable( GL_CULL_FACE );
    glEnable(GL_DEPTH_TEST);
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc(GL_GREATER, 0.0);
    glEnable(GL_LINE_SMOOTH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    glClearColor( .0f, .0f, .0f, .0f );
    glViewport( 0, 0, width, height);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    vwidth = 100 * (640.0/480.0);
    vleft = (100 - vwidth) / 2;
    vheight = 100.0;

    glOrtho(vleft, vwidth + vleft, 0, 100, -100, 100);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
