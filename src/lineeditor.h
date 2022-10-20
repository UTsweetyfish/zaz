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

#ifndef __LINEEDITOR_H__
#define __LINEEDITOR_H__

#include "scene.h"

#define BLINK_RATE 50

namespace Scenes
{
struct LineEditor
{
    std::string txt;
    uint pos;
    double width;
    double y;
    bool blinkOn;
    int blinkCountdown;
    bool shift;
    uint maxLength;

    uint utf8_strlen(const char *s);
    LineEditor(std::string txt, double width, double y, uint maxLength);
    ~LineEditor();
    void Render();
    void Logic(FrameEvents events);
};
}

#endif //__LINEEDITOR_H__
