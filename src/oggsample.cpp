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
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

namespace Scenes
{
OggSample::OggSample(const string filename)
{
    char pcmout[4096];

    FILE *inphile;
    OggVorbis_File vf;
    int eof=0;
    int current_section;

    inphile = fopen(settings.getCFilename(filename), "rb");
    if (inphile == NULL)
    {
        ERR("Could not open " + filename);
    }

    if (ov_open_callbacks(inphile, &vf, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0)
    {
        fclose(inphile);
        ERR("Not an ogg file: " + filename);
    }

    vorbis_info *vi=ov_info(&vf,-1);

    channels = vi->channels;
    length = (Uint32)ov_pcm_total(&vf, -1) * channels;
    offs = 0;

    buff = (Sint16*)malloc((size_t)length * 2);

    while (!eof)
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        long ret=ov_read(&vf,pcmout,sizeof(pcmout),1,2,1,&current_section);
#else
        long ret=ov_read(&vf,pcmout,sizeof(pcmout),0,2,1,&current_section);
#endif
        if (ret == 0)
        {
            /* EOF */
            eof=1;
        }
        else if (ret < 0)
        {
            // some error happened .... who cares :)
        }
        else
        {
            /* we don't bother dealing with sample rate changes, etc, but
            you'll have to */

            memcpy(((char *)buff) + offs, pcmout, ret);
            offs+=ret;
        }
    }

    ov_clear(&vf);

    offs = 0;
    loaded = true;

    fclose(inphile);
}

OggSample::~OggSample()
{
    if (loaded)
    {
        free(buff);
    }
}

uint OggSample::getLength()
{
    return length;
}

Sint16 * OggSample::getSampleData(uint requested_length, uint &returned_length)
{
    returned_length =  length;
    return buff;
}

bool OggSample::getStreaming()
{
    return false;
};

uint OggSample::getNumChannels()
{
    return channels;
};

void OggSample::Restart()
{
    offs = 0;
};
}
