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

#include <sstream>
#include <ctime>
#include "editor.h"
#include "textureloader.h"

Editor::Editor(SDL_Surface *surf, Level &level, uint fps)
        : Scene(surf, fps), level(level), mx(0), my(0), msgTime(0),
        selectedPoint(-1), hoverPoint(-1), selPointIsControl(false), hoverPointIsControl(false),
        hoverPointIsInvertedControl(false), grab(false), snap(false), showHelp(false), showOverlay(true),
        backgroundTex(0), overlayTex(0)
{
    // if level empty - fill in some default data
    if (level.paths.size() == 0)
    {
        New();
    }

    currentPath = &level.paths[0];
    backgroundTex = LoadTextureFile(level.backgroundTexFilename.c_str());
    if (!level.overlayTexFilename.empty())
        overlayTex = LoadTextureFile(level.overlayTexFilename.c_str());
}

void Editor::New()
{
    level.paths.clear();

    level.paths.push_back(Bezier());
    level.paths.push_back(Bezier());
    level.paths.push_back(Bezier());

    level.paths[0].points.push_back(BezierPoint(45, 50, 45, 56));
    level.paths[0].points.push_back(BezierPoint(55, 50, 55, 56));

    level.paths[1].points.push_back(BezierPoint(10, 50, 10, 56));
    level.paths[1].points.push_back(BezierPoint(20, 50, 20, 56));

    level.paths[2].points.push_back(BezierPoint(80, 50, 80, 56));
    level.paths[2].points.push_back(BezierPoint(90, 50, 90, 56));

    currentPath = &level.paths[0];
}

Editor::~Editor()
{
    if (backgroundTex)
    {
        glDeleteTextures(1, &backgroundTex);
    }

    if (overlayTex)
    {
        glDeleteTextures(1, &overlayTex);
    }
}

