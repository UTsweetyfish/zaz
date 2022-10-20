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

#include "common.h"
#include "ogvexport.h"
#include "error.h"

namespace Scenes
{
OgvExport::OgvExport(string philename, int width, int height, int fps, int quality, int videorate, int audiorate)
        : fps(fps), width(width), height(height), firstFrame(true)
{
    //cout << "Exporting to " << philename << " (" << width << "*" << height << ") @ " << fps << "fps" << endl ;

    audiosamples = 44100 / fps;


    th_info_init(&theora_ti);

    theora_ti.pic_width = width;
    theora_ti.pic_height = height;
    theora_ti.frame_width = ((width + 15) >>4)<<4;
    theora_ti.frame_height = ((height + 15)>>4)<<4;
    theora_ti.pic_x = 0;
    theora_ti.pic_y = 0;
    theora_ti.fps_numerator = fps;
    theora_ti.fps_denominator = 1;
    theora_ti.aspect_numerator = 1;
    theora_ti.aspect_denominator = 1;
    theora_ti.colorspace = TH_CS_ITU_REC_470BG;
    theora_ti.pixel_fmt = TH_PF_420;
    theora_ti.target_bitrate = videorate;
    theora_ti.quality = quality;
    theora_ti.keyframe_granule_shift=6;

    /*        theora_ti.dropframes_p = 0;
            theora_ti.quick_p = 1;
            theora_ti.keyframe_auto_p = 1;
            theora_ti.keyframe_frequency = 64;
            theora_ti.keyframe_frequency_force = 64;
            theora_ti.keyframe_data_target_bitrate = (ogg_uint32_t)(videorate * 1.5);
            theora_ti.keyframe_mindistance = 8;
            theora_ti.noise_sensitivity = 1;
    */
    vorbis_info_init(&vi);
    if (vorbis_encode_init(&vi, 2, 44100, audiorate, audiorate, audiorate))
        ERR("Could not initialize vorbis encoder");

    OggOpen(philename);
    yuvbuffer = (unsigned char *)malloc(width * height * 3);
    rgb2yuvtable = (unsigned char *)malloc(256*256*256*3);
    if (rgb2yuvtable)
        CreateRgb2YuvTable();
}

OgvExport::~OgvExport()
{
    OggWriteVideoFrame(1);

    // add a mute audiopacket with e_o_s
    char *snd = (char *)malloc(audiosamples * 2);
    memset(snd, 0, audiosamples * 2);
    OggWriteAudioFrame(snd, 1);
    free(snd);

    OggClose();
    free(yuvbuffer);
    if (rgb2yuvtable)
        free(rgb2yuvtable);
    th_info_clear(&theora_ti);
}

void OgvExport::FeedFrame(void *gfx, void *snd, bool last)
{
    if (!firstFrame)
    {
        OggWriteVideoFrame(0);
    }

    firstFrame = false;
    Rgb2Yuv((unsigned char *)gfx);
    OggWriteAudioFrame((char *)snd, last);
}

void OgvExport::CreateRgb2YuvTable()
{
    for (unsigned int rr = 0; rr < 256; rr++)
        for (unsigned int gg = 0; gg < 256; gg++)
            for (unsigned int bb = 0; bb < 256; bb++)
            {
                unsigned char *addr = rgb2yuvtable + ((rr << 16) + (gg << 8) + bb) * 3;

                double r = rr;
                double g = gg;
                double b = bb;

                addr[0] = clampd( 0.299 * r
                                  + 0.587 * g
                                  + 0.114 * b);

                addr[1] = clampd((0.436 * 255
                                  - 0.14713 * r
                                  - 0.28886 * g
                                  + 0.436 * b) / 0.872);
                addr[2] = clampd((0.615 * 255
                                  + 0.615 * r
                                  - 0.51499 * g
                                  - 0.10001 * b) / 1.230);

            }
}

void OgvExport::Rgb2Yuv(unsigned char *rgb)
{
    uint x;
    uint y;

    if (rgb2yuvtable)
    {
        for (y = 0; y < height; y++)
            for (x = 0; x < width; x++)
            {
                unsigned char *p = rgb + (((height - 1 - y) * width * 3) + (x * 3));
                unsigned char *yp = yuvbuffer + (3 * (x + width * y) + 0);
                unsigned char *addr = rgb2yuvtable + ((((unsigned long)p[0]) << 16) + (((unsigned long)p[1]) << 8) + p[2]) *3;
                yp[0] = addr[0];
                yp[1] = addr[1];
                yp[2] = addr[2];
            }
    }
    else
    {
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                double r;
                double g;
                double b;

                r = rgb[((height - 1 - y) * width * 3) + (x * 3) + 0];
                g = rgb[((height - 1 - y) * width * 3) + (x * 3) + 1];
                b = rgb[((height - 1 - y) * width * 3) + (x * 3) + 2];

                yuvbuffer[3 * (x + width * y) + 0] = clampd(
                                                         0.299 * r
                                                         + 0.587 * g
                                                         + 0.114 * b);
                yuvbuffer[3 * (x + width * y) + 1] = clampd((0.436 * 255
                                                     - 0.14713 * r
                                                     - 0.28886 * g
                                                     + 0.436 * b) / 0.872);
                yuvbuffer[3 * (x + width * y) + 2] = clampd((0.615 * 255
                                                     + 0.615 * r
                                                     - 0.51499 * g
                                                     - 0.10001 * b) / 1.230);

            }
        }
    }
}

