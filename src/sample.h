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

#ifndef SAMPLE_H_INCLUDED
#define SAMPLE_H_INCLUDED

#include "common.h"
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

namespace Scenes
{
class Sample
{
    // the sample data is always 16bit signed interlaced left / right channel
public:
    virtual Sint16 * getSampleData(uint requested_length, uint &returned_length) = 0; // if returned_length < requested length - end of sample
    virtual bool getStreaming() = 0; // if not seekable, the sample will be used only once
    virtual uint getNumChannels() = 0; // 1 or 2

    virtual uint getLength() = 0; // * length in samples
    virtual void Restart() = 0; // may be called for streaming sample
    virtual ~Sample() {};
    // * - will not be called if getSeekable() == false
};


class WaveSample : Sample
{
private:
    SDL_AudioSpec spec;
    Uint32 length;
    Uint32 offs;
    Sint16 *buff;
    bool loaded;

public:
    WaveSample(const string filename);
    ~WaveSample();

    uint getLength();
    Sint16 * getSampleData(uint requested_length, uint &returned_length); // if returned_length < requested length - end of sample
    bool getStreaming(); // if not seekable, the sample will be used only once
    uint getNumChannels(); // 1 or 2
    void Restart(); // may be called for unseekable sample

};

class OggSample : Sample
{
private:
    Uint32 length;
    Uint32 offs;
    Sint16 *buff;
    int channels;
    bool loaded;

public:
    OggSample(const string filename);
    ~OggSample();

    uint getLength();
    Sint16 * getSampleData(uint requested_length, uint &returned_length); // if returned_length < requested length - end of sample
    bool getStreaming(); // if not seekable, the sample will be used only once
    uint getNumChannels(); // 1 or 2
    void Restart(); // may be called for unseekable sample
};

class StreamingOggSample : Sample
{
private:
    Uint64 length;
    Sint16 *buff;
    uint bufflen;
    int channels;
    bool loaded;
    vorbis_info *vi;
    OggVorbis_File vf;
    FILE *inphile;

public:
    StreamingOggSample(const string filename);
    ~StreamingOggSample();

    uint getLength();
    Sint16 * getSampleData(uint requested_length, uint &returned_length); // if returned_length < requested length - end of sample
    bool getStreaming(); // if not seekable, the sample will be used only once
    uint getNumChannels(); // 1 or 2
    void Restart(); // may be called for unseekable sample*/
};
}


#endif // SAMPLE_H_INCLUDED
