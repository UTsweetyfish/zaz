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

#include "audiobuffer.h"

namespace Scenes
{
AudioBuffer::AudioBuffer()
{
    bufflen = 0;
    buff = 0;
    playing = false;
    sample = 0;
}


void AudioBuffer::Play(Sample *smpl, int vol, int pan, bool loop)
{
    if (vol > 100)
        vol = 100;

    if (vol < 0)
        vol = 0;

    if (pan > 100)
        pan = 100;

    if (pan < -100)
        pan = -100;

    sample = smpl;
    this->vol = vol;
    this->pan = pan;
    this->loop = loop;
    streaming = sample->getStreaming();

    if (streaming)
        sample->Restart();

    // if not streaming sample - preload & premix the whole thing
    if (!streaming)
    {
        offset = 0;
        length = sample->getLength();

        if (sample->getNumChannels() == 1)
            length *= 2;

        if (buff)
            free(buff);

        buff = (Sint16*)malloc(length * 4);
        bufflen = length;

        PanAndMix();
    }

    playing = true;
};

uint AudioBuffer::PanAndMix(uint req_lenth)
{
    uint rl;
    Sint16 *tb = sample->getSampleData(req_lenth, rl);

    double v1 = (double)vol / 100;
    double v2 = (double)vol / 100;

    if (pan != 0)
    {
        if (pan > 0)
            v1 = ((double)(vol - pan)) / 100;

        if (v1 < 0)
            v1 = 0;

        if (pan < 0)
            v2 = ((double)(vol + pan)) / 100;

        if (v2 < 0)
            v2 = 0;

    }

    bool mixasis = false;

    if ((vol == 100) && (pan == 0))
        mixasis = true;

    Sint16 *lc = tb;
    Sint16 *rc = tb + 1;
    uint inc = 2;

    if (sample->getNumChannels() == 1)
    {
        rc = lc;
        inc = 1;
        rl*=2;
    }

    for (uint f = 0; f < rl; f+=2)
    {
        if (!mixasis)
        {
            buff[f] = (Sint16)(((double)*lc) * v1);
            buff[f + 1] = (Sint16)(((double)*rc) * v2);
        }
        else
        {
            buff[f] = *lc;
            buff[f + 1] = *rc;
        }

        lc += inc;
        rc += inc;
    }

    return rl;
}

AudioBuffer::~AudioBuffer()
{
    if (buff)
        free(buff);
}

Sint16 *AudioBuffer::getMix(uint requested_length, uint &returned_length)
{
    if (!streaming)
    {
        uint oldoffs = offset;

        if ((offset + requested_length) < length)
        {
            returned_length = requested_length;
        }
        else
        {
            returned_length = length - offset;
            if (loop)
            {
                offset = 0;
            }
            else
            {
                playing = false;
            }

        }

        offset += returned_length;
        return buff + oldoffs;
    }

    // streaming
    if (requested_length != bufflen)
    {
        if (buff)
            free(buff);

        buff = (Sint16*)malloc(requested_length * 4);
        bufflen = requested_length;
    }

    returned_length = PanAndMix(requested_length);
    if (returned_length < requested_length)
    {
        if (loop)
        {
            sample->Restart();
        }
        else
        {
            playing = false;
        }
    }

    return buff;
}

void AudioBuffer::Stop()
{
    playing = false;
}

}
