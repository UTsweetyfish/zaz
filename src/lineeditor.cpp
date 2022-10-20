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

#include "lineeditor.h"
#include <GL/gl.h>

using namespace Scenes;

LineEditor::LineEditor(std::string txt, double width, double y, uint maxLength)
        :txt(txt), pos(txt.size()), width(width), y(y), blinkOn(false), blinkCountdown(BLINK_RATE), shift(false),
        maxLength(maxLength)
{
    if (txt.size() > maxLength)
        txt.erase(maxLength);

    SDL_EnableUNICODE(SDL_ENABLE);
}

LineEditor::~LineEditor()
{
    SDL_EnableUNICODE(SDL_DISABLE);
}

void LineEditor::Render()
{
    glLoadIdentity();

    glPushMatrix();
    glTranslated((100 - width) / 2, y + 5, 5);
    glBegin(GL_QUADS);
    glColor3d( 0, 0, 1.0);
    glVertex3d(0, 0, 0);
    glVertex3d(0, -5, 0);
    glColor3d( .1, .1, .7);
    glVertex3d(width, -5, 0);
    glVertex3d(width, 0, 0);
    glEnd();
    glPopMatrix();

    glColor3d(1.0, 1.0, 1.0);
    FTBBox b = font->BBox(txt.c_str());
    double tw = b.Upper().X() / 6.67;
    glPushMatrix();
    glTranslated((100 - (tw + 0.2)) / 2, y + 1, 6);
    glScaled(0.15, 0.15, 0.15);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    font->Render(txt.c_str());
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glPopMatrix();
    glTranslated(((100 - (tw + 0.2)) / 2) + tw + 0.2, y + 1, 6);
    glLineWidth(2.0);
    if (blinkOn)
    {
        glBegin(GL_LINES);
        glVertex3d(0, 0, 0);
        glVertex3d(2, 0, 0);
        glEnd();
    }

}

uint LineEditor::utf8_strlen(const char *s)
{
    int i = 0;
    int j = 0;

    while (s[i])
    {
        if ((s[i] & 0xC0) != 0x80)
            j++;
        i++;
    }
    return (j);
}

void LineEditor::Logic(FrameEvents events)
{
    blinkCountdown--;

    if (blinkCountdown < 0)
    {
        blinkCountdown = BLINK_RATE;
        blinkOn = ! blinkOn;
    }

    if (!events.empty)
    {
        // suppress enters
        for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
        {
            if (*i == SDLK_RETURN || *i == SDLK_KP_ENTER)
                return;
        }


        // read shift state
        for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
        {
            if (*i == SDLK_RSHIFT || *i == SDLK_LSHIFT)
                shift = true;
        }

        for (vector<SDLKey>::iterator i = events.keyUp.begin(); i != events.keyUp.end(); ++i)
        {
            if (*i == SDLK_RSHIFT || *i == SDLK_LSHIFT)
                shift = false;
        }

        char c = 0;
        bool addchar = true;
        for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
        {
            if (*i == SDLK_BACKSPACE)
            {
                if (txt.size())
                {
                    if (txt[txt.size() -1] < 0)
                        txt = txt.substr(0, txt.size() - 1);


                    txt = txt.substr(0, txt.size() - 1);
                }
                addchar = false;
            }

            if (*i >= SDLK_a && *i <= SDLK_z) // add a char
            {
                c = (*i - SDLK_a) + (shift?'A':'a');
            }

            if (*i >= SDLK_0 && *i <= SDLK_9) // add a num
            {
                c = (*i - SDLK_0) + '0';
            }

            if (*i == SDLK_SPACE)
                c = ' ';
        }

        bool unicode_added = false;

        if (addchar)
            for (vector<Uint16>::iterator i = events.unicode.begin(); i != events.unicode.end(); ++i)
            {
                char uc[8];
                wchar_t wc[2];

                char illegal[] = "/\\:*?\"<>|"; // illegal windows characters for file names
#ifdef WIN32
                wc[0] = *i;
                wc[1] = 0;

                int rc = WideCharToMultiByte(CP_UTF8, 0, wc, -1, uc, 8, NULL, NULL);
#else
                int rc = wctomb(uc, wchar_t(*i));

                if (rc > 0)
                    uc[rc] = 0;

#endif

                if (rc > 0)
                {
                    bool legal = true;
                    int l = strlen(illegal);
                    for (int lc = 0; lc < l; lc++)
                        if (uc[0] == illegal[lc])
                            legal = false;

                    if (legal)
                    {
                        txt.append(uc);
                        unicode_added = true;
                    }
                }
            }

        if (!unicode_added && c != 0)
        {
            txt.append(&c, 1);
        }

        while (utf8_strlen(txt.c_str()) > maxLength)
        {
            if (txt[txt.size() -1] < 0)
                txt = txt.substr(0, txt.size() - 1);

            txt = txt.substr(0, txt.size() - 1);
        }

        pos = txt.size();
    }
}
