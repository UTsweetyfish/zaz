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

#include "sample.h"

namespace Scenes
{
WaveSample::WaveSample(const string filename)
{
    if (NULL == SDL_LoadWAV(settings.getCFilename (filename), &spec, (Uint8**)&buff, &length))
    {
        ERR(SDL_GetError());
    }

    offs = 0;
    length = length / 2; // 16bit samples only

    loaded = true;
}

WaveSample::~WaveSample()
{
    if (loaded)
    {
        SDL_FreeWAV((Uint8 *)buff);
    }
}

uint WaveSample::getLength()
{
    return length;
}

Sint16 * WaveSample::getSampleData(uint requested_length, uint &returned_length)
{
    returned_length = length;
    return buff;
}

bool WaveSample::getStreaming()
{
    return false;
};

uint WaveSample::getNumChannels()
{
    return spec.channels;
};

void WaveSample::Restart()
{
    offs = 0;
};
}
