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

#ifndef FRAME_EVENTS_H_INCLUDED
#define FRAME_EVENTS_H_INCLUDED

#include "common.h"
#include <vector>

namespace Scenes
{

class FrameEvents
{
public:
    FrameEvents(vector<SDLKey>kup, vector<SDLKey>kdn, double mx, double my, int relmx, int relmy, bool bup[], bool bdn[]);
    FrameEvents();

    vector<Uint16>unicode;
    vector<SDLKey>keyUp;
    vector<SDLKey>keyDown;
    double mouseX, mouseY;
    int relmouseX, relmouseY;
    bool buttUp[3];
    bool buttDown[3];
    bool empty;
};

}
#endif // FRAME_EVENTS_H_INCLUDED
