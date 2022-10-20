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

#ifndef __AUDIOBUFFER_H__
#define __AUDIOBUFFER_H__

#include "sample.h"

namespace Scenes
{
class AudioBuffer
{
    friend class Mixer;
private:
    Sample *sample;
    uint offset;
    uint length;
    bool playing;
    bool loop;
    int pan, vol;
    bool streaming;
    uint bufflen;
    Sint16 *buff;
    uint PanAndMix(uint req_length = 0);

public:
    Sample *getSample()
    {
        return sample;
    };
    Sint16 *getMix(uint requested_length, uint &returned_length);
    void Play(Sample *smpl, int vol, int pan, bool loop);
    void Stop();

    bool operator==(const Sample *s)
    {
        if (sample == s)
            return true;

        return false;
    };

    //void Reset(); // useful if we don't want that sample requeued (because it's deleted)

    AudioBuffer();
    ~AudioBuffer();
};
}

#endif // __AUDIOBUFFER_H__
