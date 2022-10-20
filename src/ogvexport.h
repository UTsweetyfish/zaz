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

 This file is based (as in cut and paste) on png2theora.c example
 distributed with OggTheora
 */

#ifndef __OGVEXPORT_H__
#define __OGVEXPORT_H__

#include <cstdlib>
#include <iostream>
#include <ogg/ogg.h>
#include <theora/theoraenc.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>

using namespace std;

namespace Scenes
{
class OgvExport
{
    uint fps;
    uint width;
    uint height;

    FILE *ogg_fp;
    ogg_stream_state ogg_os_t;
    ogg_stream_state ogg_os_v;

    th_enc_ctx *theora_td;
    th_info theora_ti;
    vorbis_info      vi;
    vorbis_comment   vc;
    vorbis_block     vb;
    vorbis_dsp_state vd;

    uint audiosamples;
    unsigned char *yuvbuffer;
    bool firstFrame;

    unsigned char *rgb2yuvtable;

    void OggClose();
    void OggOpen(const string pathname);
    void Rgb2Yuv(unsigned char* rgb);
    void CreateRgb2YuvTable();
    void OggWriteVideoFrame(int last);
    void OggWriteAudioFrame(char *buff, int last = 0);

    unsigned char clampd(double d)
    {
        if (d < 0)  return 0;
        if (d > 255)  return (unsigned char)255;
        return (unsigned char)d;
    }

public:
    OgvExport(string philename, int width, int height, int fps, int quality = 63, int videorate = 0, int audiorate = 192000);
    void FeedFrame(void *gfx, void *snd, bool last = false);
    ~OgvExport();
};
}

#endif //__OGVEXPORT_H__