void OgvExport::OggOpen(const string pathname)
{
    ogg_packet op;
    ogg_page og;
    th_comment tc;

    ogg_fp = fopen(pathname.c_str(), "wb");
    if (!ogg_fp)
        ERR(pathname + ":could not open file");

    if (ogg_stream_init(&ogg_os_t, rand()))
        ERR(pathname + ":could not init stream");

    if ((theora_td = th_encode_alloc(&theora_ti)) == NULL)
        ERR(pathname + ":could not init theora encoder");


    th_comment_init(&tc);
    if (th_encode_flushheader(theora_td,&tc,&op)<=0)
    {
        ERR(pathname + ":Internal Theora library error.");
    }

    ogg_stream_packetin(&ogg_os_t,&op);

    if (ogg_stream_pageout(&ogg_os_t,&og)!=1)
    {
        ERR(pathname + ":Internal Ogg library error.");
    }

    fwrite(og.header,1,og.header_len,ogg_fp);
    fwrite(og.body,1,og.body_len,ogg_fp);

    /* create the remaining theora headers */
    int ret;
    for (;;)
    {
        ret=th_encode_flushheader(theora_td,&tc,&op);
        if (ret<0)
        {
            ERR(pathname + ":Internal Theora library error.");
        }
        else if (!ret)break;
        ogg_stream_packetin(&ogg_os_t,&op);
    }

    // vorbis headers
    if (ogg_stream_init(&ogg_os_v, rand()))
        ERR(pathname + ":could not init vorbis encoder");


    vorbis_comment_init(&vc);
    vorbis_analysis_init(&vd,&vi);
    vorbis_block_init(&vd,&vb);

    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;

    vorbis_analysis_headerout(&vd,&vc,&header,&header_comm,&header_code);

    ogg_stream_packetin(&ogg_os_v,&header);
    if (ogg_stream_pageout(&ogg_os_v,&og)!=1)
        ERR(pathname + "Internal Ogg library error");

    fwrite(og.header,1,og.header_len,ogg_fp);
    fwrite(og.body,1,og.body_len,ogg_fp);

    ogg_stream_packetin(&ogg_os_v,&header_comm);
    ogg_stream_packetin(&ogg_os_v,&header_code);

    /* flush at the end of the headers */
    if (ogg_stream_flush(&ogg_os_t, &og))
    {
        fwrite(og.header, og.header_len, 1, ogg_fp);
        fwrite(og.body, og.body_len, 1, ogg_fp);
    }

    if (ogg_stream_flush(&ogg_os_v, &og))
    {
        fwrite(og.header, og.header_len, 1, ogg_fp);
        fwrite(og.body, og.body_len, 1, ogg_fp);
    }

}

void OgvExport::OggClose(void)
{
    ogg_packet op;
    ogg_page og;

    th_encode_packetout(theora_td, 1, &op);
    while (ogg_stream_pageout(&ogg_os_t, &og))
    {
        fwrite(og.header, og.header_len, 1, ogg_fp);
        fwrite(og.body, og.body_len, 1, ogg_fp);
    }
    if (ogg_stream_flush(&ogg_os_t, &og))
    {
        fwrite(og.header, og.header_len, 1, ogg_fp);
        fwrite(og.body, og.body_len, 1, ogg_fp);
    }

    th_info_clear(&theora_ti);
    th_encode_free(theora_td);

    fflush(ogg_fp);
    fclose(ogg_fp);

    ogg_stream_clear(&ogg_os_t);
    ogg_stream_clear(&ogg_os_v);
}

