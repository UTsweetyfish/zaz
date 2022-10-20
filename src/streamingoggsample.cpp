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
StreamingOggSample::StreamingOggSample(const string filename)
{
    bufflen = 0;
    buff = 0;

    inphile = fopen(settings.getCFilename (filename), "rb");
    if (inphile == NULL)
    {
        ERR("Could not open " + filename);
    }

    if (ov_open_callbacks(inphile, &vf, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0)
    {
        fclose(inphile);
        ERR("Not an ogg file: " + filename);
    }

    vi=ov_info(&vf,-1);

    channels = vi->channels;
    length = ov_pcm_total(&vf, -1) * channels;

    loaded = true;
}

StreamingOggSample::~StreamingOggSample()
{
    if (loaded)
    {
        ov_clear(&vf);

        fclose(inphile);

        free(buff);
    }
}

uint StreamingOggSample::getLength()
{
    return 0;
}

Sint16 * StreamingOggSample::getSampleData(uint requested_length, uint &returned_length)
{
    if (bufflen != requested_length)
    {
        if (buff)
            free(buff);

        buff = (Sint16*)malloc(requested_length * 2);
        bufflen = requested_length;
    }

    int eof=0;
    int current_section;

    uint offset = 0;
    uint rem = requested_length * 2;

    while (!eof && rem)
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        long ret=ov_read(&vf,((char *)buff) + offset,rem,1,2,1,&current_section);
#else
        long ret=ov_read(&vf,((char *)buff) + offset,rem,0,2,1,&current_section);
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
            rem-=ret;
            offset+=ret;
        }
    }

    returned_length = requested_length - (rem / 2);
    return buff;
}

bool StreamingOggSample::getStreaming()
{
    return true;
};

uint StreamingOggSample::getNumChannels()
{
    return channels;
};

void StreamingOggSample::Restart()
{
    ov_raw_seek(&vf,0);
};
}
