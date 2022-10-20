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

#include "frame_events.h"

namespace Scenes
{
FrameEvents::FrameEvents(vector<SDLKey>kup, vector<SDLKey>kdn, double mx, double my, int relmx, int relmy, bool bup[], bool bdn[])
{
    unicode.clear();
    keyUp = kup;
    keyDown = kdn;
    mouseX = mx;
    mouseY = my;
    relmouseX = relmx;
    relmouseY = relmy;

    for (int i = 0; i < 3; i++)
    {
        buttUp[i] = bup[i];
        buttDown[i] = bdn[i];
    }

    empty = false;
}

FrameEvents::FrameEvents()
{
    empty = true;
    for (int i = 0; i < 3; i++)
    {
        buttUp[i] = buttDown[i] = false;
    }
}

}