void OgvExport::OggWriteAudioFrame(char *buff, int last)
{
    ogg_packet op;
    ogg_page og;

    float **vbuff = vorbis_analysis_buffer(&vd, audiosamples);
    /* uninterleave samples */
    int count = 0;
    for (uint i=0; i<audiosamples; i++)
    {
        for (int j=0; j<2; j++)
        {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            vbuff[j][i]= ((buff[count]<<8) | (0xff00&(int)buff[count + 1]))/32768.f;
#else
            vbuff[j][i]= ((buff[count+1]<<8) | (0x00ff&(int)buff[count]))/32768.f;
#endif
            count+=2;
        }
    }

    vorbis_analysis_wrote(&vd,audiosamples);

    while (vorbis_analysis_blockout(&vd,&vb)==1)
    {

        /* analysis, assume we want to use bitrate management */
        vorbis_analysis(&vb,NULL);
        vorbis_bitrate_addblock(&vb);

        /* weld packets into the bitstream */
        while (vorbis_bitrate_flushpacket(&vd,&op))
        {
            ogg_stream_packetin(&ogg_os_v,&op);
        }

        while (ogg_stream_pageout(&ogg_os_v, &og))
        {
            fwrite(og.header, og.header_len, 1, ogg_fp);
            fwrite(og.body, og.body_len, 1, ogg_fp);
        }

    }
}

void OgvExport::OggWriteVideoFrame(int last)
{
    th_ycbcr_buffer yuv_buf;
    ogg_packet op;
    ogg_page og;

    ulong yuv_w;
    ulong yuv_h;

    unsigned char *yuv_y;
    unsigned char *yuv_u;
    unsigned char *yuv_v;

    uint x;
    uint y;

    /* Must hold: yuv_w >= w */
    yuv_w = (width + 15) & ~15;

    /* Must hold: yuv_h >= h */
    yuv_h = (height + 15) & ~15;

    yuv_y = (unsigned char *)malloc(yuv_w * yuv_h);
    yuv_u = (unsigned char *)malloc(yuv_w * yuv_h / 4);
    yuv_v = (unsigned char *)malloc(yuv_w * yuv_h / 4);


    yuv_buf[0].width = yuv_w;
    yuv_buf[0].height = yuv_h;
    yuv_buf[0].stride = yuv_w;
    yuv_buf[0].data = yuv_y;

    yuv_buf[1].width = yuv_w >> 1;
    yuv_buf[1].height = yuv_h >> 1;
    yuv_buf[1].stride = yuv_w >> 1;
    yuv_buf[1].data = yuv_u;

    yuv_buf[2].width = yuv_w >> 1;
    yuv_buf[2].height = yuv_h >> 1;
    yuv_buf[2].stride = yuv_w >> 1;
    yuv_buf[2].data = yuv_v;
    /*
    		yuv_buf.y_width = yuv_w;
            yuv_buf.y_height = yuv_h;
            yuv_buf.y_stride = yuv_w;
            yuv_buf.uv_width = yuv_w >> 1;
            yuv_buf.uv_height = yuv_h >> 1;
            yuv_buf.uv_stride = yuv_w >> 1;
            yuv_buf.y = yuv_y;
            yuv_buf.u = yuv_u;
            yuv_buf.v = yuv_v;
    */
    for (y = 0; y < yuv_h; y++)
    {
        for (x = 0; x < yuv_w; x++)
        {
            yuv_y[x + y * yuv_w] = 0;
        }
    }

    for (y = 0; y < yuv_h; y += 2)
    {
        for (x = 0; x < yuv_w; x += 2)
        {
            yuv_u[(x >> 1) + (y >> 1) * (yuv_w >> 1)] = 0;
            yuv_v[(x >> 1) + (y >> 1) * (yuv_w >> 1)] = 0;
        }
    }

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            yuv_y[x + y * yuv_w] = yuvbuffer[3 * (x + y * width) + 0];
        }
    }

    for (y = 0; y < height; y += 2)
    {
        for (x = 0; x < width; x += 2)
        {
            yuv_u[(x >> 1) + (y >> 1) * (yuv_w >> 1)] =
                yuvbuffer[3 * (x + y * width) + 1];
            yuv_v[(x >> 1) + (y >> 1) * (yuv_w >> 1)] =
                yuvbuffer[3 * (x + y * width) + 2];
        }
    }

    if (th_encode_ycbcr_in(theora_td, yuv_buf))
        ERR("Could not encode frame");

    if (!th_encode_packetout(theora_td, last, &op))
        ERR("could not read packets");

    if (last)
        op.e_o_s = 1;

    ogg_stream_packetin(&ogg_os_t, &op);
    while (ogg_stream_pageout(&ogg_os_t, &og))
    {
        fwrite(og.header, og.header_len, 1, ogg_fp);
        fwrite(og.body, og.body_len, 1, ogg_fp);
    }

    free(yuv_y);
    free(yuv_u);
    free(yuv_v);
}
}
