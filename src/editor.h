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

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "common.h"
#include "scene.h"
#include "bezier.h"
#include "level.h"
#include "game.h"

class Editor : public Scenes::Scene
{
    Level &level;

    // graphics
    double mx, my;
    double vwidth, vheight, vleft, vbottom;
    int width, height;
    static const int msgTimeout = 200;
    int msgTime;
    string msg;

    // edited stuff
    Bezier *currentPath;

    int selectedPoint;
    int hoverPoint;
    bool selPointIsControl;
    bool selPointIsInvertedControl;

    bool hoverPointIsControl;
    bool hoverPointIsInvertedControl;

    bool grab;
    bool snap;
    bool showHelp;
    bool showOverlay;
    int grabl;

    void Message(string m);
    void New();
    const char *StatusLine();
    GLuint backgroundTex;
    GLuint overlayTex;

public:
    Editor(SDL_Surface *surf, Level &level, uint fps = 100);
    ~Editor();
    void Render(ulong frame);
    void Logic(ulong frame);
    void GLSetup();
};


#endif //#__EDITOR_H__
