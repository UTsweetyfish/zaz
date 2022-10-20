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

#ifndef __MIXER_H__
#define __MIXER_H__

#include <SDL/SDL.h>
#include <vector>

#include "sample.h"
#include "audiobuffer.h"

namespace Scenes
{
enum MixerMode {Realtime, Nonrealtime};
const int MAXMIXERBUFFERS = 24;

void MixerCallBack(void *userdata, Uint8 *stream, int len);

class Mixer
{
private:
    MixerMode mode;
    SDL_AudioSpec audioSpec;
    Sint32 *mixBuffer;
    uint mixBufferSize;
    AudioBuffer audioBuffers[MAXMIXERBUFFERS];
    vector<Sample *>disposeList;
    SDL_mutex *mut_disposeList;
    void disposeSamples();
    bool audioHWInitialized;

public:
    Mixer(MixerMode mmode);
    ~Mixer();
    void Play();
    void Pause();
    void DisposeSample(Sample *smp);

    bool isPlaying;

    void Fill(Uint8 *stream, int len);
    AudioBuffer *EnqueueSample(Sample *smp, int vol = 100, int pan = 0, bool loop = false);
};
}

#endif // __MIXER_H__