void Editor::Render(ulong frame)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0, 1.0, 1.0);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );

    // draw background
    // render level background
    glPushMatrix();
    glTranslatef(0.0, 0.0, -5);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, backgroundTex);
    glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex3d(vleft, 100, 0);
    glTexCoord2d(0, 1);
    glVertex3d(vleft, 0, 0);
    glTexCoord2d(1, 1);

    glVertex3d(vleft + vwidth, 0, 0);
    glTexCoord2d(1, 0);
    glVertex3d(vleft + vwidth, 100, 0);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    glLoadIdentity( );
    /*
    // draw pointer
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glTranslatef(mx, my, 1);
    glBindTexture(GL_TEXTURE_2D, pointerTexture);
    glScalef(5, 5, 5);
    glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex3d(0, 0, 0);
    glTexCoord2d(0, 1);
    glVertex3d(0, -1, 0);
    glTexCoord2d(1, 1);
    glVertex3d(1, -1, 0);
    glTexCoord2d(1, 0);
    glVertex3d(1, 0, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    */
    // draw paths
    for (uint p = 0; p < level.paths.size(); p++)
    {
        glLineWidth(1);
        glPointSize(5.0);
        glBegin(GL_POINTS);
        vector<BezierPoint>::iterator iter;
        int f = 0;
        for (iter = level.paths[p].points.begin(); iter != level.paths[p].points.end(); ++iter, ++f)
        {
            glColor3f(1.0, 1.0, 1.0);

            if (iter->under)
                glColor3f(1.0, 1.0, 0.0);

            if (iter->hidden)
                glColor3f(0.0, 0.0, 1.0);

            if ((f == selectedPoint) && (currentPath == &level.paths[p]))
            {
                glColor3f(1.0, 0, 0);
            }
            else if ((f == hoverPoint) && (currentPath == &level.paths[p]))
            {
                glColor3f(0.0, 1.0, 0.0);
            };

            glVertex3d(iter->x, iter->y, 0.0);
        }
        glEnd();

        // draw control points / lines
        glPointSize(2.5);
        f = 0;
        if (currentPath == &level.paths[p])
        {
            for (iter = level.paths[p].points.begin(); iter != level.paths[p].points.end(); ++iter, ++f)
            {
                glColor3f(1.0, 1.0, 1.0);

                if ((f == (int)level.paths[p].points.size() - 1)  && (currentPath == &level.paths[p]))
                    glColor3f(1.0, 0.0, 0.0);

                glBegin(GL_LINES);
                glVertex3d(iter->x, iter->y, 0.0);
                glVertex3d(iter->cx, iter->cy, 0.0);

                glVertex3d(iter->x, iter->y, 0.0);
                glVertex3d(iter->x - (iter->cx - iter->x), iter->y - (iter->cy - iter->y), 0.0);
                glEnd();

                glBegin(GL_POINTS);
                glVertex3d(iter->cx, iter->cy, 0.0);
                glVertex3d(iter->x - (iter->cx - iter->x), iter->y - (iter->cy - iter->y), 0.0);
                glEnd();
            }
        }
        // draw bezier
        glLineWidth(5);
        glPointSize(3.0);

        glColor3d(0.0, 0.0, 0.0);
        glBegin(GL_POINTS);

        vector<XY>::iterator i;
        vector<XY> path;

        if ((p > 0) || (!level.loop))
        { // for ball paths
            path = level.paths[p].GenerateUniform(0.5);
        }
        else   // looped player path
        {
            Bezier tmpb = level.paths[p];
            BezierPoint p0 = tmpb.points[0];

            p0.cx = p0.x + (p0.x - p0.cx);
            p0.cy = p0.y + (p0.y - p0.cy);

            tmpb.points.push_back(p0);

            path = tmpb.GenerateUniform(0.5);
        }

        for (i = path.begin(); i != path.end(); ++i)
        {
            glVertex3d(i->x, i->y, 0.0);
        }
        glEnd();

        if (currentPath == &level.paths[p])
        {
            glPointSize(5.0);
            glColor3d(1.0, 0.0, 1.0);
            glBegin(GL_POINTS);
            path = level.paths[p].Generate();

            for (i = path.begin(); i != path.end(); ++i)
            {
                glVertex3d(i->x, i->y, 0.0);
            }
            glEnd();
        }
    }

    // render overlay
    if (showOverlay)
        if (overlayTex)
        {
            glPushMatrix();
            glTranslatef(0.0, 0.0, 1);
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glBindTexture(GL_TEXTURE_2D, overlayTex);
            glColor4f(1.0, 1.0, 1.0, 1.0);
            glBegin(GL_QUADS);
            glTexCoord2d(0, 0);
            glVertex3d(vleft, 100, 0);
            glTexCoord2d(0, 1);
            glVertex3d(vleft, 0, 0);
            glTexCoord2d(1, 1);
            glVertex3d(vleft + vwidth, 0, 0);
            glTexCoord2d(1, 0);
            glVertex3d(vleft + vwidth, 100, 0);

            glEnd();
            glDisable(GL_TEXTURE_2D);
            glPopMatrix();
        }

    double vleft = (100 - 100 * (640.0/480.0)) / 2;

    if (showHelp)
    {
        double y = surface->h - 30;

        string msg[] = {"---------- Editor help -----------",
                        "d     - delete current path",
                        "a     - add a path",
                        "n     - default paths",
                        "tab   - switch path",
                        "l     - loop (for player)",
                        "i     - invert player",
                        "o     - show overlay",
                        "h     - hide/unhide node",
                        "u     - underlay/un-underlay node",
                        "ctrl  - snap",
                        "space - run test",
                        "enter - run game",
                        "s     - save",
                        "f1    - help",
                        ""
                       };

        glLoadIdentity( );
        glTranslated(vleft, 98, 50);
        glScaled(0.1, 0.1, 0.1);
        glColor3f(1.0f, 1.0f, 1.0f);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        for (int imsg = 0; msg[imsg].length() != 0; ++imsg)
        {
            font4->Render(msg[imsg].c_str());
            glTranslated(0, -25, 0);
        }
    }

    glLoadIdentity( );
    glTranslated(vleft, 1, 50);
    glScaled(0.1, 0.1, 0.1);
    glColor3f(1.0f, 1.0f, 1.0f);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    font4->Render(StatusLine());

    if (msgTime)
    {
        glLoadIdentity( );
        FTBBox b = font->BBox(msg.c_str());
        double tw = b.Upper().X() / 7;

        glTranslated((100 - tw) / 2, 50, 50);
        glScaled(1.0/7.0, 1.0/7.0, 1.0/7.0);

        glColor3f(1.0f, 1.0f, 1.0f);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        font->Render(msg.c_str());
    }
}

