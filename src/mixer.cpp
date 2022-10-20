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

#include <iostream>
#include <cstdlib>
#include <climits>
#include "mixer.h"

namespace Scenes
{
Mixer::Mixer(MixerMode mmode = Realtime)
        : mode(mmode), isPlaying(false)
{
    mut_disposeList = SDL_CreateMutex();
    mixBufferSize = 0;
    mixBuffer = 0;

    audioHWInitialized = true;

    if (mode == Realtime)
    {
        SDL_AudioSpec desiredSpec;

        desiredSpec.freq = 44100;
        desiredSpec.channels = 2;
        desiredSpec.format = AUDIO_S16SYS;
        desiredSpec.samples = 1024;
        desiredSpec.callback = MixerCallBack;
        desiredSpec.userdata = (void *)this;

        if ( SDL_OpenAudio(&desiredSpec, &audioSpec) < 0 )
        {
            audioHWInitialized = false;
        }

        return;
    }
}

Mixer::~Mixer()
{
    if (mode == Realtime)
    {
        if (audioHWInitialized)
        {
            SDL_PauseAudio(1);
            SDL_CloseAudio();
        }
    }

    if (mixBufferSize)
        free(mixBuffer);

    disposeSamples();

    if (mut_disposeList != NULL)
        SDL_DestroyMutex(mut_disposeList);
}

void Mixer::Fill(Uint8 *stream, int len)
{
    uint lsamples = len / 2;

    Sint16 *buff = (Sint16 *)stream;

    uint mbs = len * sizeof(Sint32);
    if (mixBufferSize != mbs)
    {
        if (mixBufferSize)
            free(mixBuffer);

        mixBufferSize = mbs;
        mixBuffer = (Sint32 *)malloc(mbs);
    }

    memset(mixBuffer, 0, mbs);

    for (int b = 0; b < MAXMIXERBUFFERS; b++)
    {
        if (audioBuffers[b].playing)
        {
            uint l2;

            Sint16 *b2 = audioBuffers[b].getMix(lsamples, l2);

            for (uint f = 0; f < l2; f++)
                mixBuffer[f] += b2[f];

        }
    }

    // find compression ratio
    Sint32 maxval = 0;
    for (uint f = 0; f < lsamples; f++)
    {
        Sint32 m = abs(mixBuffer[f]);
        if (m > maxval)
            maxval = m;
    }

    double gain = (double)SHRT_MAX / (double)maxval;
    if (gain > 1.0)
        gain = 1.0;

    // copy back
    for (uint f = 0; f < lsamples; f++)
    {
        buff[f] = (Sint16)((double)mixBuffer[f] * gain);
    }

    disposeSamples();
}

void Mixer::disposeSamples()
{
    // dispose of unneeded samples
    SDL_mutexP(mut_disposeList);
    vector<Sample *>::iterator i;

    for (i = disposeList.begin(); i != disposeList.end(); ++i)
    {
        for (int b = 0; b < MAXMIXERBUFFERS; ++b)
        {
            if (audioBuffers[b].getSample() == *i)
            {
                audioBuffers[b].Stop();
            }
        }

        delete *i;
    }

    disposeList.clear();

    SDL_mutexV(mut_disposeList);
}

void MixerCallBack(void *userdata, Uint8 *stream, int len)
{
    Mixer *mx = reinterpret_cast<Mixer *>(userdata);
    mx->Fill(stream, len);
}

void Mixer::Play()
{
    isPlaying = true;

    if (mode == Realtime)
    {
        SDL_PauseAudio(0);
    }
}

void Mixer::Pause()
{
    isPlaying = false;

    if (mode == Realtime)
    {
        SDL_PauseAudio(1);
    }
}

AudioBuffer *Mixer::EnqueueSample(Sample *smp, int vol, int pan, bool loop)
{
    AudioBuffer *retval = 0;

    // if sample is streaming and playing, restart an already plaing buffer
    if (smp->getStreaming())
        for (int f = 0; (f < MAXMIXERBUFFERS) && (!retval); f++)
        {
            if (audioBuffers[f] == smp)
            {
                if (!audioBuffers[f].playing)
                    audioBuffers[f].Play(smp, vol, pan, loop);

                retval = &audioBuffers[f];
            }
        }

    for (int f = 0; (f < MAXMIXERBUFFERS) && (!retval); f++)
    {
        if (!audioBuffers[f].playing)
        {
            audioBuffers[f].Play(smp, vol, pan, loop);
            retval = &audioBuffers[f];
        }
    }

    return retval;
}

void Mixer::DisposeSample(Sample *smp)
{
    SDL_mutexP(mut_disposeList);

    bool found = false;

    vector<Sample *>::iterator i;

    for (i = disposeList.begin(); i != disposeList.end(); ++i)
    {
        if (*i == smp)
            found = true;
    }

    if (!found)
        disposeList.push_back(smp);

    SDL_mutexV(mut_disposeList);
}
}