const char *Editor::StatusLine()
{
    static char sline[256];

    string modedesc;

    string slp;

    if (level.loop)
        slp = "YES";

    if (!level.loop)
        slp = "NO";

    stringstream pathdesc;

    uint p = 0;

    for (uint pp = 0; pp < level.paths.size(); pp++)
        if (currentPath == &level.paths[pp])
            p = pp;

    if (p == 0)
    {
        pathdesc << "Player";
    }
    else
    {
        pathdesc << "Balls" << p;
    }

    modedesc = "Path:" + pathdesc.str();
    if (currentPath == &level.paths[0])
        modedesc += " Loop:" + slp;

    string sinvert;
    if (level.invert)
    {
        sinvert = "YES";
    }
    else sinvert = "NO";

    if (currentPath == &level.paths[0])
        modedesc += " Invert:" + sinvert;


    stringstream coords;
    coords << mx << ":" << my;

    modedesc += " " + coords.str();

    strcpy(sline, modedesc.c_str());
    return sline;
}

void Editor::Message(string m)
{
    msg = m;
    msgTime = msgTimeout;
}

void Editor::Logic(ulong frame)
{
    if (msgTime > 0)
        msgTime--;

    if (grab)
        grabl++;

    hoverPoint = -1;
    vector<BezierPoint>::iterator iter;
    int f;
    for (iter = currentPath->points.begin(), f = 0; iter != currentPath->points.end(); ++iter, ++f)
    {
        if (abs(mx - iter->x) < 1.0)
        {
            if (abs(my - iter->y) < 1.0)
            {
                hoverPoint = f;
                hoverPointIsControl = false;
            }
        }

        if (abs(mx - iter->cx) < 1.0)
        {
            if (abs(my - iter->cy) < 1.0)
            {
                hoverPoint = f;
                hoverPointIsControl = true;
                hoverPointIsInvertedControl = false;
            }
        }

        if (abs(mx - (iter->x - (iter->cx - iter->x))) < 1.0)
        {
            if (abs(my - (iter->y - (iter->cy - iter->y))) < 1.0)
            {
                hoverPoint = f;
                hoverPointIsControl = true;
                hoverPointIsInvertedControl = true;
            }
        }
    }

    if ((grab == true) && (selectedPoint != -1) && (grabl > 20))
    {
        if (!selPointIsControl)
        {
            double cx = currentPath->points[selectedPoint].cx - currentPath->points[selectedPoint].x;
            double cy = currentPath->points[selectedPoint].cy - currentPath->points[selectedPoint].y;

            currentPath->points[selectedPoint].x = mx;
            currentPath->points[selectedPoint].y = my;
            currentPath->points[selectedPoint].cx = cx + mx;
            currentPath->points[selectedPoint].cy = cy + my;
        }
        else
        {
            if (!selPointIsInvertedControl)
            {
                currentPath->points[selectedPoint].cx = mx;
                currentPath->points[selectedPoint].cy = my;
            }
            else
            {
                double cx = mx - currentPath->points[selectedPoint].x;
                double cy = my - currentPath->points[selectedPoint].y;

                currentPath->points[selectedPoint].cx = currentPath->points[selectedPoint].x - cx;
                currentPath->points[selectedPoint].cy = currentPath->points[selectedPoint].y - cy;
            }

        }
    }

    if (!events.empty)
    {
        mx = (vwidth * events.mouseX) + vleft;
        my = vheight - (vheight * events.mouseY);

        if (events.buttDown[0])
        {
            if ((grab == false) && (hoverPoint != -1))
            {
                selectedPoint = hoverPoint;
                selPointIsControl = hoverPointIsControl;
                selPointIsInvertedControl = hoverPointIsInvertedControl;
                grab = true;
                grabl = 0;
            }
        }

        if (events.buttUp[0])
            grab = false;

        if (events.buttUp[1])
        {
            double cx = mx;
            double cy = my + 10;

            if (selectedPoint != -1)
            {
                cx = (currentPath->points[selectedPoint].cx - currentPath->points[selectedPoint].x) + mx;
                cy = (currentPath->points[selectedPoint].cy - currentPath->points[selectedPoint].y) + my;
            }

            currentPath->points.push_back(BezierPoint(mx, my, cx, cy));
        }

        if (events.keyUp.size() > 0)
            for (vector<SDLKey>::iterator i = events.keyUp.begin(); i != events.keyUp.end(); ++i)
            {
                if ((*i == SDLK_LCTRL) || (*i == SDLK_RCTRL))
                    snap = false;
            }

        if (events.keyDown.size() > 0)
            for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
            {
                if (*i == SDLK_ESCAPE)
                {
                    quit = true;
                }

                if (*i == SDLK_SPACE)
                {
                    level.FixData();
                    Game game(surface, level, 0, 0, 0, 0, true);
                    game.Run();
                    GLSetup();
                    SDL_ShowCursor(SDL_ENABLE);
                    SDL_WM_GrabInput(SDL_GRAB_OFF);
                    return;
                }

                if ((*i == SDLK_RETURN) || (*i == SDLK_KP_ENTER))
                {
                    level.FixData();
                    Game game(surface, level, 0, (uint)time(0), 5, 0, false, true);
                    game.Run();
                    GLSetup();
                    SDL_ShowCursor(SDL_ENABLE);
                    SDL_WM_GrabInput(SDL_GRAB_OFF);
                    return;
                }

                if (*i == SDLK_TAB)
                {
                    uint p = 0;
                    for (uint pp = 0; pp < level.paths.size(); pp++)
                        if (currentPath == &level.paths[pp])
                            p = pp;

                    p++;
                    if (p >= level.paths.size())
                        p = 0;

                    currentPath = &level.paths[p];
                }

                if ((*i == SDLK_LCTRL) || (*i == SDLK_RCTRL))
                    snap = true;

                if (*i == SDLK_l)
                {
                    level.loop = !level.loop;
                }

                if (*i == SDLK_i)
                {
                    level.invert = !level.invert;
                }

                if (*i == SDLK_n)
                    New();

                if (*i == SDLK_s)
                {
                    level.Save();
                    Message(level.savePhilename + " saved");
                }

                if (*i == SDLK_h)
                {
                    if (currentPath != &level.paths[0])
                    {
                        if (selectedPoint != -1)
                        {
                            currentPath->points[selectedPoint].hidden = !currentPath->points[selectedPoint].hidden;
                        }
                    }
                }

                if (*i == SDLK_u)
                {
                    if (currentPath != &level.paths[0])
                    {
                        if (selectedPoint != -1)
                        {
                            currentPath->points[selectedPoint].under = !currentPath->points[selectedPoint].under;
                            if (!currentPath->points[selectedPoint].under)
                                currentPath->points[selectedPoint].hidden = false;
                        }
                    }
                }


                if (*i == SDLK_d)
                {
                    if (currentPath != &level.paths[0])
                    {
                        uint p = 0;
                        for (uint pp = 0; pp < level.paths.size(); pp++)
                            if (currentPath == &level.paths[pp])
                                p = pp;

                        level.paths.erase(level.paths.begin()+p);

                        currentPath = &level.paths[0];
                    }
                }

                if (*i == SDLK_a)
                {
                    level.paths.push_back(Bezier());
                    int l = level.paths.size() - 1;

                    level.paths[l].points.push_back(BezierPoint(45, 50, 45, 56));
                    level.paths[l].points.push_back(BezierPoint(55, 50, 55, 56));

                    currentPath = &level.paths[l];
                }

                if (*i == SDLK_F1)
                {
                    showHelp = !showHelp;
                }

                if (*i == SDLK_o)
                {
                    showOverlay = !showOverlay;
                }

            }

        if (snap && grab)
        {
            mx = (int)mx;
            my = (int)my;
        }
    }

}

void Editor::GLSetup()
{
    width = surface->w;
    height = surface->h;

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

    /* Set the clear color. */
    glClearColor( 1.0, 1.0, 1.0, 1.0 );

    /* Setup our viewport. */
    glViewport( 0, 0, width, height);

    /*
     * Change to the proje tion matrix and set
     * our viewing volume.
     */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    vwidth = 100 * (640.0/480.0);
    vleft = (100 - vwidth) / 2;
    vheight = 100.0;


    glOrtho(vleft, vwidth + vleft, 0, vheight, -100, 100);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
